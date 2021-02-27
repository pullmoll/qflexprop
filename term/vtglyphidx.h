/*****************************************************************************
 *
 *  VT - Virtual Terminal glyph index
 * Copyright © 2013-2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#pragma once
#include <QColor>
#include "vtattr.h"

/**
 * @brief The vtGlyphIdx class creates an index from a Unicode value @ref m_uc,
 * a foreground color QRgb @ref m_fg, and a set of VT attributes @ref m_attr.
 *
 * The index is used to lookup rendered glyphs in the cache @ref vtGlyps.
 */
class vtGlyphIdx
{
public:
    vtGlyphIdx(const vtAttr& attr = vtAttr(), QRgb fg = Qt::white);
    vtAttr attr() const;
    QRgb fg() const;
private:
    vtAttr m_attr;
    QRgb m_fg;
};

inline bool operator==(const vtGlyphIdx& i1, const vtGlyphIdx& i2)
{
    return i1.attr() == i2.attr() && i1.fg() == i2.fg();
}

inline uint qHash(const vtGlyphIdx &key, uint seed)
{
    return qHash(key.attr(), seed) ^ qHash(key.fg());
}
