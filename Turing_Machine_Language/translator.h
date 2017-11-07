#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <parser.h>
#include <iostream>
#include <QVector>
#include <map>

using namespace std;

class Variable;
class Array;
class Parser;

class Translator
{
public:
    Translator(string);
    ~Translator();
    void translate();
    QVector<int> getTape(int &) const;
    vector<int *> getMachineCode() const;
    vector<string> getAssemblyCode() const;
    vector<Variable> *getListOfVariables() const;
    vector<Array> *getListOfArraies() const;
    int getNumberOfTmpVariables() const;
    int getNumberOfDigits() const;
    map<string, int> getTapeCoordinates() const;
    vector<int> getComplianceTeamToState() const;
private:
    void translateToAssemblyCode(vector<string> &);
    int countCoordinate(map<string, int> tapeCoordinates, string lexem);
    void createMachineCode();
    void createTape(vector<Array> &, int, int);
    string intToString(int);
    int stringToNumber(string);
    string getStringIndex(int);
    string decToBin(int);
    void moveCreation(int, int);
    void cutCreation(int, int);
    void copyCreation(int, int);
    void findCurrentDigit();
    void createRightTransitionOnNBlocks(int n);
    void createLeftTransitionOnNBlocks(int n);
    void correcting();
    void cleaning();
    void createRightShift();
    void summary();
    void subtraction();
    void multiplication();
    void division();
    void logicalNot();
    void numericNot(int reg);
    void initialize(int, string);
    void pop(int);
    void push(int);
    void findCurrentElement();
    void shiftToArrayBeginning();
    void compareOperation(QString, int k);
    void conditionCreation();
    bool isPartOfVar(char c);
private:
    Parser *parser;
    vector<string> assemblyCode;
    vector<int *> machineCode;
    map<string, int> tapeCoordinates;
    vector<int> complianceTeamToState;
    vector<int> complianceTeamToPosition;
    int *mapTrSmplCodeToAsmCode;
    int numberOfDigits;
    int numberOfCurrentState;
    int currentPosition;
    QVector<int> *tape;
    int beginOfTape;
};

#endif // TRANSLATOR_H
