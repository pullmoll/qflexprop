/***************************************************************************************
 *
 * Qt5 Propeller 2 editor
 *
 * Copyright 🄯 2021 Jürgen Buchmüller <pullmoll@t-online.de>
 *
 * See the file LICENSE for the details of the BSD-3-Clause terms.
 *
 ***************************************************************************************/
#include <QCryptographicHash>
#include <QFile>
#include <QTextStream>
#include "propedit.h"
#include "propconst.h"
#include "util.h"

static const char prop_sha256[] = "sha256";
static const char prop_filename[] = "filename";
static const char prop_filetype[] = "filetype";

PropEdit::PropEdit(QWidget *parent,
		   const int tabsize,
		   const QString& css_linearea,
		   Options options)
    : QPlainTextEdit(parent)
    , m_lineno_area(nullptr)
    , m_highlighter(nullptr)
    , m_tabsize(tabsize)
    , m_options(options)
    , m_error_lines()
{
    setWordWrapMode(QTextOption::NoWrap);
    if (m_options.testFlag(PropEdit::PROPED_USE_LINENUMBERS)) {
	m_lineno_area = new LineNumberArea(this, css_linearea);

	connect(this, &PropEdit::blockCountChanged,
		this, &PropEdit::update_line_number_area_width);
	connect(this, &PropEdit::updateRequest,
		this, &PropEdit::update_line_number_area);
	connect(this, &PropEdit::cursorPositionChanged,
		this, &PropEdit::highlight_current_line);

	update_line_number_area_width(0);
    }

    if (m_options.testFlag(PROPED_DO_HIGHLIGHT)) {
	m_highlighter = new PropHighlighter(document(), m_options);
	highlight_current_line();
    }
    setTabStopDistance(fontMetrics().averageCharWidth() * m_tabsize);
}

/**
 * @brief Append a rule to the highlighter of this PLD editor
 * @param rule highlighter rule to append
 */
void PropEdit::append_rule(HighlightingRule rule)
{
    if (!m_highlighter)
	return;
    m_highlighter->appendRule(rule);
}

/**
 * @brief Prepend a rule to the highlighter of this PLD editor
 * @param rule highlighter rule to prepend
 */
void PropEdit::prepend_rule(HighlightingRule rule)
{
    if (!m_highlighter)
	return;
    m_highlighter->prependRule(rule);
}

/**
 * @brief Set a list of line numbers to indicate as lines containing errors
 * @param list of line numbers
 */
void PropEdit::set_error_line_list(const QList<int>& list)
{
    m_error_lines = list;
    update();
}

bool PropEdit::changed() const
{
    QString text = QPlainTextEdit::toPlainText();
    QCryptographicHash sha256(QCryptographicHash::Sha256);
    sha256.addData(text.toUtf8());
    const QByteArray old_hash = property(prop_sha256).toByteArray();
    const QByteArray new_hash = sha256.result();
    return old_hash != new_hash;
}

QString PropEdit::text() const
{
    return QPlainTextEdit::toPlainText();
}

QString PropEdit::filename() const
{
    return property(prop_filename).toString();
}

FileType PropEdit::filetype() const
{
    return static_cast<FileType>(property(prop_filetype).toInt());
}

QString PropEdit::filetype_name() const
{
    switch (filetype()) {
    case FT_BASIC:
	return QLatin1String("Basic");
    case FT_C:
	return QLatin1String("C");
    case FT_SPIN:
	return QLatin1String("Spin");
    case FT_PASM:
	return QLatin1String("PAsm (P1)");
    case FT_P2ASM:
	return QLatin1String("PAsm (P2)");
    case FT_BINARY:
	return QLatin1String("Binary");
    case FT_UNKNOWN:
    default:
	return QLatin1String("???");
    }
    return QLatin1String("<Invalid>");
}

void PropEdit::setFont(const QFont& font)
{
    QPlainTextEdit::setFont(font);
    setTabStopDistance(fontMetrics().averageCharWidth() * m_tabsize);
}

