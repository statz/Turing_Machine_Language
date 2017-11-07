#include "parser.h"
#include <iostream>
#include <QFile>
#include <QTextStream>

namespace a
{
int numberOfDigits;
vector<string> simplifiedCode;
int numberOfTmpVal = 0;
string getStringIndex(int);
}

using namespace a;

Parser::Parser(string pathToFile)
{
	input(pathToFile);
}

int Parser::input(string pathToFile)
{
    QFile inputFile(QString::fromStdString(pathToFile));
	inputFile.open(QIODevice::ReadOnly | QIODevice::Text);
	QTextStream input(&inputFile);
	string *currentLine = new string();
	char buff = 0;
	int scCount = 0;
	while(true)
	{
		string lexem;
		if(input.atEnd())
			return 0;
		if(!buff)
			input>>buff;
		if(((buff >= 'a') && (buff <= 'z')) || ((buff >= 'A') && (buff <= 'Z')))
		{
			do
			{
				lexem.push_back(buff);
				input>>buff;
			}
			while(((buff >= 'a') && (buff <= 'z')) || ((buff >= 'A') && (buff <= 'Z'))
						|| (buff == '_') || ((buff >= '0') && (buff <= '9')));
            if((lexem == "true") ||(lexem == "false")||(lexem == "return"))
				currentLine->append(lexem);
			else if ((lexem == "int")||(lexem == "bool"))
				currentLine->append(lexem+' ');
			else if(lexem == "do")
			{
				currentLine->append(lexem);
				code.push_back(*currentLine);
				currentLine = new string();
			}
			else if((lexem == "while") || (lexem == "if") || (lexem == "for"))
			{
				currentLine->append(lexem+buff);
				int count = 1;
				do
				{
					input>>buff;
					if(input.atEnd())
					{
						return 1;
					}
					if(buff == '(')
						count++;
					else if(buff == ')')
						count--;
                    if((buff != ' ')&&(buff != '\n')&&(buff != '\r')&&(buff != '\t'))
                      currentLine->push_back(buff);
				}
				while(count);
				code.push_back(*currentLine);
				currentLine = new string();
				buff = 0;
				input>>buff;
				if(buff == ';')
					buff = 0;
			}
			else
				currentLine->append(lexem);
		}
		else if((buff == '(')||(buff == ')'))
		{
			currentLine->push_back(buff);
			buff = 0;
		}
		else if((buff == '{')||(buff == '}')||(buff == ';'))
		{
			if(buff == '{')
				scCount++;
			if(buff == '}')
				scCount--;
			if(scCount < 0)
			{
				return 1;
			}
			if(buff != ';')
				currentLine->push_back(buff);
			code.push_back(*currentLine);
			currentLine = new string();
			buff = 0;
		}
		else if((buff != ' ')&&(buff != '\n')&&(buff != '\r')&&(buff != '\t'))
		{
			currentLine->push_back(buff);
			buff = 0;
		}
		else
			buff = 0;
	}
	return 0;
}

void Parser::parsing()
{
	createAListOfVariables();
	substitute();
	list<string>::iterator it = code.begin();
	it = code.begin();
	ListOfStatements* l = createListOfStatements(it);
	Block b(l);
	b.simplification();
}

vector<string> &Parser::getSimplifiedCode()
{
	return simplifiedCode;
}

vector<Array> *Parser::getListOfArraies()
{
	return &listOfArraies;
}

vector<Variable> *Parser::getListOfVariables()
{
	return &listOfVariables;
}

int Parser::getNumberOfTmpVal()
{
	return numberOfTmpVal;
}

int Parser::getNumberOfDigits()
{
	return a::numberOfDigits;
}

