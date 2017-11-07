#include "translator.h"
#include <math.h>
#include <QTextStream>
#include <QFile>
#include <QDebug>

Translator::Translator(string pathToFile)
{
    parser = new Parser(pathToFile);
    currentPosition = 0;
    numberOfCurrentState = 0;
}

Translator::~Translator()
{
    delete parser;
}

void Translator::translate()
{
    parser->parsing();
    numberOfDigits = parser->getNumberOfDigits();
    translateToAssemblyCode(parser->getSimplifiedCode());
    createMachineCode();
}

QVector<int> Translator::getTape(int &beginOfTape1) const
{
    beginOfTape1 = this->beginOfTape;
    return *tape;
}

vector<int *> Translator::getMachineCode() const
{
    return machineCode;
}

vector<string> Translator::getAssemblyCode() const
{
    return assemblyCode;
}

vector<Variable> *Translator::getListOfVariables() const
{
    return parser->getListOfVariables();
}

vector<Array> *Translator::getListOfArraies() const
{
    return parser->getListOfArraies();
}

int Translator::getNumberOfTmpVariables() const
{
    return parser->getNumberOfTmpVal();
}

int Translator::getNumberOfDigits() const
{
    return parser->getNumberOfDigits();
}

map<string, int> Translator::getTapeCoordinates() const
{
    return tapeCoordinates;
}

vector<int> Translator::getComplianceTeamToState() const
{
    return complianceTeamToState;
}


int Translator::stringToNumber(string s)
{
    int number = 0;
    size_t i = 0;
    if(s[0] == '-')
        i = 1;
    number = s[i++]-'0';
    for(; i < s.size(); i++)
        number = number*10+s[i]-'0';
    if(s[0] == '-')
        return -number;
    return number;
}

string Translator::intToString(int n)
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


string Translator::getStringIndex(int number)
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

string Translator::decToBin(int n)
{
    int count = 0;
    int tmp = 1;
    int modn = abs(n);
    while(tmp <= modn)
        tmp*=2, count++;
    string bin;
    for(int i = 0; i < 8; i++)
        bin.push_back('0');
    for(int i = 0, j = 7; i < count; i++, j--)
        bin[j] = ((modn>>i)&1) + '0';
    if(n < 0)
    {
        int i = 7;
        while(bin[i] == '0')
            i--;
        i--;
        while(i >= 0)
            if(bin[i] == '0')
                bin[i--] = '1';
            else
                bin[i--] = '0';
    }
    return bin;
}

bool Translator::isPartOfVar(char c)
{
    return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'))||(c == '_') || ((c >= '0') && (c <= '9'));
}

