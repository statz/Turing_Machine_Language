#include "operatingwindow.h"

QString getStringIndex(int number, int numberOfDigits)
{
  QString output;
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

OperatingWindow::OperatingWindow()
{
  splitter = new QSplitter(this);
  toolBar = addToolBar(tr(""));
  this->addToolBar(Qt::LeftToolBarArea, this->toolBar);
  toolBar->setMovable(false);
  setActions();
  createMenus();
}

void OperatingWindow::setMachine(QVector<int> tape, int &beginOfTape, vector<int *> states)
{
  tapeView = new QGraphicsView(this);
  tapeViewScene = new QGraphicsScene();
  tapeView->setScene(tapeViewScene);
  QVector<int *> a = QVector<int *>::fromStdVector(states);
  machine = new Machine(tape, beginOfTape, a);
  redraw();
  statesView = new QTableWidget(states.size(), 5, this);
  for(size_t i = 0; i < states.size(); i++)
    for(int j = 0; j < 5; j++)
    {
      QTableWidgetItem *ti = new QTableWidgetItem(QString::number(states[i][j]));
      statesView->setItem(i, j, ti);
    }
  statesView->resizeColumnsToContents();
  statesView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  statesView->verticalHeader()->hide();
  //statesView->horizontalHeader()->setEnabled(false);
  statesView->selectRow(0);
  //statesView->setSelectionMode(QAbstractItemView::NoSelection);
}

void OperatingWindow::setAsmCode(vector<string> assemblyCode)
{
  assemblyCodeView = new QTableWidget(assemblyCode.size(), 1, this);
  for(size_t i = 0; i < assemblyCode.size(); i++)
  {
    QTableWidgetItem *ti = new QTableWidgetItem(QString::fromStdString(assemblyCode[i]));
    assemblyCodeView->setItem(i, 0, ti);
  }
  assemblyCodeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
  assemblyCodeView->verticalHeader()->hide();
  assemblyCodeView->horizontalHeader()->setEnabled(false);
  assemblyCodeView->selectRow(0);
  assemblyCodeView->resizeColumnsToContents();

}

void OperatingWindow::setArrsAndVars(vector<Array> *listOfArraies, vector<Variable> *listOfVariables, int numberOfTmpVal,
                                     vector<int> complianceTeamToState, int numberOfDigits)
{
  this->complianceTeamToState = complianceTeamToState;
  int numOfArrsAndVars =listOfVariables->size()+numberOfTmpVal+3;
  for(size_t i = 0; i < listOfArraies->size(); i++)
    numOfArrsAndVars += listOfArraies->at(i).length+3;
  QString **arr = new QString*[numOfArrsAndVars];
  for(int i = 0; i < numOfArrsAndVars; i++)
    arr[i] = new QString[3];
  int k  = 0;
  for(int i = numberOfTmpVal-1; i >= 0; i--)
    arr[k][0] = "tmp"+getStringIndex(i+1, numberOfDigits), arr[k++][1] = "0";
  arr[k++][0] = "freg";
  arr[k++][0] = "sreg";
  arr[k++][0] = "treg";
  for(size_t i = 1; i <= listOfVariables->size(); i++)
    arr[k++][0].append("var"+getStringIndex(i, numberOfDigits)+"("+QString::fromStdString(listOfVariables->at(i-1).name)+")");
  for(size_t i = 0; i < listOfArraies->size(); i++)
  {
    QString name = "arr"+getStringIndex(i+1, numberOfDigits);
    arr[k++][0]=name;
    arr[k][0] = name+"_tmpreg";
    arr[k++][1] = "1";
    arr[k][0]=name+"_adrreg";
    arr[k++][1] = "1";
    for(int j = 0; j < listOfArraies->at(i).length; j++)
      arr[k][0]= QString::number(j), arr[k++][1] = "1";
  }
  for(int i = 0 ; i < numOfArrsAndVars; i++)
    arr[i][2] = "0";
  variablesTree = new TreeModel(arr, numOfArrsAndVars, this);
  variablesTreeView = new QTreeView(this);
  variablesTreeView->setModel(variablesTree);
  splitter->addWidget(statesView);
  splitter->addWidget(assemblyCodeView);
  splitter->addWidget(variablesTreeView);
}

OperatingWindow::~OperatingWindow()
{

}

void OperatingWindow::redraw()
{
  byte *output;
  machine->getTape(output, beginOfOutput, endOfOutput);
  tapeViewScene->setSceneRect(0, 0, (endOfOutput-beginOfOutput+1)*25, 25);
  for(int i = 0; i < endOfOutput-beginOfOutput+1; i++)
  {
    QBrush cellBrush(Qt::SolidPattern);
    switch(output[i])
    {
    case 0 :
      cellBrush.setColor(Qt::white);
      break;
    case 1:
      cellBrush.setColor(Qt::black);
      break;
    case 2 :
      cellBrush.setColor(Qt::yellow);
    }
    tapeViewScene->addRect(i*25, 0, 25, 25, QPen(Qt::red,0), cellBrush);
  }
  int currentPosition = machine->getCurrentPosition();
  if((currentPosition >= beginOfOutput) && (currentPosition <= endOfOutput))
    tapeViewScene->addRect((currentPosition-beginOfOutput)*25, 0, 25, 25, QPen(Qt::red,0), QBrush(Qt::green,Qt::CrossPattern));
  delete [] output;
}

void OperatingWindow::refreshVariablesValue()
{
  byte *tape;
  machine->getTape(tape, beginOfOutput, endOfOutput);
  int c = variablesTree->rootItem->childCount();
  int begin = -14;
  for(int i = 0; i < c; i++)
  {
    int tmpC = variablesTree->rootItem->child(i)->childCount();
    int k = 128;
    char n = 0;
    if(!tmpC)
    {
      begin += 18;
      int j = begin+1;
      while(k > 0)
      {
        n += k*tape[j];
        j += 2;
        k /= 2;
      }
      variablesTree->rootItem->child(i)->changeData(QString::number(n));
    }
    else
    {
      begin += 18;
      int j = begin+1;
      while(k > 0)
      {
        n += k*tape[j];
        j += 2;
        k /= 2;
      }
      variablesTree->rootItem->child(i)->child(0)->changeData(QString::number(n));
      k = 128;
      n = 0;
      begin += 18;
      j = begin+1;
      while(k > 0)
      {
        n += k*tape[j];
        j += 2;
        k /= 2;
      }
      variablesTree->rootItem->child(i)->child(1)->changeData(QString::number(n));
      for(int l = 2; l < variablesTree->rootItem->child(i)->childCount(); l++)
      {
        k = 128;
        n = 0;
        begin += 18+2;
        j = begin+1;
        while(k > 0)
        {
          n += k*tape[j];
          j += 2;
          k /= 2;
        }
        variablesTree->rootItem->child(i)->child(l)->changeData(QString::number(n));
      }
    }
  }
  variablesTree->layoutChanged();
}

void OperatingWindow::createMenus()
{
  fileMenu = menuBar()->addMenu(tr("&Файл"));
  fileMenu->addAction(exitAct);
  ////////////////////////////////////////////////////
  controlMenu = menuBar()->addMenu(tr("&Управление"));
  controlMenu->addAction(makeMoveOnState);
  controlMenu->addAction(makeMoveOnAsmComm);
  controlMenu->addAction(toEndOfProgramm);
}

void OperatingWindow::setActions()
{
  statusBar()->showMessage(tr("Ready"));
  numberOfMove = new QSpinBox(this);
  numberOfMove->setMinimum(1);
  numberOfMove->setMaximum(1000);
  toolBar->addWidget(numberOfMove);
  //////////////////////////////////////////////////////////////////////////
  makeMoveOnState = new QAction(QIcon(":/icons/arrow_right.png"), tr("&Шаг(состояния)"), this);
  makeMoveOnState->setStatusTip(tr("Вперед на заданное кол-во шагов"));
  toolBar->addAction(makeMoveOnState);
  connect(makeMoveOnState, SIGNAL(triggered()), this, SLOT(makeMoveOnStateSlot()));
  toolBar->addSeparator();
  //////////////////////////////////////////////////////////////////////////
  numberOfCommands = new QSpinBox(this);
  numberOfCommands->setMinimum(1);
  numberOfCommands->setMaximum(100);
  toolBar->addWidget(numberOfCommands);
  //////////////////////////////////////////////////////////////////////////
  makeMoveOnAsmComm = new QAction(QIcon(":/icons/arrow_right_double.png"), tr("&Шаг(комманды)"), this);
  makeMoveOnAsmComm->setStatusTip(tr("Вперед на заданное кол-во команд"));
  toolBar->addAction(makeMoveOnAsmComm);
  connect(makeMoveOnAsmComm, SIGNAL(triggered()), this, SLOT(makeMoveOnAsmCommSlot()));
  toolBar->addSeparator();
  ///////////////////////////////////////////////////////////////////////////
  toEndOfProgramm = new QAction(QIcon(":/icons/arrow_end.png"), tr("&До конца"), this);
  toEndOfProgramm->setStatusTip(tr("Выполнить программу полностью"));
  toolBar->addAction(toEndOfProgramm);
  connect(toEndOfProgramm, SIGNAL(triggered()), this, SLOT(toEndOfProgrammSlot()));
  ////////////////////////////////////////////////////////////////////////////
  exitAct = new QAction(QIcon(":/icons/exit.png"),tr("В&ыход"), this);
  exitAct->setShortcuts(QKeySequence::Quit);
  exitAct->setStatusTip(tr("Выйти из приложения"));
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));
}

