/*****************************************************************************
 *
 * Qt5 serial terminal emulator
 * Copyright (c) 2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#include <QPair>
#include <QEventLoop>
#include <QSerialPortInfo>
#include <QLocale>
#include <QFileDialog>
#include <QTextStream>
#include <QStandardPaths>
#include <QTimer>
#include "serterm.h"
#include "vtscrollarea.h"
#include "ui_serterm.h"
#include "idstrings.h"
#include "util.h"

SerTerm::SerTerm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SerTerm)
    , m_dev(nullptr)
    , m_font_family()
    , m_zoom(100)
    , m_download_path()
    , m_caps_lock(false)
    , m_num_lock(false)
    , m_scroll_lock(false)
    , m_local_echo(false)
{
    ui->setupUi(this);
    load_config();
    setup_terminal();
    setFocusPolicy(Qt::StrongFocus);
}

SerTerm::~SerTerm()
{
    save_config();
    delete ui;
}

void SerTerm::set_device(QIODevice* dev)
{
    m_dev = dev;
}

void SerTerm::term_set_size(int width, int height)
{
    ui->vterm->term_set_size(width, height);
}

void SerTerm::term_set_width(int width)
{
    ui->vterm->term_set_columns(width);
}

void SerTerm::term_set_height(int height)
{
    ui->vterm->term_set_rows(height);
}

void SerTerm::term_toggle_80_132()
{
    ui->vterm->term_toggle_80_132();
}

void SerTerm::putch(uchar ch)
{
    ui->vterm->putch(ch);
}

int SerTerm::write(const QByteArray& data)
{
    return ui->vterm->write(data);
}

int SerTerm::write(const char* data, size_t len)
{
    return ui->vterm->write(data, len);
}

void SerTerm::display_text(const QString& filename)
{
    ui->vterm->display_text(filename);
}

void SerTerm::display_maps()
{
    ui->vterm->display_maps();
}

void SerTerm::set_font_family(const QString& family)
{
    ui->vterm->set_font_family(family);
}

void SerTerm::set_zoom(int percent)
{
    ui->vterm->set_zoom(percent);
}

void SerTerm::reset()
{
    reset_prop();
    m_dev->readAll();
}
void SerTerm::term_clear()
{
    ui->vterm->clear();
}

void SerTerm::term_width(bool on)
{
    ui->vterm->term_set_columns(on ? 132 : 80);
}


void SerTerm::geometry_changed()
{
    QTimer::singleShot(10, this, SLOT(term_fit_best()));
}

void SerTerm::term_fit_best()
{
    ui->scrollArea->updateGeometry();
}

void SerTerm::zoom_original()
{
    ui->vterm->set_zoom(100);
}

void SerTerm::zoom_in()
{
    int zoom = ui->vterm->zoom();
    if (zoom < 300) {
	m_zoom = zoom + 4;
	ui->vterm->set_zoom(m_zoom);
    }
}

void SerTerm::zoom_out()
{
    int zoom = ui->vterm->zoom();
    if (zoom > 5) {
	m_zoom = zoom - 4;
	ui->vterm->set_zoom(m_zoom);
    }
}

void SerTerm::setup_terminal()
{
    bool ok;

    ok = connect(ui->vterm, &vt220::UpdateSize,
	    ui->scrollArea, &vtScrollArea::UpdateSize,
	    Qt::UniqueConnection);
    Q_ASSERT(ok);

    ok = connect(ui->vterm, &vt220::UpdateCursor,
	    ui->scrollArea, &vtScrollArea::UpdateCursor,
	    Qt::UniqueConnection);
    Q_ASSERT(ok);

    ui->toolbar->setIconSize(QSize(20, 20));
    ui->toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);

    QAction* act_reset = new QAction(QIcon(":/images/reset.png"), tr("Reset"));
    ok = connect(act_reset, &QAction::triggered,
	    this, &SerTerm::reset_triggered);
    Q_ASSERT(ok);
    ui->toolbar->addAction(act_reset);

    QAction* act_version = new QAction(QIcon(":/images/version.png"), tr("Print version"));
    ok = connect(act_version, &QAction::triggered,
	    this, &SerTerm::version_triggered);
    Q_ASSERT(ok);
    ui->toolbar->addAction(act_version);

    QAction* act_monitor = new QAction(QIcon(":/images/monitor.png"), tr("Enter monitor"));
    ok = connect(act_monitor, &QAction::triggered,
	    this, &SerTerm::monitor_triggered);
    Q_ASSERT(ok);
    ui->toolbar->addAction(act_monitor);

    QAction* act_taqoz = new QAction(QIcon(":/images/taqoz.png"), tr("Enter TAQOZ"));
    ok = connect(act_taqoz, &QAction::triggered,
	    this, &SerTerm::taqoz_triggered);
    Q_ASSERT(ok);
    ui->toolbar->addAction(act_taqoz);

    QAction* act_sendfile = new QAction(QIcon(":/images/sendfile.png"), tr("Send file"));
    ok = connect(act_sendfile, &QAction::triggered,
	    this, &SerTerm::sendfile_triggered);
    Q_ASSERT(ok);
    ui->toolbar->addAction(act_sendfile);

    ui->vterm->set_font_family(m_font_family);
    ui->vterm->set_zoom(m_zoom);
}

void SerTerm::setup_signals()
{
    bool ok;
    ok = connect(ui->vterm, &vt220::term_response,
		 this, &SerTerm::term_response,
		 Qt::UniqueConnection);
    Q_ASSERT(ok);

    ok = connect(ui->vterm, &vt220::UpdateSize,
		 this, &SerTerm::geometry_changed,
		 Qt::UniqueConnection);
    Q_ASSERT(ok);
}

void SerTerm::save_config()
{
    QSettings s;

    s.beginGroup(id_terminal);
    m_zoom = ui->vterm->zoom();
    s.setValue(id_zoom, m_zoom);
    s.endGroup();
}

void SerTerm::load_config()
{
    QSettings s;

    s.beginGroup(id_terminal);
    m_zoom = s.value(id_zoom, 100).toInt();
    m_font_family = s.value(id_font_family, QString()).toString();
    s.endGroup();

    QStringList download_paths = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation);
    if (download_paths.isEmpty()) {
	download_paths += QString("%1/Downloads").arg(QDir::homePath());
    }
    m_download_path = s.value(id_download_path, download_paths.first()).toString();
}

/**
 * @brief Preset a QFileDialog for loading an existing source file
 * @param title window title
 * @return QString with the full path, or empty if cancelled
 */