void Translator::translateToAssemblyCode(vector<string> &simplifiedCode)
{
    mapTrSmplCodeToAsmCode = new int[simplifiedCode.size()];
    for(size_t i = 0; i < simplifiedCode.size(); i++)
    {
        string *tmp;
        mapTrSmplCodeToAsmCode[i] = assemblyCode.size();
        string currentString = simplifiedCode[i];
        if((currentString[0] == 'a') || (currentString[0] == 'v') || (currentString[0] == 't'))
        {
            string receiver, firstOperand;
            size_t i = 0;
            while(currentString[i] != '=')
                receiver.push_back(currentString[i++]);
            i++;
            if((currentString[i] != '-')&&(currentString[i] != '!'))
            {
                while(isPartOfVar(currentString[i]) && (i < currentString.size()))
                    firstOperand.push_back(currentString[i++]);
                if(firstOperand[0] == 'a' && !((firstOperand[numberOfDigits+3+1] >= '0')&&(firstOperand[numberOfDigits+3+1] <= '9')))
                {
                    tmp = new string ("copy "+firstOperand.substr(numberOfDigits+4, firstOperand.size()-numberOfDigits-5)+' '
                                      +firstOperand.substr(0, 3+numberOfDigits)+"_adrreg");
                    assemblyCode.push_back(*tmp);
                    tmp = new string("pop "+firstOperand.substr(0, 3+numberOfDigits));
                    assemblyCode.push_back(*tmp);
                    firstOperand = firstOperand.substr(0, 3+numberOfDigits)+"_tmpreg";
                }
                if(i == currentString.size())
                {
                    if(receiver[0] == 'a')
                    {
                        if(!((receiver[numberOfDigits+3+1] >= '0')&&(receiver[numberOfDigits+3+1] <= '9')))
                        {
                            if((firstOperand[0] >= '0')&&(firstOperand[0] <= '9'))
                                tmp = new string("num_init "+receiver.substr(0, 3+numberOfDigits)+"_tmpreg "+decToBin(stringToNumber(firstOperand)));
                            else if(firstOperand[0] != 'a')
                                tmp = new string("copy "+firstOperand +' '+ receiver.substr(0, 3+numberOfDigits)+"_tmpreg");
                            if(firstOperand[0] != 'a')
                                assemblyCode.push_back(*tmp);
                            tmp = new string ("copy "+receiver.substr(numberOfDigits+4, receiver.size()-numberOfDigits-5)+' '
                                              +receiver.substr(0, 3+numberOfDigits)+"_adrreg");
                            assemblyCode.push_back(*tmp);
                            if(!((firstOperand[0] >= '0')&&(firstOperand[0] <= '9')))
                            {
                                tmp = new string("copy "+firstOperand+' '+receiver.substr(0, 3+numberOfDigits)+"_tmpreg");
                                assemblyCode.push_back(*tmp);
                            }
                            tmp = new string("push "+receiver.substr(0, 3+numberOfDigits));
                            assemblyCode.push_back(*tmp);
                        }
                        else//
                        {
                            if((firstOperand[0] >= '0')&&(firstOperand[0] <= '9'))
                                tmp = new string("num_init "+receiver+' '+decToBin(stringToNumber(firstOperand)));
                            else if(firstOperand[0] != 'a')
                                tmp = new string("copy "+firstOperand +' '+ receiver);
                            if(firstOperand[0] != 'a')
                                assemblyCode.push_back(*tmp);
                        }
                    }
                    else
                    {
                        if((firstOperand[0] >= '0')&&(firstOperand[0] <= '9'))
                            tmp = new string("num_init "+receiver+' '+decToBin(stringToNumber(firstOperand)));
                        else
                            tmp = new string("copy "+firstOperand+' '+receiver);
                        assemblyCode.push_back(*tmp);
                    }
                }
                else
                {
                    string secondOperand, op;
                    op.push_back(currentString[i++]);
                    if(currentString[i] == '=' || currentString[i] == '&' || currentString[i] == '|')
                        op.push_back(currentString[i++]);
                    while (i < currentString.size())
                        secondOperand.push_back(currentString[i++]);
                    if(secondOperand[0] == 'a' && !((secondOperand[numberOfDigits+3+1] >= '0')&&(secondOperand[numberOfDigits+3+1] <= '9')))
                    {
                        string *tmp;
                        tmp = new string ("copy "+secondOperand.substr(numberOfDigits+4, secondOperand.size()-numberOfDigits-5)+' '
                                          +secondOperand.substr(0, 4+numberOfDigits-1)+"_adrreg");
                        assemblyCode.push_back(*tmp);
                        tmp = new string("pop "+secondOperand.substr(0, 3+numberOfDigits));
                        assemblyCode.push_back(*tmp);
                        secondOperand = secondOperand.substr(0, 3+numberOfDigits)+"_tmpreg";
                    }
                    if((firstOperand[0] >= '0')&&(firstOperand[0] <= '9'))
                        tmp = new string("num_init freg "+decToBin(stringToNumber(firstOperand)));
                    else
                        tmp = new string("copy "+firstOperand+" freg");
                    assemblyCode.push_back(*tmp);
                    if((secondOperand[0] >= '0')&&(secondOperand[0] <= '9'))
                        tmp = new string("num_init sreg "+decToBin(stringToNumber(secondOperand)));
                    else
                        tmp = new string("copy "+secondOperand+" sreg");
                    assemblyCode.push_back(*tmp);
                    if(op == "==")
                        tmp = new string("cmp_eq");
                    else if(op == "!=")
                        tmp = new string("cmp_neq");
                    else if(op == "<")
                        tmp = new string("cmp_l");
                    else if(op == ">")
                        tmp = new string("cmp_m");
                    else if(op == "<=")
                        tmp = new string("cmp_loeq");
                    else if(op == ">=")
                        tmp = new string("cmp_moeq");
                    else if(op == "&&")
                        tmp = new string("and");
                    else if(op == "||")
                        tmp = new string("or");
                    else if(op == "+")
                        tmp = new string("summ");
                    else if(op == "-")
                        tmp = new string("min");
                    else if(op == "*")
                        tmp = new string("mult");
                    else if(op == "/")
                        tmp = new string("div");
                    assemblyCode.push_back(*tmp);
                    if(receiver[0] == 'a' && !((receiver[numberOfDigits+3+1] >= '0')&&(receiver[numberOfDigits+3+1] <= '9')))
                    {
                        tmp = new string ("copy "+receiver.substr(numberOfDigits+4, receiver.size()-numberOfDigits-5)
                                          +receiver.substr(0, 3+numberOfDigits)+"_adrreg");
                        assemblyCode.push_back(*tmp);
                        tmp = new string("copy "+receiver.substr(0, 3+numberOfDigits)+"_tmpreg treg");
                        assemblyCode.push_back(*tmp);
                        tmp = new string("push "+receiver.substr(0, 3+numberOfDigits));
                        assemblyCode.push_back(*tmp);
                    }
                    else
                    {
                        tmp = new string("cut treg "+receiver);
                        assemblyCode.push_back(*tmp);
                    }
                }
            }
            else
            {
                i++;
                while (i < currentString.size())
                    firstOperand.push_back(currentString[i++]);
                string* tmp;
                if((firstOperand[0] >= '0')&&(firstOperand[0] <= '9'))
                {
                    firstOperand = '-'+firstOperand;
                    tmp = new string("num_init "+receiver+' '+decToBin(stringToNumber(firstOperand)));
                    assemblyCode.push_back(*tmp);
                }
                else
                {
                    if(firstOperand[0] == 'a' && !((firstOperand[numberOfDigits+3+1] >= '0')&&(firstOperand[numberOfDigits+3+1] <= '9')))
                    {
                        tmp = new string ("copy "+firstOperand.substr(0, 3+numberOfDigits)+"_adrreg"
                                          +firstOperand.substr(numberOfDigits+4, firstOperand.size()-numberOfDigits-5));
                        assemblyCode.push_back(*tmp);
                        tmp = new string("pop "+firstOperand.substr(0, 3+numberOfDigits));
                        assemblyCode.push_back(*tmp);
                        firstOperand = firstOperand.substr(0, 3+numberOfDigits)+"_tmpreg";
                        tmp = new string("copy "+firstOperand+" freg");
                    }
                    else
                        tmp = new string("copy "+firstOperand+" freg");
                    assemblyCode.push_back(*tmp);
                    if(currentString[currentString.find('=')+1] == '-')
                        tmp = new string("not");
                    else
                        tmp = new string("lognot");
                    assemblyCode.push_back(*tmp);
                    tmp = new string("cut freg "+receiver);
                    assemblyCode.push_back(*tmp);
                }
            }
        }
        else if((currentString[0] == 'i'))
        {
            string *tmp = new string(currentString, 0, currentString.size());
            assemblyCode.push_back(*tmp);
            i++;
            tmp = new string(simplifiedCode[i], 0, simplifiedCode[i].size());
            assemblyCode.push_back(*tmp);
        }
        else if(currentString[0] == 'g')
        {
            string *tmp = new string(currentString, 0, currentString.size());
            assemblyCode.push_back(*tmp);
        }
        else if(currentString[0] == 'e')
        {
            string *tmp = new string("close");
            assemblyCode.push_back(*tmp);
        }
    }
    for(size_t i = 0; i < assemblyCode.size(); i++)
    {
        string currentString = assemblyCode[i];
        if((currentString[0] == 'i')||(currentString[0] == 'e')||(currentString[0] == 'g'))
        {
            int k = currentString.find_last_of(' ');
            string *tmp = new string(currentString, 0, k+1);
            k++;
            tmp->append(intToString(mapTrSmplCodeToAsmCode[stringToNumber(currentString.substr(k))]));
            assemblyCode[i] = *tmp;
        }
    }
}

