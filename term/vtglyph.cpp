/*****************************************************************************
 *
 *  VT - Virtual Terminal glyph rendering
 * Copyright © 2013-2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#include <QPainter>
#include "vtglyph.h"

vtGlyph::vtGlyph(const vtAttr& _attr, const QFont& font, const int fw, const int fh, QRgb fg)
    : attr(_attr)
    , bbx()
    , img()
    , width()
{
    switch (attr.mark().category()) {
    case QChar::Mark_SpacingCombining:
    case QChar::Mark_Enclosing:
	width = 2;
	break;
    default:
	width = 1;
    }
    bbx = QRect(0, 0, width*fw, fh);
    render(font, fg);
}

/**
 * @brief Render the glyph using the specified @p font and foreground color @p fg
 * @param font font to use
 * @param fg QRgb of the foreground color
 */
void vtGlyph::render(const QFont& font, QRgb fg)
{
    QImage buff(bbx.size(), QImage::Format_ARGB32);
    buff.fill(Qt::transparent);
    QPainter painter(&buff);
    painter.setBackgroundMode(Qt::TransparentMode);
    painter.setPen(QColor(fg));
    painter.setFont(font);
    const int pw = painter.pen().width();
    painter.drawText(bbx.adjusted(0,0,pw,pw), Qt::AlignLeft | Qt::AlignTop, attr.code());
    if (!attr.mark().isNull()) {
	painter.drawText(bbx.adjusted(0,0,pw,pw), Qt::AlignLeft | Qt::AlignTop, attr.mark());
    }
    painter.end();
    img = buff.convertToFormat(QImage::Format_ARGB32_Premultiplied);
}
