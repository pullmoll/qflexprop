/***************************************************************************************
 *
 * Qt5 Propeller 2 hex and base64 loader
 *
 * Copyright 🄯 2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 ***************************************************************************************/
#pragma once
#include <QObject>
#include <QByteArray>
#include <QIODevice>

class PropLoad : public QObject
{
    Q_OBJECT
public:
    typedef enum {
	Prop_Hex,
	Prop_Txt
    } PropLoadMode;

    PropLoad(QIODevice* dev, QObject* parent = nullptr);

    bool verbose() const;
    PropLoadMode mode() const;
    quint32 clock_freq() const;
    quint32 clock_mode() const;
    quint32 user_baud() const;
    bool use_checksum() const;

    bool load_file(const QByteArray& data, bool patch_mode = false);
    bool load_file(const QString& filename, bool patch_mode = false);

public slots:
    void set_verbose(bool on = true);
    void set_mode(PropLoadMode mode);
    void set_clock_freq(quint32 clock_freq);
    void set_clock_mode(quint32 clock_mode);
    void set_user_baud(quint32 user_baud);
    void set_use_checksum(bool use_checksum = true);

signals:
    void Error(const QString& text);
    void Message(const QString& text);
    void Progress(qint64 value, qint64 total);

private:
    //! The magic constant for checksums to be subtracted from:
    //! The "Prop" byte sequence read as little endian,
    //! i.e. 'P' is least significant byte
    static constexpr quint32 Prop = ('P' << 0) | ('r' << 8) | ('o' << 16) | ('p' << 24);
    //! The number of bytes per chunk to upload
    static constexpr int chunksize = 128;

    QIODevice* m_dev;	    //!< Serial i/o device to talk to
    bool m_verbose;	    //!< if true, be verbose during transfer
    PropLoadMode m_mode;    //!< which load mode to use: Hex or Txt (base64)
    quint32 m_clock_freq;   //!< clock frequency to patch in
    quint32 m_clock_mode;   //!< clock mode to patch in
    quint32 m_user_baud;    //!< user baud rate to patch in

    bool m_use_checksum;    //!< if true, calculate and verify the checksum

    quint32 compute_checksum(const QByteArray& data);
    bool load_single_file_base64(const QByteArray& data, bool patch_mode = false);
    bool load_single_file_hex(const QByteArray& data, bool patch_mode = false);
    bool load_single_file(const QString& filename, bool patch_mode = false);
};
