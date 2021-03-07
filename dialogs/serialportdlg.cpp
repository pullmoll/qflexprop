/***************************************************************************************
 *
 * Qt5 Propeller 2 serial port configuration dialog
 *
 * Copyright 🄯 2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 ***************************************************************************************/
#include <QLineEdit>
#include <QSerialPortInfo>
#include <QSettings>
#include "idstrings.h"
#include "serialportdlg.h"
#include "ui_serialportdlg.h"

static const char blankString[] = QT_TRANSLATE_NOOP("SerialPortDialog", "N/A");

SerialPortDlg::SerialPortDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SerialPortDialog)
    , m_int_validator(new QIntValidator(0, 4000000, this))
    , m_settings()
{
    ui->setupUi(this);

    ui->cb_baud_rate->setInsertPolicy(QComboBox::NoInsert);

    connect(ui->buttonBox,
	    &QDialogButtonBox::clicked,
	    this, &SerialPortDlg::apply_or_close);
    connect(ui->cb_ports_info,
	    QOverload<int>::of(&QComboBox::currentIndexChanged),
	    this, &SerialPortDlg::show_port_info);
    connect(ui->cb_baud_rate,
	    QOverload<int>::of(&QComboBox::currentIndexChanged),
	    this, &SerialPortDlg::check_custom_baud_rate_policy);
    connect(ui->cb_ports_info,
	    QOverload<int>::of(&QComboBox::currentIndexChanged),
	    this, &SerialPortDlg::check_custom_device_policy);

    fill_ports_parameters();
    fill_ports_info();

    QSettings s;
    s.beginGroup(objectName());
    restoreGeometry(s.value(id_window_geometry).toByteArray());
    s.endGroup();
}

SerialPortDlg::~SerialPortDlg()
{
    if (QDialog::Accepted == result()) {
	save_settings();
    }
    delete ui;
}

SerialPortDlg::Settings SerialPortDlg::settings()
{
    read_settings();
    return m_settings;
}

void SerialPortDlg::set_settings(const SerialPortDlg::Settings& s)
{
    m_settings.name = s.name;
    m_settings.baud_rate = s.baud_rate;
    m_settings.data_bits = s.data_bits;
    m_settings.stop_bits = s.stop_bits;
    m_settings.parity = s.parity;
    m_settings.flow_control = s.flow_control;
    m_settings.local_echo = s.local_echo;
    setup_dialog();
}

QString SerialPortDlg::map_string(const QVariantMap& map, const QString& key)
{
    if (map.contains(key)) {
	QString str = map[key].toString();
	if (!str.isEmpty())
	    return str;
    }
    return tr(blankString);
}

QString SerialPortDlg::map_int(const QVariantMap& map, const QString& key, int width, int base, QChar fillchar)
{
    int val = 0;
    if (map.contains(key)) {
	bool ok;
	val = map[key].toInt(&ok);
	if (!ok)
	    return map[key].toString();
    }
    return QString("%1").arg(val, width, base, fillchar);
}

void SerialPortDlg::show_port_info(int idx)
{
    if (idx == -1)
	return;

    const QVariantMap map = ui->cb_ports_info->itemData(idx).toMap();
    ui->lbl_description->setText(map_string(map, id_description));
    ui->lbl_manufacturer->setText(map_string(map, id_manufacturer));
    ui->lbl_serial_number->setText(map_string(map, id_serial_number));
    ui->lbl_location->setText(map_string(map, id_location));
    ui->lbl_vendor_id->setText(QString("0x%1").arg(map_int(map, id_vendor_id, 4, 16, QChar('0'))));
    ui->lbl_product_id->setText(QString("0x%1").arg(map_int(map, id_product_id, 4, 16, QChar('0'))));
}

void SerialPortDlg::apply_or_close(QAbstractButton *button)
{
    QDialogButtonBox::StandardButton sbutton = ui->buttonBox->standardButton(button);
    switch (sbutton) {
    case QDialogButtonBox::Ok:
	read_settings();
	save_settings();
	accept();
	break;
    case QDialogButtonBox::Cancel:
	reject();
	break;
    default:
	qCritical("%s: must handle button %d", __func__, sbutton);
    }
    close();
}

void SerialPortDlg::check_custom_baud_rate_policy(int idx)
{
    const bool isCustomBaudRate = !ui->cb_baud_rate->itemData(idx).isValid();
    ui->cb_baud_rate->setEditable(isCustomBaudRate);
    if (isCustomBaudRate) {
	ui->cb_baud_rate->clearEditText();
	QLineEdit *edit = ui->cb_baud_rate->lineEdit();
	edit->setValidator(m_int_validator);
    }
}