bool Parser::createAListOfVariables()
{
	for(list<string>::iterator it = code.begin(); it != code.end(); it++)
	{
		string currentLine = *it;
		string lexem;
		size_t j = 0;
		int flag = false;
		while(j < currentLine.size())
		{
			lexem.clear();
			if(((currentLine[j] >= 'a') && (currentLine[j] <= 'z')) || ((currentLine[j] >= 'A') && (currentLine[j] <= 'Z')) || (currentLine[j] == '_'))
			{
				do
				{lexem.push_back(currentLine[j++]);}
				while(((currentLine[j] >= 'a') && (currentLine[j] <= 'z')) || ((currentLine[j] >= 'A') && (currentLine[j] <= 'Z'))
							|| (currentLine[j] == '_') || ((currentLine[j] >= '0') && (currentLine[j] <= '9')));
				switch(specification(lexem))
				{
				case 1:
					if(lexem == "int")
						flag = 1;
					else
						flag = 2;
					break;
				case 2:
                    if(flag)
					{
                        if(currentLine[j] != '[')
						{
							Variable* variable = new Variable(lexem, flag);
							listOfVariables.push_back(*variable);
						}
						else
						{
                            j++;
							int a = 0;
							int b = 1;
							while(currentLine[j] != ']')
							{
								a = a*b+(currentLine[j]-'0');
								b *= 10;
								j++;
							}
							Array *array = new Array(lexem, flag, a);
							listOfArraies.push_back(*array);
						}
					}
					else if((!searchInListOfVar(lexem)) && (!searchInListOfArr(lexem)))
						return 1;
				}
			}
			else
            {
                j++;
                if(currentLine[j-1] == '=')
                    flag = 0;
            }
		}
	}
	return 0;
}

int Parser::specification(const string &lexem)
{
	if((lexem == "break")||(lexem == "continue")||(lexem == "true")||(lexem == "false")||(lexem == "return")
		 ||(lexem == "while") || (lexem == "if") || (lexem == "for") || (lexem == "do") || (lexem == "else"))
		return 0;
	else if((lexem == "int")||(lexem == "bool"))
		return 1;
	else return 2;
}

int Parser::searchInListOfVar(const string &lexem)
{
	for(size_t i = 0; i < listOfVariables.size(); i++)
		if(lexem == listOfVariables[i].name)
			return i+1;
	return false;
}

int Parser::searchInListOfArr(const string &lexem)
{
	for(size_t i = 0; i < listOfArraies.size(); i++)
		if(lexem == listOfArraies[i].name)
			return i+1;
	return false;
}

string a::getStringIndex(int number)
{
	string output;
	output.resize(numberOfDigits);
	int i = numberOfDigits-1;
	int a = 10, b = 1;
	do
	{
		output[i--] = (number%a/b)+'0';
		a *= 10;
		b *= 10;
	}
	while(a < number);
	while(i >= 0)
		output[i--] = '0';
	return output;
}

void Parser::substitute()
{
	for(list<string>::iterator it = code.begin(); it != code.end(); it++)
	{
		string currentLine = *it;
		string lexem;
		string* newVersion;
		size_t j = 0;
		numberOfDigits = 1;
		for(size_t i = 10; i < (listOfVariables.size() >= listOfArraies.size() ?
														listOfVariables.size() : listOfArraies.size()) ; i *= 10)
			numberOfDigits++;
		newVersion = new string();
		while(j < currentLine.size())
		{
			lexem.clear();
			if(((currentLine[j] >= 'a') && (currentLine[j] <= 'z')) || ((currentLine[j] >= 'A') && (currentLine[j] <= 'Z')))
			{
				do
				{
					lexem.push_back(currentLine[j]);
					j++;
				}
				while(((currentLine[j] >= 'a') && (currentLine[j] <= 'z')) || ((currentLine[j] >= 'A') && (currentLine[j] <= 'Z'))
							|| (currentLine[j] == '_') || ((currentLine[j] >= '0') && (currentLine[j] <= '9')));
				if(specification(lexem) == 2)
				{
					int rez = searchInListOfVar(lexem);
					if(rez)
						newVersion->append("var"+getStringIndex(rez));
					else
					{
						rez = searchInListOfArr(lexem);
						newVersion->append("arr"+getStringIndex(rez));
					}
				}
				else
					newVersion->append(lexem);
			}
			else
				newVersion->push_back(currentLine[j++]);
		}
		it = code.erase(it);
		it = code.insert(it, *newVersion);
	}
}

