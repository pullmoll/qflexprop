#include "loadelf.h"

#define	IDENT_SIGNIFICANT_BYTES	16
static const char ident[IDENT_SIGNIFICANT_BYTES] = {
    0x7f, 'E', 'L', 'F',                        // magic number
    0x01,                                       // class
    0x01,                                       // data
    0x01,                                       // version
    0x00,                                       // os / abi identification
    0x00,                                       // abi version
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00    // padding
};

static const QLatin1String dot_symtab(".symtab");
static const QLatin1String dot_strtab(".strtab");

LoadElf::LoadElf(QObject *parent)
    : QObject(parent)
    , m_file(nullptr)
    , m_hdr()
    , m_string_offs(0)
    , m_symbol_offs(0)
    , m_symbol_string_offs(0)
    , m_symbol_cnt(0)
{
}

uchar LoadElf::info_bind(const uchar i)
{
    return i >> 4;
}

uchar LoadElf::info_type(const uchar i)
{
    return i & 0x0f;
}

bool LoadElf::section_in_program_segment(const ElfSectionHdr& s, const ElfProgramHdr& p)
{
    return (s.offset >= p.offset &&
	    s.offset <  p.offset + p.filesz &&
	    s.addr   >= p.vaddr  &&
	    s.addr   <  p.vaddr  + p.memsz);
}

bool LoadElf::program_segments_match(const ElfProgramHdr& p1, const ElfProgramHdr& p2)
{
    return (p1.offset == p2.offset &&
	    p1.vaddr  == p2.vaddr);
}

bool LoadElf::is_elf()
{
    if (!m_file)
	return false;
    qint64 done = m_file->read(reinterpret_cast<char*>(&m_hdr), sizeof(m_hdr));
    if (done != sizeof(m_hdr))
	return false;
    return 0 == memcmp(ident, m_hdr.ident, IDENT_SIGNIFICANT_BYTES);
}

bool LoadElf::open(QIODevice *fp)
{
    ElfSectionHdr section;

    m_file = fp;
    if (!is_elf())
	return false;

    // get the string section offset
    if (!load_section_table_entry(m_hdr.shstrndx, section))
	return false;

    m_string_offs = section.offset;

    // get the symbol table section offset
    if (find_section_table_entry(dot_symtab, section)) {
	m_symbol_offs = section.offset;
	m_symbol_cnt = section.size / sizeof(ElfSymbol);
	if (find_section_table_entry(dot_strtab, section)) {
	    m_symbol_string_offs = section.offset;
	} else {
	    m_symbol_offs = 0;
	    m_symbol_cnt = 0;
	}
    }
    return true;
}

bool LoadElf::program_size(quint32& start, quint32& size, quint32& cog_images_size)
{
    start = 0xffffffffUL;
    size = 0;
    cog_images_size = 0;
    quint32 end = 0;
    quint32 cog_images_start = 0xffffffffUL;
    quint32 cog_images_end = 0;
    bool cog_images_found = false;
    ElfProgramHdr program;

    for (quint16 i = 0; i < m_hdr.phnum; ++i) {
	if (!load_program_table_entry(i, program)) {
	    emit Message(QString("Can't read ELF program header %1").arg(i));
	    return false;
	}

	if (program.paddr < COG_DRIVER_IMAGE_BASE) {
	    if (program.paddr < start)
		start = program.paddr;
	    if (program.paddr + program.filesz > end)
		end = program.paddr + program.filesz;
	} else {
	    if (program.paddr < cog_images_start)
		cog_images_start = program.paddr;
	    if (program.paddr + program.filesz > cog_images_end)
		cog_images_end = program.paddr + program.filesz;
	    cog_images_found = true;
	}
    }
    size = end - start;
    if (cog_images_found)
	cog_images_size = cog_images_end - cog_images_start;
    return true;
}

int LoadElf::find_program_segment(const QString& name, ElfProgramHdr& program)
{
    ElfSectionHdr section;
    if (!find_section_table_entry(name, section))
	return -1;
    return find_program_table_entry(section, program);
}