/*
    num_init+
    not
    lognot
    copy+
    cut+
    pop_heap
    push
    cmp_eq//==
    cmp_neq//!=
    cmp_l//<
    cmp_m//>
    cmp_loeq//<=
    cmp_moeq//>=
    and
    or
    summ
    min
    mult
    div
*/


int Translator::countCoordinate(map<string, int> coordinates, string lexem)
{
    if(!(lexem[3+numberOfDigits] == '['))
        return coordinates[lexem];
    else
        return coordinates[lexem.substr(0, 3+numberOfDigits)]+stringToNumber(lexem.substr(4+numberOfDigits, lexem.size()-5-numberOfDigits));
}

void Translator::createMachineCode()
{
    vector<int *> gotoCoordinates;
    vector<Array> *listOfArraies = parser->getListOfArraies();
    vector<Variable> *listOfVariables = parser->getListOfVariables();
    int nOTV = parser->getNumberOfTmpVal();
    createTape(*listOfArraies, listOfVariables->size(), nOTV);
    numberOfDigits = parser->getNumberOfDigits();
    for(int i = nOTV-1; i >= 0; i--)
        tapeCoordinates["tmp"+getStringIndex(i+1)] = -i-4;
    tapeCoordinates["freg"] = -3;
    tapeCoordinates["sreg"] = -2;
    tapeCoordinates["treg"] = -1;
    for(size_t i = 1; i <= listOfVariables->size(); i++)
        tapeCoordinates["var"+getStringIndex(i)] = i-1;
    int tmp = listOfVariables->size();
    for(size_t i = 0; i < listOfArraies->size(); i++)
    {
        string name = "arr"+getStringIndex(i+1);
        tapeCoordinates[name+"_tmpreg"] = tmp++;
        tapeCoordinates[name+"_adrreg"] = tmp++;
        tapeCoordinates[name] = tmp++;
        tmp += listOfArraies->at(i).length;
    }
    createRightShift();
    for(size_t i = 0; i < assemblyCode.size(); i++)
    {
        complianceTeamToState.push_back(numberOfCurrentState);
        complianceTeamToPosition.push_back(currentPosition);
        if((assemblyCode[i][0] == 'n')&&(assemblyCode[i][1] == 'u'))
        {
            string reciever;
            int j = 9;
            while(assemblyCode[i][j] != ' ')
                reciever.push_back(assemblyCode[i][j++]);
            j++;
            string number(assemblyCode[i], j);
            initialize(countCoordinate(tapeCoordinates, reciever), number);
        }
        else if((assemblyCode[i][0] == 'n')&&(assemblyCode[i][1] == 'o'))
            numericNot(-3);
        else if((assemblyCode[i][0] == 'c')&&(assemblyCode[i][1] == 'o'))
        {
            string operand;
            int j = 5;
            while(assemblyCode[i][j] != ' ')
                operand.push_back(assemblyCode[i][j++]);
            j++;
            string reciever(assemblyCode[i], j);
            copyCreation(countCoordinate(tapeCoordinates,operand), countCoordinate(tapeCoordinates,reciever));
        }
        else if((assemblyCode[i][0] == 'c')&&(assemblyCode[i][1] == 'u'))
        {
            string operand;
            int j = 4;
            while(assemblyCode[i][j] != ' ')
                operand.push_back(assemblyCode[i][j++]);
            j++;
            string reciever(assemblyCode[i], j);
            cutCreation(countCoordinate(tapeCoordinates,operand), countCoordinate(tapeCoordinates,reciever));
        }
        else if(assemblyCode[i][0] == 's')
            summary();
        else if((assemblyCode[i][0] == 'c')&&(assemblyCode[i][1] == 'l'))
        {
            int *arr = new int[5];
            arr[0] = numberOfCurrentState, arr[1] = 0, arr[2] = 0, arr[3] = 0, arr[4] = 2, machineCode.push_back(arr);
            arr = new int[5];
            arr[0] = numberOfCurrentState, arr[1] = 1, arr[2] = 0, arr[3] = 1, arr[4] = 2, machineCode.push_back(arr);
            arr = new int[5];
            arr[0] = numberOfCurrentState, arr[1] = 2, arr[2] = 0, arr[3] = 2, arr[4] = 2, machineCode.push_back(arr);
            numberOfCurrentState++;
        }
        else if((assemblyCode[i][0] == 'm')&&(assemblyCode[i][1] == 'i'))
            subtraction();
        else if((assemblyCode[i][0] == 'm')&&(assemblyCode[i][1] == 'u'))
            multiplication();
        else if((assemblyCode[i][0] == 'p')&&(assemblyCode[i][1] == 'u'))
            push(countCoordinate(tapeCoordinates, assemblyCode[i].substr(5))-1);
        else if((assemblyCode[i][0] == 'p')&&(assemblyCode[i][1] == 'o'))
            pop(countCoordinate(tapeCoordinates, assemblyCode[i].substr(4))-1);
        else if((assemblyCode[i][0] == 'c')&&(assemblyCode[i][1] == 'm'))
        {
            if(assemblyCode[i][4] == 'e')
                compareOperation("://tm_code/eqv.txt", 24);
            else if(assemblyCode[i][4] == 'n')
                compareOperation("://tm_code/neqv.txt", 25);
            else if((assemblyCode[i][4] == 'l')&&(assemblyCode[i].size() == 5))
                compareOperation("://tm_code/less.txt", 26);
            else if((assemblyCode[i][4] == 'm')&&(assemblyCode[i].size() == 5))
                compareOperation("://tm_code/more.txt", 26);
            else if((assemblyCode[i][4] == 'l')&&(assemblyCode[i][5] == 'o'))
                compareOperation(":/tm_code/loeqv.txt", 24);
            else if((assemblyCode[i][4] == 'm')&&(assemblyCode[i][5] == 'o'))
                compareOperation("://tm_code/moeqv.txt", 24);
        }
        else if(assemblyCode[i][0] == 'i')
        {
            cutCreation(-4, -3);
            int firstGoto = stringToNumber(assemblyCode[i].substr(12+numberOfDigits));
            int secondGoto= stringToNumber(assemblyCode[i+1].substr(10));
            conditionCreation();
            int *arr = new int[3];
            arr[0] = machineCode.size()-4, arr[1] = firstGoto;//
            arr[2] = i;
            gotoCoordinates.push_back(arr);
            arr = new int[3];
            arr[0] = machineCode.size()-1, arr[1] = secondGoto;//
            arr[2] = i++;
            gotoCoordinates.push_back(arr);
            complianceTeamToState.pop_back();
            complianceTeamToPosition.pop_back();
            complianceTeamToState.push_back(numberOfCurrentState);
            complianceTeamToPosition.push_back(currentPosition);
            complianceTeamToState.push_back(numberOfCurrentState);
            complianceTeamToPosition.push_back(currentPosition);
        }
        else if(assemblyCode[i][0] == 'g')
        {
            int *arr = new int[3];
            arr[0] = machineCode.size()-1, arr[1] = stringToNumber(assemblyCode[i].substr(5));//
            arr[2] = i;
            gotoCoordinates.push_back(arr);
        }
    }
    for(size_t i = 0; i < gotoCoordinates.size(); i++)
    {
        int k = gotoCoordinates[i][2];
        int cp = complianceTeamToPosition[k];
        int tp = complianceTeamToPosition[gotoCoordinates[i][1]];
        if(cp != tp)
        {
            machineCode[gotoCoordinates[i][0]][2] = numberOfCurrentState;
            if(cp < complianceTeamToPosition[gotoCoordinates[i][1]])
                createRightTransitionOnNBlocks(complianceTeamToPosition[gotoCoordinates[i][1]] - cp);
            else
                createLeftTransitionOnNBlocks(cp - complianceTeamToPosition[gotoCoordinates[i][1]]);
            machineCode[machineCode.size()-1][2] = complianceTeamToState[gotoCoordinates[i][1]];
        }
        else
            machineCode[gotoCoordinates[i][0]][2] = complianceTeamToState[gotoCoordinates[i][1]];
    }
    complianceTeamToState[0] = 0;
}

