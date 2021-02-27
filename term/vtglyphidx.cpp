/*****************************************************************************
 *
 *  VT - Virtual Terminal glyph index
 * Copyright © 2013-2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#include "vtglyphidx.h"

/**
 * @brief vtGlyphIdx constructor
 * @param uc Unicode value
 * @param fg foreground color
 * @param attr glyph attributes
 */
vtGlyphIdx::vtGlyphIdx(const vtAttr& attr, QRgb fg)
    : m_attr(attr)
    , m_fg(fg)
{
}

QRgb vtGlyphIdx::fg() const
{
    return m_fg;
}

vtAttr vtGlyphIdx::attr() const
{
    return m_attr;
}