QString SerTerm::load_file(const QString& title)
{
    QFileDialog dlg(this);
    QSettings s;
    s.beginGroup(id_grp_serterm);
    QStringList documents = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    QString srcdflt = documents.isEmpty()
		      ? QDir::homePath()
		      : documents.first();
    QString srcdir = s.value(id_sourcedir, srcdflt).toString();
    QString filename = s.value(id_filename).toString();
    QStringList history = s.value(id_history).toStringList();
    s.endGroup();
    QStringList filetypes = {
	{"All files (*.*)"},
	{"Text (*.txt)"},
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
    s.beginGroup(id_grp_serterm);
    s.setValue(id_sourcedir, srcdir);
    history.insert(0, filename);
    if (history.size() > 30)
	history.takeLast();
    s.setValue(id_filename, info.fileName());
    s.setValue(id_history, history);
    s.endGroup();

    return filename;
}

void SerTerm::reset_prop()
{
    QSerialPort* port = qobject_cast<QSerialPort*>(m_dev);
    if (port) {
	QEventLoop loop(this);
	port->setDataTerminalReady(false);
	loop.processEvents(QEventLoop::AllEvents, 10);
	port->setDataTerminalReady(true);
    }
}

void SerTerm::reset_triggered(bool checked)
{
    Q_UNUSED(checked);
    reset_prop();
}

void SerTerm::version_triggered(bool checked)
{
    Q_UNUSED(checked);
    QSerialPort* port = qobject_cast<QSerialPort*>(m_dev);
    if (port) {
	reset_prop();
	QByteArray version("> Prop_Chk 0 0 0 0\015");
	port->write(version);
    }
}

void SerTerm::monitor_triggered(bool checked)
{
    Q_UNUSED(checked);
    QSerialPort* port = qobject_cast<QSerialPort*>(m_dev);
    if (port) {
	reset_prop();
	QByteArray monitor("> \004");
	port->write(monitor);
    }
}

void SerTerm::taqoz_triggered(bool checked)
{
    Q_UNUSED(checked);
    QSerialPort* port = qobject_cast<QSerialPort*>(m_dev);
    if (port) {
	reset_prop();
	QByteArray taqoz("> \033");
	port->write(taqoz);
    }
}

void SerTerm::sendfile_triggered(bool checked)
{
    Q_UNUSED(checked);
    QEventLoop loop(this);
    QString filename = load_file(tr("Select file to send"));
    if (filename.isEmpty())
	return;
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly)) {
	QTextStream str(&file);
	str.setCodec("UTF-8");

	while (!str.atEnd()) {
	    QString line = str.readLine();
	    QByteArray utf8 =  line.toUtf8() + '\n';
	    emit term_response(utf8);
	    if (m_local_echo) {
		ui->vterm->write(utf8);
	    }
	    loop.processEvents();
	}
	file.close();
    }
}

/**
 * @brief Handle key press events
 * @param event pointer to the QKeyEvent
 */