void SerialPortDlg::check_custom_device_policy(int idx)
{
    const bool isCustomPath = !ui->cb_ports_info->itemData(idx).isValid();
    ui->cb_ports_info->setEditable(isCustomPath);
    if (isCustomPath)
	ui->cb_ports_info->clearEditText();
}

void SerialPortDlg::fill_ports_parameters()
{
    QLocale locale = QLocale::system();
    ui->cb_baud_rate->addItem(locale.toString(Serial_Baud1200), Serial_Baud1200);
    ui->cb_baud_rate->addItem(locale.toString(Serial_Baud2400), Serial_Baud2400);
    ui->cb_baud_rate->addItem(locale.toString(Serial_Baud4800), Serial_Baud4800);
    ui->cb_baud_rate->addItem(locale.toString(Serial_Baud9600), Serial_Baud9600);
    ui->cb_baud_rate->addItem(locale.toString(Serial_Baud19200), Serial_Baud19200);
    ui->cb_baud_rate->addItem(locale.toString(Serial_Baud38400), Serial_Baud38400);
    ui->cb_baud_rate->addItem(locale.toString(Serial_Baud115200), Serial_Baud115200);
    ui->cb_baud_rate->addItem(locale.toString(Serial_Baud230400), Serial_Baud230400);
    ui->cb_baud_rate->addItem(locale.toString(Serial_Baud921600), Serial_Baud921600);
    ui->cb_baud_rate->addItem(locale.toString(Serial_Baud2000000), Serial_Baud2000000);
    ui->cb_baud_rate->addItem(tr("Custom"));

    foreach(const QSerialPort::DataBits key, data_bits_str.keys()) {
	if (QSerialPort::UnknownDataBits != key)
	    ui->cb_data_bits->addItem(data_bits_str.value(key), key);
    }

    foreach(const QSerialPort::Parity key, parity_str.keys()) {
	if (QSerialPort::UnknownParity != key)
	    ui->cb_parity->addItem(parity_str.value(key), key);
    }

    foreach(const QSerialPort::StopBits key, stop_bits_str.keys()) {
	if (QSerialPort::UnknownStopBits != key)
	    ui->cb_stop_bits->addItem(stop_bits_str.value(key), key);
    }

    foreach(const QSerialPort::FlowControl key, flow_control_str.keys()) {
	if (QSerialPort::UnknownFlowControl != key)
	    ui->cb_flow_control->addItem(tr(flow_control_str.value(key)), key);
    }
}

void SerialPortDlg::fill_ports_info()
{
    ui->cb_ports_info->clear();
    const auto infos = QSerialPortInfo::availablePorts();

    for (const QSerialPortInfo &info : infos) {
	QVariantMap map;
	map.insert(id_port_name, info.portName());
	map.insert(id_description, info.description());
	map.insert(id_manufacturer, info.manufacturer());
	map.insert(id_serial_number, info.serialNumber());
	map.insert(id_location, info.systemLocation());
	map.insert(id_vendor_id, info.vendorIdentifier());
	map.insert(id_product_id, info.productIdentifier());
	ui->cb_ports_info->addItem(info.portName(), map);
    }

    ui->cb_ports_info->addItem(tr("Custom"));
}