ListOfStatements *Parser::createListOfStatements(list<string>::iterator &it)
{
	ListOfStatements *listOfStatements = new ListOfStatements();
	while(true)
	{
		string tmp = *it;
		if((tmp[0] == 'v') || (tmp[0] == 'a'))
		{
			Expression *expression = new Expression(tmp);
			listOfStatements->push(expression);
			it++;
		}
		else if((tmp[0] == 'i') &&(tmp[1] == 'n'))
		{
            size_t rez = tmp.find('=');
			if(rez != std::string::npos)
			{
				string str(tmp, 4, tmp.size()-4);
				Expression *expression = new Expression(str);
				listOfStatements->push(expression);
			}
			it++;
		}
		else if((tmp[0] == 'b') &&(tmp[1] == 'o'))
		{
            size_t rez = tmp.find('=');
			if(rez != std::string::npos)
			{
				string str(tmp, 5, tmp.size()-5);
				Expression *expression = new Expression(str);
				listOfStatements->push(expression);
			}
			it++;
		}
		else if((tmp[0] == 'i') &&(tmp[1] == 'f'))
		{
			string expression(tmp, 3, tmp.size()-4);
			it++;
			Expression *logExpr = new Expression(expression);
			ListOfStatements *tmpLOSI = createListOfStatements(it);//тело if
			Ramification *ram;
			if((*it)[0] == 'e')
			{
				it++;
				ListOfStatements *tmpLOSE = createListOfStatements(it);//тело else
				ram = new Ramification(logExpr, tmpLOSI, tmpLOSE);
			}
			else
				ram = new Ramification(logExpr, tmpLOSI);
			listOfStatements->push(ram);
		}
		else if(tmp[0] == 'd')
		{
			it++;
			ListOfStatements *tmpLOS = createListOfStatements(it);
			tmp = *it;
			string expression(tmp, 6, tmp.size()-7);
			Expression *logExpr = new Expression(expression);
			CycleDo *cd = new CycleDo(logExpr, tmpLOS);
			listOfStatements->push(cd);
			it++;
		}
		else if(tmp[0] == 'w')
		{
			tmp = *it;
			Expression *logExpr;
			string expression(tmp, 6, tmp.size()-7);
			logExpr = new Expression(expression);
			it++;
			ListOfStatements *tmpLOS = createListOfStatements(it);
			CycleWhile *cw = new CycleWhile(logExpr, tmpLOS);
			listOfStatements->push(cw);
		}
		else if(tmp[0] == 'f')
		{
			int i = 4;
			string firstStr;
			while(tmp[i] != ';')
				firstStr.push_back(tmp[i++]);
			i++;
			string secondStr;
			while(tmp[i] != ';')
				secondStr.push_back(tmp[i++]);
			i++;
			string thirdStr;
			while(tmp[i] != ')')
				thirdStr.push_back(tmp[i++]);
			Expression *firstExpr = new Expression(firstStr);
			Expression *logExpr = new Expression(secondStr);
			Expression *secondExpr = new Expression(thirdStr);
			it++;
			ListOfStatements *tmpLOS = createListOfStatements(it);
			CycleFor *cf = new CycleFor(firstExpr, logExpr, secondExpr, tmpLOS);
			listOfStatements->push(cf);
		}
		else if(tmp[0] == 'r')
		{
			OperatorReturn *opr = new OperatorReturn();
			listOfStatements->push(opr);
			it++;
		}
		else if(tmp[0] == '}')
		{
			it++;
			break;
		}
		else// '{' или объявление
			it++;
	}
	return listOfStatements;
}


void ListOfStatements::push(Statement *dat)
{
	if(head)
		last->next = new Link(dat), last = last->next;
	else
		head = new Link(dat), last = head;
}

Statement *ListOfStatements::popFront()
{
	if(head)
	{
		Statement *a = head->data;
		head = head->next;
		return a;
	}
	return 0;
}

