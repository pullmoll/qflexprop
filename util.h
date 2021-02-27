/*****************************************************************************
 *
 * Qt5 Propeller 2 utility class
 *
 * Copyright © 2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#pragma once
#include <QString>
#include <QMap>
#include <QFile>
#include <QFileInfo>
#include "proptypes.h"


/**
 * @brief Type of file opened in the editor
 */
typedef enum {
    FT_UNKNOWN,
    FT_BASIC,
    FT_C,
    FT_SPIN,
    FT_SPIN2,
    FT_PASM,
    FT_P2ASM,
    FT_BINARY,
}   FileType;

class Util
{
public:
    Util();

    static const QString to_hex(const QByteArray& data);
    static const QString to_asc(const QByteArray& data);
    static const QString dump(const QString& _func, const QByteArray& data, const int bytes_per_line = 16);

    static const QByteArray fkey_str(const uchar ch1, const QVector<uchar>& ch2 = QVector<uchar>(), int kmod = 0);
    static const QByteArray fkey_str(const uchar ch1, const char* ch2 = nullptr, int kmod = 0);

    FileType filetype(const QString& filename) const;

    static void put_le32(QByteArray& data, int offs, quint32 value);
    static quint32 get_le32(const QByteArray& data, int offs);
};

extern Util util;
