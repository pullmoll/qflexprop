/*****************************************************************************
 *
 * Qt5 Propeller 2 utility class
 *
 * Copyright © 2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#include "util.h"

static const QMultiMap<FileType,QString> g_filetype_to_suffix = {
    {FT_UNKNOWN, QLatin1String("*")},
    {FT_BASIC, QLatin1String("BAS")},
    {FT_BASIC, QLatin1String("BASIC")},
    {FT_C, QLatin1String("C")},
    {FT_SPIN, QLatin1String("SPIN")},
    {FT_SPIN2, QLatin1String("SPIN2")},
    {FT_PASM, QLatin1String("PASM")},
    {FT_P2ASM, QLatin1String("P2ASM")},
    {FT_BINARY, QLatin1String("BINARY")},
    {FT_BINARY, QLatin1String("BIN")},
};

static QMultiMap<QString,FileType> g_suffix_to_filetype;

Util::Util()
{
    foreach(const QString& suffix, g_filetype_to_suffix.values()) {
	const int idx = g_filetype_to_suffix.values().indexOf(suffix);
	FileType filetype = g_filetype_to_suffix.keys().at(idx);
	g_suffix_to_filetype.insert(suffix, filetype);
    }
}

const QString Util::to_hex(const QByteArray& data)
{
    return data.toHex(' ');
}

const QString Util::to_asc(const QByteArray& data)
{
    QString result;
    result.resize(data.length());
    int idx = 0;
    foreach(char ch, data) {
	result[idx++] = ch < 32 || ch > 126 ? QChar(L'·') : QChar(ch);
    }
    return result;
}

const QString Util::dump(const QString& _func, const QByteArray& data, const int bytes_per_line)
{
    QStringList dump;
    int offs = 0;
    while (offs < data.length()) {
	const QString bytes = to_hex(data.mid(offs, bytes_per_line));
	const QString ascii = to_asc(data.mid(offs, bytes_per_line));
	if (_func.isEmpty()) {
	    dump += QString("%1: %2 - %3")
		    .arg(offs, 4, 16, QChar('0'))
		    .arg(bytes, -3 * bytes_per_line)
		    .arg(ascii);
	} else {
	    dump += QString("%1: %2: %3 - %4")
		    .arg(_func)
		    .arg(offs, 4, 16, QChar('0'))
		    .arg(bytes, -3 * bytes_per_line)
		    .arg(ascii);
	}
	offs += bytes_per_line;
    }
    return dump.join(QChar::LineFeed);
}

const QByteArray Util::fkey_str(const uchar ch1, const QVector<uchar>& ch2, int kmod)
{
    const QString mod = kmod ? QString(";%1").arg(kmod + 1) : QString();
    QByteArray data = QByteArray::fromRawData(reinterpret_cast<const char *>(&ch1), 1) +
		      QByteArray::fromRawData(reinterpret_cast<const char *>(ch2.constData()), ch2.length()) +
		      mod.toLatin1();
    return data;
}

const QByteArray Util::fkey_str(const uchar ch1, const char* ch2, int kmod)
{
    QVector<uchar> str;
    for (const char* src = ch2; *src; src++)
	str += static_cast<uchar>(*src);
    return fkey_str(ch1, str, kmod);
}

FileType Util::filetype(const QString& filename) const
{
    QFileInfo info(filename);
    if (!info.exists())
	return FT_UNKNOWN;

    const QString& suffix = info.suffix().toUpper();
    if (g_suffix_to_filetype.contains(suffix))
	return g_suffix_to_filetype.value(suffix);

    return FT_UNKNOWN;
}

QString Util::filetype_name(FileType type) const
{
    switch (type) {
    case FT_BASIC:
	return QLatin1String("Basic");
    case FT_C:
	return QLatin1String("C");
    case FT_SPIN:
	return QLatin1String("Spin");
    case FT_SPIN2:
	return QLatin1String("Spin (P2)");
    case FT_PASM:
	return QLatin1String("PAsm (P1)");
    case FT_P2ASM:
	return QLatin1String("PAsm (P2)");
    case FT_BINARY:
	return QLatin1String("Binary");
    case FT_UNKNOWN:
    default:
	return QLatin1String("???");
    }
    return QLatin1String("<Invalid>");
}

/**
 * @brief Put a little endian 32 bit value in a @p data buffer starting at @p offs
 *
 * This function will not write beyond end of data.
 *
 * @param data reference to a QByteArray to modify
 * @param offs offset into byte array
 * @param value value to store
 */
void Util::put_le32(QByteArray& data, int offs, quint32 value)
{
    Q_ASSERT(offs>=0);
    uchar* p = reinterpret_cast<uchar *>(data.data() + offs);
    const int size = data.size();
    if (offs+0 < size)
	p[0] = static_cast<uchar>(value >>  0);
    if (offs+1 < size)
	p[1] = static_cast<uchar>(value >>  8);
    if (offs+2 < size)
	p[2] = static_cast<uchar>(value >> 16);
    if (offs+3 < size)
	p[3] = static_cast<uchar>(value >> 24);
}

/**
 * @brief Get a little endian 32 bit value from a @p data buffer starting at @p offs
 *
 * This function will not read beyond end of data and instead return 0x00
 * for the bytes which are off scope.
 *
 * @param data const reference to a QByteArray to read from
 * @param offs offset into byte array
 * @return 32 bit value from offs+0 ... offs+3
 */
quint32 Util::get_le32(const QByteArray& data, int offs)
{
    Q_ASSERT(offs>=0);
    const uchar* p = reinterpret_cast<const uchar *>(data.constData() + offs);
    const int size = data.size();
    const quint32 b0 = offs+0 < size ? p[0] : 0;
    const quint32 b1 = offs+1 < size ? p[1] : 0;
    const quint32 b2 = offs+2 < size ? p[2] : 0;
    const quint32 b3 = offs+3 < size ? p[3] : 0;
    return (b0 <<  0) | (b1 <<  8) | (b2 << 16) | (b3 << 24);
}

Util util;
