/***************************************************************************************
 *
 * Qt5 Propeller 2 types and enumerations
 *
 * Copyright 🄯 2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 ***************************************************************************************/
#pragma once
#include <QSerialPort>

/**
 * @brief Enumeration of baud rates also beyond the default maximum of 115200
 */
typedef enum {
    Serial_Baud1200 = QSerialPort::Baud1200,
    Serial_Baud2400 = QSerialPort::Baud2400,
    Serial_Baud4800 = QSerialPort::Baud4800,
    Serial_Baud9600 = QSerialPort::Baud9600,
    Serial_Baud19200 = QSerialPort::Baud19200,
    Serial_Baud38400 = QSerialPort::Baud38400,
    Serial_Baud115200 = QSerialPort::Baud115200,
    Serial_Baud230400 = 2*QSerialPort::Baud115200,
    Serial_Baud921600 = 8*QSerialPort::Baud115200,
    Serial_Baud2000000 = 2000000
}   Serial_BaudRate;
