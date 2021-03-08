/*****************************************************************************
 *
 *  VT - Virtual Terminal emulation (VT220 and variants, i.e. ANSI)
 *  Copyleft (c) 2013-2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 *****************************************************************************/
#pragma once
#include <QObject>
#include <QWidget>
#include <QBitArray>
#include <QBitmap>
#include <QPainter>
#include <QEvent>
#include <QPaintEvent>
#include <QTimerEvent>
#include <QKeyEvent>

#include "vtchar.h"
#include "vtline.h"
#include "vtglyph.h"
#include "vtglyphs.h"

typedef QHash<uchar,uint> cmapHash;

class vt220 : public QWidget
{
    Q_OBJECT
public:
    static constexpr int VT_FAILURE = -1;
    static constexpr int VT_SUCCESS = 0;

    enum Terminal {
	VT100,	    //!< VT100
	VT101,	    //!< VT101
	VT102,	    //!< VT102
	VT125,	    //!< VT125
	VT200,	    //!< VT220/VT240
	VT300,	    //!< VT320/VT340
	VT400,	    //!< VT420
	VT500	    //!< VT510/VT520/VT525
    };

    /** @brief virtual terminal cursor state */
    class Cursor
    {
    public:
	explicit Cursor()
	    : x(0), y(0), newx(0), phase(0), on(0)
	{}
	qint32 x;
	qint32 y;
	qint32 newx;
	qint32 phase;
	bool on;
    };

    explicit vt220(QWidget* parent = nullptr);