void Translator::createTape(vector<Array> &listOfArraies, int numberOfVariables, int nOTV)
{
    tape = new QVector<int>();
    tape->push_back(2), tape->push_back(2);
    beginOfTape = -(nOTV*18+55);//
    for(int i = 0; i < nOTV+3+numberOfVariables; i++)
    {
        for(int j = 0; j < 8*2; j++)
            tape->push_back(0);
        tape->push_back(2), tape->push_back(2);
    }
    for(size_t i = 0; i < listOfArraies.size(); i++)
    {
        for(int k = 0; k < 2; k++)
        {
            for(int j = 0; j < 8*2; j++)
                tape->push_back(0);
            tape->push_back(2), tape->push_back(2);
        }
        for(int j = 0; j < listOfArraies[i].length; j++)
        {
            tape->push_back(0), tape->push_back(2);
            for(int j = 0; j < 2*8; j++)
                tape->push_back(0);
            tape->push_back(2), tape->push_back(2);
        }
    }
}

void Translator::createLeftTransitionOnNBlocks(int n)
{
    QFile inputFile("://tm_code/leftshift.txt");
    inputFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream stream(&inputFile);
    int *arr;
    for(int j = 0; j < n+1; j++)//+1 - так надо
    {
        stream.seek(0);
        for(int i = 0; i < 2*3; i++)
        {
            arr = new int[5];
            stream>>arr[0]>>arr[1]>>arr[2]>>arr[3]>>arr[4];
            arr[0] += numberOfCurrentState;
            arr[2] += numberOfCurrentState;
            machineCode.push_back(arr);
        }
        numberOfCurrentState += 2;
    }
    machineCode[machineCode.size()-1][4] = 1;
    createRightShift();
}