void PropEdit::setText(const QString& text)
{
    QCryptographicHash sha256(QCryptographicHash::Sha256);
    sha256.addData(text.toUtf8());
    QByteArray old_hash = property(prop_sha256).toByteArray();
    QByteArray new_hash = sha256.result();
    if (old_hash == new_hash)
	return;
    setProperty(prop_sha256, new_hash);
    setPlainText(text);
}

void PropEdit::setFilename(const QString& filename)
{
    FileType filetype = util.filetype(filename);
    setProperty(prop_filetype, filetype);
    setProperty(prop_filename, filename);
}

bool PropEdit::load(const QString& filename)
{
    QFile file(filename);
    FileType filetype = util.filetype(filename);
    setProperty(prop_filetype, filetype);
    if (file.open(QIODevice::ReadOnly)) {
	QTextStream str(&file);
	str.setCodec("UTF-8");
	QString text = str.readAll();
	file.close();
	setText(text);
	setProperty(prop_filename, filename);
	return true;
    }
    return false;
}

bool PropEdit::save(const QString& save_filename)
{
    QString filename = save_filename.isEmpty() ?
			   property(prop_filename).toString() :
			   save_filename;
    QString backup = QString("%1~").arg(filename);
    QFile::remove(backup);
    QFile::rename(filename, backup);
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
	QString text = QPlainTextEdit::toPlainText();
	QCryptographicHash sha256(QCryptographicHash::Sha256);
	sha256.addData(text.toUtf8());
	QByteArray hash = sha256.result();
	QTextStream str(&file);
	str.setCodec("UTF-8");
	setPlainText(text);
	file.close();
	setProperty(prop_sha256, hash);
	setProperty(prop_filename, filename);
	FileType filetype = util.filetype(filename);
	setProperty(prop_filetype, filetype);
	return true;
    }
    return false;
}

/**
 * @brief Return the width (in pixels) of the highlighter line number area
 * @return width in pixels
 */
int PropEdit::line_number_area_width()
{
    int digits = 2;
    int max = qMax(1, blockCount());
    while (max >= 10) {
	max /= 10;
	++digits;
    }

    const int space = 3 + fontMetrics().horizontalAdvance(QString(digits, QChar('9')));

    return space;

}

/**
 * @brief Update the line number area width of this PLD editor
 */
void PropEdit::update_line_number_area_width(int /* newBlockCount */)
{
    setViewportMargins(line_number_area_width(), 0, 0, 0);
}

/**
 * @brief Update the line number area e.g. after lines have changed
 * @param rect update rectangle
 * @param dy delta y to scroll
 */
void PropEdit::update_line_number_area(const QRect &rect, int dy)
{
    if (dy) {
	m_lineno_area->scroll(0, dy);
    } else {
	m_lineno_area->update(0, rect.y(), m_lineno_area->width(), rect.height());
    }

    if (rect.contains(viewport()->rect()))
	update_line_number_area_width(0);
}

/**
 * @brief Resize event for the line number area
 * @param e pointer to the QResizeEvent
 */
void PropEdit::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    if (!m_lineno_area)
	return;

    QRect cr = contentsRect();
    m_lineno_area->setGeometry(QRect(cr.left(), cr.top(), line_number_area_width(), cr.height()));
}

/**
 * @brief Highlight the current line in the line number area
 */
void PropEdit::highlight_current_line()
{
    if (!m_lineno_area)
	return;

    const QRgb rgb = m_highlighter->background().color().rgb();
    const QString style = QString("background: #%1;").arg(rgb, 8, 16, QChar('0'));
    setStyleSheet(style);

    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
	QTextEdit::ExtraSelection selection;

	selection.format.setBackground(QColor(color_line_number_area));
	selection.format.setProperty(QTextFormat::FullWidthSelection, true);
	selection.cursor = textCursor();
	selection.cursor.clearSelection();
	extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);

}

/**
 * @brief Handle the paint event for the line number area
 * @param event pointer to the QPaintEvent
 */
