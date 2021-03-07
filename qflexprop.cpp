/*****************************************************************************
 *
 * Qt5 Propeller 2 main window for editor, compiler, terminal
 *
 * Copyright © 2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#include <QFile>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QMessageBox>
#include <QTextStream>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSettings>
#include <QCryptographicHash>
#include <QScrollArea>
#include <QSplitter>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <cinttypes>
#include <fcntl.h>
#include "util.h"
#include "idstrings.h"
#include "propedit.h"
#include "qflexprop.h"
#include "aboutdlg.h"
#include "ui_qflexprop.h"
#include "flexspindlg.h"
#include "serterm.h"
#include "serialportdlg.h"
#include "propload.h"

QFlexProp::QFlexProp(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::QFlexProp)
    , m_dev(nullptr)
    , m_transfer(QMutex::NonRecursive)
    , m_fixedfont()
    , m_leds({id_pwr, id_ri, id_dcd, id_dtr, id_dsr, id_rts, id_cts, id_txd, id_rxd, id_brk, id_fe, id_pe,})
    , m_enabled_leds({
	{id_pwr, true  },
	{id_dtr, true  },
	{id_dsr, true  },
	{id_rts, true  },
	{id_cts, true  },
	{id_rxd, true  },
	{id_txd, true  },
	{id_ri,  false },
	{id_dcd, false },
	{id_brk, false },
	{id_fe,  false },
	{id_pe,  false },
    })
    , m_labels()
    , m_stty_operation()
    , m_port_name()
    , m_baud_rate(Serial_Baud230400)
    , m_data_bits(QSerialPort::Data8)
    , m_parity(QSerialPort::NoParity)
    , m_stop_bits(QSerialPort::OneStop)
    , m_flow_control(QSerialPort::NoFlowControl)
    , m_local_echo(false)
    , m_flexspin_binary()
    , m_flexspin_include_paths()
    , m_flexspin_optimize(1)
    , m_flexspin_listing(false)
    , m_flexspin_warnings(true)
    , m_flexspin_errors(false)
    , m_flexspin_hub_address(0)
    , m_flexspin_skip_coginit(false)
{
    ui->setupUi(this);

    setup_widget();
    setup_statusbar();
    load_settings();
    setup_port();
    setup_signals();
    tab_changed(0);

    QTimer::singleShot(100, this, &QFlexProp::open_port);
}

QFlexProp::~QFlexProp()
{
    save_settings();
    delete ui;
}

/**
 * @brief Return a pointer to the currently active propEdit in the tab widget
 * @return pointer to propEdit or nullptr if none selection
 */
PropEdit* QFlexProp::current_editor() const
{
    const int curtab = ui->tabWidget->currentIndex();
    QWidget *wdg = ui->tabWidget->widget(curtab);
    if (!wdg) {
	qCritical("%s: current tab %d has no widget?", __func__,
	       curtab);
	return nullptr;
    }

    // Check if this tab has a PropEdit
    QString pe_id = QString("pe_%1").arg(curtab);
    PropEdit *pe = wdg->findChild<PropEdit *>(pe_id);
    if (!pe) {
	qDebug("%s: current tab %d has no PropEdit '%s'?", __func__,
	       curtab, qPrintable(pe_id));
	return nullptr;
    }

    // Redundant check if this tab's PropEdit name is ok
    QString pe_name = pe->objectName();
    if (!pe_name.startsWith(QLatin1String("pe_"))) {
	qDebug("%s: current tab %d PropEdit name is wrong '%s'?", __func__,
	       curtab, qPrintable(pe_id));
	return nullptr;
    }
    return pe;
}

QTextBrowser* QFlexProp::current_browser() const
{
    const int curtab = ui->tabWidget->currentIndex();
    QWidget *wdg = ui->tabWidget->widget(curtab);
    if (!wdg) {
	qDebug("%s: current tab %d has no widget?", __func__,
	       curtab);
	return nullptr;
    }

    QString tb_id = QString("tb_%1").arg(curtab);
    QTextBrowser *tb = wdg->findChild<QTextBrowser *>(tb_id);
    if (!tb) {
	qDebug("%s: current tab %d has no text browser '%s'?", __func__,
	       curtab, qPrintable(tb_id));
	return nullptr;
    }

    QString tb_name = tb->objectName();
    if (!tb_name.startsWith(QLatin1String("tb_"))) {
	qDebug("%s: current tab %d text browser name is wrong '%s'?", __func__,
	       curtab, qPrintable(tb_id));
	return nullptr;
    }
    return tb;
}

/**
 * @brief Preset a QFileDialog for loading an existing source file
 * @param title window title
 * @return QString with the full path, or empty if cancelled
 */
QString QFlexProp::load_file(const QString& title)
{
    QFileDialog dlg(this);
    QSettings s;
    QString srcdflt = QString("%1/p2tools").arg(QDir::homePath());
    QString srcdir = s.value(id_sourcedir, srcdflt).toString();
    QString filename = s.value(id_filename).toString();
    QStringList history = s.value(id_history).toStringList();
    QStringList filetypes = {
	{"All files (*.*)"},
	{"Basic (*.bas)"},
	{"C source (*.c)"},
	{"Spin (*.spin)"},
	{"Assembler (*.p2asm)"},
    };

    dlg.setWindowTitle(title);
    dlg.setAcceptMode(QFileDialog::AcceptOpen);
    dlg.setDirectory(srcdir);
    dlg.setFileMode(QFileDialog::ExistingFile);
    dlg.setHistory(history);
    dlg.setNameFilters(filetypes);
    dlg.setOption(QFileDialog::DontUseNativeDialog, true);
    dlg.setViewMode(QFileDialog::Detail);
    if (!filename.isEmpty())
	dlg.selectFile(filename);

    if (QFileDialog::Accepted != dlg.exec())
	return QString();

    QStringList files = dlg.selectedFiles();
    if (files.isEmpty())
	return QString();

    filename = files.first();
    QFileInfo info(filename);
    srcdir = info.dir().absolutePath();
    s.setValue(id_sourcedir, srcdir);
    history.insert(0, filename);
    if (history.size() > 30)
	history.takeLast();
    s.setValue(id_filename, info.fileName());
    s.setValue(id_history, history);

    return filename;
}