void Translator::createRightTransitionOnNBlocks(int n)
{
    QFile inputFile("://tm_code/rightshift.txt");
    inputFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream stream(&inputFile);
    for(int j = 0; j < n; j++)
    {
        stream.seek(0);
        for(int i = 0; i < 2*3; i++)
        {
            int *arr = new int[5];
            stream>>arr[0]>>arr[1]>>arr[2]>>arr[3]>>arr[4];
            arr[0] += numberOfCurrentState;
            arr[2] += numberOfCurrentState;
            machineCode.push_back(arr);
        }
        numberOfCurrentState += 2;
    }
}

void Translator::createRightShift()
{
    int *arr = new int[5];
    arr[0] = numberOfCurrentState, arr[1] = 0, arr[2] = 0, arr[3] = 0, arr[4] = 2, machineCode.push_back(arr);
    arr = new int[5];
    arr[0] = numberOfCurrentState, arr[1] = 1, arr[2] = 0, arr[3] = 1, arr[4] = 2, machineCode.push_back(arr);
    arr = new int[5];
    arr[0] = numberOfCurrentState, arr[1] = 2, arr[2] = ++numberOfCurrentState, arr[3] = 2, arr[4] = 1, machineCode.push_back(arr);
}

void Translator::findCurrentDigit()
{
    QFile inputFile("://tm_code/findcurrentdigit.txt");
    inputFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream stream(&inputFile);
    for(int i = 0; i < 6*3; i++)
    {
        int *arr = new int[5];
        stream>>arr[0]>>arr[1]>>arr[2]>>arr[3]>>arr[4];
        arr[0] += numberOfCurrentState;
        arr[2] += numberOfCurrentState;
        machineCode.push_back(arr);
    }
    numberOfCurrentState += 6;
}

void Translator::correcting()
{
    QFile inputFile("://tm_code/correcting.txt");
    inputFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream stream(&inputFile);
    for(int i = 0; i < 3*3; i++)
    {
        int *arr = new int[5];
        stream>>arr[0]>>arr[1]>>arr[2]>>arr[3]>>arr[4];
        arr[0] += numberOfCurrentState;
        arr[2] += numberOfCurrentState;
        machineCode.push_back(arr);
    }
    numberOfCurrentState += 3;
}

void Translator::cleaning()
{
    QFile inputFile("://tm_code/cleaning.txt");
    inputFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream stream(&inputFile);
    for(int i = 0; i < 3*3; i++)
    {
        int *arr = new int[5];
        stream>>arr[0]>>arr[1]>>arr[2]>>arr[3]>>arr[4];
        arr[0] += numberOfCurrentState;
        arr[2] += numberOfCurrentState;
        machineCode.push_back(arr);
    }
    numberOfCurrentState += 3;
}