void PropEdit::line_number_area_paint_event(QPaintEvent *event)
{
    if (!m_lineno_area)
	return;

    QPainter painter(m_lineno_area);
    painter.setFont(font());
    painter.fillRect(event->rect(), color_line_number_area);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {

	if (block.isVisible() && bottom >= event->rect().top()) {

	    QString number = QString::number(blockNumber + 1);

	    if (m_error_lines.contains(blockNumber + 1)) {
		painter.fillRect(0, top, m_lineno_area->width(), fontMetrics().height(), Qt::red);
	    }

	    painter.drawText(0, top,
			     m_lineno_area->width() - 4,
			     fontMetrics().height(),
			     Qt::AlignRight, number);
	}

	block = block.next();
	top = bottom;
	bottom = top + qRound(blockBoundingRect(block).height());
	++blockNumber;
    }

}

/**
 * @brief prop_highlighter constructor
 * @param doc pointer to the QTextDocument to highlight
 * @param options selected options
 */
PropHighlighter::PropHighlighter(QTextDocument *doc, PropEdit::Options options)
    : QSyntaxHighlighter(doc)
    , m_options(options)
{
    //
    // Highlight section names ?
    //
    if (m_options.testFlag(PropEdit::PROPED_USE_SECTIONS)) {
	HighlightingRule rule;
	QStringList sections = g_tokens.list(g_sections);
	sectionsFormat.setFontUnderline(true);
	sectionsFormat.setBackground(QColor(color_background));
	sectionsFormat.setForeground(QColor(color_section));
	rule.pattern = QRegExp(QString("^(%1)")
			       .arg(sections.join(QChar('|'))));
	rule.format = sectionsFormat;
	highlightingRules.append(rule);
    }

    // Highlight operators?
    if (m_options.testFlag(PropEdit::PROPED_USE_OPERATORS)) {
	HighlightingRule rule;
	operatorFormat.setBackground(QColor(color_background));
	operatorFormat.setForeground(QColor(color_operator));
	operatorFormat.setFontWeight(QFont::Bold);
	QStringList keywords = g_tokens.list_esc(g_operator);
	rule.pattern = QRegExp(QString("(%1)")
			       .arg(keywords.join(QChar('|'))),
			       Qt::CaseInsensitive);
	rule.format = operatorFormat;
	highlightingRules.append(rule);
    }

    // Highlight decimal constant
    if (m_options.testFlag(PropEdit::PROPED_USE_DEC)) {
	HighlightingRule rule;
	decFormat.setBackground(QColor(color_background));
	decFormat.setForeground(QColor(color_dec));
	rule.pattern = QRegExp(QString("\\W[0-9_]+\\b"));
	rule.format = decFormat;
	highlightingRules.append(rule);
    }

    // Highlight binary constant
    if (m_options.testFlag(PropEdit::PROPED_USE_BIN)) {
	HighlightingRule rule;
	binFormat.setBackground(QColor(color_background));
	binFormat.setForeground(QColor(color_bin));
	rule.pattern = QRegExp(QString("\\W\\%[0-1_]+\\b"));
	rule.format = binFormat;
	highlightingRules.append(rule);
    }

    // Highlight hexadecimal constan
    if (m_options.testFlag(PropEdit::PROPED_USE_HEX)) {
	HighlightingRule rule;
	hexFormat.setBackground(QColor(color_background));
	hexFormat.setForeground(QColor(color_hex));
	rule.pattern = QRegExp(QString("\\W\\$[0-9A-Fa-f_]+"));
	rule.format = hexFormat;
	highlightingRules.append(rule);
    }

    // Highlight float constant
    if (m_options.testFlag(PropEdit::PROPED_USE_FLOAT)) {
	HighlightingRule rule;
	fltFormat.setBackground(QColor(color_background));
	fltFormat.setForeground(QColor(color_flt));
	rule.pattern = QRegExp(QString("\\W[0-9]+\\.[0-9]*"));
	rule.format = fltFormat;
	highlightingRules.append(rule);
    }

    // Highlight keywords (reserved names) ?
    // i.e. BYTE, WORD, LONG, ORG, ORG, ORGF, RES, FIT, ...
    if (m_options.testFlag(PropEdit::PROPED_USE_KEYWORDS)) {
	HighlightingRule rule;
	QStringList keywords = g_tokens.list_esc(g_keywords);
	keywordFormat.setFontWeight(QFont::ExtraBold);
	keywordFormat.setBackground(QColor(color_background));
	keywordFormat.setForeground(QColor(color_keyword));
	rule.pattern = QRegExp(QString("\\b(%1)\\b")
			       .arg(keywords.join(QChar('|'))),
			       Qt::CaseInsensitive);
	rule.format = keywordFormat;
	highlightingRules.append(rule);
    }

    // Highlight preprocessor statements
    // i.e. #define, #undef, #if, #ifdef, #else, ...
    if (m_options.testFlag(PropEdit::PROPED_USE_PREPROC)) {
	HighlightingRule rule;
	QStringList keywords = g_tokens.list_esc(g_preproc);
	preprocFormat.setBackground(QColor(color_background));
	preprocFormat.setForeground(QColor(color_preproc));
	rule.pattern = QRegExp(QString("(^|\\b)(%1)\\b")
			       .arg(keywords.join(QChar('|'))));
	rule.format = preprocFormat;
	highlightingRules.append(rule);
    }

    // Enable Multi-Line Comments ?
    // Starting with "{{", ending with "}}"
    if (options.testFlag(PropEdit::PROPED_USE_MULTI_LINE_COMMENTS)) {
	HighlightingRule rule;
	multiLineCommentFormat.setBackground(QColor(color_background));
	multiLineCommentFormat.setForeground(QColor(color_comment));
	commentStartExpression = QRegExp(QRegExp::escape("{{"));
	rule.pattern = commentStartExpression;
	rule.format = multiLineCommentFormat;
	highlightingRules.append(rule);
	commentEndExpression = QRegExp(QRegExp::escape("}}"));
    }

    // Enable Single-Line Comments ? (starting with "'" until end-of-line...)
    if (m_options.testFlag(PropEdit::PROPED_USE_SINGLE_LINE_COMMENTS)) {
	HighlightingRule rule;
	singleLineCommentFormat.setBackground(QColor(color_background));
	singleLineCommentFormat.setForeground(QColor(color_comment));
	rule.pattern = QRegExp("'[^\n]*");
	rule.format = singleLineCommentFormat;
	highlightingRules.append(rule);
    }
}

