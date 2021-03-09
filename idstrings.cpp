/*****************************************************************************
 *
 * Qt5 Propeller 2 string identifiers and lookup QMaps
 *
 * Copyright © 2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#include <QMap>
#include <QSerialPort>
#include "idstrings.h"


const QLatin1String id_sourcedir("sourcedir");
const QLatin1String id_filename("filename");
const QLatin1String id_history("history");

const QLatin1String id_grp_application("application");
const QLatin1String id_window_geometry("window_geomtry");
const QLatin1String id_fixedfont_family("fixedfont_family");
const QLatin1String id_fixedfont_weight("fixedfont_weight");
const QLatin1String id_fixedfont_size("fixedfont_size");

const QLatin1String id_grp_preferences("preferences");
const QLatin1String id_grp_serialport("serialport");
#if defined(Q_OS_LINUX)
const QLatin1String id_default_com("ttyUSB0");
#elif defined(Q_OS_WIN)
const QLatin1String id_default_com("COM3");
#elif defined(Q_OS_MACOS)
const QLatin1String id_default_com("cu.usb0");
#endif

const QLatin1String id_default("default");
const QLatin1String id_name("name");
const QLatin1String id_baud_rate("baud_rate");
const QLatin1String id_data_bits("data_bits");
const QLatin1String id_parity("parity");
const QLatin1String id_stop_bits("stop_bits");
const QLatin1String id_flow_control("flow_control");
const QLatin1String id_local_echo("local_echo");
const QLatin1String id_parity_data_stop("data_parity_stop");

const QLatin1String id_port_name("port_name");
const QLatin1String id_description("description");
const QLatin1String id_manufacturer("manufacturer");
const QLatin1String id_serial_number("serial_number");
const QLatin1String id_location("location");
const QLatin1String id_product_id("product_id");
const QLatin1String id_vendor_id("vendor_id");

const QLatin1String id_grp_enabled("enabled");
const QLatin1String id_status("status");

const QLatin1String id_terminal("terminal");
const QLatin1String id_zoom("zoom");
const QLatin1String id_font_family("font_family");

const QLatin1String id_grp_serterm("serterm");

const QLatin1String id_dcd("dcd");
const QLatin1String id_dsr("dsr");
const QLatin1String id_dtr("dtr");
const QLatin1String id_cts("cts");
const QLatin1String id_rts("rts");
const QLatin1String id_rxd("rxd");
const QLatin1String id_txd("txd");
const QLatin1String id_ri("ri");
const QLatin1String id_brk("brk");
const QLatin1String id_fe("fe");
const QLatin1String id_pe("pe");
const QLatin1String id_pwr("pwr");

const QLatin1String id_progress("progress");
const QLatin1String id_download_path("downloads");

const QLatin1String id_tab("tab_");
const QLatin1String id_splitter("spl");
const QLatin1String id_scrollarea("sa");
const QLatin1String id_propedit("pe");
const QLatin1String id_textbrowser("tb");
const QLatin1String id_frame("frm");
const QLatin1String id_label("lbl");
const QLatin1String id_linenumber("lnum");

const QLatin1String str_unknown("?");

#if defined(Q_OS_LINUX)
const QLatin1String p2tools_path("/usr/libexec/p2tools");
#elif defined(Q_OS_WIN)
const QLatin1String p2tools_path("C:/Program Files (x86)/p2tools");
#elif defined(Q_OS_MACOS)
// FIXME: what is the default install path for the p2tools on MacOS?
const QLatin1String p2tools_path("");
#endif
const QLatin1String id_grp_flexspin("flexspin");
const QLatin1String id_compile_verbose_upload("quiet_mode");
const QLatin1String id_compile_switch_to_term("switch_to_term)");
const QLatin1String id_flexspin_executable("executable");
const QLatin1String id_flexspin_quiet("quiet");
const QLatin1String id_flexspin_include_paths("include_paths");
const QLatin1String id_flexspin_optimize("optimize");
const QLatin1String id_flexspin_listing("listing");
const QLatin1String id_flexspin_warnings("warnings");
const QLatin1String id_flexspin_errors("errors");
const QLatin1String id_flexspin_hub_address("hub_adress");
const QLatin1String id_flexspin_skip_coginit("skip_coginit");

const char id_process_tb[] = "tb";		//!< for property QTextBrowser* of QObjects
const char id_tab_lst[] = "lst";		//!< for property lst listing of tab widgets
const char id_tab_p2asm[] = "p2asm";		//!< for property p2asm (intermediate) of tab widgets
const char id_tab_binary[] = "binary";		//!< for property binary output of tab widgets

const char prop_sha256[] = "sha256";
const char prop_filename[] = "filename";
const char prop_filetype[] = "filetype";

const QMap<int,QString> direction_str {
    {QSerialPort::Direction(0), QStringLiteral("?")},
    {QSerialPort::Input, QStringLiteral("←")},
    {QSerialPort::Output, QStringLiteral("→")},
    {QSerialPort::AllDirections, QStringLiteral("↔")}
};

const QMap<QSerialPort::DataBits,QString> data_bits_str {
    {QSerialPort::Data5, QLatin1String("5")},
    {QSerialPort::Data6, QLatin1String("6")},
    {QSerialPort::Data7, QLatin1String("7")},
    {QSerialPort::Data8, QLatin1String("8")},
    {QSerialPort::UnknownDataBits, QLatin1String("?")}
};

const QMap<QSerialPort::StopBits,QString> stop_bits_str {
    {QSerialPort::OneStop, QLatin1String("1")},
    {QSerialPort::TwoStop, QLatin1String("2")},
#if defined(Q_OS_WINDOWS)
    {QSerialPort::OneAndHalfStop, QLatin1String("1,5")},
#endif
    {QSerialPort::UnknownStopBits, QLatin1String("?")}
};

const QMap<QSerialPort::Parity,char> parity_char {
    {QSerialPort::NoParity, 'N'},
    {QSerialPort::EvenParity, 'E'},
    {QSerialPort::OddParity, 'O'},
    {QSerialPort::SpaceParity, 'S'},
    {QSerialPort::MarkParity, 'M'},
    {QSerialPort::UnknownParity, '?'}
};

const QMap<QSerialPort::Parity,const char*> parity_str {
    {QSerialPort::NoParity, QT_TRANSLATE_NOOP("SerTerm", "None")},
    {QSerialPort::EvenParity, QT_TRANSLATE_NOOP("SerTerm", "Even")},
    {QSerialPort::OddParity, QT_TRANSLATE_NOOP("SerTerm", "Odd")},
    {QSerialPort::SpaceParity, QT_TRANSLATE_NOOP("SerTerm", "Space")},
    {QSerialPort::MarkParity, QT_TRANSLATE_NOOP("SerTerm", "Mark")},
    {QSerialPort::UnknownParity, QT_TRANSLATE_NOOP("SerTerm", "Unknown")}
};

const QMap<QSerialPort::FlowControl,const char*> flow_control_str {
    {QSerialPort::NoFlowControl, QT_TRANSLATE_NOOP("SerTerm", "None")},
    {QSerialPort::HardwareControl, QT_TRANSLATE_NOOP("SerTerm", "RTS/CTS")},
    {QSerialPort::SoftwareControl, QT_TRANSLATE_NOOP("SerTerm", "XON/XOFF")},
    {QSerialPort::UnknownFlowControl, QT_TRANSLATE_NOOP("SerTerm", "Unknown")}
};

const QMap<QSerialPort::FlowControl,const char *> flow_ctrl_str {
    {QSerialPort::NoFlowControl, QT_TRANSLATE_NOOP("SerTerm", "NO")},
    {QSerialPort::HardwareControl, QT_TRANSLATE_NOOP("SerTerm", "HW")},
    {QSerialPort::SoftwareControl, QT_TRANSLATE_NOOP("SerTerm", "SW")},
    {QSerialPort::UnknownFlowControl, QT_TRANSLATE_NOOP("SerTerm", "??")}
};

const QMap<QSerialPort::FlowControl,const char *> flow_ctrl_tooltip {
    {QSerialPort::NoFlowControl, QT_TRANSLATE_NOOP("SerTerm", "No flow control.")},
    {QSerialPort::HardwareControl, QT_TRANSLATE_NOOP("SerTerm", "Hardware flow control (RTS/CTS).")},
    {QSerialPort::SoftwareControl, QT_TRANSLATE_NOOP("SerTerm", "Software flow control (XON/XOFF).")},
    {QSerialPort::UnknownFlowControl, QT_TRANSLATE_NOOP("SerTerm", "Invalid flow control setting.")}
};

const QMap<QString,QString> pinout_leds = {
    {id_dcd, QT_TRANSLATE_NOOP("SerTerm", "Status of the Data Carrier Detect line.")},
    {id_dsr, QT_TRANSLATE_NOOP("SerTerm", "Status of the Data Set Ready line.")},
    {id_dtr, QT_TRANSLATE_NOOP("SerTerm", "Status of the Data Terminal Ready line.")},
    {id_cts, QT_TRANSLATE_NOOP("SerTerm", "Status of the Clear To Send line.")},
    {id_rts, QT_TRANSLATE_NOOP("SerTerm", "Status of the Request To Send line.")},
    {id_rxd, QT_TRANSLATE_NOOP("SerTerm", "Status of the Receive Data buffer.")},
    {id_txd, QT_TRANSLATE_NOOP("SerTerm", "Status of the Transmit Data buffer.")},
    {id_brk, QT_TRANSLATE_NOOP("SerTerm", "Status of the Break signal.")},
    {id_ri,  QT_TRANSLATE_NOOP("SerTerm", "Status of the Ring Indicator signal.")},
    {id_fe,  QT_TRANSLATE_NOOP("SerTerm", "Status of the Framing Error detector.")},
    {id_pe,  QT_TRANSLATE_NOOP("SerTerm", "Status of the Parity Error detector.")},
    {id_pwr, QT_TRANSLATE_NOOP("SerTerm", "Power status.")}
};
