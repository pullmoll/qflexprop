/*****************************************************************************
 *
 *  VT - Virtual Terminal line buffer
 * Copyright © 2013-2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#pragma once
#include "vtattr.h"

class vtLine : public QVector<vtAttr>
{
public:
    vtLine(int size = -1, const vtAttr& att = vtAttr());
    int decdwl() const;
    int decdhl() const;
    bool bottom() const;

    void set_decswl();
    void set_decshl();
    void set_decdwl();
    void set_decdhl(bool bottom = false);

private:
    int m_decdwl;
    int m_decdhl;
    bool m_decdhl_bottom;
};

typedef QList<vtLine> vtPage;	//!< a QList of lines makes a page (screen)