void Translator::moveCreation(int firstOp, int secondOp)
{
    int *arr;
    if(currentPosition < firstOp)
        createRightTransitionOnNBlocks(firstOp-currentPosition);
    else
        createLeftTransitionOnNBlocks(currentPosition-firstOp);
    int beginOfCoping = machineCode.size();
    findCurrentDigit();
    int tmp = machineCode.size()+1;
    if(firstOp < secondOp)
    {
        arr = new int[5];
        arr[0] = numberOfCurrentState, arr[1] = 0, arr[2] = numberOfCurrentState+1, arr[3] = 0, arr[4] = 1, machineCode.push_back(arr);
        arr = new int[5];
        arr[0] = numberOfCurrentState, arr[1] = 1, arr[2] = -1, arr[3] = 1, arr[4] = 1, machineCode.push_back(arr);
        arr = new int[5];
        arr[0] = numberOfCurrentState++, arr[1] = 2, arr[2] = 0, arr[3] = 2, arr[4] = 2, machineCode.push_back(arr);
        //coping 0
        createRightTransitionOnNBlocks(secondOp-firstOp);
        findCurrentDigit();
        int tmp1 = machineCode.size();
        arr = new int[5];
        arr[0] = numberOfCurrentState, arr[1] = 0, arr[2] = -1, arr[3] = 0, arr[4] = 0, machineCode.push_back(arr);
        arr = new int[5];
        arr[0] = numberOfCurrentState, arr[1] = 1, arr[2] = -1, arr[3] = 0, arr[4] = 0, machineCode.push_back(arr);
        arr = new int[5];
        arr[0] = numberOfCurrentState++, arr[1] = 2, arr[2] = 0, arr[3] = 2, arr[4] = 2, machineCode.push_back(arr);
        //coping 1
        machineCode[tmp][2] = numberOfCurrentState;
        createRightTransitionOnNBlocks(secondOp-firstOp);
        findCurrentDigit();
        arr = new int[5];
        arr[0] = numberOfCurrentState, arr[1] = 0, arr[2] = numberOfCurrentState+1, arr[3] = 1, arr[4] = 0, machineCode.push_back(arr);
        arr = new int[5];
        arr[0] = numberOfCurrentState, arr[1] = 1, arr[2] = numberOfCurrentState+1, arr[3] = 1, arr[4] = 0, machineCode.push_back(arr);
        arr = new int[5];
        arr[0] = numberOfCurrentState++, arr[1] = 2, arr[2] = 0, arr[3] = 2, arr[4] = 2, machineCode.push_back(arr);
        //
        machineCode[tmp1][2] = machineCode[tmp1+1][2] = numberOfCurrentState;
        createLeftTransitionOnNBlocks(secondOp-firstOp);
    }
    else
    {
        arr = new int[5];
        arr[0] = numberOfCurrentState, arr[1] = 0, arr[2] = numberOfCurrentState+1, arr[3] = 0, arr[4] = 0, machineCode.push_back(arr);
        arr = new int[5];
        arr[0] = numberOfCurrentState, arr[1] = 1, arr[2] = -1, arr[3] = 1, arr[4] = 0, machineCode.push_back(arr);
        arr = new int[5];
        arr[0] = numberOfCurrentState++, arr[1] = 2, arr[2] = 0, arr[3] = 2, arr[4] = 2, machineCode.push_back(arr);
        //coping 0
        createLeftTransitionOnNBlocks(firstOp-secondOp);
        findCurrentDigit();
        int tmp1 = machineCode.size();
        arr = new int[5];
        arr[0] = numberOfCurrentState, arr[1] = 0, arr[2] = -1, arr[3] = 0, arr[4] = 1, machineCode.push_back(arr);
        arr = new int[5];
        arr[0] = numberOfCurrentState, arr[1] = 1, arr[2] = -1, arr[3] = 0, arr[4] = 1, machineCode.push_back(arr);
        arr = new int[5];
        arr[0] = numberOfCurrentState++, arr[1] = 2, arr[2] = 0, arr[3] = 2, arr[4] = 2, machineCode.push_back(arr);
        //coping 1
        machineCode[tmp][2] = numberOfCurrentState;
        createLeftTransitionOnNBlocks(firstOp-secondOp);
        findCurrentDigit();
        machineCode[machineCode.size()-3][3] = 1;
        arr = new int[5];
        arr[0] = numberOfCurrentState, arr[1] = 0, arr[2] = numberOfCurrentState+1, arr[3] = 1, arr[4] = 1, machineCode.push_back(arr);
        arr = new int[5];
        arr[0] = numberOfCurrentState, arr[1] = 1, arr[2] = numberOfCurrentState+1, arr[3] = 1, arr[4] = 1, machineCode.push_back(arr);
        arr = new int[5];
        arr[0] = numberOfCurrentState++, arr[1] = 2, arr[2] = 0, arr[3] = 2, arr[4] = 2, machineCode.push_back(arr);
        //
        machineCode[tmp1][2] = machineCode[tmp1+1][2] = numberOfCurrentState;
        createRightTransitionOnNBlocks(firstOp-secondOp);
    }
    machineCode[machineCode.size()-1][2] = beginOfCoping/3;
    machineCode[beginOfCoping+1][2] = numberOfCurrentState+1;
}

void Translator::cutCreation(int firstOp, int secondOp)
{
    moveCreation(firstOp, secondOp);
    cleaning();
    createRightShift();
    if(firstOp < secondOp)
    {
        createRightTransitionOnNBlocks(secondOp-firstOp-1);
        correcting();
    }
    else
    {
        createLeftTransitionOnNBlocks(firstOp-secondOp+1);
        correcting();
    }
    createRightShift();
    currentPosition = secondOp+1;
}

void Translator::copyCreation(int firstOp, int secondOp)
{
    moveCreation(firstOp, secondOp);
    correcting();
    createRightShift();
    if(firstOp < secondOp)
    {
        createRightTransitionOnNBlocks(secondOp-firstOp-1);
        correcting();
    }
    else
    {
        createLeftTransitionOnNBlocks(firstOp-secondOp+1);
        correcting();
    }
    createRightShift();
    currentPosition = secondOp+1;
}