    QSize sizeHint() const override;
    QString font_family() const;
    QSize term_geometry() const;
    int zoom() const;
    int vprintf(const char *fmt, va_list ap);
    int printf(const char *fmt, ...);

signals:
    void term_response(QByteArray response);
    void UpdateCursor(const QRect& rect);
    void UpdateSize();

public slots:
    void clear();
    void set_font(int width, int height, int descend);
    void term_reset(Terminal term = VT200, int width = 80, int height = 25);
    void term_set_size(int width = -1, int height = -1);
    void term_set_columns(int width = -1);
    void term_set_rows(int height = -1);
    void term_toggle_80_132();
    void putch(uchar ch);
    int write(const QByteArray& data);
    int write(const char *data, size_t len);
    void display_text(const QString& filename);
    void display_maps();
    void set_font_family(const QString& family);
    void set_zoom(int percent);
    void cursor_slot();

protected:
    bool event(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void timerEvent(QTimerEvent* event) override;

private:
    static constexpr int font_w = 9;
    static constexpr int font_h = 16;
    static constexpr int font_d = 4;
    // 0d00ff81
    static constexpr quint32 CTRL_ACTION =
	    (1u << NUL) |
	    (1u << BEL) |
	    (1u <<  BS) |
	    (1u <<  HT) |
	    (1u <<  LF) |
	    (1u <<  VT) |
	    (1u <<  FF) |
	    (1u <<  CR) |
	    (1u <<  SO) |
	    (1u <<  SI) |
	    (1u << CAN) |
	    (1u << SUB) |
	    (1u << ESC);

    // 08000f50
    static constexpr quint32 CTRL_ALWAYS =
	    (1u << EOT) |
	    (1u << ACK) |
	    (1u <<  BS) |
	    (1u <<  HT) |
	    (1u <<  LF) |
	    (1u <<  VT) |
	    (1u << ESC);

    static constexpr uint UC_REPLACE = 0x0000fffdul;
    static constexpr uint UC_INVALID = 0xfffffffful;

    enum CharacterMap {
	MAP_VT500 = -2,
	MAP_INVALID,
	MAP_LATIN1,
	MAP_DECGR,
	MAP_IBMPC,
	MAP_USER,
	NRCS_USASCII,
	NRCS_BRITISH,
	NRCS_DUTCH,
	NRCS_FINNISH,
	NRCS_FRENCH_CANADIAN,
	NRCS_FRENCH,
	NRCS_GERMAN,
	NRCS_ITALIAN,
	NRCS_NORWEGIAN_DANISH,
	NRCS_SPANISH,
	NRCS_SWEDISH,
	NRCS_SWISS,
	NRCS_PORTUGESE,
	NRCS_GREEK,
	NRCS_HEBREW,
	NRCS_TURKISH,
	NRCS_COUNT
    };

    enum EscapeState {
	ESnormal,					//!< normal character
	ESesc,						//!< after ESC character
	ESspc,						//!< after ESC SP (interface control)
	EScsi,						//!< CSI control sequence introducer (ESC [)
	ESgetargs,					//!< expecting CSI arguments
	ESgotargs,					//!< parsed all CSI arguments
	ESfunckey,					//!< function key
	ESstring,					//!< string after SOS until ST
	ESperc,						//!< CSI %
	EShash,						//!< CSI #
	ESosc,						//!< OSC operating system call (ESC ])
	ESpalette,					//!< Defining a palette entry
	ESsetG0_vt200,					//!< ESC (
	ESsetG0_vt500,					//!< ESC ( %
	ESsetG1_vt200,					//!< ESC )
	ESsetG1_vt500,					//!< ESC ) %
	ESsetG2_vt200,					//!< ESC *
	ESsetG2_vt500,					//!< ESC * %
	ESsetG3_vt200,					//!< ESC +
	ESsetG3_vt500,					//!< ESC + %
    };

    Terminal m_terminal;
    QString m_font_family;				//!< Font family to use
    int m_backlog_max;					//!< max. number of lines to keep in backlog
    vtPage m_backlog;					//!< lines which scrolled out of view
    vtPage m_screen;					//!< A number of vtLine with columns of vtAttr attributes
    int m_blink_timer;					//!< blink timer id
    qint64 m_screen_time;				//!< screen off seconds since epoch
    bool m_blink_phase;					//!< blink on/off phase
    bool m_conceal_off;					//!< concealed display off
    int m_zoom;						//!< Zoom factor in percent
    int m_font_w;					//!< Width of a glyph cell in pixels
    int m_font_h;					//!< Height of a glyph cell in pixels
    int m_font_d;					//!< Descent of a glyph cell in pixels
    int m_width;					//!< Terminal width in cells (columns)
    int m_height;					//!< Terminal height in cells (rows)
    int m_top;						//!< Scroll range top line (zero based)
    int m_bottom;					//!< Scroll range bottom line (zero based)
    qint32 m_palsize;					//!< Size of palette
    QVector<QRgb> m_pal;				//!< Palette colors
    vtGlyphs m_glyphs;					//!< QHash of glyphs, i.e. rendered QImages of the font
    vtAttr m_def;					//!< Default attributes
    vtAttr m_att;					//!< Current attributes
    vtAttr m_att_saved;					//!< Saved attributes
    Cursor m_cursor;					//!< Current cursor position and flags
    Cursor m_cursor_saved;				//!< Saved cursor position and flags
    qint32 m_cc_mask;					//!< current complement colors mask
    qint32 m_cc_save;					//!< saved complement colors mask
    quint32 m_cursor_type;				//!< cursor type
    QString m_string;					//!< string collected after SOS until ST
    QVector<int> m_csi_args;				//!< list of CSI parameters
    QBitArray m_tabstop;				//!< tabstop poisitions; marked with 1 bits
    qint32 m_state;					//!< decoder state
    qint32 m_deccolm;					//!< DEC column mode (80 or 132)
    bool m_ques;					//!< true if question mark in CSI
    bool m_decscnm;					//!< inverse video
    bool m_togmeta;					//!< toggle meta character
    bool m_deccm;					//!< DEC cursor mode (0: off, 1: on)
    bool m_decim;					//!< DEC insert mode
    bool m_decom;					//!< DEC origin mode
    bool m_deccr;					//!< DEC send CRLF or LF (0: LF, 1: CRLF)
    bool m_decckm;					//!< cursor keys
    bool m_decawm;					//!< DEC auto wrap mode
    bool m_decarm;					//!< DEC auto repeat mode
    bool m_repmouse;					//!< report mouse
    bool m_dspctrl;					//!< display control characters
    bool m_s8c1t;					//!< DEC send control codes (7 or 8 bit)
    qint32 m_ansi;					//!< ANSI conformance level
    qint32 m_uc;					//!< underline color
    qint32 m_hc;					//!< half-bright color
    qint32 m_bell_pitch;				//!< bell pitch in Hertz
    qint32 m_bell_duration;				//!< bell duration in milli seconds
    qint32 m_blank_time;				//!< console blanking timeout
    qint32 m_vesa_time;					//!< VESA blanking timeout
    QHash<CharacterMap,QString> m_charmap_name;
    QVector<cmapHash> m_charmaps;			//!< 8 bit character code to Unicode translation tables
    QVector<cmapHash> m_gmaps;				//!< 8 bit character code to Unicode translation tables
    cmapHash m_trans;					//!< 8 bit character code to Unicode translation table
    int m_shift;					//!< single shift to back to charset if non-zero
    bool m_utf_mode;					//!< Unicode UTF-8 mode (0: off, 1: on)
    int m_utf_more;					//!< number of expected UTF-8 codes until char
    uint m_utf_code;					//!< Unicode UTF-8 code (glyph index)
    uint m_utf_code_min;				//!< Unicode UTF-8 minimum code for given # of encoded bytes

    void add_backlog(const vtLine& line);
    void update_cell(int x, int y);
    void outch(int x, int y, const vtAttr& pa);
    void zap(int x0, int y0, int x1, int y1, quint32 code);
    void set_cursor(bool on);
    void set_newx(int newx);
    void vt_scroll_dn();
    void vt_scroll_up();
    void vt_BS();
    void vt_TAB();
    void vt_RI();
    void vt_IND();
    void vt_NEL();
    void vt_HTS();
    void vt_LF();
    void vt_VT();
    void vt_FF();
    void vt_CR();
    void vt_CUP(int x, int y);
    void vt_CUU(int n);
    void vt_CUD(int n);
    void vt_CUF(int n);
    void vt_CUB(int n);
    void vt_CHA(int n);
    void vt_CNL(int n);
    void vt_CPL(int n);
    void vt_CHT(int n);
    void vt_CUA(int x, int y);
    void vt_DECSET();
    void vt_RM();
    void vt_ED(int n);
    void vt_EL(int n);
    void vt_IL(int n);
    void vt_DL(int n);
    void vt_ICH(int n);
    void vt_DCH(int n);
    void vt_ECH(int n);
    void vt_LS0();
    void vt_LS1();
    void vt_LS2();
    void vt_LS3();
    void vt_LS1R();
    void vt_LS2R();
    void vt_LS3R();
    void vt_SS2();
    void vt_SS3();
    void vt_DCS();
    void vt_SPA();
    void vt_EPA();
    void vt_SOS();
    void vt_ST();
    void vt_OSC();
    void vt_PM();
    void vt_APC();
    void vt_SGR();
    CharacterMap select_map_vt200(uchar ch);
    CharacterMap select_map_vt500(uchar ch);
    void vt_palette_reset();
    void vt_tabstop_reset();
    void vt_DECID(int Ps = 0);
    void save();
    void restore();
};
