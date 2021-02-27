/*****************************************************************************
 *
 *  VT - Virtual Terminal scroll area
 * Copyright © 2013-2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#pragma once
#include <QWidget>
#include <QScrollArea>

/**
 * @brief The qsScrollArea class derives from QScrollArea and handles
 * the contained widget's sizeHint() appropriately.
 */
class vtScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit vtScrollArea(QWidget* parent = nullptr);
    QSize sizeHint() const;


public slots:
    void UpdateCursor(const QRect& rect);
    void UpdateSize();

protected:
    bool event(QEvent* e);

private:
    mutable QSize m_widget_size;
};