void Translator::summary()
{
    if(currentPosition > -3)
        createLeftTransitionOnNBlocks(currentPosition-(-3));
    else
        createRightTransitionOnNBlocks(currentPosition-(-3));
    QFile inputFile("://tm_code/plus.txt");
    inputFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream stream(&inputFile);
    for(int i = 0; i < 44*3; i++)
    {
        int *arr = new int[5];
        stream>>arr[0]>>arr[1]>>arr[2]>>arr[3]>>arr[4];
        arr[0] += numberOfCurrentState;
        arr[2] += numberOfCurrentState;
        machineCode.push_back(arr);
    }
    numberOfCurrentState += 44;
    createLeftTransitionOnNBlocks(1);//т.к. +1
    cleaning();
    createRightShift();
    cleaning();
    createRightShift();
    correcting();
    createRightShift();
    currentPosition = 0;
}

void Translator::subtraction()
{
    numericNot(-2);
    summary();
}

void Translator::multiplication()
{
    if(currentPosition > -3)
        createLeftTransitionOnNBlocks(currentPosition-(-3));
    else
        createRightTransitionOnNBlocks(currentPosition-(-3));
    QFile inputFile("://tm_code/multiplication.txt");
    inputFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream stream(&inputFile);
    for(int i = 0; i < 54*3; i++)
    {
        int *arr = new int[5];
        stream>>arr[0]>>arr[1]>>arr[2]>>arr[3]>>arr[4];
        arr[0] += numberOfCurrentState;
        arr[2] += numberOfCurrentState;
        machineCode.push_back(arr);
    }
    numberOfCurrentState += 54;
    cleaning();
    createRightShift();
    cleaning();
    createRightShift();
    correcting();
    createRightShift();
    currentPosition = 0;
}

void Translator::division()
{

}

void Translator::logicalNot()
{

}

void Translator::numericNot(int reg)
{
    if(currentPosition > reg)
        createLeftTransitionOnNBlocks(currentPosition-reg);
    else
        createRightTransitionOnNBlocks(currentPosition-reg);
    QFile inputFile("://tm_code/not.txt");
    inputFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream stream(&inputFile);
    for(int i = 0; i < 5*3; i++)
    {
        int *arr = new int[5];
        stream>>arr[0]>>arr[1]>>arr[2]>>arr[3]>>arr[4];
        arr[0] += numberOfCurrentState;
        arr[2] += numberOfCurrentState;
        machineCode.push_back(arr);
    }
    numberOfCurrentState += 5;
    createLeftTransitionOnNBlocks(0);
    currentPosition = reg;
}

void Translator::initialize(int reciever, string number)
{
    if(currentPosition > reciever)
        createLeftTransitionOnNBlocks(currentPosition-reciever);
    else if(currentPosition < reciever)
        createRightTransitionOnNBlocks(reciever-currentPosition);
    int *arr = new int[5];
    //костыль для массивов
    arr[0] = numberOfCurrentState, arr[1] = 0, arr[2] = numberOfCurrentState+1, arr[3] = 0, arr[4] = 1, machineCode.push_back(arr);
    arr = new int[5];
    arr[0] = numberOfCurrentState, arr[1] = 1, arr[2] = 0, arr[3] = 1, arr[4] = 2, machineCode.push_back(arr);
    arr = new int[5];
    arr[0] = numberOfCurrentState++, arr[1] = 2, arr[2] =0, arr[3] = 2, arr[4] = 2, machineCode.push_back(arr);
    //
    arr = new int[5];
    arr[0] = numberOfCurrentState, arr[1] = 0, arr[2] = numberOfCurrentState+3, arr[3] = (int)number[0]-(int)'0', arr[4] = 1, machineCode.push_back(arr);
    arr = new int[5];
    arr[0] = numberOfCurrentState, arr[1] = 1, arr[2] = numberOfCurrentState+3, arr[3] = (int)number[0]-(int)'0', arr[4] = 1 , machineCode.push_back(arr);
    arr = new int[5];
    arr[0] = numberOfCurrentState++, arr[1] = 2, arr[2] = numberOfCurrentState, arr[3] = 2, arr[4] = 1, machineCode.push_back(arr);
    //
    for(int i = 0; i < 8; i++)
    {
        arr = new int[5];
        arr[0] = numberOfCurrentState, arr[1] = 0, arr[2] = numberOfCurrentState+1, arr[3] = 0, arr[4] = 1, machineCode.push_back(arr);
        arr = new int[5];
        arr[0] = numberOfCurrentState, arr[1] = 1, arr[2] = 0, arr[3] = 1, arr[4] = 2, machineCode.push_back(arr);
        arr = new int[5];
        arr[0] = numberOfCurrentState++, arr[1] = 2, arr[2] =0, arr[3] = 2, arr[4] = 2, machineCode.push_back(arr);
        //
        arr = new int[5];
        arr[0] = numberOfCurrentState, arr[1] = 0, arr[2] = numberOfCurrentState+1, arr[3] = (int)number[i]-(int)'0', arr[4] = 1, machineCode.push_back(arr);
        arr = new int[5];
        arr[0] = numberOfCurrentState, arr[1] = 1, arr[2] = numberOfCurrentState+1, arr[3] = (int)number[i]-(int)'0', arr[4] = 1 , machineCode.push_back(arr);
        arr = new int[5];
        arr[0] = numberOfCurrentState++, arr[1] = 2, arr[2] = 0, arr[3] = 2, arr[4] = 2, machineCode.push_back(arr);
    }
    createRightShift();
    createRightShift();
    currentPosition = reciever+1;
}

