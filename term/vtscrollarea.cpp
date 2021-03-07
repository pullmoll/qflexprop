/*****************************************************************************
 *
 *  VT - Virtual Terminal scroll area
 * Copyright © 2013-2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#include <QEvent>
#include <QScrollBar>
#include "vtscrollarea.h"
#include "vt220.h"

vtScrollArea::vtScrollArea(QWidget* parent)
    : QScrollArea(parent)
    , m_widget_size()
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

QSize vtScrollArea::sizeHint() const
{
    const int f = 2 * frameWidth();
    const int h = fontMetrics().height();
    QSize sz(f, f);
    QWidget* wdg = widget();

    if (wdg) {
	if (!m_widget_size.isValid()) {
	    m_widget_size = widgetResizable() ?
				wdg->sizeHint() :
				wdg->size();
	}
	sz += m_widget_size;
    } else {
	sz += QSize(12 * h, 8 * h);
    }

    if (verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOn)
	sz.setWidth(sz.width() + verticalScrollBar()->sizeHint().width());

    if (horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOn)
	sz.setHeight(sz.height() + horizontalScrollBar()->sizeHint().height());

    return sz;
}

bool vtScrollArea::event(QEvent *e)
{
    vt220* vterm = qobject_cast<vt220*>(widget());
    if (vterm) {
	if (QEvent::LayoutRequest == e->type()) {
	    QSize new_size = widgetResizable()
			     ? vterm->sizeHint()
			     : vterm->size();
	    if (m_widget_size != new_size) {
		m_widget_size = new_size;
		updateGeometry();
	    }
	}
    }
    return QScrollArea::event(e);
}

void vtScrollArea::UpdateCursor(const QRect& rect)
{
    QEventLoop loop(this);
    ensureVisible(rect.x(), rect.y());
    loop.processEvents();
}

void vtScrollArea::UpdateSize()
{
    QEventLoop loop(this);
    updateGeometry();
    loop.processEvents();
}
