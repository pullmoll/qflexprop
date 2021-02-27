/*****************************************************************************
 *
 *  VT - Virtual Terminal glyph hash
 * Copyright © 2013-2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#include "vtglyphs.h"

vtGlyphs::vtGlyphs(const QFont& font, int fw, int fh)
    : m_glyphs()
    , m_font(font)
    , m_fw(fw)
    , m_fh(fh)
{
}

void vtGlyphs::clear()
{
    m_glyphs.clear();
}

const vtGlyph& vtGlyphs::glyph(const vtAttr& attr, QRgb fg) const
{
    vtGlyphIdx idx(attr, fg);
    if (!m_glyphs.contains(idx)) {
	QFont font(m_font);
	font.setBold(attr.bold());
	font.setItalic(attr.italic());
	vtGlyph glyph(attr, font, m_fw, m_fh, fg);
	m_glyphs.insert(idx, glyph);
    }
    return m_glyphs[idx];
}