void SerialPortDlg::setup_dialog(bool load_settings)
{
    QLocale locale = QLocale::system();
    QSettings s;
    Settings settings;
    int idx;

    s.beginGroup(objectName());
    // Restore previous window geometry
    restoreGeometry(s.value(id_window_geometry).toByteArray());
    s.endGroup();

    s.beginGroup(id_grp_preferences);
    if (settings.name.isEmpty()) {
	s.beginGroup(id_default);
	settings.name = s.value(id_name, id_default_com).toString();
	s.endGroup();
    }
    s.endGroup();

    if (load_settings) {
	s.beginGroup(id_grp_serialport);
	s.beginGroup(settings.name);
	settings.baud_rate = static_cast<Serial_BaudRate>(s.value(id_baud_rate, Serial_Baud230400).toInt());
	settings.data_bits = static_cast<QSerialPort::DataBits>(s.value(id_data_bits, QSerialPort::Data8).toInt());
	settings.parity = static_cast<QSerialPort::Parity>(s.value(id_parity, QSerialPort::NoParity).toInt());
	settings.stop_bits = static_cast<QSerialPort::StopBits>(s.value(id_stop_bits, QSerialPort::OneStop).toInt());
	settings.flow_control = static_cast<QSerialPort::FlowControl>(s.value(id_flow_control, QSerialPort::NoFlowControl).toInt());
	settings.local_echo = s.value(id_local_echo, false).toBool();
	s.endGroup();
	s.endGroup();

    }

    idx = ui->cb_ports_info->findText(settings.name);
    if (idx >= 0) {
	ui->cb_ports_info->setCurrentIndex(idx);
    }

    const QString baud_str = locale.toString(settings.baud_rate);
    idx = ui->cb_baud_rate->findData(baud_str);
    if (idx >= 0) {
	ui->cb_baud_rate->setCurrentIndex(idx);
    } else {
	ui->cb_baud_rate->setCurrentIndex(ui->cb_baud_rate->count() - 1);
	ui->cb_baud_rate->setEditText(QString::number(settings.baud_rate));
    }

    idx = ui->cb_data_bits->findData(settings.data_bits);
    if (idx >= 0) {
	ui->cb_data_bits->setCurrentIndex(idx);
    }

    idx = ui->cb_parity->findData(settings.parity);
    if (idx >= 0) {
	ui->cb_parity->setCurrentIndex(idx);
    }

    idx = ui->cb_flow_control->findData(settings.flow_control);
    if (idx >= 0) {
	ui->cb_flow_control->setCurrentIndex(idx);
    }

    ui->cb_local_echo->setChecked(settings.local_echo);

    settings.str.baud_rate = locale.toString(settings.baud_rate);
    settings.str.data_bits = data_bits_str.value(settings.data_bits);
    settings.str.parity = parity_str.value(settings.parity);
    settings.str.stop_bits = stop_bits_str.value(settings.stop_bits);
    settings.str.flow_control = flow_control_str.value(settings.flow_control);

    m_settings = settings;
    read_settings();
}

void SerialPortDlg::save_settings()
{
    QSettings s;
    s.beginGroup(objectName());
    s.setValue(id_window_geometry, saveGeometry());
    s.endGroup();

    s.beginGroup(id_grp_preferences);
    s.beginGroup(id_default);
    s.setValue(id_name, m_settings.name);
    s.endGroup();

    s.beginGroup(m_settings.name);
    s.setValue(id_baud_rate, m_settings.baud_rate);
    s.setValue(id_data_bits, m_settings.data_bits);
    s.setValue(id_parity, m_settings.parity);
    s.setValue(id_stop_bits, m_settings.stop_bits);
    s.setValue(id_flow_control, m_settings.flow_control);
    s.setValue(id_local_echo, m_settings.local_echo);
    s.endGroup();
    s.endGroup();
}

void SerialPortDlg::read_settings()
{
    m_settings.name = ui->cb_ports_info->currentText();

    int idx = ui->cb_baud_rate->currentIndex();
    if (ui->cb_baud_rate->itemData(ui->cb_baud_rate->currentIndex()).isNull()) {
	m_settings.baud_rate = static_cast<Serial_BaudRate>(ui->cb_baud_rate->currentText().toInt());
    } else {
	m_settings.baud_rate = static_cast<Serial_BaudRate>(ui->cb_baud_rate->itemData(idx).toInt());
    }
    m_settings.str.baud_rate = QString::number(m_settings.baud_rate);

    idx = ui->cb_data_bits->currentIndex();
    m_settings.data_bits = static_cast<QSerialPort::DataBits>(ui->cb_data_bits->itemData(idx).toInt());
    m_settings.str.data_bits = data_bits_str.value(m_settings.data_bits);

    idx = ui->cb_parity->currentIndex();
    m_settings.parity = static_cast<QSerialPort::Parity>(ui->cb_parity->itemData(idx).toInt());
    m_settings.str.parity = parity_str.value(m_settings.parity);

    idx = ui->cb_stop_bits->currentIndex();
    m_settings.stop_bits = static_cast<QSerialPort::StopBits>(ui->cb_stop_bits->itemData(idx).toInt());
    m_settings.str.stop_bits = stop_bits_str.value(m_settings.stop_bits);

    idx = ui->cb_flow_control->currentIndex();
    m_settings.flow_control = static_cast<QSerialPort::FlowControl>(ui->cb_flow_control->itemData(idx).toInt());
    m_settings.str.flow_control = flow_control_str.value(m_settings.flow_control);

    m_settings.local_echo = ui->cb_local_echo->isChecked();
}