QByteArray LoadElf::load_program_segment(const ElfProgramHdr& program)
{
    qint64 pos = m_file->seek(program.offset);
    if (pos != program.offset)
	return QByteArray();
    QByteArray buf = m_file->read(program.filesz);
    if (static_cast<quint32>(buf.size()) != program.filesz)
	return QByteArray();
    return buf;
}

int LoadElf::find_section_table_entry(const QString& name, ElfSectionHdr& section)
{
    for (quint16 i = 0; i < m_hdr.shnum; ++i) {
	if (!load_section_table_entry(i, section)) {
	    emit Message(tr("Can't read ELF section header %1").arg(i));
	    return 1;
	}
	qint64 pos = m_file->seek(m_string_offs + section.name);
	if (static_cast<size_t>(pos) != m_string_offs + section.name)
	    return false;
	QByteArray ascii_name = m_file->read(ELFNAMEMAX);
	QString this_name = QString::fromLatin1(ascii_name);
	if (name == this_name)
	    return true;
    }
    return false;
}

bool LoadElf::load_section_table_entry(quint16 i, ElfSectionHdr& section)
{
    memset(&section, 0, sizeof(section));
    qint64 pos = m_file->seek(m_hdr.shoff + i * m_hdr.shentsize);
    if (pos != m_hdr.shoff + i * m_hdr.shentsize)
	return false;
    qint64 done = m_file->read(reinterpret_cast<char*>(&section), sizeof(section));
    if (done != sizeof(section))
	return false;
    return true;
}

int LoadElf::find_program_table_entry(ElfSectionHdr& section, ElfProgramHdr& program)
{
    for (quint16 i = 0; i < m_hdr.shnum; ++i) {
	if (!load_program_table_entry(i, program)) {
	    emit Message(tr("Can't read ELF program header %1").arg(i));
	    return -1;
	}
	if (section_in_program_segment(section, program))
	    return i;
    }
    return -1;
}

int LoadElf::load_program_table_entry(quint16 i, ElfProgramHdr& program)
{
    memset(&program, 0, sizeof(program));
    qint64 pos = m_file->seek(m_hdr.phoff + i * m_hdr.phentsize);
    if (pos != m_hdr.phoff + i * m_hdr.phentsize)
	return -1;
    qint64 done = m_file->read(reinterpret_cast<char*>(&program), sizeof(program));
    if (done != sizeof(program))
	return -1;
    return done;
}

bool LoadElf::find_elf_symbol(const QString& find_name, ElfSymbol& symbol)
{
    for (quint32 i = 1; i < m_symbol_cnt; ++i) {
	QString name;
	if (load_elf_symbol(i, name, symbol) && name == find_name)
	    return true;
    }
    return false;
}

bool LoadElf::load_elf_symbol(size_t i, QString& name, ElfSymbol& symbol)
{
    qint64 pos = m_file->seek(m_symbol_offs + i * sizeof(ElfSymbol));
    if (static_cast<size_t>(pos) != m_symbol_offs + i * sizeof(ElfSymbol))
	return false;
    qint64 done = m_file->read(reinterpret_cast<char *>(&symbol), sizeof(symbol));
    if (static_cast<size_t>(done) < sizeof(symbol))
	return false;
    if (symbol.name) {
	pos = m_file->seek(m_symbol_offs + symbol.name);
	if (static_cast<size_t>(pos) != m_symbol_offs + symbol.name)
	    return -1;
	QByteArray ascii_name = m_file->read(ELFNAMEMAX);
	name = QString::fromLatin1(ascii_name);
    }
    return true;
}

