#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <list>
#include <vector>

using namespace std;

class Parser;
class Variable;
class Array;
class ListOfStatements;
class Statement;
class Block;
class CycleFor;
class CycleWhile;
class CycleDo;
class Ramification;
class Expression;
class OperatorReturn;

class Parser
{
public:
  Parser(string);
  void parsing();
  vector<string> &getSimplifiedCode();
  vector<Array> *getListOfArraies();
  vector<Variable> *getListOfVariables();
  int getNumberOfTmpVal();
  int getNumberOfDigits();
private:
  int input(string);
private:
  bool createAListOfVariables();
  int specification(const string &);
  int searchInListOfVar(const string &);
  int searchInListOfArr(const string &);
  void substitute();
  ListOfStatements *createListOfStatements(list<string>::iterator &);
private:
  vector<Variable> listOfVariables;
  vector<Array> listOfArraies;
  list<string> code;
};

class Variable
{
public:
  Variable(const string& a, const int& b)
  {
	name = a;
	type = b;
  }
  string name;
  int type;
};

class Array
{
public:
  Array(string &a, int& b, int& c) : name(a), type(b), length(c){}
  string name;
  int type;
  int length;
};

class ListOfStatements
{
private:
  struct Link
  {
	Statement *data;
	Link *next;
	Link(Statement *dat) : data(dat), next(0){}
  };
  Link *head;
  Link *last;
public:
  ListOfStatements() : head(0){}
  void push(Statement *);
  Statement *popFront();
};

class Statement
{
public:
  virtual void simplification() = 0;
  string intToString(int n);
};

class Block : public Statement
{
public:
  Block();
  Block(ListOfStatements* );
  void simplification();
  void correcting(int);
protected:
  ListOfStatements *body;
  void bodySimplification();
};

class CycleFor : public Block
{
public:
  CycleFor(Expression *, Expression *, Expression *, ListOfStatements *);
  void simplification();
private:
  Expression *firstExpression;
  Expression *condition;
  Expression *secondExpression;
};

class CycleWhile : public Block
{
public:
  CycleWhile(Expression *, ListOfStatements *);
  void simplification();
private:
  Expression *condition;
};

class CycleDo : public Block
{
public:
  CycleDo(Expression *, ListOfStatements *);
  void simplification();
private:
  Expression *condition;
};

class Ramification : public Block
{
public:
  Ramification(Expression *, ListOfStatements *);
  Ramification(Expression *, ListOfStatements *, ListOfStatements *);
  void simplification();
private:
  Expression *condition;
  ListOfStatements *elseBlock;
};

class Expression : public Statement
{
public:
  Expression(string);
  void simplification();
private:
  string toReversePolishNotation();
  void fromReversePolishNotation(string);
private:
  string expression;
};



class OperatorReturn : public Statement
{
public:
  void simplification();
};

#endif // PARSER_H
