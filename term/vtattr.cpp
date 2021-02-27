/*****************************************************************************
 *
 *  VT - virtual terminal attributes
 * Copyright © 2013-2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#include "vtattr.h"

vtAttr::vtAttr(quint32 code, quint32 mark)
    : m_code(code)
    , m_mark(mark)
    , m_flag({0})
{
    m_flag.a.bgcolor = 0;
    m_flag.a.fgcolor = 7;
    m_flag.a.charset = 0;
    m_flag.a.gl = 0;
    m_flag.a.gr = 1;
    m_flag.a.width = 1;
    m_flag.a.bold = false;
    m_flag.a.faint = false;
    m_flag.a.italic = false;
    m_flag.a.inverse = false;
    m_flag.a.underline = false;
    m_flag.a.blink = false;
    m_flag.a.conceal = false;
    m_flag.a.crossed = false;
    m_flag.a.underldbl = false;
}

/**
 * @brief Return the Unicode value of the VT attribute
 * @return quint32 with Unicode value
 */
QChar vtAttr::code() const
{
    return m_code;
}

/**
 * @brief Set the Unicode value of the VT attribute
 * @param code Unicode value
 */
void vtAttr::set_code(uint code)
{
    m_code = code;
}

/**
 * @brief Return the Unicode value of the mark (combining, enclosing, non-spacing, ...) VT attribute
 * @return quint32 with Unicode value
 */
QChar vtAttr::mark() const
{
    return m_mark;
}

/**
 * @brief Set the Unicode value of the VT attribute
 * @param mark Unicode value
 */
void vtAttr::set_mark(uint mark)
{
    m_mark = mark;
}

/**
 * @brief Return the various attribute flags packed in one quint32
 * @param mark Unicode value
 */
uint vtAttr::flag() const
{
    return m_flag.w;
}

/**
 * @brief Set the various attribute flags packed in one quint32
 * @param flag combined flags
 */
void vtAttr::set_flag(uint flag)
{
    m_flag.w = flag;
}

/**
 * @brief Return the glyph width in character celss
 * @return width in cells
 */
uchar vtAttr::width() const
{
    return m_flag.a.width;
}

/**
 * @brief Setthe glyph width in character celss
 * @param width in cells
 */
void vtAttr::set_width(uchar width)
{
    m_flag.a.width = width;
}

/**
 * @brief Return the foreground color
 * @return uchar with foreground color index
 */
uchar vtAttr::fgcolor() const
{
    return m_flag.a.fgcolor;
}

/**
 * @brief Set the foreground color index
 * @param fg foreground color index
 */
void vtAttr::set_fgcolor(uchar fg)
{
    m_flag.a.fgcolor = fg;
}

/**
 * @brief Return the background color
 * @return uchar with background color index
 */
uchar vtAttr::bgcolor() const
{
    return m_flag.a.bgcolor;
}

/**
 * @brief Set the background color index
 * @param bg background color index
 */
void vtAttr::set_bgcolor(uchar bg)
{
    m_flag.a.bgcolor = bg;
}

/**
 * @brief Return the selected character set
 * @return uchar with character set index
 */
uchar vtAttr::charset() const
{
    return m_flag.a.charset;
}

/**
 * @brief Set the selected character set index
 * @param charset character set index (G0, G1, G2, or G3)
 */
void vtAttr::set_charset(uchar charset)
{
    m_flag.a.charset = charset;
}

/**
 * @brief Return the left character set (GL)
 * @return uchar with left character set
 */
uchar vtAttr::gl() const
{
    return m_flag.a.gl;
}

/**
 * @brief Set the the left character set (GL)
 * @param set left character set
 */
void vtAttr::set_gl(uchar set)
{
    m_flag.a.gl = set;
}

/**
 * @brief Return the right character set (GR)
 * @return uchar with right character set
 */
uchar vtAttr::gr() const
{
    return m_flag.a.gr;
}

/**
 * @brief Set the the right character set (GR)
 * @param set right character set
 */
void vtAttr::set_gr(uchar set)
{
    m_flag.a.gr = set;
}