QBrush PropHighlighter::background()
{
    return QBrush(color_background);
}

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

/**
 * @brief Highlight a block of text
 * @param text const reference to the QString with the text to highlight
 */
void PropHighlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {

	QRegExp expression(rule.pattern);
	int index = expression.indexIn(text);

	while (index >= 0) {
	    const int length = expression.matchedLength();
	    setFormat(index, length, rule.format);
	    index = expression.indexIn(text, index + length);
	}

    }

    setCurrentBlockState(0);

    // Multi-Line Comments supported ?
    if (m_options.testFlag(PropEdit::PROPED_USE_MULTI_LINE_COMMENTS)) {

	int startIndex = 0;
	if (previousBlockState() != 1)
	    startIndex = commentStartExpression.indexIn(text);

	while (startIndex >= 0) {
	    const int endIndex = commentEndExpression.indexIn(text, startIndex);
	    int commentLength;
	    if (endIndex == -1) {
		setCurrentBlockState(1);
		commentLength = text.length() - startIndex;
	    } else {
		commentLength = endIndex - startIndex
			+ commentEndExpression.matchedLength();
	    }

	    setFormat(startIndex, commentLength, multiLineCommentFormat);
	    startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
	}
    }
}

void PropHighlighter::appendRule(HighlightingRule rule)
{
    highlightingRules.append(rule);
}

void PropHighlighter::prependRule(HighlightingRule rule)
{
    highlightingRules.prepend(rule);
}

LineNumberArea::LineNumberArea(PropEdit* editor, QString css)
    : QWidget(editor)
{
    codeEditor = editor;
    setStyleSheet(css);
}

QSize LineNumberArea::sizeHint() const
{
    return QSize(codeEditor->line_number_area_width(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent* event)
{
    codeEditor->line_number_area_paint_event(event);
}
