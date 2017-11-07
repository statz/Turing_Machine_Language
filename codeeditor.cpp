#include "codeeditor.h"



CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
	lineNumberArea = new LineNumberArea(this);
	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
	connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
	updateLineNumberAreaWidth(0);
    highlightCurrentLine();
    this->setTabStopWidth(8);
}

int CodeEditor::lineNumberAreaWidth()
{
	int digits = 1;
	int max = qMax(1, blockCount());
    while (max >= 10)
    {
		max /= 10;
		++digits;
	}
	int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;
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

void CodeEditor::highlightCurrentLine()
{
	QList<QTextEdit::ExtraSelection> extraSelections;

	if (!isReadOnly()) {
		QTextEdit::ExtraSelection selection;
		QColor lineColor = QColor(Qt::yellow).lighter(160);
		selection.format.setBackground(lineColor);
		selection.format.setProperty(QTextFormat::FullWidthSelection, true);
		selection.cursor = textCursor();
		selection.cursor.clearSelection();
		extraSelections.append(selection);
	}
	setExtraSelections(extraSelections);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
	QPainter painter(lineNumberArea);
	painter.fillRect(event->rect(), Qt::lightGray);
	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int) blockBoundingRect(block).height();
	while (block.isValid() && top <= event->rect().bottom()) {
		if (block.isVisible() && bottom >= event->rect().top()) {
			QString number = QString::number(blockNumber + 1);
			painter.setPen(Qt::black);
			painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
							 Qt::AlignRight, number);
		}
		block = block.next();
		top = bottom;
		bottom = top + (int) blockBoundingRect(block).height();
		++blockNumber;
	}
}
////////////////////////////////////////////////////////////////////////////////////
StatesListHandler::StatesListHandler(const QString &s, int n)
{
	input = s;
	sizeOfAlphabet = n;
}

bool StatesListHandler::cleaning()
{
	bool flagOfSpace = true;
	for (int i = 0; i < input.size(); i++)
	{
		if(input[i] >= '0' && input[i] <= '9')
		{
			tmpString.append(input[i]);
			flagOfSpace = false;
		}
		else if(input[i] == ' ')
		{
			if(!flagOfSpace)
			{
				tmpString.append(' ');
				flagOfSpace = true;
			}
		}
		else if(input[i] == ';')
		{
			if(tmpString[tmpString.size()-1] == ' ')
				tmpString[tmpString.size()-1] = ';';
			else
				tmpString.append(';');
			flagOfSpace = true;
		}
		else if(input[i] == '/')
			while(input[++i] != '\n' && i != input.size());
		else if(input[i] == '\n')
		{
			flagOfSpace = true;
			continue;
		}
		else
			return true;
	}
	return false;
}

bool StatesListHandler::firstVerification()
{
	int count = 0;
	for(int i = 0; i < tmpString.size(); i++)
		if(tmpString[i] == ' ')
			count++;
		else if(tmpString[i] == ';')
		{
			if(count != 4)
				return true;
			count = 0;
		}
	return false;
}

bool StatesListHandler::secondVerification(QVector<int> &states)
{
	for(int i = 0; i < states.size()/3; i++)
		if((states[i*3] < 0) || (states[i*3+1] < 0) || (states[i*3+1] > 2)
				|| (states[i*3+2] < 0) || (states[i*3+2] > 2))
			return 1;
	return 0;
}

bool StatesListHandler::recombination(QVector<int> &states, int &numberOfStates)
{
	QVector<int *> currentStates;
	int i  = 0;
	while(i < tmpString.size())
	{
		int* currentState = new int[5];
		for(int j = 0; j < 5; j++)
		{
			currentState[j] = 0;
			while(tmpString[i] <= '9' && tmpString[i] >= '0')
				currentState[j] = currentState[j]*10 + tmpString[i++].toLatin1()-'0';
			i++;
		}
		currentStates.prepend(currentState);
	}
	int max = 0;
	for(int i = 0; i < currentStates.size(); i++)
		if(currentStates[i][0] > max)
			max = currentStates[i][0];
	numberOfStates = max+1;
	states.resize((max+1)*sizeOfAlphabet*3);
	states.fill(0);
	for(int i = 0; i < currentStates.size(); i++)
	{
		if(currentStates[i][1] < 0 || currentStates[i][1] >= sizeOfAlphabet)
			return 1;
		int currentIndex = currentStates[i][0]*sizeOfAlphabet*3+currentStates[i][1]*3;
		states[currentIndex] = currentStates[i][2];
		states[currentIndex+1] = currentStates[i][3];
		states[currentIndex+2] = currentStates[i][4];
	}
	return 0;
}

bool StatesListHandler::handling(QVector<int> &states, int &numberOfStates)
{
	if(cleaning())
		return 1;
	if(firstVerification())
		return 1;
	if(recombination(states, numberOfStates))
	{
		states.clear();
		return 1;
	}
	if(secondVerification(states))
	{
		states.clear();
		return 1;
	}
	return 0;
}

Highlighter::Highlighter(QTextDocument *parent) : QSyntaxHighlighter(parent)
{
  HighlightingRule rule;
  keywordFormat.setForeground(Qt::darkBlue);
  keywordFormat.setFontWeight(QFont::Bold);
  QStringList keywordPatterns;

  numericConstantFormat.setForeground(Qt::green);
  rule.pattern = QRegExp("\\b[0-9]+\\b");
  rule.format = numericConstantFormat;
  highlightingRules.append(rule);

  variableFormat.setForeground(Qt::blue);
  rule.pattern = QRegExp("\\b[A-Za-z][A-Za-z0-9_]*\\b");
  rule.format = variableFormat;
  highlightingRules.append(rule);
  keywordPatterns << "\\bint\\b"<< "\\bbool\\b"<< "\\bfor\\b" << "\\bdo\\b"
                  << "\\bwhile\\b" << "\\bif\\b" << "\\breturn\\b" << "\\belse\\b";
  foreach (const QString &pattern, keywordPatterns)
  {
	rule.pattern = QRegExp(pattern);
	rule.format = keywordFormat;
	highlightingRules.append(rule);
  }
  highlightingRules.append(rule);
  singleLineCommentFormat.setForeground(Qt::red);
  rule.pattern = QRegExp("//[^\n]*");
  rule.format = singleLineCommentFormat;
  highlightingRules.append(rule);
  multiLineCommentFormat.setForeground(Qt::red);
  highlightingRules.append(rule);
  commentStartExpression = QRegExp("/\\*");
  commentEndExpression = QRegExp("\\*/");
}

void Highlighter::highlightBlock(const QString &text)
{
  foreach (const HighlightingRule &rule, highlightingRules)
  {
	QRegExp expression(rule.pattern);
	int index = expression.indexIn(text);
	while (index >= 0)
	{
	  int length = expression.matchedLength();
	  setFormat(index, length, rule.format);
	  index = expression.indexIn(text, index + length);
	}
  }
  setCurrentBlockState(0);
  int startIndex = 0;
  if (previousBlockState() != 1)
	startIndex = commentStartExpression.indexIn(text);
  while (startIndex >= 0)
  {
	int endIndex = commentEndExpression.indexIn(text, startIndex);
	int commentLength;
	if (endIndex == -1)
	{
	  setCurrentBlockState(1);
	  commentLength = text.length() - startIndex;
	}
	else
	  commentLength = endIndex - startIndex + commentEndExpression.matchedLength();
	setFormat(startIndex, commentLength, multiLineCommentFormat);
	startIndex = commentStartExpression.indexIn(text, startIndex + commentLength);
  }
}
