/*****************************************************************************
 *
 *  VT - Virtual Terminal glyph rendering
 *  Copyleft (c) 2013-2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#pragma once
#include <QFont>
#include <QBitmap>
#include <QImage>
#include "vtattr.h"

/**
 * @brief The vtGlyph class contains a QImage with the rendered glyph for a Unicode value.
 *
 * <ul>
 * <li>QChar with the Unicode character code in @ref code.</li>
 * <li>QChar with the Unicode mark code in @ref mark.</li>
 * <li>Bounding box QRect in @ref bbx..</li>
 * <li>Rendered QImage in @ref img..</li>
 * <li>Width in cells in @ref width (1 or 2)..</li>
 * </ul>
 */
class vtGlyph
{
public:
    explicit vtGlyph(const vtAttr& _attr = vtAttr(),
		     const QFont& font = QFont(),
		     const int fw = 8,
		     const int fh = 16,
		     QRgb fg = Qt::white);
    vtAttr attr;
    QChar mark;
    QRect bbx;
    QImage img;
    int width;

private:
    void render(const QFont& font, QRgb fg);
};