void Translator::pop(int adrReg)
{
    if(currentPosition > adrReg)
        createLeftTransitionOnNBlocks(currentPosition-adrReg);
    else if(currentPosition < adrReg)
        createRightTransitionOnNBlocks(adrReg-currentPosition);
    findCurrentElement();
    QFile inputFile("://tm_code/pop.txt");
    inputFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream stream(&inputFile);
    for(int i = 0; i < 35*3; i++)
    {
        int *arr = new int[5];
        stream>>arr[0]>>arr[1]>>arr[2]>>arr[3]>>arr[4];
        arr[0] += numberOfCurrentState;
        arr[2] += numberOfCurrentState;
        machineCode.push_back(arr);
    }
    numberOfCurrentState += 35;
    currentPosition = adrReg-1;
}

void Translator::push(int adrReg)
{
    if(currentPosition > adrReg)
        createLeftTransitionOnNBlocks(currentPosition-adrReg);
    else if(currentPosition < adrReg)
        createRightTransitionOnNBlocks(adrReg-currentPosition);
    findCurrentElement();
    createLeftTransitionOnNBlocks(2);
    QFile inputFile("://tm_code/push.txt");
    inputFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream stream(&inputFile);
    for(int i = 0; i < 34*3; i++)
    {
        int *arr = new int[5];
        stream>>arr[0]>>arr[1]>>arr[2]>>arr[3]>>arr[4];
        arr[0] += numberOfCurrentState;
        arr[2] += numberOfCurrentState;
        machineCode.push_back(arr);
    }
    numberOfCurrentState += 34;
    cleaning();
    createRightShift();
    createRightTransitionOnNBlocks(1);
    int tmp = machineCode.size();
    int *arr = new int[5];
    arr[0] = numberOfCurrentState, arr[1] = 0, arr[2] = -1, arr[3] = 0, arr[4] = 1, machineCode.push_back(arr);
    arr = new int[5];
    arr[0] = numberOfCurrentState, arr[1] = 1, arr[2] = numberOfCurrentState+1, arr[3] = 0, arr[4] = 1 , machineCode.push_back(arr);
    arr = new int[5];
    arr[0] = numberOfCurrentState++, arr[1] = 2, arr[2] = 0, arr[3] = 2, arr[4] = 2, machineCode.push_back(arr);
    createRightTransitionOnNBlocks(1);
    machineCode[machineCode.size()-1][2] = machineCode[tmp][0];
    machineCode[tmp][2] = numberOfCurrentState;
    createRightShift();
    correcting();
    machineCode[machineCode.size()-3][4] = 0;
    shiftToArrayBeginning();
    currentPosition = adrReg-1;
}

void Translator::findCurrentElement()
{
    QFile inputFile("://tm_code/findcurrentelement.txt");
    inputFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream stream(&inputFile);
    for(int i = 0; i < 15*3; i++)
    {
        int *arr = new int[5];
        stream>>arr[0]>>arr[1]>>arr[2]>>arr[3]>>arr[4];
        arr[0] += numberOfCurrentState;
        arr[2] += numberOfCurrentState;
        machineCode.push_back(arr);
    }
    numberOfCurrentState += 15;
}

void Translator::shiftToArrayBeginning()
{
    QFile inputFile("://tm_code/shifttoarraybeginning.txt");
    inputFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream stream(&inputFile);
    for(int i = 0; i < 5*3; i++)
    {
        int *arr = new int[5];
        stream>>arr[0]>>arr[1]>>arr[2]>>arr[3]>>arr[4];
        arr[0] += numberOfCurrentState;
        arr[2] += numberOfCurrentState;
        machineCode.push_back(arr);
    }
    numberOfCurrentState += 5;
}

void Translator::compareOperation(QString path, int k)
{
    if(currentPosition > -3)
        createLeftTransitionOnNBlocks(currentPosition-(-3));
    else if(currentPosition < -3)
        createRightTransitionOnNBlocks((-3)-currentPosition);
    QFile inputFile(path);
    inputFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream stream(&inputFile);
    for(int i = 0; i < k*3; i++)
    {
        int *arr = new int[5];
        stream>>arr[0]>>arr[1]>>arr[2]>>arr[3]>>arr[4];
        arr[0] += numberOfCurrentState;
        arr[2] += numberOfCurrentState;
        machineCode.push_back(arr);
    }
    numberOfCurrentState += k;
    createLeftTransitionOnNBlocks(3);
    cleaning();
    createRightShift();
    cleaning();
    createRightShift();
    currentPosition = -1;
}

void Translator::conditionCreation()
{
    if(currentPosition > -3)
        createLeftTransitionOnNBlocks(currentPosition-(-3));
    else if(currentPosition < -3)
        createRightTransitionOnNBlocks((-3)-currentPosition);
    QFile inputFile("://tm_code/condition.txt");
    inputFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QTextStream stream(&inputFile);
    for(int i = 0; i < 4*3; i++)
    {
        int *arr = new int[5];
        stream>>arr[0]>>arr[1]>>arr[2]>>arr[3]>>arr[4];
        arr[0] += numberOfCurrentState;
        arr[2] += numberOfCurrentState;
        machineCode.push_back(arr);
    }
    numberOfCurrentState += 4;
    currentPosition = -2;
}
