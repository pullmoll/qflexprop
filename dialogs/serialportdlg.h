/***************************************************************************************
 *
 * Qt5 Propeller 2 serial port configuration dialog
 *
 * Copyright 🄯 2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 ***************************************************************************************/
#pragma once
#include <QDialog>
#include <QDialogButtonBox>
#include <QSerialPort>
#include "proptypes.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SerialPortDialog; }
class QIntValidator;
QT_END_NAMESPACE

class SerialPortDlg : public QDialog
{
    Q_OBJECT
public:
    struct Settings {
	QString name;
	Serial_BaudRate baud_rate;
	QSerialPort::DataBits data_bits;
	QSerialPort::Parity parity;
	QSerialPort::StopBits stop_bits;
	QSerialPort::FlowControl flow_control;
	bool local_echo;
	struct {
	    QString baud_rate;
	    QString data_bits;
	    QString parity;
	    QString stop_bits;
	    QString flow_control;
	} str;
    };

    explicit SerialPortDlg(QWidget *parent = nullptr);
    ~SerialPortDlg();

    Settings settings();
    void set_settings(const Settings& s);

private slots:
    void show_port_info(int idx);
    void apply_or_close(QAbstractButton* button);
    void check_custom_baud_rate_policy(int idx);
    void check_custom_device_policy(int idx);

private:
    QString map_string(const QVariantMap& map, const QString& key);
    QString map_int(const QVariantMap& map, const QString& key, int width = 0, int base = 10, QChar fillchar = QChar(' '));
    void fill_ports_parameters();
    void fill_ports_info();
    void setup_dialog(bool load_settings = false);
    void save_settings();
    void read_settings();

private:
    Ui::SerialPortDialog *ui = nullptr;
    QIntValidator *m_int_validator = nullptr;
    Settings m_settings;
};
