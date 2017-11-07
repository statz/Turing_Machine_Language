#ifndef MACHINE_H
#define MACHINE_H

#include <QVector>

typedef unsigned char byte;

class Machine
{
public:
    Machine(QVector<int> &newTape, int &beginOfNewTape, QVector<int *> &newStates);
	void getTape(byte *&, int &, int &);
	int getCurrentState();
	int getCurrentPosition();
	int getCurrentSimbol();
	bool isReady();
	void makeStep();
    bool isStopped();
private:
	QVector<int> tape;
	int ***states;
	int currentState, currentPosition;
    bool stop;
	int beginOfTape;
	int endOfTape;
	int sizeOfTape;
};

#endif // MACHINE_H