/**
 * @brief Set all attributes from another vtAttr
 * @param other const reference to another vtAttr
 */
void vtAttr::set_all(const vtAttr& other)
{
    m_flag.w = other.m_flag.w;
}

/**
 * @brief Return true, if the bold attribute is selected
 * @return true if bold, or false otherwise
 */
bool vtAttr::bold() const
{
    return m_flag.a.bold;
}

/**
 * @brief Set the the bold attribute
 * @param set if true, bold is set, otherwise reset
 */
void vtAttr::set_bold(bool set)
{
    m_flag.a.bold = set;
}

/**
 * @brief Return true, if the faint attribute is selected
 * @return true if faint, or false otherwise
 */
bool vtAttr::faint() const
{
    return m_flag.a.faint;
}

/**
 * @brief Set the the faint attribute
 * @param set if true, faint is set, otherwise reset
 */
void vtAttr::set_faint(bool set)
{
    m_flag.a.faint = set;
}

/**
 * @brief Return true, if the italic attribute is selected
 * @return true if italic, or false otherwise
 */
bool vtAttr::italic() const
{
    return m_flag.a.italic;
}

/**
 * @brief Set the the italic attribute
 * @param set if true, italic is set, otherwise reset
 */
void vtAttr::set_italic(bool set)
{
    m_flag.a.italic = set;
}

/**
 * @brief Return true, if the inverse attribute is selected
 * @return true if inverse, or false otherwise
 */
bool vtAttr::inverse() const
{
    return m_flag.a.inverse;
}

/**
 * @brief Set the the inverse attribute
 * @param set if true, inverse is set, otherwise reset
 */
void vtAttr::set_inverse(bool set)
{
    m_flag.a.inverse = set;
}

/**
 * @brief Return true, if the underline attribute is selected
 * @return true if underline, or false otherwise
 */
bool vtAttr::underline() const
{
    return m_flag.a.underline;
}

/**
 * @brief Set the the underline attribute
 * @param set if true, underline is set, otherwise reset
 */
void vtAttr::set_underline(bool set)
{
    m_flag.a.underline = set;
}

/**
 * @brief Return true, if the blink attribute is selected
 * @return true if blink, or false otherwise
 */
bool vtAttr::blink() const
{
    return m_flag.a.blink;
}

/**
 * @brief Set the the blink attribute
 * @param set if true, blink is set, otherwise reset
 */
void vtAttr::set_blink(bool set)
{
    m_flag.a.blink = set;
}

/**
 * @brief Return true, if the conceal attribute is selected
 * @return true if conceal, or false otherwise
 */
bool vtAttr::conceal() const
{
    return m_flag.a.conceal;
}

/**
 * @brief Set the the conceal attribute
 * @param set if true, conceal is set, otherwise reset
 */
void vtAttr::set_conceal(bool set)
{
    m_flag.a.conceal = set;
}

/**
 * @brief Return true, if the crossed-out attribute is selected
 * @return true if crossed-out, or false otherwise
 */
bool vtAttr::crossed() const
{
    return m_flag.a.crossed;
}

/**
 * @brief Set the the crossed-out attribute
 * @param set if true, crossed-out is set, otherwise reset
 */
void vtAttr::set_crossed(bool set)
{
    m_flag.a.crossed = set;
}

/**
 * @brief Return true, if the double underline attribute is selected
 * @return true if double underline, or false otherwise
 */
bool vtAttr::underldbl() const
{
    return m_flag.a.underldbl;
}

/**
 * @brief Set the the double underline attribute
 * @param set if true, double underline is set, otherwise reset
 */
void vtAttr::set_underldbl(bool set)
{
    m_flag.a.underldbl = set;
}

/**
 * @brief Return true, if this vtAttr is equal to the @p other
 * @param other const reference to another vtAttr
 * @return true if equal, or false otherwise
 */
bool vtAttr::operator==(const vtAttr& other)
{
    return m_code == other.m_code &&
	    m_mark == other.m_mark &&
	    m_flag.w == other.m_flag.w;
}
