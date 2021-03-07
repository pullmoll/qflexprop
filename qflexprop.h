/*****************************************************************************
 *
 * Qt5 Propeller 2 main window for editor, compiler, terminal
 *
 * Copyright © 2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#pragma once
#include <QMainWindow>
#include <QStatusBar>
#include <QSerialPort>
#include <QTimer>
#include <QHash>
#include <QLabel>
#include <QComboBox>
#include <QTextBrowser>
#include <QFont>
#include <QMutex>
#include <QProcess>
#include "proptypes.h"

QT_BEGIN_NAMESPACE
namespace Ui { class QFlexProp; }
QT_END_NAMESPACE

class PropEdit;

class QFlexProp : public QMainWindow
{
    Q_OBJECT

public:
    QFlexProp(QWidget *parent = nullptr);
    ~QFlexProp();


private slots:
    void log_message(const QString& message);
    void log_status(const QString& message = QString(), bool icon = false);
    void log_error(const QString& message = QString(), bool icon = true);

    void update_baud_rate(qint32 baudRate, QSerialPort::Directions directions = QSerialPort::AllDirections);
    void update_parity_data_stop();
    void update_data_bits(QSerialPort::DataBits dataBits);
    void update_parity(QSerialPort::Parity parity);
    void update_stop_bits(QSerialPort::StopBits stopBits);
    void update_dtr(bool set);
    void update_rts(bool set);
    void error_occured(QSerialPort::SerialPortError error);
    void update_break_enable(bool set);
    void update_flow_control(QSerialPort::FlowControl flowctl);

    void dev_close();
    void dev_write_data(const QByteArray& data);
    void dev_ready_read();

    void update_pinout(bool redo = false);
    void tab_changed(int index);

    void load_settings();
    void save_settings();
    void setup_widget();
    void setup_signals();
    void setup_statusbar();
    void setup_port();
    void open_port();
    void close_port();

    void on_action_New_triggered();
    void on_action_Open_triggered();
    void on_action_Save_triggered();
    void on_action_Save_as_triggered();
    void on_action_Close_triggered();
    void on_action_Quit_triggered();

    void on_action_Select_all_triggered();
    void on_action_Delete_triggered();
    void on_action_Cut_triggered();
    void on_action_Copy_triggered();
    void on_action_Paste_triggered();

    void on_action_Settings_triggered();
    void on_action_Configure_serialport_triggered();
    void on_action_Configure_flexspin_triggered();

    void on_action_Compile_triggered();
    void on_action_Upload_P2_triggered();
    void on_action_Run_triggered();

    void on_action_About_triggered();
    void on_action_About_Qt5_triggered();

    void channelReadyRead(int channel);
    void printError(const QString& message);
    void printMessage(const QString& message);

private:
    Ui::QFlexProp *ui;
    QIODevice* m_dev;				//!< serial port (or tty)
    QMutex m_transfer;				//!< held while in file transfer
    QFont m_fixedfont;
    QStringList m_leds;				//!< list of LED names
    QHash<QString,bool> m_enabled_leds;		//!< hash of LED enabled (visible) status
    QHash<QString,QLabel*> m_labels;		//!< labels for LEDs
    QString m_stty_operation;			//!< serial port most recent operation
    QString m_port_name;			//!< serial port device name
    Serial_BaudRate m_baud_rate;		//!< serial port baud rate
    QSerialPort::DataBits m_data_bits;		//!< serial port data bits
    QSerialPort::Parity m_parity;		//!< serial port parity type
    QSerialPort::StopBits m_stop_bits;		//!< serial port stop bits
    QSerialPort::FlowControl m_flow_control;	//!< serial port flow control
    bool m_local_echo;				//!< Local echo flag

    QString m_flexspin_binary;
    QStringList m_flexspin_include_paths;
    int m_flexspin_optimize;
    bool m_flexspin_listing;
    bool m_flexspin_warnings;
    bool m_flexspin_errors;
    quint32 m_flexspin_hub_address;
    bool m_flexspin_skip_coginit;

    int insert_tab(const QString& filename);
    PropEdit* current_editor() const;
    QTextBrowser* current_browser() const;
    QString load_file(const QString& title);
    QString save_file(const QString& filename, const QString& title);

    bool flexspin(QString* p_p2asm = nullptr, QByteArray* p_binary = nullptr);

    QPixmap led(const QString& type, int state);
    static QString quoted(const QString& src, const QChar quote = QChar('"'));
};