/**
 * @brief Preset a QFileDialog for saving a source file
 * @param title window title
 * @return QString with the full path, or empty if cancelled
 */
QString QFlexProp::save_file(const QString& filename, const QString& title)
{
    QFileDialog dlg(this);
    QSettings s;
    QString srcdflt = QString("%1/p2tools").arg(QDir::homePath());
    QString srcdir = s.value(id_sourcedir, srcdflt).toString();
    QStringList history = s.value(id_history).toStringList();
    QStringList filetypes = {
	{"All files (*.*)"},
	{"Basic (*.bas)"},
	{"C source (*.c)"},
	{"Spin (*.spin)"},
	{"Assembler (*.p2asm)"},
    };

    dlg.setWindowTitle(title);
    dlg.setAcceptMode(QFileDialog::AcceptSave);
    dlg.setDirectory(srcdir);
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setNameFilters(filetypes);
    dlg.setOption(QFileDialog::DontUseNativeDialog, true);
    dlg.setViewMode(QFileDialog::Detail);
    dlg.selectFile(filename);

    if (QFileDialog::Accepted != dlg.exec())
	return QString();

    QStringList files = dlg.selectedFiles();
    if (files.isEmpty())
	return QString();

    QString save_filename = files.first();
    QFileInfo info(save_filename);
    srcdir = info.dir().absolutePath();
    s.setValue(id_sourcedir, srcdir);
    history.insert(0, save_filename);
    if (history.size() > 30)
	history.takeLast();
    s.setValue(id_filename, info.fileName());
    s.setValue(id_history, history);

    return save_filename;
}

void QFlexProp::setup_widget()
{
    QString title = QString("%1 %2")
		    .arg(qApp->applicationName())
		    .arg(qApp->applicationVersion());
    if (m_dev) {
	if (m_dev->isOpen()) {
	    title += tr(" (%1)").arg(m_port_name);
	} else {
	    title += tr(" (%1 failed)").arg(m_port_name);
	}
    } else {
	title += tr(" (no port)");
    }
    setWindowTitle(title);
    ui->statusbar->showMessage(title);
}

void QFlexProp::setup_signals()
{
    SerTerm* st = ui->tabWidget->findChild<SerTerm*>(id_terminal);
    Q_ASSERT(st != nullptr);
    connect(st, &SerTerm::term_response,
	    this, &QFlexProp::dev_write_data,
	    Qt::UniqueConnection);
    connect(st, &SerTerm::update_pinout,
	    this, &QFlexProp::update_pinout,
	    Qt::UniqueConnection);
    connect(ui->tabWidget, &QTabWidget::currentChanged,
	    this, &QFlexProp::tab_changed);
}

void QFlexProp::dev_ready_read()
{

    if (m_transfer.tryLock()) {
	qint64 avail = m_dev->bytesAvailable();
	QByteArray data = m_dev->read(avail);
	DBG_DATA("%s: recv %d bytes\n%s", __func__, data.length(),
		 qPrintable(util.dump(__func__, data)));
	ui->terminal->write(data);
	m_transfer.unlock();
    } else {
	DBG_DATA("%s: ignored for transfer", __func__);
    }
    update_pinout(true);
}

void QFlexProp::dev_close()
{
    SerTerm* st = ui->tabWidget->findChild<SerTerm*>(id_terminal);
    Q_ASSERT(st != nullptr);
    if (m_dev) {
	qDebug("%s: deleting m_dev", __func__);
	m_dev->deleteLater();
	m_dev = nullptr;
	st->set_device(m_dev);
    }
}

void QFlexProp::dev_write_data(const QByteArray& data)
{
    Q_ASSERT(m_dev);
    DBG_DATA("%s: xmit %d bytes\n%s", __func__, data.length(), qPrintable(util.dump(__func__, data)));
    m_dev->write(data);
}

