/***************************************************************************************
 *
 * Qt5 Propeller 2 editor
 *
 * Copyright 🄯 2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 ***************************************************************************************/
#pragma once
#include <QString>
#include <QPlainTextEdit>
#include <QObject>
#include <QWidget>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QPainter>
#include "util.h"

class LineNumberArea;
class PropHighlighter;

typedef struct {
    QRegExp pattern;
    QTextCharFormat format;
}   HighlightingRule;

/**
 * @brief PropEdit class
 * The PropEdit class is derived from the QPlainTextEdit
 * and offers syntax highlighting and line number options.
 */
class PropEdit : public QPlainTextEdit
{
    Q_OBJECT
    Q_FLAGS(Options)
public:
    enum Option {
	PE_NONE                     = 0,
	PE_USE_LINENUMBERS          = (1 << 0),
	PE_DO_HIGHLIGHT             = (1 << 1),
	PE_USE_PREPROC              = (1 << 2),
	PE_USE_KEYWORDS             = (1 << 3),
	PE_USE_CONDITIONALS         = (1 << 4),
	PE_USE_SECTIONS             = (1 << 5),
	PE_USE_OPERATORS            = (1 << 6),
	PE_USE_BIN		    = (1 << 7),
	PE_USE_DEC		    = (1 << 8),
	PE_USE_HEX		    = (1 << 9),
	PE_USE_FLOAT		    = (1 <<10),
	PE_USE_STRING		    = (1 <<11),
	PE_USE_IN_LINE_COMMENTS	    = (1 <<13),
	PE_USE_SINGLE_LINE_COMMENTS = (1 <<14),
	PE_USE_MULTI_LINE_COMMENTS  = (1 <<15),

	PE_DEFAULT = PE_NONE
	    | PE_USE_LINENUMBERS
	    | PE_DO_HIGHLIGHT
	    | PE_USE_PREPROC
	    | PE_USE_KEYWORDS
	    | PE_USE_CONDITIONALS
	    | PE_USE_SECTIONS
	    | PE_USE_OPERATORS
	    | PE_USE_BIN
	    | PE_USE_DEC
	    | PE_USE_HEX
	    | PE_USE_FLOAT
	    | PE_USE_STRING
	    | PE_USE_IN_LINE_COMMENTS
	    | PE_USE_SINGLE_LINE_COMMENTS
	    | PE_USE_MULTI_LINE_COMMENTS
    };

    Q_DECLARE_FLAGS(Options, Option)

    PropEdit(QWidget *parent = nullptr,
	     const int tabsize = 8,
	     const QString& css_linearea = QString(),
	     Options options = PE_DEFAULT);

    void line_number_area_paint_event(QPaintEvent *event);
    int  line_number_area_width();
    void append_rule(HighlightingRule rule);
    void prepend_rule(HighlightingRule rule);
    void set_error_line_list(const QList<int>& list = QList<int>());

    bool changed() const;
    QString text() const;
    QString filename() const;
    FileType filetype() const;
    QString filetype_name() const;

public slots:
    bool load(const QString& filename = QString());
    bool save(const QString& filename = QString());
    void setFont(const QFont& font);
    void setText(const QString& text);
    void setFilename(const QString& filename);
    void gotoLineNumber(int lnum);

protected:
    void resizeEvent(QResizeEvent *event);
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void zoom_in();
    void zoom_out();
    void update_line_number_area_width(int newBlockCount);
    void highlight_current_line();
    void update_line_number_area(const QRect& rect, int dy);

private:
    static constexpr QRgb color_line_number_area = qRgb(0xf0,0xf0,0xf0);

    QWidget* m_lineno_area;
    PropHighlighter* m_highlighter;
    int m_tabsize;
    PropEdit::Options m_options;
    QList<int> m_error_lines;
};

/**
 * @brief The Highlighter class is derived from the QSyntaxHighlighter class
 * and implements highlighting for a number of rules.
 */
class PropHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    PropHighlighter(QTextDocument *doc = nullptr, PropEdit::Options options = PropEdit::PE_DEFAULT);

    QBrush background();
    void appendRule(HighlightingRule rule);
    void prependRule(HighlightingRule rule);

protected:
    void highlightBlock(const QString &text) Q_DECL_OVERRIDE;

private:
    static constexpr QRgb color_background  = qRgb(0xf8, 0xfc, 0xf8);
    static constexpr QRgb color_preproc	    = qRgb(0x20, 0x20, 0x7f);
    static constexpr QRgb color_keyword	    = qRgb(0x00, 0x9f, 0x9f);
    static constexpr QRgb color_conditional = qRgb(0x9f, 0x00, 0x9f);
    static constexpr QRgb color_section	    = qRgb(0x00, 0x5f, 0x5f);
    static constexpr QRgb color_operator    = qRgb(0x60, 0x40, 0x60);
    static constexpr QRgb color_comment	    = qRgb(0x30, 0x80, 0x30);
    static constexpr QRgb color_bin	    = qRgb(0x80, 0x20, 0x60);
    static constexpr QRgb color_dec	    = qRgb(0x80, 0x20, 0x20);
    static constexpr QRgb color_hex	    = qRgb(0xa0, 0x60, 0x20);
    static constexpr QRgb color_flt	    = qRgb(0xa0, 0x20, 0x60);
    static constexpr QRgb color_str	    = qRgb(0x30, 0x30, 0xff);

    PropEdit::Options m_options;
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat inLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat sectionsFormat;
    QTextCharFormat operatorFormat;
    QTextCharFormat keywordFormat;
    QTextCharFormat conditionalFormat;
    QTextCharFormat preprocFormat;
    QTextCharFormat binFormat;
    QTextCharFormat decFormat;
    QTextCharFormat hexFormat;
    QTextCharFormat fltFormat;
    QTextCharFormat strFormat;
};

class LineNumberArea : public QWidget
{
public:
    LineNumberArea(PropEdit *editor, QString css);

    QSize sizeHint() const;

protected:
    void paintEvent(QPaintEvent *event);

private:
    PropEdit *codeEditor;
};
