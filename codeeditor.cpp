/***********************************************
 gSafe document editor

 Author:
    (C) 2026  Deák Péter (hyper80@gmail.com)
*/

#include "codeeditor.h"
#include <QFont>
#include <QFontDatabase>
#include <QPainter>
#include <QTextBlock>
#include <QKeyEvent>
#include <QTextCursor>
#include <QRegularExpression>
#include <QTextDocument>

CodeEditor::CodeEditor(QWidget *parent)
: QPlainTextEdit(parent), lineNumberArea(new LineNumberArea(this))
{
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSize(11);
    setFont(font);
    setLineWrapMode(QPlainTextEdit::NoWrap);

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int CodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) { max /= 10; ++digits; }
    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void CodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) 
    {
        if (block.isVisible() && bottom >= event->rect().top()) 
        {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width() - 4, fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if(!isReadOnly()) 
    {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(153,217,234);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    // Highlight entire lines that start with '//' in light green
    {
        QRegularExpression commentLineRe("^\\s*//");
        for (QTextBlock block = document()->firstBlock(); block.isValid(); block = block.next()) 
        {
            QString text = block.text();
            if (commentLineRe.match(text).hasMatch()) 
            {
                QTextEdit::ExtraSelection s;
                s.format.setForeground(QColor(0, 150, 0));
                QTextCursor c(block);
                c.select(QTextCursor::LineUnderCursor);
                s.cursor = c;
                extraSelections.append(s);
            }
        }
    }

    // Brace matching: check char before and after cursor and highlight pair
    QTextCursor cur = textCursor();
    int pos = cur.position();
    QChar cBefore = (pos > 0) ? document()->characterAt(pos - 1) : QChar();
    QChar cAfter = document()->characterAt(pos);

    const QChar openers[3] = {'(', '{', '['};
    const QChar closers[3] = {')', '}', ']'};

    // helper to add selection at position
    auto addSelAt = [&](int p) 
    {
        if (p < 0) return;
        QTextEdit::ExtraSelection s;
        QTextCursor c(document());
        c.setPosition(p);
        c.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
        s.cursor = c;
        s.format.setBackground(QColor(240, 240, 120));
        extraSelections.append(s);
    };

    bool matched = false;
    // if before is opener, search forward
    for (int i = 0; i < 3 && !matched; ++i) 
    {
        if (cBefore == openers[i]) 
        {
            QChar opener = openers[i];
            QChar closer = closers[i];
            int depth = 1;
            int len = document()->characterCount();
            for (int p = pos; p < len; ++p) 
            {
                QChar ch = document()->characterAt(p);
                if (ch == opener) 
                    depth++;
                else if (ch == closer) 
                {
                    depth--;
                    if (depth == 0) 
                    {
                        addSelAt(pos - 1);
                        addSelAt(p);
                        matched = true;
                        break;
                    }
                }
            }
        }
    }

    // if after is closer, search backward
    for (int i = 0; i < 3 && !matched; ++i) 
    {
        if (cAfter == closers[i]) 
        {
            QChar opener = openers[i];
            QChar closer = closers[i];
            int depth = 1;
            for (int p = pos - 2; p >= 0; --p) 
            {
                QChar ch = document()->characterAt(p);
                if (ch == closer) depth++;
                else if (ch == opener) 
                {
                    depth--;
                    if (depth == 0) 
                    {
                        addSelAt(p);
                        addSelAt(pos);
                        matched = true;
                        break;
                    }
                }
            }
        }
    }
    setExtraSelections(extraSelections);
}

void CodeEditor::keyPressEvent(QKeyEvent *e)
{
    // Auto-indent on Enter
    if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) 
    {
        QTextCursor tc = textCursor();
        int curPos = tc.position();
        // get current line
        QTextCursor lineStart = tc;
        lineStart.movePosition(QTextCursor::StartOfLine);
        lineStart.select(QTextCursor::LineUnderCursor);
        QString line = lineStart.selectedText();
        QRegularExpression re("^(\\s*)");
        QRegularExpressionMatch m = re.match(line);
        QString indent = m.hasMatch() ? m.captured(1) : QString();
        // if previous non-space char is '{' increase indent
        QTextCursor before = textCursor();
        before.setPosition(curPos);
        if (curPos > 0) 
        {
            before.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
            QString prev = before.selectedText();
        }
        insertPlainText("\n" + indent);
        return;
    }

    // Auto-closing pairs
    if (!e->text().isEmpty())
    {
        QChar ch = e->text().at(0);
        QChar closing;
        bool doPair = true;
        if (ch == '(') closing = ')';
        else if (ch == '{') closing = '}';
        else if (ch == '[') closing = ']';
        else if (ch == '"') closing = '"';
        else if (ch == '\'') closing = '\'';
        else doPair = false;

        if (doPair) 
        {
            QTextCursor tc = textCursor();
            if (tc.hasSelection()) 
            {
                QString sel = tc.selectedText();
                tc.insertText(QString(ch) + sel + QString(closing));
            } 
            else 
            {
                tc.insertText(QString(ch) + QString(closing));
                tc.movePosition(QTextCursor::Left);
            }
            setTextCursor(tc);
            return;
        }
    }

    QPlainTextEdit::keyPressEvent(e);
}