QStringList LoadElf::elf_file_info()
{
    QStringList list;
    ElfSectionHdr section;
    ElfProgramHdr program;

    /* show file header */
    list += QLatin1String("ELF Header:");
    QString ident = QLatin1String("  ident:    ");
    for (size_t i = 0; i < sizeof(m_hdr.ident); ++i)
	ident += QString(" %1").arg(m_hdr.ident[i], 2, 16, QChar(0));
    list += ident;
    list += QString("  type:      %1").arg(m_hdr.type, 4, 16, QChar('0'));
    list += QString("  machine:   %1").arg(m_hdr.machine, 4, 16, QChar('0'));
    list += QString("  version:   %1").arg(m_hdr.version, 8, 16, QChar('0'));
    list += QString("  entry:     %1").arg(m_hdr.entry, 8, 16, QChar('0'));
    list += QString("  phoff:     %1").arg(m_hdr.phoff, 8, 16, QChar('0'));
    list += QString("  shoff:     %1").arg(m_hdr.shoff, 8, 16, QChar('0'));
    list += QString("  flags:     %1").arg(m_hdr.flags, 8, 16, QChar('0'));
    list += QString("  ehsize:    %1").arg(m_hdr.entry);
    list += QString("  phentsize: %1").arg(m_hdr.phentsize);
    list += QString("  phnum:     %1").arg(m_hdr.phnum);
    list += QString("  shentsize: %1").arg(m_hdr.shentsize);
    list += QString("  shnum:     %1").arg(m_hdr.shnum);
    list += QString("  shstrndx:  %1").arg(m_hdr.shstrndx);

    /* show the section table */
    for (quint16 i = 0; i < m_hdr.shnum; ++i) {
	QString name;
	if (!load_section_table_entry(i, section)) {
	    list += QString("error: can't read section header %1").arg(i);
	    return list;
	}
	qint64 pos = m_file->seek(m_string_offs + section.name);
	if (static_cast<size_t>(pos) != m_string_offs + section.name)
	    continue;
	QByteArray ascii_name = m_file->read(ELFNAMEMAX);
	name = QString::fromLatin1(ascii_name);
	list += QString("SectionHdr %1:").arg(i);
	list += QString("  name:      %1 %2")
	       .arg(section.name, 8, 16, QChar(0))
	       .arg(name);
	list += section_hdr_info(section);
    }

    /* show the program table */
    for (quint16 i = 0; i < m_hdr.phnum; ++i) {
	if (!load_program_table_entry(i, program)) {
	    list += QString("error: can't read program header %1").arg(i);
	    return list;
	}
	list += QString("ProgramHdr %1:").arg(i);
	list += program_hdr_info(program);
    }

    // show the symbol table
    for (size_t i = 1; i < m_symbol_cnt; ++i) {
	QString name;
	ElfSymbol symbol;
	if (load_elf_symbol(i, name, symbol) && symbol.name > 0 && STB_GLOBAL == info_bind(symbol.info))
	    list += QString("  %1 %2: %3\n")
		   .arg(symbol.name, 8, 16, QChar('0'))
		   .arg(name)
		   .arg(symbol.value, 8, 16, QChar('0'));
    }
    return list;
}

QStringList LoadElf::section_hdr_info(const ElfSectionHdr& section)
{
    QStringList str;
    str += QString("  type:      %1").arg(section.type, 8, 16, QChar('0'));
    str += QString("  flags:     %1").arg(section.flags, 8, 16, QChar('0'));
    str += QString("  addr:      %1").arg(section.addr, 8, 16, QChar('0'));
    str += QString("  offset:    %1").arg(section.offset, 8, 16, QChar('0'));
    str += QString("  size:      %1").arg(section.size, 8, 16, QChar('0'));
    str += QString("  link:      %1").arg(section.link, 8, 16, QChar('0'));
    str += QString("  info:      %1").arg(section.info, 8, 16, QChar('0'));
    str += QString("  addralign: %1").arg(section.addralign, 8, 16, QChar('0'));
    str += QString("  entsize:   %1").arg(section.entsize, 8, 16, QChar('0'));
    return str;
}

QStringList LoadElf::program_hdr_info(const ElfProgramHdr& program)
{
    QStringList str;
    str += QString("  type:      %1").arg(program.type, 8, 16, QChar('0'));
    str += QString("  offset:    %1").arg(program.offset, 8, 16, QChar('0'));
    str += QString("  vaddr:     %1").arg(program.vaddr, 8, 16, QChar('0'));
    str += QString("  paddr:     %1").arg(program.paddr, 8, 16, QChar('0'));
    str += QString("  filesz:    %1").arg(program.filesz, 8, 16, QChar('0'));
    str += QString("  memsz:     %1").arg(program.memsz, 8, 16, QChar('0'));
    str += QString("  flags:     %1").arg(program.flags, 8, 16, QChar('0'));
    str += QString("  align:     %1").arg(program.align, 8, 16, QChar('0'));
    return str;
}