string Statement::intToString(int n)
{
	string tmp;
	int a, b;
	a = 10;
	b = 1;
	do
	{
		tmp.insert(tmp.begin(), (char)((n%a/b)+'0'));
		a *= 10;
		b *= 10;
	}
	while(b<=n);
	return tmp;
}

Block::Block()
{
}

Block::Block(ListOfStatements *body)
{
	this->body = body;
}

void Block::simplification()
{
	int beginOfCycle = simplifiedCode.size();
	bodySimplification();
	correcting(beginOfCycle);
	string *tmp = new string("end");
	simplifiedCode.push_back(*tmp);
}

void Block::correcting(int beginOfCycle)
{
	for(size_t i = beginOfCycle; i < simplifiedCode.size(); i++)
		if(simplifiedCode[i] == "return")
			simplifiedCode[i] = "goto "+intToString(simplifiedCode.size());
}

void Block::bodySimplification()
{
	while(true)
	{
		Statement *tmp = body->popFront();
		if(tmp)
			tmp->simplification();
		else
			break;
	}
}

CycleFor::CycleFor(Expression *firstExpression, Expression *condition, Expression *secondExpression, ListOfStatements *body)
{
	this->firstExpression = firstExpression;
	this->condition = condition;
	this->secondExpression = secondExpression;
	this->body = body;
}

void CycleFor::simplification()
{
	firstExpression->simplification();
    int tmp = simplifiedCode.size();
    string *tmp0 = new string("goto ");
    simplifiedCode.push_back(*tmp0);
	int beg = simplifiedCode.size();
	bodySimplification();
	secondExpression->simplification();
    int cend = simplifiedCode.size();;
	condition->simplification();
    string *tmp1 = new string("if tmp"+getStringIndex(1)+" goto "+intToString(beg));
	simplifiedCode.push_back(*tmp1);
	string *tmp2 = new string("else goto "+intToString(simplifiedCode.size()+1));
	simplifiedCode.push_back(*tmp2);
    simplifiedCode[tmp] += intToString(cend);
}


CycleWhile::CycleWhile(Expression *condition, ListOfStatements *body)
{
	this->condition = condition;
	this->body = body;
}

void CycleWhile::simplification()
{
	int beg = simplifiedCode.size();
	condition->simplification();
    string *tmp1 = new string("if tmp"+getStringIndex(1)+" goto "+intToString(simplifiedCode.size()+2));
	simplifiedCode.push_back(*tmp1);
	int tmp = simplifiedCode.size();
	string *tmp2 = new string("else goto ");
	simplifiedCode.push_back(*tmp2);
	bodySimplification();
	simplifiedCode[tmp].append(intToString(simplifiedCode.size()));
    string *tmp3 = new string("goto " + intToString(beg));
    simplifiedCode.push_back(*tmp3);
}


CycleDo::CycleDo(Expression *condition, ListOfStatements *body)
{
	this->condition = condition;
	this->body = body;
}

void CycleDo::simplification()
{
	int beg = simplifiedCode.size();
	bodySimplification();
	condition->simplification();
	string *tmp1 = new string("if tmp"+getStringIndex(1)+" goto "+intToString(beg));
	simplifiedCode.push_back(*tmp1);
	string *tmp2 = new string("else goto "+intToString(simplifiedCode.size()+1));
	simplifiedCode.push_back(*tmp2);
}


Ramification::Ramification(Expression *condition, ListOfStatements *body)
{
	this->condition = condition;
	this->body = body;
	this->elseBlock = 0;
}

Ramification::Ramification(Expression *condition, ListOfStatements *body, ListOfStatements *elseBlock)
{
	this->condition = condition;
	this->body = body;
	this->elseBlock = elseBlock;
}

