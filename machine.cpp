#include "machine.h"
#include <memory.h>
#include <QDebug>

Machine::Machine(QVector<int> &newTape, int &beginOfNewTape, QVector<int *> &newStates)
{
  //set tape
  sizeOfTape = newTape.size();
  for(int i = 0; i < sizeOfTape; i++)
      tape.append(newTape[i]);
  tape.append(0);
  tape.append(0);
  tape.prepend(0);
  tape.prepend(0);
  currentPosition = 0;
  //set states
  int numberOfStates = newStates.size()/3;
  states = new int**[numberOfStates];
  for(int i = 0; i < numberOfStates; i++)
  {
      states[i] = new int*[3];
      for(int j = 0; j < 3; j++)
          states[i][j] = new int [3];
  }
  for (int i = 0; i < numberOfStates; i++)
      for (int j = 0; j < 3; j++)
      {
        states[i][j][0] = newStates[i*3+j][2];
        states[i][j][1] = newStates[i*3+j][3];
        states[i][j][2] = newStates[i*3+j][4];
       }
    currentState = 0;
  //
  beginOfTape = beginOfNewTape-2;
  endOfTape = beginOfTape+sizeOfTape-1+4;
  currentState = 0;
  stop = false;
}

void Machine::getTape(byte *&output, int &beginOfOutput, int &endOfOutput)
{
	output = new byte[tape.size()];
	if(tape.empty())
	{
		beginOfOutput = 0;
		endOfOutput = -1;
		return;
	}
	beginOfOutput = beginOfTape;
	endOfOutput = endOfTape;
	for(int i = 0; i < tape.size(); i++)
		output[i] = tape[i];
}

int Machine::getCurrentState()
{
	return currentState;
}

int Machine::getCurrentPosition()
{
	return currentPosition;
}

int Machine::getCurrentSimbol()
{
	return tape[currentPosition-beginOfTape];
}

bool Machine::isReady()
{
	return !tape.empty() && states;
}

void Machine::makeStep()
{
    if (!stop)
	{
		int tmpState = currentState;
		currentState = states[currentState][tape.at(currentPosition-beginOfTape)][0];
		int valueOfTmpPosition = tape.at(currentPosition-beginOfTape);
		tape[currentPosition-beginOfTape] = states[tmpState][valueOfTmpPosition][1];
		switch(states[tmpState][valueOfTmpPosition][2]){
		case(0) : currentPosition--;
			if(currentPosition == beginOfTape)
			{
				tape.prepend(0);
				beginOfTape--;
				sizeOfTape++;
			}
			break;
		case(1) : currentPosition++;
			if(currentPosition == endOfTape)
			{
				tape.append(0);
				endOfTape++;
				sizeOfTape++;
			}
			break;
		case(2):
            stop = true;
		}
    }
}

bool Machine::isStopped()
{
 return stop;
}
