/*****************************************************************************
 *
 *  VT - Virtual Terminal control characters
 *  Copyleft (c) 2013-2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#pragma once
#include <QtCore>

static constexpr uchar NUL = 0x00;	    //!< Null
static constexpr uchar SOH = 0x01;	    //!< Start Of Header
static constexpr uchar STX = 0x02;	    //!< Start Of Text
static constexpr uchar ETX = 0x03;	    //!< End Of Text
static constexpr uchar EOT = 0x04;	    //!< End Of Transmission
static constexpr uchar ENQ = 0x05;	    //!< Enquiry
static constexpr uchar ACK = 0x06;	    //!< Acknowledge
static constexpr uchar BEL = 0x07;	    //!< Bell
static constexpr uchar BS  = 0x08;	    //!< Back Space
static constexpr uchar HT  = 0x09;	    //!< Horizontal Tabulation
static constexpr uchar LF  = 0x0a;	    //!< Line Feed
static constexpr uchar VT  = 0x0b;	    //!< Vertical Tabulation
static constexpr uchar FF  = 0x0c;	    //!< Form Feed
static constexpr uchar CR  = 0x0d;	    //!< Carriage Return
static constexpr uchar SO  = 0x0e;	    //!< Shift Out
static constexpr uchar SI  = 0x0f;	    //!< Shift In
static constexpr uchar DLE = 0x10;	    //!< Data Link Escape
static constexpr uchar DC1 = 0x11;	    //!< Device Control 1
static constexpr uchar XON = DC1;	    //!< XON
static constexpr uchar DC2 = 0x12;	    //!< Device Control 2
static constexpr uchar DC3 = 0x13;	    //!< Device Control 3
static constexpr uchar XOFF= DC3;	    //!< XOFF
static constexpr uchar DC4 = 0x14;	    //!< Device Control 4
static constexpr uchar NAK = 0x15;	    //!< Negative Acknowledge
static constexpr uchar SYN = 0x16;	    //!< Synchronous Idle
static constexpr uchar ETB = 0x17;	    //!< End Of Transmission Block
static constexpr uchar CAN = 0x18;	    //!< Cancel
static constexpr uchar EM  = 0x19;	    //!< End Of Medium
static constexpr uchar SUB = 0x1a;	    //!< Substitute
static constexpr uchar ESC = 0x1b;	    //!< Escape
static constexpr uchar FS  = 0x1c;	    //!< File Separator
static constexpr uchar GS  = 0x1d;	    //!< Group Separator
static constexpr uchar RS  = 0x1e;	    //!< Record Separator
static constexpr uchar US  = 0x1f;	    //!< Unit Separator

static constexpr uchar DEL = 0x7f;	    //!< Delete
static constexpr uchar IND = 0x84;	    //!< Index
static constexpr uchar NEL = 0x85;	    //!< Next Line
static constexpr uchar HTS = 0x88;	    //!< Horizontal Tabulation Set
static constexpr uchar RI  = 0x8d;	    //!< Reverse Index
static constexpr uchar SS2 = 0x8e;	    //!< Single Shift 2
static constexpr uchar SS3 = 0x8f;	    //!< Single Shift 3
static constexpr uchar DCS = 0x90;	    //!< Device Control String
static constexpr uchar SPA = 0x96;	    //!< Start of Protected Area
static constexpr uchar EPA = 0x97;	    //!< End of Protected Area
static constexpr uchar SOS = 0x98;	    //!< Start Of String
static constexpr uchar DECID = 0x9a;	    //!< DEC identifty terminal
static constexpr uchar CSI = 0x9b;	    //!< Control Sequence Introducer
static constexpr uchar ST  = 0x9c;	    //!< String Terminator
static constexpr uchar OSC = 0x9d;	    //!< Operating System Command
static constexpr uchar PM  = 0x9e;	    //!< Personal Message
static constexpr uchar APC = 0x9f;	    //!< Application Command
static constexpr uchar SHY = 0xad;	    //!< Soft Hyphen