void OperatingWindow::resizeEvent(QResizeEvent *)
{
  tapeView->setGeometry(this->toolBar->width(), this->toolBar->y(), this->width()-this->toolBar->width(), 45);
  splitter->setGeometry(this->toolBar->width(), this->tapeView->height()+20, this->tapeView->width(), this->height()-this->tapeView->y()-65);
}

void OperatingWindow::closeEvent(QCloseEvent *)
{
  delete machine;
}

void OperatingWindow::makeMoveOnStateSlot()
{
  int currentState;
  for(int i = 0; i < numberOfMove->value(); i++)
  {
    machine->makeStep();
    currentState = machine->getCurrentState();
    size_t j = 0;
    while(complianceTeamToState[j] < currentState)
      j++;
    if(complianceTeamToState[j] == currentState)
      refreshVariablesValue();
    j--;
    assemblyCodeView->selectRow(j);
  }
  statesView->selectRow(currentState*3+machine->getCurrentSimbol());
  redraw();
}

void OperatingWindow::makeMoveOnAsmCommSlot()
{
  int currentState;
  size_t j;
  do
  {
    machine->makeStep();
    currentState = machine->getCurrentState();
    j = 0;
    while(complianceTeamToState[j] < currentState)
      j++;
    if(complianceTeamToState[j] == currentState)
      break;
  }
  while(true);
  refreshVariablesValue();
  j--;
  assemblyCodeView->selectRow(j);
  statesView->selectRow(currentState*3+machine->getCurrentSimbol());
  redraw();
}

void OperatingWindow::toEndOfProgrammSlot()
{
  while(!machine->isStopped())
    machine->makeStep();
  redraw();
  refreshVariablesValue();
}