void Ramification::simplification()
{
	condition->simplification();
    string *tmp = new string("if tmp"+getStringIndex(1)+" goto "+intToString(simplifiedCode.size()+2));
	simplifiedCode.push_back(*tmp);
	tmp = new string;
	simplifiedCode.push_back(*tmp);
	int tmp1 = simplifiedCode.size()-1;
	bodySimplification();
	if(elseBlock)
	{
        simplifiedCode[tmp1]="else goto "+intToString(simplifiedCode.size()+1);
		int tmp2 = simplifiedCode.size();
		tmp = new string();
		simplifiedCode.push_back(*tmp);
		while(true)
		{
			Statement *tmp = elseBlock->popFront();
			if(tmp)
				tmp->simplification();
			else
				break;
		}
        simplifiedCode[tmp2]="goto "+intToString(simplifiedCode.size());
	}
	else
        simplifiedCode[tmp1]="else goto "+intToString(simplifiedCode.size());
}


Expression::Expression(string expression)
{
	this->expression = expression;
}

void Expression::simplification()
{
	fromReversePolishNotation(toReversePolishNotation());
}

int priority (string op)
{
	return op[0] < 0 ? 1 :op == "." ? 5 :op == "_" || op == "!" ? 4  : op == "=" ? 0 :op == "+" || op == "-" ? 2 :op == "*" || op == "/" ? 3 : op == "<" || op == ">"|| op == "<="|| op == ">="|| op == "=="|| op == "!=" || op == "||" || op == "&&" ? 1 :-1;
}

bool isPartOfVar(char c)
{
	return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'))||(c == '_') || ((c >= '0') && (c <= '9'));
}

string Expression::toReversePolishNotation()
{
	bool mayUnary = true;
	string outputString;
	vector<string> operatorsStack;
	for (size_t i=0; i<expression.length(); i++)
	{
		if (expression[i] == '(')
		{
			operatorsStack.push_back ("(");
			mayUnary = true;
		}
		else if (expression[i] == '[')
		{
			operatorsStack.push_back(".");
			operatorsStack.push_back ("(");
			mayUnary = true;
		}
		else if ((expression[i] == ')')||(expression[i] == ']'))
		{
			while(operatorsStack.back() != "(")
			{
				outputString.append(operatorsStack.back()+' ');
				operatorsStack.pop_back();
			}
			operatorsStack.pop_back();
			mayUnary = false;
		}
		else if(!isPartOfVar(expression[i]))
		{
			string curop;
			curop.push_back(expression[i]);
			if((expression[i+1] == '=') ||(expression[i+1] == '|') ||(expression[i+1] == '&'))
			{
				curop.push_back(expression[i+1]);
				i++;
			}
			else if (mayUnary && curop[0] == '-')
				curop[0] = '_';
			while (!operatorsStack.empty() && (priority(operatorsStack.back()) >= priority(curop)))
				outputString.append(operatorsStack.back()+' '), operatorsStack.pop_back();
			if(curop.size() == 2)
			{
				curop[0] = -curop[0];
				curop[1] = -curop[1];
			}
			curop = curop;
			operatorsStack.push_back(curop);
			mayUnary = true;
		}
		else
		{
			string operand;
			while (i < expression.length() && isPartOfVar(expression[i]))
				operand += expression[i++];
			i--;
			outputString.append(operand+' ');
			mayUnary = false;
		}
	}
	while (!operatorsStack.empty())
		outputString.append(operatorsStack.back()+' '), operatorsStack.pop_back();
	return outputString;
}