QPixmap QFlexProp::led(const QString& type, int state)
{
    // This is how the led_*.png resource images are laid out
    static const QHash<QString,int> leds_xpos = {
	{id_dcd,  0},
	{id_dsr,  1},
	{id_dtr,  2},
	{id_cts,  3},
	{id_rts,  4},
	{id_rxd,  5},
	{id_txd,  6},
	{id_ri,   7},
	{id_brk,  8},
	{id_fe,   9},
	{id_pe,  10},
	{id_pwr, 11},
    };
    QString name = QString("led_%1.png").arg(state);
    QPixmap pix = QPixmap(QString(":/images/%1").arg(name));
    QPixmap led = pix.copy(leds_xpos.value(type) * 64, 0, 64, 64);
    return led.scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void QFlexProp::load_settings()
{
    QSettings s;
    bool ok;

    s.beginGroup(id_grp_application);
    restoreGeometry(s.value(id_window_geometry).toByteArray());
    // TODO: sane defaults?
#if defined(Q_OS_LINUX)
    QString font_default = QLatin1String("Monospace");
#elif defined(Q_OS_WIN)
    QString font_default = QLatin1String("Courier New");
#elif defined(Q_OS_MACOS)
    QString font_default = QLatin1String("Courier");
#endif
    // TODO: add preferences dialog for the font, weight and size
    QString font_family = s.value(id_fixedfont_family, font_default).toString();
    int font_weight = s.value(id_fixedfont_weight, QFont::Normal).toInt();
    int font_size = s.value(id_fixedfont_size, 12).toInt();
    s.endGroup();
    m_fixedfont = QFont(font_family, font_size, font_weight);

    s.beginGroup(id_grp_serialport);
    m_port_name = s.value(id_port_name, QLatin1String("ttyUSB0")).toString();
    s.beginGroup(m_port_name);
    m_baud_rate = static_cast<Serial_BaudRate>(s.value(id_baud_rate, Serial_Baud230400).toInt());
    m_data_bits = static_cast<QSerialPort::DataBits>(s.value(id_data_bits, m_data_bits).toInt());
    m_parity = static_cast<QSerialPort::Parity>(s.value(id_parity, m_parity).toInt());
    m_stop_bits = static_cast<QSerialPort::StopBits>(s.value(id_stop_bits, m_stop_bits).toInt());
    m_flow_control = static_cast<QSerialPort::FlowControl>(s.value(id_flow_control, m_flow_control).toInt());
    m_local_echo = s.value(id_local_echo, false).toBool();
    s.endGroup();
    s.endGroup();

    s.beginGroup(id_grp_flexspin);
    QString binary_dflt = QString("%1/bin/flexspin").arg(p2tools_path);
    m_flexspin_binary = s.value(id_flexspin_binary, binary_dflt).toString();
    QStringList include_paths_default;
    include_paths_default += QString("%1/include").arg(p2tools_path);
    m_flexspin_include_paths = s.value(id_flexspin_include_paths, include_paths_default).toStringList();
    m_flexspin_optimize = s.value(id_flexspin_optimize, 1).toInt(&ok);
    if (!ok) {
	m_flexspin_optimize = 1;
    }
    m_flexspin_listing = s.value(id_flexspin_listing, false).toBool();
    m_flexspin_warnings = s.value(id_flexspin_warnings, true).toBool();
    m_flexspin_errors = s.value(id_flexspin_errors, false).toBool();
    m_flexspin_hub_address = s.value(id_flexspin_hub_address, 0).toUInt(&ok);
    if (!ok) {
	m_flexspin_hub_address = 0;
    }
    m_flexspin_skip_coginit = s.value(id_flexspin_skip_coginit, false).toBool();
    s.endGroup();
}

void QFlexProp::save_settings()
{
    QSettings s;


    s.beginGroup(id_grp_application);
    // Save window geometry
    s.setValue(id_window_geometry, saveGeometry());
    s.endGroup();

    s.beginGroup(id_grp_serialport);
    s.setValue(id_port_name, m_port_name);
    s.beginGroup(m_port_name);
    s.setValue(id_baud_rate, m_baud_rate);
    s.setValue(id_data_bits, m_data_bits);
    s.setValue(id_parity, m_parity);
    s.setValue(id_stop_bits, m_stop_bits);
    s.setValue(id_flow_control, m_flow_control);
    s.setValue(id_local_echo, m_local_echo);
    s.endGroup();
    s.endGroup();

    s.beginGroup(id_grp_flexspin);
    s.setValue(id_flexspin_binary, m_flexspin_binary);
    s.setValue(id_flexspin_include_paths, m_flexspin_include_paths);
    s.setValue(id_flexspin_optimize, m_flexspin_optimize);
    s.setValue(id_flexspin_listing, m_flexspin_listing);
    s.setValue(id_flexspin_warnings, m_flexspin_warnings);
    s.setValue(id_flexspin_errors, m_flexspin_errors);
    s.setValue(id_flexspin_hub_address, m_flexspin_hub_address);
    s.setValue(id_flexspin_skip_coginit, m_flexspin_skip_coginit);
    s.endGroup();
}

void QFlexProp::update_pinout(bool redo)
{
    static const int off = 0;
    static const int red = 1;
    static const int grn = 2;
    static const int yel = 3;

    if (m_labels.contains(id_pwr)) {
	m_labels[id_pwr]->setPixmap(led(id_pwr, m_dev->isOpen() ? yel : off));
    }
    if (m_labels.contains(id_rxd)) {
	m_labels[id_rxd]->setPixmap(led(id_rxd, m_dev->bytesAvailable() > 0 ? yel : grn));
    }
    if (m_labels.contains(id_txd)) {
	m_labels[id_txd]->setPixmap(led(id_txd, m_dev->bytesToWrite() > 0 ? yel : grn));
    }

    QSerialPort* stty = qobject_cast<QSerialPort*>(m_dev);
    if (stty) {
	QSerialPort::PinoutSignals pin = stty->pinoutSignals();
	const QSerialPort::SerialPortError err = stty->error();

	if (m_labels.contains(id_dcd)) {
	    m_labels[id_dcd]->setPixmap(led(id_dcd, pin.testFlag(QSerialPort::DataCarrierDetectSignal) ? red : off));
	}
	if (m_labels.contains(id_dtr)) {
	    m_labels[id_dtr]->setPixmap(led(id_dtr, pin.testFlag(QSerialPort::DataTerminalReadySignal) ? grn : off));
	}
	if (m_labels.contains(id_dsr)) {
	    m_labels[id_dsr]->setPixmap(led(id_dsr, pin.testFlag(QSerialPort::DataSetReadySignal) ? red : off));
	}
	if (m_labels.contains(id_rts)) {
	    m_labels[id_rts]->setPixmap(led(id_rts, pin.testFlag(QSerialPort::RequestToSendSignal) ? grn : off));
	}
	if (m_labels.contains(id_cts)) {
	    m_labels[id_cts]->setPixmap(led(id_cts, pin.testFlag(QSerialPort::ClearToSendSignal) ? red : off));
	}
	if (m_labels.contains(id_brk)) {
	    m_labels[id_brk]->setPixmap(led(id_brk, stty->isBreakEnabled() ? red : off));
	}
	if (m_labels.contains(id_ri)) {
	    m_labels[id_ri]->setPixmap(led(id_ri, pin.testFlag(QSerialPort::RingIndicatorSignal) ? red : off));
	}
	if (m_labels.contains(id_fe)) {
	    m_labels[id_fe]->setPixmap(led(id_fe, QSerialPort::FramingError == err ? red : off));
	}
	if (m_labels.contains(id_pe)) {
	    m_labels[id_pe]->setPixmap(led(id_pe, QSerialPort::ParityError == err ? red : off));
	}

	if (redo) {
	    update_baud_rate(stty->baudRate(), QSerialPort::AllDirections);
	    update_parity_data_stop();
	    update_flow_control(stty->flowControl());
	    QTimer::singleShot(25, this, SLOT(update_pinout()));
	}
    } else if (redo) {
	QTimer::singleShot(25, this, SLOT(update_pinout()));
    }
}

void QFlexProp::tab_changed(int index)
{
    Q_UNUSED(index)
    PropEdit* pe = current_editor();
    bool enable = pe != nullptr;
    ui->action_Show_listing->setEnabled(enable && !pe->property(id_tab_p2asm).isNull());
    ui->action_Show_binary->setEnabled(enable && !pe->property(id_tab_binary).isNull());
    ui->action_Compile->setEnabled(enable);
    ui->action_Upload_P2->setEnabled(enable);
    ui->action_Run->setEnabled(enable);
}

void QFlexProp::log_message(const QString& message)
{
    QEventLoop loop(this);
    log_status(message);
    loop.processEvents();
}


void QFlexProp::log_status(const QString& message, bool icon)
{
    QComboBox* cb_status = ui->statusbar->findChild<QComboBox*>(id_status);
    if (!cb_status) {
	qDebug("%s: %s", __func__, qPrintable(message));
	return;
    }
    const int index = cb_status->count();
    cb_status->addItem(message);
    if (icon) {
	cb_status->setItemData(index, QIcon(":/images/status.png"), Qt::DecorationRole);
    }
    cb_status->setCurrentIndex(index);
    QEventLoop loop(this);
    loop.processEvents();
}

void QFlexProp::log_error(const QString& message, bool icon)
{
    QComboBox* cb_status = ui->statusbar->findChild<QComboBox*>(id_status);
    if (!cb_status) {
	qDebug("%s: %s", __func__, qPrintable(message));
	return;
    }
    const int index = cb_status->count();
    cb_status->addItem(message);
    if (icon) {
	cb_status->setItemData(index, QIcon(":/images/error.png"), Qt::DecorationRole);
    }
    cb_status->setItemData(index, QColor(qRgb(0xff,0x40,0x00)), Qt::ForegroundRole);
    cb_status->setCurrentIndex(index);
    QEventLoop loop(this);
    loop.processEvents();
}

void QFlexProp::update_baud_rate(qint32 baudRate, QSerialPort::Directions directions)
{
    if (!m_labels.contains(id_baud_rate))
	return;

    QLocale locale = QLocale::system();
    QLabel* lbl_baud = m_labels[id_baud_rate];
    QString baud = locale.toString(baudRate);
    QString dir = direction_str.value(directions);
    lbl_baud->setText(QString("%1%2").arg(dir).arg(baud));
}

void QFlexProp::update_parity_data_stop()
{
    if (!m_labels.contains(id_parity_data_stop))
	return;

    QSerialPort* stty = qobject_cast<QSerialPort*>(m_dev);
    if (stty) {
	QLabel* lbl_pds = m_labels[id_parity_data_stop];
	Q_ASSERT(lbl_pds);
	QString parity = m_dev ? QString(parity_char.value(stty->parity())) : str_unknown;
	QString data = m_dev ? data_bits_str.value(stty->dataBits()) : str_unknown;
	QString stop = m_dev ? stop_bits_str.value(stty->stopBits()) : str_unknown;
	QString dps = QString("%1%2%3")
		      .arg(parity)
		      .arg(data)
		      .arg(stop);
	lbl_pds->setText(dps);
    }
}

void QFlexProp::update_data_bits(QSerialPort::DataBits)
{
    update_parity_data_stop();
}

void QFlexProp::update_parity(QSerialPort::Parity)
{
    update_parity_data_stop();
}

void QFlexProp::update_stop_bits(QSerialPort::StopBits)
{
    update_parity_data_stop();
}

void QFlexProp::update_dtr(bool set)
{
    Q_UNUSED(set)
    update_pinout();
}

void QFlexProp::update_rts(bool set)
{
    Q_UNUSED(set)
    update_pinout();
}

void QFlexProp::error_occured(QSerialPort::SerialPortError error)
{
    QString message;
    switch (error) {
    case QSerialPort::NoError:
	message = tr("Opened device %1.").arg(m_port_name);
	log_status(message);
	return;
    case QSerialPort::DeviceNotFoundError:
	message = tr("Device %1 not found.").arg(m_port_name);
	break;
    case QSerialPort::PermissionError:
	message = tr("Insufficient permission to access device %1.").arg(m_port_name);
	break;
    case QSerialPort::OpenError:
	message = tr("Could not open device %1.").arg(m_port_name);
	break;
    case QSerialPort::ParityError:
	message = tr("Parity error on device %1.").arg(m_port_name);
	break;
    case QSerialPort::FramingError:
	message = tr("Framing error on device %1.").arg(m_port_name);
	break;
    case QSerialPort::BreakConditionError:
	message = tr("Break conidition error on device %1.").arg(m_port_name);
	break;
    case QSerialPort::WriteError:
	message = tr("Write error on device %1.").arg(m_port_name);
	break;
    case QSerialPort::ReadError:
	message = tr("Read error on device %1.").arg(m_port_name);
	break;
    case QSerialPort::ResourceError:
	message = tr("Resource error on device %1.").arg(m_port_name);
	break;
    case QSerialPort::UnsupportedOperationError:
	message = tr("Unsupported operation on device %1: %2.")
		  .arg(m_port_name)
		  .arg(m_stty_operation);
	break;
    case QSerialPort::UnknownError:
	message = tr("Unknown error on device %1.").arg(m_port_name);
	break;
    case QSerialPort::TimeoutError:
	// message = tr("Timeout on device %1.").arg(m_port_name);
	break;
    case QSerialPort::NotOpenError:
	message = tr("Device %1 is not opened.").arg(m_port_name);
	break;
    }
    if (!message.isEmpty())
	log_error(message);
}

void QFlexProp::update_break_enable(bool set)
{
    Q_UNUSED(set)
    update_pinout(true);
}

void QFlexProp::update_flow_control(QSerialPort::FlowControl flow_control)
{
    if (!m_labels.contains(id_flow_control))
	return;
    QLabel* lbl_flow = m_labels[id_flow_control];
    lbl_flow->setText(flow_ctrl_str.value(flow_control));
    lbl_flow->setToolTip(flow_ctrl_tooltip.value(flow_control));
}

void QFlexProp::setup_statusbar()
{
    const QFrame::Shape shape = QFrame::WinPanel;
    const QFrame::Shadow shadow = QFrame::Raised;

    delete m_labels.value(id_baud_rate);
    QLabel* lbl_baud = new QLabel;
    m_labels.insert(id_baud_rate, lbl_baud);
    lbl_baud->setObjectName(id_baud_rate);
    lbl_baud->setFrameShape(shape);
    lbl_baud->setFrameShadow(shadow);
    lbl_baud->setText("-");
    lbl_baud->setToolTip(tr("Currently selected baud rate (bits per second)."));
    ui->statusbar->addPermanentWidget(lbl_baud);

    delete m_labels.value(id_parity_data_stop);
    QLabel* lbl_dps = new QLabel;
    m_labels.insert(id_parity_data_stop, lbl_dps);
    lbl_dps->setObjectName(id_parity_data_stop);
    lbl_dps->setFrameShape(shape);
    lbl_dps->setFrameShadow(shadow);
    lbl_dps->setText("???");
    lbl_dps->setToolTip(tr("Number of data bits, parity, and number of stop bits per character."));
    ui->statusbar->addPermanentWidget(lbl_dps);

    delete m_labels.value(id_flow_control);
    QLabel* lbl_flow = new QLabel;
    m_labels.insert(id_flow_control, lbl_flow);
    lbl_flow->setObjectName(id_flow_control);
    lbl_flow->setFrameShape(shape);
    lbl_flow->setFrameShadow(shadow);
    lbl_flow->setText("-");
    lbl_flow->setToolTip(tr("Type of flow control."));
    ui->statusbar->addPermanentWidget(lbl_flow);

    foreach(const QString& key, m_leds) {
	delete m_labels.value(key);
	QLabel* lbl = new QLabel;
	m_labels.insert(key, lbl);
	lbl->setIndent(0);
	lbl->setObjectName(key);
	lbl->setPixmap(led(key, 0));
	lbl->setToolTip(pinout_leds.value(key));
	ui->statusbar->addPermanentWidget(lbl);
    }

    QComboBox* cb_status = ui->statusbar->findChild<QComboBox*>(id_status);
    delete cb_status;
    cb_status = new QComboBox;
    cb_status->setObjectName(id_status);
    cb_status->setToolTip(tr("Most recent status message."));
    ui->statusbar->addWidget(cb_status, 1);

    log_status(tr("%1 %2 says \"%3\"")
		  .arg(qApp->applicationName())
		  .arg(qApp->applicationVersion())
		  .arg(tr("Hello!")));
}

void QFlexProp::setup_port()
{
    SerTerm* st = ui->tabWidget->findChild<SerTerm*>(id_terminal);
    Q_ASSERT(st);
    bool ok;

    QSerialPortInfo si(m_port_name);
    if (si.isNull()) {
	m_dev = new QFile(m_port_name);
	m_labels[id_baud_rate]->setVisible(false);
	m_labels[id_parity_data_stop]->setVisible(false);
	m_labels[id_flow_control]->setVisible(false);
	foreach(const QString& key, m_leds) {
	    m_labels[key]->setVisible(false);
	}
	m_labels[id_pwr]->setVisible(m_enabled_leds.value(id_pwr, false));
	m_labels[id_rxd]->setVisible(m_enabled_leds.value(id_rxd, false));
	m_labels[id_txd]->setVisible(m_enabled_leds.value(id_txd, false));
    } else {
	m_dev = new QSerialPort(si);
	QSerialPort* stty = qobject_cast<QSerialPort*>(m_dev);
	if (stty) {
	    m_labels[id_baud_rate]->setVisible(true);
	    m_labels[id_parity_data_stop]->setVisible(true);
	    m_labels[id_flow_control]->setVisible(true);
	    foreach(const QString& key, m_leds) {
		m_labels[key]->setVisible(m_enabled_leds.value(key, false));
	    }

	    ok = connect(stty, &QSerialPort::baudRateChanged,
			 this,  &QFlexProp::update_baud_rate,
			 Qt::UniqueConnection);
	    Q_ASSERT(ok);

	    ok = connect(stty, &QSerialPort::dataBitsChanged,
			 this, &QFlexProp::update_data_bits,
			 Qt::UniqueConnection);
	    Q_ASSERT(ok);

	    ok = connect(stty, &QSerialPort::parityChanged,
			 this, &QFlexProp::update_parity,
			 Qt::UniqueConnection);
	    Q_ASSERT(ok);

	    ok = connect(stty, &QSerialPort::stopBitsChanged,
			 this, &QFlexProp::update_stop_bits,
			 Qt::UniqueConnection);
	    Q_ASSERT(ok);

	    ok = connect(stty, &QSerialPort::flowControlChanged,
			 this, &QFlexProp::update_flow_control,
			 Qt::UniqueConnection);
	    Q_ASSERT(ok);

	    ok = connect(stty, &QSerialPort::dataTerminalReadyChanged,
			 this, &QFlexProp::update_dtr,
			 Qt::UniqueConnection);
	    Q_ASSERT(ok);

	    ok = connect(stty, &QSerialPort::requestToSendChanged,
			 this, &QFlexProp::update_rts,
			 Qt::UniqueConnection);
	    Q_ASSERT(ok);

	    ok = connect(stty, &QSerialPort::breakEnabledChanged,
			 this, &QFlexProp::update_break_enable,
			 Qt::UniqueConnection);
	    Q_ASSERT(ok);

	    ok = connect(stty, &QSerialPort::errorOccurred,
			 this, &QFlexProp::error_occured,
			 Qt::UniqueConnection);
	    Q_ASSERT(ok);
	}
    }

    ok = connect(m_dev, &QSerialPort::readyRead,
		 this, &QFlexProp::dev_ready_read,
		 Qt::UniqueConnection);
    if (st) {
	st->set_device(m_dev);
    }
    Q_ASSERT(ok);
}

void QFlexProp::open_port()
{
    setup_port();

    QSerialPort* stty = qobject_cast<QSerialPort*>(m_dev);
    if (stty) {
	stty->setPortName(m_port_name);

	m_stty_operation = tr("setBaudRate(%1)").arg(m_baud_rate);
	stty->setBaudRate(m_baud_rate);

	m_stty_operation = tr("setParity(%1)").arg(parity_str.value(m_parity));
	stty->setParity(m_parity);

	m_stty_operation = tr("setDataBits(%1)").arg(data_bits_str.value(m_data_bits));
	stty->setDataBits(m_data_bits);

	m_stty_operation = tr("setStopBits(%1)").arg(stop_bits_str.value(m_stop_bits));
	stty->setStopBits(m_stop_bits);

	m_stty_operation = tr("setFlowControl(%1)").arg(flow_control_str.value(m_flow_control));
	stty->setFlowControl(m_flow_control);

	m_stty_operation = tr("open(%1)").arg(QLatin1String("QIODevice::ReadWrite"));
	if (stty->open(QIODevice::ReadWrite)) {
	    m_stty_operation = tr("setDataTerminalReady(%1)").arg("true");
	    stty->setDataTerminalReady(true);

	    if (m_flow_control == QSerialPort::HardwareControl) {
		m_stty_operation = tr("setRequestToSend(%1)").arg("true");
		stty->setRequestToSend(true);
	    }
	} else {
	    qDebug("%s: failed to %s", __func__, qPrintable(m_stty_operation));
	}
    }
#if defined(Q_OS_LINUX)
    else do {
	int fdm = ::open(m_port_name.toLatin1().constData(), O_RDWR);
	if (fdm < 0) {
	    log_error(tr("Could not open device %1: %2")
		      .arg(m_port_name)
		      .arg(strerror(errno)));
	    break;
	}
	if (grantpt(fdm) < 0) {
	    log_error(tr("Could not grant access to slave for %1 (%2): %3")
		      .arg(m_port_name)
		      .arg(fdm)
		      .arg(strerror(errno)));
	    break;
	}
	if (unlockpt(fdm) < 0) {
	    log_error(tr("Could not clear slave's lock flag for %1 (%2): %3")
		      .arg(m_port_name)
		      .arg(fdm)
		      .arg(strerror(errno)));
	    break;
	}
	const char* pts = ptsname(fdm);
	if (nullptr == pts) {
	    log_error(tr("Could not get ptsname for %1 (%2): %3")
		      .arg(m_port_name)
		      .arg(fdm)
		      .arg(strerror(errno)));
	    break;
	}
	m_dev = new QFile(QString::fromLatin1(pts));
	if (!m_dev->open(QIODevice::ReadWrite)) {
	    log_error(tr("Could not open device %1: %2")
		      .arg(m_port_name)
		      .arg(m_dev->errorString()), true);
	}
	break;
    } while (0);
#endif

    setup_widget();
    update_parity_data_stop();
    update_pinout();
}

void QFlexProp::close_port()
{
    disconnect(m_dev);
    m_dev->close();
    setup_widget();
    update_pinout();
}

/**
 * @brief Insert a new tab into the main window's QTabWidget
 * @param filename name of the file to load (if it exists)
 * @return tab identifier (zero based)
 */
int QFlexProp::insert_tab(const QString& filename)
{
    QLocale locale = QLocale::system();
    QSettings s;
    const int tabs = ui->tabWidget->count();
    const int curtab = tabs - 1;
    QWidget *tab;
    QVBoxLayout *vlay;
    QSplitter *spl;
    QScrollArea *sa;
    QTextBrowser *tb;
    PropEdit *pe;

    tab = new QWidget();
    tab->setObjectName(QString("tab_%1").arg(curtab));

    vlay = new QVBoxLayout(tab);
    vlay->setObjectName(QString("vlay_%1").arg(curtab));

    spl = new QSplitter(Qt::Vertical);
    spl->setObjectName(QString("spl_%1").arg(curtab));

    vlay->addWidget(spl);

    sa = new QScrollArea();
    sa->setObjectName(QString("sa_%1").arg(curtab));
    sa->setWidgetResizable(true);

    pe = new PropEdit();
    pe->setObjectName(QString("pe_%1").arg(curtab));
    pe->setGeometry(QRect(0, 0, 512, 512));
    pe->setFont(m_fixedfont);

    sa->setWidget(pe);
    spl->addWidget(sa);

    tb = new QTextBrowser();
    tb->setObjectName(QString("tb_%1").arg(curtab));
    tb->setWordWrapMode(QTextOption::NoWrap);
    tb->setFont(QFont(QStringLiteral("Monospace"), tb->font().pointSize()));
    spl->addWidget(tb);

    pe->setFilename(filename);
    QFileInfo info(filename);
    QString title = QString("%1 [%2]")
		    .arg(info.fileName())
		    .arg(pe->filetype_name());
    if (info.exists()) {
	if (pe->load(filename)) {
	    ui->statusbar->showMessage(tr("Loaded file '%1' (%2 Bytes).")
				    .arg(info.fileName())
				   .arg(locale.toString(info.size())));
	    ui->tabWidget->setCurrentIndex(curtab);
	} else {
	    ui->statusbar->showMessage(tr("Could not load file '%1'.")
				    .arg(info.fileName()));
	}
    }
    ui->tabWidget->insertTab(curtab, tab, title);
    ui->tabWidget->setCurrentIndex(curtab);

    // Make the splitter bottom (text browser) height 1/6th
    // its default height of half the height of the tab
    QList<int> sizes = spl->sizes();
    sizes[0] += sizes[1] * 5 / 6;
    sizes[1] = sizes[1] / 6;
    spl->setSizes(sizes);

    return curtab;
}

void QFlexProp::on_action_New_triggered()
{
    QSettings s;
    QString srcdflt = QString("%1/p2tools").arg(QDir::homePath());
    const QString srcdir = s.value(id_sourcedir, srcdflt).toString();
    const QString new_filename = QString("%1/newfile.spin").arg(srcdir);
    const QString bak_filename = QString("%1~").arg(new_filename);

    QFile::remove(bak_filename);
    QFile::rename(new_filename, bak_filename);
    insert_tab(new_filename);
}

void QFlexProp::on_action_Open_triggered()
{
    QString filename = load_file(tr("Open source file"));
    if (filename.isEmpty())
	return;
    insert_tab(filename);
}

void QFlexProp::on_action_Save_triggered()
{
    QLocale locale = QLocale::system();
    PropEdit *pe = current_editor();
    if (!pe) {
	return;
    }

    if (pe->changed()) {
	QString text = pe->text();
	QString filename = pe->filename();
	QFileInfo info(filename);
	ui->statusbar->showMessage(tr("File '%1' did not change.")
				   .arg(info.fileName()));
	return;
    }

    QFileInfo info(pe->filename());
    if (pe->save()) {
	ui->statusbar->showMessage(tr("Saved file '%1' (%2 Bytes).")
				   .arg(info.fileName())
				   .arg(locale.toString(info.size())));
    } else {
	ui->statusbar->showMessage(tr("Could not save file '%1'.")
				   .arg(info.fileName()));
    }
}

void QFlexProp::on_action_Save_as_triggered()
{
    QLocale locale = QLocale::system();
    PropEdit *pe = current_editor();
    if (!pe) {
	QMessageBox::critical(this, tr("No propEdit widget!"),
		     tr("There is selected does not contain a propEdit widget."),
		     QMessageBox::Close, QMessageBox::Close);
	return;
    }
    QString filename = pe->filename();
    QString save_as = save_file(filename, tr("Save source file"));
    if (save_as.isEmpty()) {
	ui->statusbar->showMessage(tr("Saving file '%1' cancelled.")
				   .arg(filename));
	return;
    }

    QFileInfo info(save_as);
    if (pe->save(save_as)) {
	ui->statusbar->showMessage(tr("Saved file '%1' (%2 Bytes).")
				   .arg(info.fileName())
				   .arg(locale.toString(info.size())));
    } else {
	ui->statusbar->showMessage(tr("Could not save file '%1'.")
				   .arg(info.fileName()));
    }
}

void QFlexProp::on_action_Close_triggered()
{
    PropEdit *pe = current_editor();
    if (!pe) {
	return;
    }
    if (pe->changed()) {
	int res = QMessageBox::information(this,
					   tr("File '%1' changed!").arg(pe->filename()),
					   tr("The file '%1' was modified. Do you want to save it before closing the tab?").arg(pe->filename()),
					   QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel);
	if (res == QMessageBox::Cancel) {
	    return;
	}
	if (res == QMessageBox::Yes) {
	    pe->save(pe->filename());
	}
    }
    ui->tabWidget->removeTab(ui->tabWidget->currentIndex());
}

void QFlexProp::on_action_Quit_triggered()
{
    close();
}

void QFlexProp::on_action_Select_all_triggered()
{

}

void QFlexProp::on_action_Delete_triggered()
{

}

void QFlexProp::on_action_Cut_triggered()
{

}

void QFlexProp::on_action_Copy_triggered()
{

}

void QFlexProp::on_action_Paste_triggered()
{

}

void QFlexProp::on_action_Configure_serialport_triggered()
{
    bool was_open = m_dev->isOpen();
    QSettings s;
    SerialPortDlg::Settings settings;
    SerialPortDlg dlg(this);
    settings.name = m_port_name;
    settings.baud_rate = m_baud_rate;
    settings.data_bits = m_data_bits;
    settings.parity = m_parity;
    settings.stop_bits = m_stop_bits;
    settings.flow_control = m_flow_control;
    settings.local_echo = m_local_echo;
    dlg.set_settings(settings);

    if (QDialog::Accepted != dlg.exec())
	return;

    settings = dlg.settings();
    if (was_open) {
	close_port();
    }
    // Use the selected settings for the global serial port settings
    m_port_name = settings.name;
    m_baud_rate = settings.baud_rate;
    m_parity = settings.parity;
    m_data_bits = settings.data_bits;
    m_stop_bits = settings.stop_bits;
    m_flow_control = settings.flow_control;
    m_local_echo = settings.local_echo;
    qDebug("%s: port name    : %s", __func__, qPrintable(m_port_name));
    qDebug("%s: baud rate    : %d", __func__, m_baud_rate);
    qDebug("%s: parity       : %s", __func__, qPrintable(parity_str.value(m_parity)));
    qDebug("%s: data bits    : %s", __func__, qPrintable(data_bits_str.value(m_data_bits)));
    qDebug("%s: stop bits    : %s", __func__, qPrintable(stop_bits_str.value(m_stop_bits)));
    qDebug("%s: flow control : %s", __func__, qPrintable(flow_control_str.value(m_flow_control)));
    qDebug("%s: local echo   : %s", __func__, m_local_echo ? "on" : "off");
    if (was_open) {
	open_port();
    } else {
	setup_widget();
    }
}

void QFlexProp::on_action_Configure_flexspin_triggered()
{
    FlexspinDlg dlg(this);
    FlexspinDlg::Settings f;
    f.binary = m_flexspin_binary;
    f.include_paths = m_flexspin_include_paths;
    f.optimize = m_flexspin_optimize;
    f.listing = m_flexspin_listing;
    f.warnings = m_flexspin_warnings;
    f.errors = m_flexspin_errors;
    f.hub_address = m_flexspin_hub_address;
    f.skip_coginit = m_flexspin_skip_coginit;
    dlg.set_settings(f);

    int res = dlg.exec();
    if (QDialog::Accepted != res)
	return;

    f = dlg.settings();
    m_flexspin_binary = f.binary;
    m_flexspin_include_paths = f.include_paths;
    m_flexspin_optimize = f.optimize;
    m_flexspin_listing = f.listing;
    m_flexspin_warnings = f.warnings;
    m_flexspin_errors = f.errors;
    m_flexspin_hub_address = f.hub_address;
    m_flexspin_skip_coginit = f.skip_coginit;

    QSettings s;
    s.beginGroup(id_grp_flexspin);
    s.setValue(id_flexspin_binary, m_flexspin_binary);
    s.setValue(id_flexspin_include_paths, m_flexspin_include_paths);
    s.setValue(id_flexspin_optimize, m_flexspin_optimize);
    s.setValue(id_flexspin_listing, m_flexspin_listing);
    s.setValue(id_flexspin_warnings, m_flexspin_warnings);
    s.setValue(id_flexspin_errors, m_flexspin_errors);
    s.setValue(id_flexspin_hub_address, m_flexspin_hub_address);
    s.setValue(id_flexspin_skip_coginit, m_flexspin_skip_coginit);
    s.endGroup();
}

QString QFlexProp::quoted(const QString& src)
{
    if (src.contains(QChar::Space)) {
	return QString("'%1'").arg(src);
    }
    return src;
}

bool QFlexProp::flexspin(QString* p_p2asm, QByteArray* p_binary)
{
    PropEdit *pe = current_editor();
    if (!pe) {
	return false;
    }
    QTextBrowser *tb = current_browser();
    Q_ASSERT(tb);
    tb->clear();

    QFile src(pe->filename());
    QStringList args;

    // compile for Prop2
    args += QStringLiteral("-2");

    // append include paths
    foreach(const QString& include_path, m_flexspin_include_paths) {
	// We need to quote paths with embedded spaces (e.g. Windows)
	args += QString("-L %1").arg(quoted(include_path));
    }

    if (m_flexspin_hub_address > 0) {
	args += QString("-H %1").arg(m_flexspin_hub_address, 4, 16, QChar('0'));
	// Add flag for skip coginit
	if (m_flexspin_skip_coginit)
	    args += QStringLiteral("-E");
    }

    // generate a listing
    if (m_flexspin_listing)
	args += QStringLiteral("-l");

    // add option for warnings
    if (m_flexspin_warnings)
	args += QStringLiteral("-Wall");

    // add option for errors
    if (m_flexspin_errors)
	args += QStringLiteral("-Werror");

    // add source filename
    args += src.fileName();

    tb->setTextColor(Qt::blue);
    tb->append(QString("%1 %2")
	       .arg(m_flexspin_binary)
	       .arg(args.join(QStringLiteral(" \\\n\t"))));

    QProcess process(this);
    process.setProperty(id_process_tb, QVariant::fromValue(tb));
    connect(&process, &QProcess::channelReadyRead,
	    this, &QFlexProp::channelReadyRead);

    process.start(m_flexspin_binary, args);
    if (QProcess::Starting == process.state()) {
	if (!process.waitForStarted()) {
	    qCritical("%s: result code %d", __func__, process.exitCode());
	    tb->setTextColor(Qt::red);
	    tb->append(tr("Result code %1.").arg(process.exitCode()));
	    return false;
	}
    }
    do {
	if (!process.waitForFinished()) {
	    qCritical("%s: result code %d", __func__, process.exitCode());
	    tb->setTextColor(Qt::red);
	    tb->append(tr("Result code %1.").arg(process.exitCode()));
	    return false;
	}
    } while (QProcess::Running == process.state());

    // check, load, and remove intermediate p2asm file
    QFileInfo info(src.fileName());
    QString p2asm_filename = QString("%1/%2.p2asm")
			     .arg(info.absoluteDir().path())
			     .arg(info.baseName());
    QFile p2asm(p2asm_filename);
    if (p2asm.exists()) {
	if (p2asm.open(QIODevice::ReadOnly)) {
	    QString listing = QString::fromUtf8(p2asm.readAll());
	    pe->setProperty(id_tab_p2asm, listing);
	    if (p_p2asm) {
		// caller wants the listing
		*p_p2asm = listing;
	    }
	    p2asm.close();
	}
	p2asm.remove();
    }

    // check, load, and remove resulting binary file
    QString binary_filename = QString("%1/%2.binary")
				.arg(info.absoluteDir().path())
				.arg(info.baseName());
    QFile binfile(binary_filename);
    if (binfile.exists()) {
	if (binfile.open(QIODevice::ReadOnly)) {
	    QByteArray binary = binfile.readAll();
	    pe->setProperty(id_tab_binary, binary);
	    if (p_binary) {
		// caller wants the binary
		*p_binary = binary;
	    }
	    binfile.close();
	}
	binfile.remove();
    }
    return true;
}

void QFlexProp::on_action_Compile_triggered()
{
    flexspin();
}

void QFlexProp::on_action_Upload_P2_triggered()
{
    PropEdit* pe = current_editor();
    Q_ASSERT(pe);
    QByteArray binary = pe->property(id_tab_binary).toByteArray();
    if (binary.isEmpty()) {
	// Need to compile first
	flexspin(nullptr, &binary);
    }
}

void QFlexProp::on_action_Run_triggered()
{
    SerTerm* st = ui->tabWidget->findChild<SerTerm*>(id_terminal);
    Q_ASSERT(st);
    QTextBrowser* tb = current_browser();
    Q_ASSERT(tb);

    QByteArray binary;
    flexspin(nullptr, &binary);

    m_transfer.lock();
    st->reset();
    PropLoad phex(m_dev, this);
    phex.set_verbose();
    // phex.set_use_checksum(false);
    phex.setProperty(id_process_tb, QVariant::fromValue(tb));
    connect(&phex, &PropLoad::Error,
	    this, &QFlexProp::printError);
    connect(&phex, &PropLoad::Message,
	    this, &QFlexProp::printMessage);
    phex.load_file(binary);
    m_transfer.unlock();
}

void QFlexProp::on_action_About_triggered()
{
    AboutDlg dlg(this);
    dlg.exec();
}

void QFlexProp::on_action_About_Qt5_triggered()
{
    qApp->aboutQt();
}

void QFlexProp::channelReadyRead(int channel)
{
    QProcess* process = qobject_cast<QProcess*>(sender());
    if (!process)
	return;
    QTextBrowser* tb = qvariant_cast<QTextBrowser*>(process->property(id_process_tb));
    if (!tb)
	return;
    process->setReadChannel(static_cast<QProcess::ProcessChannel>(channel));
    QString message = QString::fromUtf8(process->readAll());
    switch (channel) {
    case QProcess::StandardOutput:
	printMessage(message);
	break;
    case QProcess::StandardError:
	printError(message);
	break;
    }
}

void QFlexProp::printError(const QString& message)
{
    QEventLoop loop(this);
    QTextBrowser* tb = qvariant_cast<QTextBrowser*>(sender()->property(id_process_tb));
    if (!tb)
	return;
    tb->setTextColor(Qt::red);
    tb->append(message);
    loop.processEvents();
}

void QFlexProp::printMessage(const QString& message)
{
    QEventLoop loop(this);
    QTextBrowser* tb = qvariant_cast<QTextBrowser*>(sender()->property(id_process_tb));
    if (!tb)
	return;
    tb->setTextColor(Qt::black);
    tb->append(message);
    loop.processEvents();
}
