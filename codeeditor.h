#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QMainWindow>
#include <QtCore>
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QtWidgets>


class CodeEditor;
class Highlighter;

class CodeEditor : public QPlainTextEdit
{
	Q_OBJECT

public:
	CodeEditor(QWidget *parent = 0);
	void lineNumberAreaPaintEvent(QPaintEvent *event);
	int lineNumberAreaWidth();

protected:
	void resizeEvent(QResizeEvent *event);

private slots:
	void updateLineNumberAreaWidth(int newBlockCount);
	void highlightCurrentLine();
	void updateLineNumberArea(const QRect &, int);

private:
	QWidget *lineNumberArea;
};

//////////////////////////////////////////////////////

class LineNumberArea : public QWidget
{
public:
	LineNumberArea(CodeEditor *editor) : QWidget(editor)
	{
		codeEditor = editor;
	}

	QSize sizeHint() const
	{
		return QSize(codeEditor->lineNumberAreaWidth(), 0);
	}

protected:
	void paintEvent(QPaintEvent *event)
	{
		codeEditor->lineNumberAreaPaintEvent(event);
	}

private:
	CodeEditor *codeEditor;
};

//////////////////////////////////////////////////////////////////

class StatesListHandler
{
public:
	StatesListHandler(const QString &, int);
	bool handling(QVector<int> &, int &);
private:
	bool cleaning();
	bool firstVerification();
	bool secondVerification(QVector<int> &);
	bool recombination(QVector<int> &, int&);
public:
	QString input, tmpString;
	int sizeOfAlphabet;
};


class Highlighter : public QSyntaxHighlighter
{
	Q_OBJECT

public:
	Highlighter(QTextDocument *parent = 0);

protected:
	void highlightBlock(const QString &text);

private:
	struct HighlightingRule
	{
		QRegExp pattern;
		QTextCharFormat format;
	};
	QVector<HighlightingRule> highlightingRules;

	QRegExp commentStartExpression;
	QRegExp commentEndExpression;
	QTextCharFormat keywordFormat;
	QTextCharFormat singleLineCommentFormat;
	QTextCharFormat multiLineCommentFormat;
	QTextCharFormat variableFormat;
	QTextCharFormat numericConstantFormat;
};
#endif // CODEEDITOR_H
