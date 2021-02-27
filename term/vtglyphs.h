/*****************************************************************************
 *
 *  VT - Virtual Terminal glyph hash
 * Copyright © 2013-2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#pragma once
#include "vtglyph.h"
#include "vtglyphidx.h"

/**
 * @brief The vtGlyphs class implements a QHash of @ref vtGlyph by their @ref vtGlypIdx.
 *
 * The class is used to avoid repeated rendering of glyphs using the
 * same Unicode value, font, foreground color
 */
class vtGlyphs
{
public:
    explicit vtGlyphs(const QFont& font = QFont(), int fw = 8, int fh = 12);
    void clear();
    const vtGlyph& glyph(const vtAttr& attr = vtAttr(), QRgb fg = Qt::white) const;
private:
    mutable QHash<vtGlyphIdx,vtGlyph> m_glyphs;
    QFont m_font;
    int m_fw;
    int m_fh;
};
