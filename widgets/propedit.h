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
 * @brief The propEdit class is derived from the QPlainTextEdit
 * and offers syntax highlighting and line number @ref Options.
 */
class PropEdit : public QPlainTextEdit
{
    Q_OBJECT
    Q_FLAGS(Options)
public:
    enum Option {
	PROPED_NONE                     = 0,
	PROPED_USE_LINENUMBERS          = (1 << 0),
	PROPED_DO_HIGHLIGHT             = (1 << 1),
	PROPED_USE_PREPROC              = (1 << 2),
	PROPED_USE_KEYWORDS             = (1 << 3),
	PROPED_USE_SECTIONS             = (1 << 4),
	PROPED_USE_OPERATORS            = (1 << 5),
	PROPED_USE_BIN			= (1 << 6),
	PROPED_USE_DEC			= (1 << 7),
	PROPED_USE_HEX			= (1 << 8),
	PROPED_USE_FLOAT		= (1 << 9),
	PROPED_USE_SINGLE_LINE_COMMENTS = (1 <<14),
	PROPED_USE_MULTI_LINE_COMMENTS  = (1 <<15),

	PROPED_DEFAULT = PROPED_NONE
	    | PROPED_USE_LINENUMBERS
	    | PROPED_DO_HIGHLIGHT
	    | PROPED_USE_PREPROC
	    | PROPED_USE_KEYWORDS
	    | PROPED_USE_SECTIONS
	    | PROPED_USE_OPERATORS
	    | PROPED_USE_BIN
	    | PROPED_USE_DEC
	    | PROPED_USE_HEX
	    | PROPED_USE_FLOAT
	    | PROPED_USE_SINGLE_LINE_COMMENTS
	    | PROPED_USE_MULTI_LINE_COMMENTS
    };

    Q_DECLARE_FLAGS(Options, Option)

    PropEdit(QWidget *parent = nullptr,
	     const int tabsize = 8,
	     const QString& css_linearea = QString(),
	     Options options = PROPED_DEFAULT);

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
    bool load(const QString& filename);
    bool save(const QString& save_filename = QString());

public slots:
    void setFont(const QFont& font);
    void setText(const QString& text);
    void setFilename(const QString& filename);

protected:
    void resizeEvent(QResizeEvent *event);

private slots:
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
    PropHighlighter(QTextDocument *doc = nullptr, PropEdit::Options options = PropEdit::PROPED_DEFAULT);

    QBrush background();
    void appendRule(HighlightingRule rule);
    void prependRule(HighlightingRule rule);

protected:
    void highlightBlock(const QString &text) Q_DECL_OVERRIDE;

private:
    static constexpr QRgb color_background = qRgb(0xfc,0xfc,0xf4);
    static constexpr QRgb color_preproc = qRgb(0x20, 0x20, 0xdf);
    static constexpr QRgb color_keyword = qRgb(0x00,0x9f,0x9f);
    static constexpr QRgb color_section = qRgb(0x00,0x5f,0x5f);
    static constexpr QRgb color_operator = qRgb(0x20, 0x40, 0x80);
    static constexpr QRgb color_comment = qRgb(0x30, 0x80, 0x30);
    static constexpr QRgb color_bin = qRgb(0x80, 0x20, 0xc0);
    static constexpr QRgb color_dec = qRgb(0x80, 0x20, 0x20);
    static constexpr QRgb color_hex = qRgb(0xa0, 0x60, 0x20);
    static constexpr QRgb color_flt = qRgb(0x60, 0xa0, 0x20);

    PropEdit::Options m_options;
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat sectionsFormat;
    QTextCharFormat operatorFormat;
    QTextCharFormat keywordFormat;
    QTextCharFormat preprocFormat;
    QTextCharFormat binFormat;
    QTextCharFormat decFormat;
    QTextCharFormat hexFormat;
    QTextCharFormat fltFormat;
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
