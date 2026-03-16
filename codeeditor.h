/***********************************************
 gSafe document editor

 Author:
    (C) 2026  Deák Péter (hyper80@gmail.com)
*/
#pragma once
#include <QPlainTextEdit>
#include <QWidget>
#include <QSize>

#define VERSION "1.004"

class QPaintEvent;
class QResizeEvent;
class QRect;
class QKeyEvent;

class LineNumberArea;

class CodeEditor : public QPlainTextEdit 
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);

    void keyPressEvent(QKeyEvent *e) override;

    int lineNumberAreaWidth();
    void lineNumberAreaPaintEvent(QPaintEvent *event);
protected:
    void resizeEvent(QResizeEvent *event) override;
private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);
private:
    void highlightCurrentLine();

    QWidget *lineNumberArea;
};

class LineNumberArea : public QWidget 
{
public:
    explicit LineNumberArea(CodeEditor *editor) : QWidget(editor), codeEditor(editor) {}
    QSize sizeHint() const override { return QSize(codeEditor->lineNumberAreaWidth(), 0); }
protected:
    void paintEvent(QPaintEvent *event) override { codeEditor->lineNumberAreaPaintEvent(event); }
private:
    CodeEditor *codeEditor;
};
 
