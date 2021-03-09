#include <QDateTime>
#include "propload.h"
#include "util.h"

#define	WAIT_FOR_BYTES_WRITTEN	0

PropLoad::PropLoad(QIODevice* dev, QObject* parent)
    : QObject(parent)
    , m_dev(dev)
    , m_verbose(false)
    , m_mode(Prop_Hex)
    , m_clock_freq(80000000)
    , m_clock_mode(0)
    , m_user_baud(Serial_Baud230400)
    , m_use_checksum(true)
{
}

/**
 * @brief Return the verbose flag
 * @return bool true if verbose
 */
bool PropLoad::verbose() const
{
    return m_verbose;
}

/**
 * @brief Return the clock frequency for patching
 * @return clock frequency
 */
quint32 PropLoad::clock_freq() const
{
    return m_clock_freq;
}

/**
 * @brief Return the clock mode for patching
 * @return clock mode
 */
quint32 PropLoad::clock_mode() const
{
    return m_clock_mode;
}

/**
 * @brief Return the user baud rate for patching
 * @return baud rate
 */
quint32 PropLoad::user_baud() const
{
    return m_user_baud;
}

/**
 * @brief Return the use checksum flag
 * @return bool true if checksum is to be used
 */
bool PropLoad::use_checksum() const
{
    return m_use_checksum;
}

/**
 * @brief Load a
 * @return bool true if checksum is to be used
 */
bool PropLoad::load_data(const QByteArray& data, bool patch_mode)
{
    switch (m_mode) {
    case Prop_Hex:
	return load_single_data_hex(data, patch_mode);
    case Prop_Txt:
	return load_single_data_txt(data, patch_mode);
    }
    emit Error(tr("Invalid PropMode (%2).")
	       .arg(m_mode));
    return false;
}

bool PropLoad::load_file(const QString& filename, bool patch_mode)
{
    return load_single_file(filename, patch_mode);
}

void PropLoad::set_verbose(bool on)
{
    m_verbose = on;
}

void PropLoad::set_mode(PropLoad::PropLoadMode mode)
{
    m_mode = mode;
}

void PropLoad::set_clock_freq(quint32 clock_freq)
{
    m_clock_freq = clock_freq;
}

void PropLoad::set_clock_mode(quint32 clock_mode)
{
    m_clock_mode = clock_mode;
}

void PropLoad::set_user_baud(quint32 user_baud)
{
    m_user_baud = user_baud;
}

void PropLoad::set_use_checksum(bool use_checksum)
{
    m_use_checksum = use_checksum;
}

/**
 * @brief Compute a checksum of unsigned 32 bit little endian values in @p data
 * @param data const reference to the byte array to checksum
 * @return sum of 32 bit little endian values in @p data
 */
quint32 PropLoad::compute_checksum(const QByteArray& data)
{
    quint32 checksum = 0;
    for (int offs = 0; offs < data.size(); offs += sizeof(quint32))
	checksum += util.get_le32(data, offs);
    return checksum;
}

/**
 * @brief Upload a single file using base64 encoding
 * @param data const reference to the data block to send
 * @param patch_mode if true, patch in the clock frequence, mode, and user baud
 * @return true on success, or false on error
 */
