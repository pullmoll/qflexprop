/*****************************************************************************
 *
 *  VT - Virtual Terminal line buffer
 * Copyright © 2013-2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#include "vtline.h"

/**
 * @brief vtLine constructor
 * @param size number of columns
 * @param att default attribute for this line
 */
vtLine::vtLine(int size, const vtAttr& att)
    : m_decdwl(1)
    , m_decdhl(1)
    , m_decdhl_bottom(false)
{
    fill(att, size);
}

/**
 * @brief Return status of DEC double width line
 * @return true if double width line, or false otherwise
 */
int vtLine::decdwl() const
{
    return m_decdwl;
}

/**
 * @brief Return status of DEC double height line
 * @return true if double height line, or false otherwise
 */
int vtLine::decdhl() const
{
    return m_decdhl_bottom ? 1 : m_decdhl;
}

/**
 * @brief Return status of hidden by DEC double height line above
 * @return true if hidden by double height line above, or false otherwise
 */
bool vtLine::bottom() const
{
    return m_decdhl_bottom;
}

/**
 * @brief Set the DEC single height line status
 */
void vtLine::set_decshl()
{
    m_decdhl = 1;
    m_decdhl_bottom = false;
}

/**
 * @brief Set the DEC double height line status
 * @param bottom if true, this line is hidden by the double height line above
 */
void vtLine::set_decdhl(bool bottom)
{
    m_decdhl = 2;
    m_decdhl_bottom = bottom;
}

/**
 * @brief Set the DEC single width line status
 */
void vtLine::set_decswl()
{
    m_decdwl = 1;
}

/**
 * @brief Set the DEC double width line status
 */
void vtLine::set_decdwl()
{
    m_decdwl = 2;
}
