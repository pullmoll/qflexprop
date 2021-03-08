/*
    Copyright (c) 2021 Jürgen Buchmüller <pullmoll@t-online.de>

    Derived from loadelf.c and loadelf.h which is

    Copyright (c) 2011 David Michael Betz

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software
    and associated documentation files (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge, publish, distribute,
    sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or
    substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
    BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#pragma once
#include <QObject>
#include <QIODevice>

typedef struct  {
    quint8  ident[16];
    quint16 type;
    quint16 machine;
    quint32 version;
    quint32 entry;
    quint32 phoff;
    quint32 shoff;
    quint32 flags;
    quint16 ehsize;
    quint16 phentsize;
    quint16 phnum;
    quint16 shentsize;
    quint16 shnum;
    quint16 shstrndx;
} ElfHdr;

typedef struct {
    quint32 name;
    quint32 type;
    quint32 flags;
    quint32 addr;
    quint32 offset;
    quint32 size;
    quint32 link;
    quint32 info;
    quint32 addralign;
    quint32 entsize;
} ElfSectionHdr;

typedef struct {
    quint32 type;
    quint32 offset;
    quint32 vaddr;
    quint32 paddr;
    quint32 filesz;
    quint32 memsz;
    quint32 flags;
    quint32 align;
} ElfProgramHdr;

typedef struct {
    quint32 name;
    quint32 value;
    quint32 size;
    quint8  info;
    quint8  other;
    quint16 shndx;
} ElfSymbol;

typedef struct {
    ElfHdr hdr;
    quint32 string_offs;
    quint32 symbol_offs;
    quint32 symbol_string_offs;
    quint32 symbol_cnt;
    QIODevice *fp;
} ElfContext;

class LoadElf : public QObject
{
    Q_OBJECT

public:
    explicit LoadElf(QObject *parent = nullptr);

    bool is_elf();
    bool open(QIODevice *fp);
    bool program_size(quint32& start, quint32& size, quint32& pCogImagesSize);
    int find_section_table_entry(const QString& name, ElfSectionHdr& section);
    int find_program_segment(const QString& name, ElfProgramHdr& program);
    QByteArray load_program_segment(const ElfProgramHdr& program);
    bool load_section_table_entry(quint16 i, ElfSectionHdr& section);
    int load_program_table_entry(quint16 i, ElfProgramHdr& program);
    bool find_elf_symbol(const QString& find_name, ElfSymbol& symbol);
    bool load_elf_symbol(size_t i, QString& name, ElfSymbol& symbol);
    QStringList elf_file_info();

signals:
    void Message(const QString& text);

private:

    //! base address of cog driver overlays to be loaded into eeprom
    static constexpr ulong COG_DRIVER_IMAGE_BASE = 0xc0000000;

    static constexpr uchar ST_NULL     = 0;
    static constexpr uchar ST_PROGBITS = 1;
    static constexpr uchar ST_SYMTAB   = 2;
    static constexpr uchar ST_STRTAB   = 3;
    static constexpr uchar ST_RELA     = 4;
    static constexpr uchar ST_HASH     = 5;
    static constexpr uchar ST_DYNAMIC  = 6;
    static constexpr uchar ST_NOTE     = 7;
    static constexpr uchar ST_NOBITS   = 8;
    static constexpr uchar ST_REL      = 9;
    static constexpr uchar ST_SHLIB    = 10;
    static constexpr uchar ST_DYNSYM   = 11;

    static constexpr uchar SF_WRITE    = 1;
    static constexpr uchar SF_ALLOC    = 2;
    static constexpr uchar SF_EXECUTE  = 4;

    static constexpr uchar PT_NULL     = 0;
    static constexpr uchar PT_LOAD     = 1;

    static constexpr int ELFNAMEMAX  = 128;

    static uchar info_bind(const uchar i);
    static uchar info_type(const uchar i);

    static constexpr uchar STB_LOCAL   = 0;
    static constexpr uchar STB_GLOBAL  = 1;
    static constexpr uchar STB_WEAK    = 2;

    QIODevice *m_file;
    ElfHdr m_hdr;
    size_t m_string_offs;
    size_t m_symbol_offs;
    size_t m_symbol_string_offs;
    size_t m_symbol_cnt;

    void CloseElfFile(ElfContext* c);
    int find_program_table_entry(ElfSectionHdr& section, ElfProgramHdr& program);
    static bool section_in_program_segment(const ElfSectionHdr& s, const ElfProgramHdr& p);
    static bool program_segments_match(const ElfProgramHdr& p1, const ElfProgramHdr& p2);
    static QStringList section_hdr_info(const ElfSectionHdr& section);
    static QStringList program_hdr_info(const ElfProgramHdr& program);
};