bool PropLoad::load_single_data_txt(const QByteArray& data, bool patch_mode)
{
    static const QByteArray::Base64Options opts = QByteArray::OmitTrailingEquals;
    int totnum = 0;
    quint32 checksum = 0;

    if (m_verbose)
	emit Message(tr("Loading %1 bytes.").arg(data.size()));

    emit Progress(0, data.size());
    QByteArray prop_txt("> Prop_Txt 0 0 0 0");
    if (m_verbose)
	emit Message(tr("Sending Prop_Txt header '%1'.")
		     .arg(QString::fromLatin1(prop_txt)));

    if (m_dev->write(prop_txt) != prop_txt.size()) {
	emit Error(tr("Failed to send prop_txt '%1' %2 bytes")
		   .arg(QString::fromLatin1(prop_txt))
		   .arg(prop_txt.size()));
	return false;
    }

    for (int offs = 0; offs < data.size(); offs += chunksize) {
	QByteArray block = data.mid(offs, chunksize);
	if (block.size() & 3) {
	    // pad block to multiples of 32 bit with zeroes
	    block.append(4 - (block.size() & 3), 0);
	}

	// If patch_mode is enabled, patch the first block
	if (patch_mode) {
	    patch_mode = false;
	    util.put_le32(block, 0x14, m_clock_freq);
	    util.put_le32(block, 0x18, m_clock_mode);
	    util.put_le32(block, 0x1c, m_user_baud);
	}

	// If checksumming is enabled, add the block to the checksum
	if (m_use_checksum)
	    checksum += compute_checksum(block);

	// Send the block as hex bytes
	QByteArray buffer = QByteArray("> ") + block.toBase64(opts);
	if (m_verbose)
	    emit Message(tr("Send %1 bytes block @0x%2 '%3'")
			 .arg(block.size())
			 .arg(offs, 4, 16, QChar('0'))
			 .arg(QString::fromLatin1(buffer)));

	if (m_dev->write(buffer) != buffer.length()) {
	    emit Error(tr("Failed to send data block at offset 0x%1, %2 bytes")
		       .arg(offs, 4, 16, QChar('0'))
		       .arg(block.size()));
	    return false;
	}

	// Wait for the block to be written
#if WAIT_FOR_BYTES_WRITTEN > 0
	// FIXME: do we need this?
	if (!m_dev->waitForBytesWritten(30000)) {
	    emit Error(tr("Failed to transfer %1 bytes block.")
		       .arg(buffer.size()));
	    return false;
	}
#endif
	emit Progress(offs, data.size());
	totnum += block.size();
    }

    if (m_use_checksum) {
	checksum = Prop - checksum;
	QByteArray checksum_data(4, 0);
	util.put_le32(checksum_data, 0, checksum);
	QByteArray buffer = QByteArray(" ") + checksum_data.toBase64(opts) + QByteArray("?");

	if (m_verbose)
	    emit Message(tr("Send checksum '%1'.")
			 .arg(QString::fromLatin1(buffer)));

	if (m_dev->write(buffer) != buffer.length()) {
	    emit Error(tr("Failed to send checksum at 0x%1 (%2) bytes")
		       .arg(checksum, 8, 16, QChar('0'))
		       .arg(checksum_data.size()));
	    return false;
	}

	// Wait until the checksum is written
#if WAIT_FOR_BYTES_WRITTEN > 0
	// FIXME: do we need this?
	if (!m_dev->waitForBytesWritten(30000)) {
	    emit Error(tr("Failed to transfer %1 bytes block.")
		       .arg(buffer.size()));
	    return false;
	}
#endif

	buffer.clear();
	// Now wait for a reply from the Prop
	if (m_dev->waitForReadyRead(1000)) {
	    // and read 1 byte
	    buffer = m_dev->read(1);
	}

	if (buffer.length() != 1 || buffer[0] != '.') {
	    QString message = tr("Failed to transfer %1 bytes of data.")
			      .arg(data.size());
	    message += QChar::LineFeed + tr("Error response was '%1'")
		       .arg(QString::fromLatin1(buffer));
	    emit Error(message);
	    return false;
	}
	if (m_verbose)
	    emit Message(tr("Checksum 0x%1 validated.")
			 .arg(checksum, 8, 16, QChar('0')));

    } else {
	// No checksum mode: write a tilde (~)

	QByteArray buffer("~");
	if (m_dev->write(buffer) != buffer.length()) {
	    emit Error(tr("Failed to send skip '%1' (%2) bytes")
		       .arg(QString::fromLatin1(buffer))
		       .arg(buffer.size()));
	    return false;
	}
    }
    emit Progress(data.size(), data.size());

    emit Message(tr("%1 bytes of data loaded.")
		 .arg(data.size()));

    return true;
}

/**
 * @brief Upload a single file using hexadecimal encoding
 * @param data const reference to the data block to send
 * @param patch_mode if true, patch in the clock frequence, mode, and user baud
 * @return true on success, or false on error
 */