void Expression::fromReversePolishNotation(string str)
{
	int lastTmpIndex = 1;
	vector<string> varStack;
	size_t i = 0;
	while(i < str.size())
	{
		if(str[i] == ' ')
		{
			i++;
			continue;
		}

		if((str[i] == 'v')||(str[i] == 'a'))
		{
			string *tmp = new string();
			size_t j;
			for(j = i; j < i+3+numberOfDigits; j++)
				tmp->push_back(str[j]);
			varStack.push_back(*tmp);
			i = j;
		}
		else if((str[i] >= '0')&&(str[i] <= '9'))
		{
			string *tmp = new string;
			while((str[i] >= '0')&&(str[i] <= '9'))
				tmp->push_back(str[i++]);
			varStack.push_back(*tmp);
		}
		else if((str[i] == '_')||(str[i] == '!'))
		{
			string *tmpName = new string("tmp"+getStringIndex(lastTmpIndex++));
            if(lastTmpIndex-1 > numberOfTmpVal)
                numberOfTmpVal = lastTmpIndex-1;
			string *tmp = new string;
			*tmp = *tmpName;
			tmp->push_back('=');
			if(str[i] == '!')
				tmp->push_back('!');
			else
				tmp->push_back('-');
			tmp->append(varStack.back());
			if(varStack.back()[0] == 't')
				lastTmpIndex--;
			else if((varStack.back()[0] == 'a') &&(varStack.back()[4+numberOfDigits] == 't'))
				lastTmpIndex--;
			varStack.pop_back();
			simplifiedCode.push_back(*tmp);
			varStack.push_back(*tmpName);
			i++;
		}
		else if((str[i]=='.'))
		{
			string tmp1 = varStack.back();
			varStack.pop_back();
			string tmp2 = varStack.back();
			varStack.pop_back();
			if((tmp1[0] != 't') && !((tmp1[0] >= '0')&&(tmp1[0] <= '9')))
			{
				string *tmp3 = new string("tmp"+getStringIndex(lastTmpIndex)+'='+tmp1);
				simplifiedCode.push_back(*tmp3);
				tmp3 = new string(tmp2+"[tmp"+getStringIndex(lastTmpIndex)+']');
				varStack.push_back(*tmp3);
			}
			else
			{
				string *tmp3 = new string(tmp2+'['+tmp1+']');
				varStack.push_back(*tmp3);
			}
			i++;
		}
		else if(str[i] == '=')
		{
			if(!simplifiedCode.empty())
			{
				string *tmp1 = new string;
				if(varStack.back()[0] == 't')
				{
					varStack.pop_back();
					tmp1->append(varStack.back());
					varStack.pop_back();
					string tmp = simplifiedCode.back();
					simplifiedCode.pop_back();
					size_t j = 0;
					while(tmp[j] != '=')
						j++;
					for( ; j < tmp.size(); j++)
						tmp1->push_back(tmp[j]);
				}
				else
				{
					string tmp2 = varStack.back();
					varStack.pop_back();
					string tmp3 = varStack.back();
					varStack.pop_back();
					tmp1->append(tmp3);
					tmp1->push_back('=');
					tmp1->append(tmp2);
				}
				simplifiedCode.push_back(*tmp1);
			}
			else
			{
				string tmp1 = varStack.back();
				varStack.pop_back();
				string tmp2 = varStack.back();
				varStack.pop_back();
				string *tmp = new string(tmp2+"="+tmp1);
				simplifiedCode.push_back(*tmp);
			}
			i++;
		}
		else
		{
			string op;
			op.push_back(str[i]);
			if(str[i] < 0)
			{
				op[0] = -op[0];
				op.push_back(-str[++i]);
			}
			string var1 = varStack.back();
			if(varStack.back()[0] == 't')
				lastTmpIndex--;
			else if((varStack.back()[0] == 'a') &&(varStack.back()[4+numberOfDigits] == 't'))
				lastTmpIndex--;
			varStack.pop_back();
			string var2 = varStack.back();
			if(varStack.back()[0] == 't')
				lastTmpIndex--;
			else if((varStack.back()[0] == 'a') &&(varStack.back()[4+numberOfDigits] == 't'))
				lastTmpIndex--;
			varStack.pop_back();
			string *tmpName = new string("tmp"+getStringIndex(lastTmpIndex++));
            if(lastTmpIndex-1 > numberOfTmpVal)
                numberOfTmpVal = lastTmpIndex-1;
			string *tmp = new string;
			*tmp = *tmpName;
			tmp->push_back('=');
			tmp->append(var2);
			tmp->append(op);
			tmp->append(var1);
			simplifiedCode.push_back(*tmp);
			varStack.push_back(*tmpName);
			i++;
		}
	}
}

void OperatorReturn::simplification()
{
	string *tmp = new string("return");
	simplifiedCode.push_back(*tmp);
}