void SerTerm::keyPressEvent(QKeyEvent* event)
{
    QString key = nullptr;
    QByteArray data;
    Qt::KeyboardModifiers mod = event->modifiers();
    const bool alt = mod.testFlag(Qt::AltModifier);
    const bool ctrl = mod.testFlag(Qt::ControlModifier);
    const bool shift = mod.testFlag(Qt::ShiftModifier);
    const bool meta = mod.testFlag(Qt::MetaModifier);
    int kmod = (shift ? 0x01 : 0x00) |
	       (alt ? 0x02 : 0x00) |
	       (ctrl ? 0x04 : 0x00) |
	       (meta ? 0x08 : 0x00);

    switch (event->key()) {
    case Qt::Key_Escape:                // misc keys
	key = QChar(ESC);
	break;
    case Qt::Key_Tab:
	key = QChar(HT);
	break;
    case Qt::Key_Backtab:
	key = QChar(VT);
	break;
    case Qt::Key_Backspace:
	key = QChar(BS);
	break;
    case Qt::Key_Return:
	key = QChar(CR);
	break;
    case Qt::Key_Enter:
	key = QChar(LF);
	break;
    case Qt::Key_Insert:
	key = QStringLiteral("%1%2~").arg(QChar(CSI)).arg(2);
	break;
    case Qt::Key_Delete:
	key = QStringLiteral("%1%2~").arg(QChar(CSI)).arg(3);
	break;
    case Qt::Key_Pause:
	break;
    case Qt::Key_Print:               // print screen
	break;
    case Qt::Key_SysReq:
	break;
    case Qt::Key_Clear:
	break;
    case Qt::Key_Home:                // cursor movement
	data = util.fkey_str(CSI, {'H'}, kmod);
	break;
    case Qt::Key_End:
	data = util.fkey_str(CSI, {'F'}, kmod);
	break;

    case Qt::Key_Left:
	data = util.fkey_str(CSI, {'D'}, kmod);
	break;
    case Qt::Key_Up:
	data = util.fkey_str(CSI, {'A'}, kmod);
	break;
    case Qt::Key_Right:
	data = util.fkey_str(CSI, {'C'}, kmod);
	break;
    case Qt::Key_Down:
	data = util.fkey_str(CSI, {'B'}, kmod);
	break;

    case Qt::Key_PageUp:
	data = util.fkey_str(CSI, {'5'}, kmod);
	break;
    case Qt::Key_PageDown:
	data = util.fkey_str(CSI, {'6'}, kmod);
	break;

    case Qt::Key_Shift:                // modifiers
	break;
    case Qt::Key_Control:
	break;
    case Qt::Key_Meta:
	break;
    case Qt::Key_Alt:
	break;

    case Qt::Key_CapsLock:
	m_caps_lock = !m_caps_lock;
	break;
    case Qt::Key_NumLock:
	m_num_lock = !m_num_lock;
	break;
    case Qt::Key_ScrollLock:
	m_scroll_lock = !m_scroll_lock;
	break;

    case Qt::Key_F1:                // function keys
	data = util.fkey_str(SS3, {'P'}, kmod);
	break;
    case Qt::Key_F2:
	data = util.fkey_str(SS3, {'Q'}, kmod);
	break;
    case Qt::Key_F3:
	data = util.fkey_str(SS3, {'R'}, kmod);
	break;
    case Qt::Key_F4:
	data = util.fkey_str(SS3, {'S'}, kmod);
	break;
    case Qt::Key_F5:
	data = util.fkey_str(CSI, "15", kmod);
	break;
    case Qt::Key_F6:
	data = util.fkey_str(CSI, "17", kmod);
	break;
    case Qt::Key_F7:
	data = util.fkey_str(CSI, "18", kmod);
	break;
    case Qt::Key_F8:
	data = util.fkey_str(CSI, "19", kmod);
	break;
    case Qt::Key_F9:
	data = util.fkey_str(CSI, "20", kmod);
	break;
    case Qt::Key_F10:
	data = util.fkey_str(CSI, "21", kmod);
	break;
    case Qt::Key_F11:
	data = util.fkey_str(CSI, "23", kmod);
	break;
    case Qt::Key_F12:
	data = util.fkey_str(CSI, "24", kmod);
	break;
    case Qt::Key_F13:
	data = util.fkey_str(CSI, "25", kmod);
	break;
    case Qt::Key_F14:
	data = util.fkey_str(CSI, "26", kmod);
	break;
    case Qt::Key_F15:
	data = util.fkey_str(CSI, "28", kmod);
	break;
    case Qt::Key_F16:
	data = util.fkey_str(CSI, "29", kmod);
	break;
    case Qt::Key_F17:
	data = util.fkey_str(CSI, "31", kmod);
	break;
    case Qt::Key_F18:
	data = util.fkey_str(CSI, "32", kmod);
	break;
    case Qt::Key_F19:
	data = util.fkey_str(CSI, "33", kmod);
	break;
    case Qt::Key_F20:
	data = util.fkey_str(CSI, "34", kmod);
	break;
    case Qt::Key_F21:
	break;
    case Qt::Key_F22:
	break;
    case Qt::Key_F23:
	break;
    case Qt::Key_F24:
	break;
    case Qt::Key_F25:                // F25 .. F35 only on X11
	break;
    case Qt::Key_F26:
	break;
    case Qt::Key_F27:
	break;
    case Qt::Key_F28:
	break;
    case Qt::Key_F29:
	break;
    case Qt::Key_F30:
	break;
    case Qt::Key_F31:
	break;
    case Qt::Key_F32:
	break;
    case Qt::Key_F33:
	break;
    case Qt::Key_F34:
	break;
    case Qt::Key_F35:
	break;

    case Qt::Key_Super_L:                 // extra keys
	break;
    case Qt::Key_Super_R:
	break;
    case Qt::Key_Menu:
	break;
    case Qt::Key_Hyper_L:
	break;
    case Qt::Key_Hyper_R:
	break;
    case Qt::Key_Help:
	break;
    case Qt::Key_Direction_L:
	break;
    case Qt::Key_Direction_R:
	break;

    // 7 bit printable ASCII
    case Qt::Key_Space:
	key = QChar::Space;
	break;
    case Qt::Key_Exclam:
	key = QChar('!');
	break;
    case Qt::Key_QuoteDbl:
	key = QChar('"');
	break;
    case Qt::Key_NumberSign:
	key = QChar('#');
	break;
    case Qt::Key_Dollar:
	key = QChar('$');
	break;
    case Qt::Key_Percent:
	key = QChar('%');
	break;
    case Qt::Key_Ampersand:
	key = QChar('&');
	break;
    case Qt::Key_Apostrophe:
	key = QChar('\'');
	break;
    case Qt::Key_ParenLeft:
	key = QChar('(');
	break;
    case Qt::Key_ParenRight:
	key = QChar(')');
	break;
    case Qt::Key_Asterisk:
	key = QChar('*');
	break;
    case Qt::Key_Plus:
	// Zoom in ?
	if (ctrl) {
	    zoom_in();
	    break;
	}
	key = QChar('+');
	break;
    case Qt::Key_Comma:
	key = QChar(',');
	break;
    case Qt::Key_Minus:
	// Zoom out ?
	if (ctrl) {
	    zoom_out();
	    break;
	}
	key = QChar('-');
	break;
    case Qt::Key_Period:
	key = QChar('.');
	break;
    case Qt::Key_Slash:
	key = QChar('/');
	break;
    case Qt::Key_0:
	// Zoom 100% ?
	if (ctrl) {
	    m_zoom = 100;
	    set_zoom(m_zoom);
	    break;
	}
	key = QChar('0');
	break;
    case Qt::Key_1:
	key = QChar('1');
	break;
    case Qt::Key_2:
	key = QChar('2');
	break;
    case Qt::Key_3:
	key = QChar('3');
	break;
    case Qt::Key_4:
	key = QChar('4');
	break;
    case Qt::Key_5:
	key = QChar('5');
	break;
    case Qt::Key_6:
	key = QChar('6');
	break;
    case Qt::Key_7:
	key = QChar('7');
	break;
    case Qt::Key_8:
	key = QChar('8');
	break;
    case Qt::Key_9:
	key = QChar('9');
	break;
    case Qt::Key_Colon:
	key = QChar(':');
	break;
    case Qt::Key_Semicolon:
	key = QChar(';');
	break;
    case Qt::Key_Less:
	key = QChar('<');
	break;
    case Qt::Key_Equal:
	key = QChar('=');
	break;
    case Qt::Key_Greater:
	key = QChar('>');
	break;
    case Qt::Key_Question:
	key = QChar('?');
	break;
    case Qt::Key_At:
	key = ctrl ? QChar(0x00)
		   : shift ? QChar('@')
			   : QChar('`');
	break;
    case Qt::Key_A:
	key = ctrl ? QChar(0x01)
		   : shift ? QChar('A')
			   : QChar('a');
	break;
    case Qt::Key_B:
	key = ctrl ? QChar(0x02)
		   : shift ? QChar('B')
			   : QChar('b');
	break;
    case Qt::Key_C:
	key = ctrl ? QChar(0x03)
		   : shift ? QChar('C')
			   : QChar('c');
	break;
    case Qt::Key_D:
	key = ctrl ? QChar(0x04)
		   : shift ? QChar('D')
			   : QChar('d');
	break;
    case Qt::Key_E:
	key = ctrl ? QChar(0x05)
		   : shift ? QChar('E')
			   : QChar('e');
	break;
    case Qt::Key_F:
	key = ctrl ? QChar(0x06)
		   : shift ? QChar('F')
			   : QChar('f');
	break;
    case Qt::Key_G:
	key = ctrl ? QChar(0x07)
		   : shift ? QChar('G')
			   : QChar('g');
	break;
    case Qt::Key_H:
	key = ctrl ? QChar(0x08)
		   : shift ? QChar('H')
			   : QChar('h');
	break;
    case Qt::Key_I:
	key = ctrl ? QChar(0x09)
		   : shift ? QChar('I')
			   : QChar('i');
	break;
    case Qt::Key_J:
	key = ctrl ? QChar(0x0a)
		   : shift ? QChar('J')
			   : QChar('j');
	break;
    case Qt::Key_K:
	key = ctrl ? QChar(0x0b)
		   : shift ? QChar('K')
			   : QChar('k');
	break;
    case Qt::Key_L:
	key = ctrl ? QChar(0x0c)
		   : shift ? QChar('L')
			   : QChar('l');
	break;
    case Qt::Key_M:
	key = ctrl ? QChar(0x0d)
		   : shift ? QChar('M')
			   : QChar('m');
	break;
    case Qt::Key_N:
	key = ctrl ? QChar(0x0e)
		   : shift ? QChar('N')
			   : QChar('n');
	break;
    case Qt::Key_O:
	key = ctrl ? QChar(0x0f)
		   : shift ? QChar('O')
			   : QChar('o');
	break;
    case Qt::Key_P:
	key = ctrl ? QChar(0x10)
		   : shift ? QChar('P')
			   : QChar('p');
	break;
    case Qt::Key_Q:
	key = ctrl ? QChar(0x11)
		   : shift ? QChar('Q')
			   : QChar('q');
	break;
    case Qt::Key_R:
	key = ctrl ? QChar(0x12)
		   : shift ? QChar('R')
			   : QChar('r');
	break;
    case Qt::Key_S:
	key = ctrl ? QChar(0x13)
		   : shift ? QChar('S')
			   : QChar('s');
	break;
    case Qt::Key_T:
	key = ctrl ? QChar(0x14)
		   : shift ? QChar('T')
			   : QChar('t');
	break;
    case Qt::Key_U:
	key = ctrl ? QChar(0x15)
		   : shift ? QChar('U')
			   : QChar('u');
	break;
    case Qt::Key_V:
	key = ctrl ? QChar(0x16)
		   : shift ? QChar('V')
			   : QChar('v');
	break;
    case Qt::Key_W:
	key = ctrl ? QChar(0x17)
		   : shift ? QChar('W')
			   : QChar('w');
	break;
    case Qt::Key_X:
	key = ctrl ? QChar(0x18)
		   : shift ? QChar('X')
			   : QChar('x');
	break;
    case Qt::Key_Y:
	key = ctrl ? QChar(0x19)
		   : shift ? QChar('Y')
			   : QChar('y');
	break;
    case Qt::Key_Z:
	key = ctrl ? QChar(0x1a)
		   : shift ? QChar('Z')
			   : QChar('z');
	break;
    case Qt::Key_BracketLeft:
	key = ctrl ? QChar(0x1b) : QChar('[');
	break;
    case Qt::Key_Backslash:
	key = ctrl ? QChar(0x1c) : QChar('\\');
	break;
    case Qt::Key_BracketRight:
	key = ctrl ? QChar(0x1d) : QChar(']');
	break;
    case Qt::Key_AsciiCircum:
	key = ctrl ? QChar(0x1e) : QChar('^');
	break;
    case Qt::Key_Underscore:
	key = ctrl ? QChar(0x1f) : QChar('_');
	break;
    case Qt::Key_QuoteLeft:
	key = QChar('"');
	break;
    case Qt::Key_BraceLeft:
	key = QChar('{');
	break;
    case Qt::Key_Bar:
	key = QChar('|');
	break;
    case Qt::Key_BraceRight:
	key = QChar('}');
	break;
    case Qt::Key_AsciiTilde:
	key = QChar('~');
	break;

    case Qt::Key_nobreakspace:
	key = QChar(0x00a0);
	break;
    case Qt::Key_exclamdown:
	key = QChar(L'¡');
	break;
    case Qt::Key_cent:
	key = QChar(L'¢');
	break;
    case Qt::Key_sterling:
	key = QChar(L'£');
	break;
    case Qt::Key_currency:
	key = QChar(L'¤');
	break;
    case Qt::Key_yen:
	key = QChar(L'¥');
	break;
    case Qt::Key_brokenbar:
	key = QChar(L'¦');
	break;
    case Qt::Key_section:
	key = QChar(L'§');
	break;
    case Qt::Key_diaeresis:
	key = QChar(L'¨');
	break;
    case Qt::Key_copyright:
	key = QChar(L'©');
	break;
    case Qt::Key_ordfeminine:
	key = QChar(L'ª');
	break;
    case Qt::Key_guillemotleft:
	// left angle quotation mark
	key = QChar(L'«');
	break;
    case Qt::Key_notsign:
	key = QChar(L'¬');
	break;
    case Qt::Key_hyphen:
	key = QChar(L'–');
	break;
    case Qt::Key_registered:
	key = QChar(L'®');
	break;
    case Qt::Key_macron:
	key = QChar(L'¯');
	break;
    case Qt::Key_degree:
	key = QChar(L'°');
	break;
    case Qt::Key_plusminus:
	key = QChar(L'±');
	break;
    case Qt::Key_twosuperior:
	key = QChar(L'²');
	break;
    case Qt::Key_threesuperior:
	key = QChar(L'³');
	break;
    case Qt::Key_acute:
	key = QChar(L'´');
	break;
    case Qt::Key_mu:
	key = QChar(L'µ');
	break;
    case Qt::Key_paragraph:
	key = QChar(L'¶');
	break;
    case Qt::Key_periodcentered:
	key = QChar(L'·');
	break;
    case Qt::Key_cedilla:
	key = QChar(L'¸');
	break;
    case Qt::Key_onesuperior:
	key = QChar(L'¹');
	break;
    case Qt::Key_masculine:
	key = QChar(L'º');
	break;
    case Qt::Key_guillemotright:
	// right angle quotation mark
	key = QChar(L'»');
	break;
    case Qt::Key_onequarter:
	key = QChar(L'¼');
	break;
    case Qt::Key_onehalf:
	key = QChar(L'½');
	break;
    case Qt::Key_threequarters:
	key = QChar(L'¾');
	break;
    case Qt::Key_questiondown:
	key = QChar(L'¿');
	break;
    case Qt::Key_Agrave:
	key = shift ? QChar(L'À') : QChar(L'à');
	break;
    case Qt::Key_Aacute:
	key = shift ? QChar(L'Á') : QChar(L'á');
	break;
    case Qt::Key_Acircumflex:
	key = shift ? QChar(L'Â') : QChar(L'â');
	break;
    case Qt::Key_Atilde:
	key = shift ? QChar(L'Ã') : QChar(L'ã');
	break;
    case Qt::Key_Adiaeresis:
	key = shift ? QChar(L'Ä') : QChar(L'ä');
	break;
    case Qt::Key_Aring:
	key = shift ? QChar(L'Å') : QChar(L'å');
	break;
    case Qt::Key_AE:
	key = shift ? QChar(L'Æ') : QChar(L'æ');
	break;
    case Qt::Key_Ccedilla:
	key = shift ? QChar(L'Ç') : QChar(L'ç');
	break;
    case Qt::Key_Egrave:
	key = shift ? QChar(L'È') : QChar(L'è');
	break;
    case Qt::Key_Eacute:
	key = shift ? QChar(L'É') : QChar(L'é');
	break;
    case Qt::Key_Ecircumflex:
	key = shift ? QChar(L'Ê') : QChar(L'ê');
	break;
    case Qt::Key_Ediaeresis:
	key = shift ? QChar(L'Ë') : QChar(L'ë');
	break;
    case Qt::Key_Igrave:
	key = shift ? QChar(L'Ì') : QChar(L'ì');
	break;
    case Qt::Key_Iacute:
	key = shift ? QChar(L'Í') : QChar(L'í');
	break;
    case Qt::Key_Icircumflex:
	key = shift ? QChar(L'Î') : QChar(L'î');
	break;
    case Qt::Key_Idiaeresis:
	key = shift ? QChar(L'Ï') : QChar(L'ï');
	break;
    case Qt::Key_ETH:
	key = shift ? QChar(L'Ð') : QChar(L'ð');
	break;
    case Qt::Key_Ntilde:
	key = shift ? QChar(L'Ñ') : QChar(L'ñ');
	break;
    case Qt::Key_Ograve:
	key = shift ? QChar(L'Ò') : QChar(L'ò');
	break;
    case Qt::Key_Oacute:
	key = shift ? QChar(L'Ó') : QChar(L'ó');
	break;
    case Qt::Key_Ocircumflex:
	key = shift ? QChar(L'Ô') : QChar(L'ô');
	break;
    case Qt::Key_Otilde:
	key = shift ? QChar(L'Õ') : QChar(L'õ');
	break;
    case Qt::Key_Odiaeresis:
	key = shift ? QChar(L'Ö') : QChar(L'ö');
	break;
    case Qt::Key_multiply:
	key = QChar(L'×');
	break;
    case Qt::Key_Ooblique:
	key = shift ? QChar(L'Ø') : QChar(L'ø');
	break;
    case Qt::Key_Ugrave:
	key = shift ? QChar(L'Ù') : QChar(L'ù');
	break;
    case Qt::Key_Uacute:
	key = shift ? QChar(L'Ú') : QChar(L'ú');
	break;
    case Qt::Key_Ucircumflex:
	key = shift ? QChar(L'Û') : QChar(L'û');
	break;
    case Qt::Key_Udiaeresis:
	key = shift ? QChar(L'Ü') : QChar(L'ü');
	break;
    case Qt::Key_Yacute:
	key = shift ? QChar(L'Ý') : QChar(L'ý');
	break;
    case Qt::Key_THORN:
	key = shift ? QChar(L'Þ') : QChar(L'þ');
	break;
    case Qt::Key_ssharp:
	key = QChar(L'ß');
	break;
    case Qt::Key_division:
	key = QChar(L'÷');
	break;
    case Qt::Key_ydiaeresis:
	key = QChar(L'ÿ');
	break;

    // International input method support (X keycode - 0xEE00, the
    // definition follows Qt/Embedded 2.3.7) Only interesting if
    // you are writing your own input method

    // International & multi-key character composition
    case Qt::Key_AltGr:
    case Qt::Key_Multi_key:  // Multi-key character compose
    case Qt::Key_Codeinput:
    case Qt::Key_SingleCandidate:
    case Qt::Key_MultipleCandidate:
    case Qt::Key_PreviousCandidate:
	break;

    // Misc Functions
    case Qt::Key_Mode_switch:  // Character set switch
	// Key_script_switch:  // Alias for mode_switch

	// Japanese keyboard support
    case Qt::Key_Kanji:  // Kanji, Kanji convert
    case Qt::Key_Muhenkan:  // Cancel Conversion
	//Key_Henkan_Mode:  // Start/Stop Conversion
    case Qt::Key_Henkan:  // Alias for Henkan_Mode
    case Qt::Key_Romaji:  // to Romaji
    case Qt::Key_Hiragana:  // to Hiragana
    case Qt::Key_Katakana:  // to Katakana
    case Qt::Key_Hiragana_Katakana:  // Hiragana/Katakana toggle
    case Qt::Key_Zenkaku:  // to Zenkaku
    case Qt::Key_Hankaku:  // to Hankaku
    case Qt::Key_Zenkaku_Hankaku:  // Zenkaku/Hankaku toggle
    case Qt::Key_Touroku:  // Add to Dictionary
    case Qt::Key_Massyo:  // Delete from Dictionary
    case Qt::Key_Kana_Lock:  // Kana Lock
    case Qt::Key_Kana_Shift:  // Kana Shift
    case Qt::Key_Eisu_Shift:  // Alphanumeric Shift
    case Qt::Key_Eisu_toggle:  // Alphanumeric toggle
	//Key_Kanji_Bangou:  // Codeinput
	//Key_Zen_Koho:  // Multiple/All Candidate(s)
	//Key_Mae_Koho:  // Previous Candidate
	break;

    // Korean keyboard support
    //
    // In fact, many Korean users need only 2 keys, Key_Hangul and
    // Key_Hangul_Hanja. But rest of the keys are good for future.

    case Qt::Key_Hangul:  // Hangul start/stop(toggle)
    case Qt::Key_Hangul_Start:  // Hangul start
    case Qt::Key_Hangul_End:  // Hangul end, English start
    case Qt::Key_Hangul_Hanja:  // Start Hangul->Hanja Conversion
    case Qt::Key_Hangul_Jamo:  // Hangul Jamo mode
    case Qt::Key_Hangul_Romaja:  // Hangul Romaja mode
	//Key_Hangul_Codeinput:  // Hangul code input mode
    case Qt::Key_Hangul_Jeonja:  // Jeonja mode
    case Qt::Key_Hangul_Banja:  // Banja mode
    case Qt::Key_Hangul_PreHanja:  // Pre Hanja conversion
    case Qt::Key_Hangul_PostHanja:  // Post Hanja conversion
	//Key_Hangul_SingleCandidate:  // Single candidate
	//Key_Hangul_MultipleCandidate:  // Multiple candidate
	//Key_Hangul_PreviousCandidate:  // Previous candidate
    case Qt::Key_Hangul_Special:  // Special symbols
	//Key_Hangul_switch:  // Alias for mode_switch
	break;

    // dead keys (X keycode - 0xED00 to avoid the conflict)
    case Qt::Key_Dead_Grave:
	key = QChar(0x0300);
	break;
    case Qt::Key_Dead_Acute:
	key = QChar(0x0301);
	break;
    case Qt::Key_Dead_Circumflex:
	key = QChar(0x0302);
	break;
    case Qt::Key_Dead_Tilde:
	key = QChar(0x0303);
	break;
    case Qt::Key_Dead_Macron:
	key = QChar(0x0304);
	break;
    case Qt::Key_Dead_Breve:
	key = QChar(0x0306);
	break;
    case Qt::Key_Dead_Abovedot:
	key = QChar(0x0307);
	break;
    case Qt::Key_Dead_Diaeresis:
	key = QChar(0x0308);
	break;
    case Qt::Key_Dead_Abovering:
	key = QChar(0x030a);
	break;
    case Qt::Key_Dead_Doubleacute:
	key = QChar(0x030b);
	break;
    case Qt::Key_Dead_Caron:
	key = QChar(0x030c);
	break;
    case Qt::Key_Dead_Cedilla:
	key = QChar(0x0327);
	break;
    case Qt::Key_Dead_Ogonek:
	key = QChar(0x0328);
	break;

    // TODO: set key = QChar(...)
    case Qt::Key_Dead_Iota:
    case Qt::Key_Dead_Voiced_Sound:
    case Qt::Key_Dead_Semivoiced_Sound:
    case Qt::Key_Dead_Belowdot:
    case Qt::Key_Dead_Hook:
    case Qt::Key_Dead_Horn:
    case Qt::Key_Dead_Stroke:
    case Qt::Key_Dead_Abovecomma:
    case Qt::Key_Dead_Abovereversedcomma:
    case Qt::Key_Dead_Doublegrave:
    case Qt::Key_Dead_Belowring:
    case Qt::Key_Dead_Belowmacron:
    case Qt::Key_Dead_Belowcircumflex:
    case Qt::Key_Dead_Belowtilde:
    case Qt::Key_Dead_Belowbreve:
    case Qt::Key_Dead_Belowdiaeresis:
    case Qt::Key_Dead_Invertedbreve:
    case Qt::Key_Dead_Belowcomma:
    case Qt::Key_Dead_Currency:
    case Qt::Key_Dead_a:
    case Qt::Key_Dead_A:
    case Qt::Key_Dead_e:
    case Qt::Key_Dead_E:
    case Qt::Key_Dead_i:
    case Qt::Key_Dead_I:
    case Qt::Key_Dead_o:
    case Qt::Key_Dead_O:
    case Qt::Key_Dead_u:
    case Qt::Key_Dead_U:
    case Qt::Key_Dead_Small_Schwa:
    case Qt::Key_Dead_Capital_Schwa:
    case Qt::Key_Dead_Greek:
    case Qt::Key_Dead_Lowline:
    case Qt::Key_Dead_Aboveverticalline:
    case Qt::Key_Dead_Belowverticalline:
    case Qt::Key_Dead_Longsolidusoverlay:
	break;

    // multimedia/internet keys - ignored by default - see QKeyEvent c'tor
    case Qt::Key_Back:
    case Qt::Key_Forward:
    case Qt::Key_Stop:
    case Qt::Key_Refresh:
    case Qt::Key_VolumeDown:
    case Qt::Key_VolumeMute:
    case Qt::Key_VolumeUp:
    case Qt::Key_BassBoost:
    case Qt::Key_BassUp:
    case Qt::Key_BassDown:
    case Qt::Key_TrebleUp:
    case Qt::Key_TrebleDown:
    case Qt::Key_MediaPlay:
    case Qt::Key_MediaStop:
    case Qt::Key_MediaPrevious:
    case Qt::Key_MediaNext:
    case Qt::Key_MediaRecord:
    case Qt::Key_MediaPause:
    case Qt::Key_MediaTogglePlayPause:
    case Qt::Key_HomePage:
    case Qt::Key_Favorites:
    case Qt::Key_Search:
    case Qt::Key_Standby:
    case Qt::Key_OpenUrl:
    case Qt::Key_LaunchMail:
    case Qt::Key_LaunchMedia:
    case Qt::Key_Launch0:
    case Qt::Key_Launch1:
    case Qt::Key_Launch2:
    case Qt::Key_Launch3:
    case Qt::Key_Launch4:
    case Qt::Key_Launch5:
    case Qt::Key_Launch6:
    case Qt::Key_Launch7:
    case Qt::Key_Launch8:
    case Qt::Key_Launch9:
    case Qt::Key_LaunchA:
    case Qt::Key_LaunchB:
    case Qt::Key_LaunchC:
    case Qt::Key_LaunchD:
    case Qt::Key_LaunchE:
    case Qt::Key_LaunchF:
    case Qt::Key_MonBrightnessUp:
    case Qt::Key_MonBrightnessDown:
    case Qt::Key_KeyboardLightOnOff:
    case Qt::Key_KeyboardBrightnessUp:
    case Qt::Key_KeyboardBrightnessDown:
    case Qt::Key_PowerOff:
    case Qt::Key_WakeUp:
    case Qt::Key_Eject:
    case Qt::Key_ScreenSaver:
    case Qt::Key_WWW:
    case Qt::Key_Memo:
    case Qt::Key_LightBulb:
    case Qt::Key_Shop:
    case Qt::Key_History:
    case Qt::Key_AddFavorite:
    case Qt::Key_HotLinks:
    case Qt::Key_BrightnessAdjust:
    case Qt::Key_Finance:
    case Qt::Key_Community:
    case Qt::Key_AudioRewind: // Media rewind
    case Qt::Key_BackForward:
    case Qt::Key_ApplicationLeft:
    case Qt::Key_ApplicationRight:
    case Qt::Key_Book:
    case Qt::Key_CD:
    case Qt::Key_Calculator:
    case Qt::Key_ToDoList:
    case Qt::Key_ClearGrab:
    case Qt::Key_Close:
    case Qt::Key_Copy:
    case Qt::Key_Cut:
    case Qt::Key_Display: // Output switch key
    case Qt::Key_DOS:
    case Qt::Key_Documents:
    case Qt::Key_Excel:
    case Qt::Key_Explorer:
    case Qt::Key_Game:
    case Qt::Key_Go:
    case Qt::Key_iTouch:
    case Qt::Key_LogOff:
    case Qt::Key_Market:
    case Qt::Key_Meeting:
    case Qt::Key_MenuKB:
    case Qt::Key_MenuPB:
    case Qt::Key_MySites:
    case Qt::Key_News:
    case Qt::Key_OfficeHome:
    case Qt::Key_Option:
    case Qt::Key_Paste:
    case Qt::Key_Phone:
    case Qt::Key_Calendar:
    case Qt::Key_Reply:
    case Qt::Key_Reload:
    case Qt::Key_RotateWindows:
    case Qt::Key_RotationPB:
    case Qt::Key_RotationKB:
    case Qt::Key_Save:
    case Qt::Key_Send:
    case Qt::Key_Spell:
    case Qt::Key_SplitScreen:
    case Qt::Key_Support:
    case Qt::Key_TaskPane:
    case Qt::Key_Terminal:
    case Qt::Key_Tools:
    case Qt::Key_Travel:
    case Qt::Key_Video:
    case Qt::Key_Word:
    case Qt::Key_Xfer:
    case Qt::Key_ZoomIn:
    case Qt::Key_ZoomOut:
    case Qt::Key_Away:
    case Qt::Key_Messenger:
    case Qt::Key_WebCam:
    case Qt::Key_MailForward:
    case Qt::Key_Pictures:
    case Qt::Key_Music:
    case Qt::Key_Battery:
    case Qt::Key_Bluetooth:
    case Qt::Key_WLAN:
    case Qt::Key_UWB:
    case Qt::Key_AudioForward: // Media fast-forward
    case Qt::Key_AudioRepeat: // Toggle repeat mode
    case Qt::Key_AudioRandomPlay: // Toggle shuffle mode
    case Qt::Key_Subtitle:
    case Qt::Key_AudioCycleTrack:
    case Qt::Key_Time:
    case Qt::Key_Hibernate:
    case Qt::Key_View:
    case Qt::Key_TopMenu:
    case Qt::Key_PowerDown:
    case Qt::Key_Suspend:
    case Qt::Key_ContrastAdjust:
	break;

    case Qt::Key_LaunchG:
    case Qt::Key_LaunchH:

    case Qt::Key_TouchpadToggle:
    case Qt::Key_TouchpadOn:
    case Qt::Key_TouchpadOff:

    case Qt::Key_MicMute:

    case Qt::Key_Red:
    case Qt::Key_Green:
    case Qt::Key_Yellow:
    case Qt::Key_Blue:

    case Qt::Key_ChannelUp:
    case Qt::Key_ChannelDown:

    case Qt::Key_Guide:
    case Qt::Key_Info:
    case Qt::Key_Settings:

    case Qt::Key_MicVolumeUp:
    case Qt::Key_MicVolumeDown:

    case Qt::Key_New:
    case Qt::Key_Open:
    case Qt::Key_Find:
    case Qt::Key_Undo:
    case Qt::Key_Redo:

    case Qt::Key_MediaLast:

    // Keypad navigation keys
    case Qt::Key_Select:
    case Qt::Key_Yes:
    case Qt::Key_No:

    // Newer misc keys
    case Qt::Key_Cancel:
    case Qt::Key_Printer:
    case Qt::Key_Execute:
    case Qt::Key_Sleep:
    case Qt::Key_Play:	    // Not the same as Key_MediaPlay
    case Qt::Key_Zoom:
	//Key_Jisho:	    // IME: Dictionary key
	//Key_Oyayubi_Left:	    // IME: Left Oyayubi key
	//Key_Oyayubi_Right:    // IME: Right Oyayubi key
    case Qt::Key_Exit:

    // Device keys
    case Qt::Key_Context1:
    case Qt::Key_Context2:
    case Qt::Key_Context3:
    case Qt::Key_Context4:
    case Qt::Key_Call:      // set absolute state to in a call (do not toggle state)
    case Qt::Key_Hangup:    // set absolute state to hang up (do not toggle state)
    case Qt::Key_Flip:
    case Qt::Key_ToggleCallHangup: // a toggle key for answering, or hanging up, based on current call state
    case Qt::Key_VoiceDial:
    case Qt::Key_LastNumberRedial:

    case Qt::Key_Camera:
    case Qt::Key_CameraFocus:
	break;
    }

    if (data.isEmpty() && !key.isEmpty()) {
	data = key.toUtf8();
    }
    if (!data.isEmpty()) {
	emit term_response(data);
	if (m_local_echo) {
	    ui->vterm->write(data);
	}
    }
}
