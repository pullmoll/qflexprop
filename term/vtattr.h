/*****************************************************************************
 *
 *  VT - Virtual Terminal attributes
 *  Copyleft (c) 2013-2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#pragma once
#include <QtCore>

static constexpr uchar C_BLK = 0;	//!< color black
static constexpr uchar C_RED = 1;	//!< color red
static constexpr uchar C_GRN = 2;	//!< color green
static constexpr uchar C_YEL = 3;	//!< color yellow
static constexpr uchar C_BLU = 4;	//!< color blue
static constexpr uchar C_MAG = 5;	//!< color magenta
static constexpr uchar C_CYN = 6;	//!< color cyan
static constexpr uchar C_WHT = 7;	//!< color white

/**
 * @brief The vTermAttribute class stores the attributes of
 * a character cell in the virtual terminal.
 * This includes the following aspects:
 *<ul>
 * <li> @ref code Unicode of the glyph</li>
 * <li> @ref mark Unicode of mark glyph (enclosing, non-spacing, ...)</li>
 * <li> @ref fgcolor foreground color</li>
 * <li> @ref bgcolor background color</li>
 * <li> @ref charset character set to use (GS0 to GS3)</li>
 * <li> @ref gl mapping for left (lower) 128 characters</li>
 * <li> @ref gr mapping for right (upper) 128 characters</li>
 * <li> @ref bold font attribute</li>
 * <li> @ref faint color attribute (use faint instead of full bright color)</li>
 * <li> @ref italic italicized glyph</li>
 * <li> @ref inverse swap fore- and background colors</li>
 * <li> @ref underline underline glyph</li>
 * <li> @ref blink glyph (in-)visisble half of the time</li>
 * <li> @ref conceal invisible glyph, except when terminal is in reveal mode</li>
 * <li> @ref crossed crossed out glyph</li>
 * <li> @ref underldbl double underlined glyph</li>
 *</ul>
 */
class vtAttr
{
public:
    explicit vtAttr(quint32 code = 0x20, quint32 mark = 0x00);

    QChar code() const;
    void set_code(uint code);

    QChar mark() const;
    void set_mark(uint mark);

    uint flag() const;
    void set_flag(uint flag);

    uchar width() const;
    void set_width(uchar width);

    uchar fgcolor() const;
    void set_fgcolor(uchar fg);

    uchar bgcolor() const;
    void set_bgcolor(uchar bg);

    void set_all(const vtAttr& other);
    bool equals(const vtAttr& other) const;

    uchar charset() const;
    void set_charset(uchar charset);

    uchar gl() const;
    void set_gl(uchar set);

    uchar gr() const;
    void set_gr(uchar set);

    bool bold() const;
    void set_bold(bool set);

    bool faint() const;
    void set_faint(bool set);

    bool italic() const;
    void set_italic(bool set);

    bool inverse() const;
    void set_inverse(bool set);

    bool underline() const;
    void set_underline(bool set);

    bool blink() const;
    void set_blink(bool set);

    bool conceal() const;
    void set_conceal(bool set);

    bool crossed() const;
    void set_crossed(bool set);

    bool underldbl() const;
    void set_underldbl(bool set);

    bool operator== (const vtAttr& other);

private:
    QChar m_code;		    //!< Unicode character code
    QChar m_mark;		    //!< Unicode combining code
    union {
	uint w;
	struct {
	    uchar fgcolor:3;	    //!< foreground color
	    uchar bgcolor:3;	    //!< background color
	    uchar charset:2;	    //!< character set (G0, G1, G2, or G3)
	    uchar gl:2;		    //!< GL character mapping
	    uchar gr:2;		    //!< GR character mapping
	    uchar width:2;	    //!< width (1 or 2 cells)
	    bool bold:1;	    //!< bold attribute
	    bool faint:1;	    //!< faint attribute (dim color)
	    bool italic:1;	    //!< italicized attribute
	    bool inverse:1;	    //!< inverse attribute (swapped foreground and background)
	    bool underline:1;	    //!< underline attribute
	    bool blink:1;	    //!< blink (flashing) attribute
	    bool conceal:1;	    //!< conceal attribute (invisible)
	    bool crossed:1;	    //!< crossed out attribute
	    bool underldbl:1;	    //!< doubly underlined attribute
	} a;
    } m_flag;
};


inline uint qHash(const vtAttr& attr, uint seed = 0)
{
    return qHash(attr.code(), seed) ^ qHash(attr.mark()) ^ qHash(attr.flag());
}
