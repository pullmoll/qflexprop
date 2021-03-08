/*****************************************************************************
 *
 * Qt5 Propeller 2 string identifiers and lookup QMaps
 *
 * Copyright © 2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#pragma once
#include <QLatin1String>
#include <QSerialPort>

#define	DEBUG_DATA	0

#if defined(DEBUG_DATA) && (DEBUG_DATA != 0)
#define	DBG_DATA(X,...)	qDebug(X, __VA_ARGS__)
#else
#define	DBG_DATA(X,...) /* X */
#endif

extern const QLatin1String id_sourcedir;
extern const QLatin1String id_filename;
extern const QLatin1String id_history;

extern const QLatin1String id_grp_application;
extern const QLatin1String id_window_geometry;
extern const QLatin1String id_fixedfont_family;
extern const QLatin1String id_fixedfont_weight;
extern const QLatin1String id_fixedfont_size;

extern const QLatin1String id_grp_preferences;
extern const QLatin1String id_default_com;
extern const QLatin1String id_default;
extern const QLatin1String id_name;
extern const QLatin1String id_baud_rate;
extern const QLatin1String id_data_bits;
extern const QLatin1String id_parity;
extern const QLatin1String id_stop_bits;
extern const QLatin1String id_flow_control;
extern const QLatin1String id_local_echo;

extern const QLatin1String id_port_name;
extern const QLatin1String id_description;
extern const QLatin1String id_manufacturer;
extern const QLatin1String id_serial_number;
extern const QLatin1String id_location;
extern const QLatin1String id_product_id;
extern const QLatin1String id_vendor_id;

extern const QLatin1String id_grp_serialport;
extern const QLatin1String id_parity_data_stop;
extern const QLatin1String id_status;

extern const QLatin1String id_terminal;
extern const QLatin1String id_zoom;
extern const QLatin1String id_font_family;

extern const QLatin1String id_grp_serterm;

extern const QLatin1String id_dcd;
extern const QLatin1String id_dsr;
extern const QLatin1String id_dtr;
extern const QLatin1String id_cts;
extern const QLatin1String id_rts;
extern const QLatin1String id_rxd;
extern const QLatin1String id_txd;
extern const QLatin1String id_ri;
extern const QLatin1String id_brk;
extern const QLatin1String id_fe;
extern const QLatin1String id_pe;
extern const QLatin1String id_pwr;

extern const QLatin1String id_progress;
extern const QLatin1String id_download_path;

extern const QLatin1String id_tab;
extern const QLatin1String id_splitter;
extern const QLatin1String id_scrollarea;
extern const QLatin1String id_propedit;
extern const QLatin1String id_textbrowser;
extern const QLatin1String id_frame;
extern const QLatin1String id_label;
extern const QLatin1String id_linenumber;

extern const QLatin1String str_unknown;

extern const QMap<int,QString> direction_str;
extern const QMap<QSerialPort::DataBits,QString> data_bits_str;
extern const QMap<QSerialPort::StopBits,QString> stop_bits_str;
extern const QMap<QSerialPort::Parity,char> parity_char;
extern const QMap<QSerialPort::Parity,const char *> parity_str;
extern const QMap<QSerialPort::FlowControl,const char*> flow_control_str;
extern const QMap<QSerialPort::FlowControl,const char *> flow_ctrl_str;
extern const QMap<QSerialPort::FlowControl,const char *> flow_ctrl_tooltip;
extern const QMap<QString,QString> pinout_leds;

extern const QLatin1String p2tools_path;
extern const QLatin1String id_grp_flexspin;
extern const QLatin1String id_compile_verbose_upload;
extern const QLatin1String id_compile_switch_to_term;
extern const QLatin1String id_flexspin_executable;
extern const QLatin1String id_flexspin_include_paths;
extern const QLatin1String id_flexspin_quiet;
extern const QLatin1String id_flexspin_optimize;
extern const QLatin1String id_flexspin_listing;
extern const QLatin1String id_flexspin_warnings;
extern const QLatin1String id_flexspin_errors;
extern const QLatin1String id_flexspin_hub_address;
extern const QLatin1String id_flexspin_skip_coginit;

extern const char id_process_tb[];
extern const char id_tab_lst[];
extern const char id_tab_p2asm[];
extern const char id_tab_binary[];