bool PropLoad::load_single_data_hex(const QByteArray& data, bool patch_mode)
{
    int totnum = 0;
    quint32 checksum = 0;

    if (m_verbose)
	emit Message(tr("Loading %1 bytes.")
		     .arg(data.size()));

    emit Progress(0, data.size());
    QByteArray prop_hex("> Prop_Hex 0 0 0 0");
    if (m_verbose)
	emit Message(tr("Sending Prop_Hex header '%1'.").arg(QString::fromLatin1(prop_hex)));

    if (m_dev->write(prop_hex) != prop_hex.size()) {
	emit Error(tr("Failed to send prop_hex '%1' %2 bytes")
		   .arg(QString::fromLatin1(prop_hex))
		   .arg(prop_hex.size()));
	return false;
    }

    for (int offs = 0; offs < data.size(); offs += chunksize) {
	QByteArray block = data.mid(offs, chunksize);
	if (block.size() & 3) {
	    // pad block to multiples of 32 bit with zeroes
	    block.append(4 - (block.size() & 3), 0);
	}

	// If patch_mode is enabled, patch the first block
	if (patch_mode) {
	    patch_mode = false;
	    util.put_le32(block, 0x14, m_clock_freq);
	    util.put_le32(block, 0x18, m_clock_mode);
	    util.put_le32(block, 0x1c, m_user_baud);
	}

	// If checksumming is enabled, add the block to the checksum
	if (m_use_checksum)
	    checksum += compute_checksum(block);

	// Send the block as hex bytes
	QByteArray buffer = QByteArray("> ") + block.toHex(' ');
	if (m_verbose)
	    emit Message(tr("Send %1 bytes block @0x%2 '%3'")
			 .arg(block.size())
			 .arg(offs, 4, 16, QChar('0'))
			 .arg(QString::fromLatin1(buffer)));

	if (m_dev->write(buffer) != buffer.length()) {
	    emit Error(tr("Failed to send data block at offset 0x%1, %2 bytes")
		       .arg(offs, 4, 16, QChar('0'))
		       .arg(block.size()));
	    return false;
	}

	// Wait for the block to be written
	if (!m_dev->waitForBytesWritten(30000)) {
	    emit Error(tr("Failed to transfer %1 bytes block.")
		       .arg(buffer.size()));
	    return false;
	}
	emit Progress(offs, data.size());
	totnum += block.size();
    }

    if (m_use_checksum) {
	checksum = Prop - checksum;
	QByteArray checksum_data(4, 0);
	util.put_le32(checksum_data, 0, checksum);
	QByteArray buffer = QByteArray(" ") + checksum_data.toHex(' ') + QByteArray("?");

	if (m_verbose)
	    emit Message(tr("Send checksum '%1'.")
			 .arg(QString::fromLatin1(buffer)));

	if (m_dev->write(buffer) != buffer.length()) {
	    emit Error(tr("Failed to send checksum at 0x%1 (%2) bytes")
		       .arg(checksum, 8, 16, QChar('0'))
		       .arg(checksum_data.size()));
	    return false;
	}

	// Wait until the checksum is written
	if (!m_dev->waitForBytesWritten(30000)) {
	    emit Error(tr("Failed to transfer %1 bytes block.")
		       .arg(buffer.size()));
	    return false;
	}

	// Now wait for a reply from the Prop
	buffer.clear();
	if (m_dev->waitForReadyRead(1000)) {
	    // and read 1 byte
	    buffer = m_dev->read(1);
	}

	if (buffer.length() != 1 || buffer[0] != '.') {
	    QString message = tr("Failed to transfer %1 bytes of data.")
			      .arg(data.size());
	    message += QChar::LineFeed + tr("Error response was '%1'")
		       .arg(QString::fromLatin1(buffer));
	    emit Error(message);
	    return false;
	}
	if (m_verbose)
	    emit Message(tr("Checksum 0x%1 validated.")
			 .arg(checksum, 8, 16, QChar('0')));

    } else {
	// No checksum mode: write a tilde (~)

	QByteArray buffer("~");
	if (m_dev->write(buffer) != buffer.length()) {
	    emit Error(tr("Failed to send skip '%1' (%2) bytes")
		       .arg(QString::fromLatin1(buffer))
		       .arg(buffer.size()));
	    return false;
	}
    }
    emit Progress(data.size(), data.size());

    if (m_verbose)
	emit Message(tr("%1 bytes of data loaded.")
		     .arg(data.size()));

    return true;
}

/**
 * @brief Upload a single file
 * @param filename const reference to a fully qualified filename to upload
 * @param patch_mode if true, patch in the clock frequence, mode, and user baud
 * @return true on success, or false on error
 */
bool PropLoad::load_single_file(const QString& filename, bool patch_mode)
{
    QFile file(filename);
    if (!file.exists()) {
	emit Error(tr("File '%1' does not exist.")
		   .arg(filename));
	return false;
    }

    if (file.open(QIODevice::ReadOnly)) {
	QByteArray data = file.readAll();
	if (m_verbose)
	    emit Message(tr("Loaded '%1' %2 bytes.")
			 .arg(filename)
			 .arg(data.size()));
	file.close();
	switch (m_mode) {
	case Prop_Hex:
	    return load_single_data_hex(data, patch_mode);
	case Prop_Txt:
	    return load_single_data_txt(data, patch_mode);
	}
	emit Error(tr("Invalid PropMode (%2).")
		   .arg(m_mode));
	return false;
    }

    emit Error(tr("Could not open '%1' for reading.")
	       .arg(filename));
    return false;
}
