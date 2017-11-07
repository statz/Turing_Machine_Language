#ifndef OPERATINGWINDOW_H
#define OPERATINGWINDOW_H

#include <QMainWindow>
#include <QToolBar>
#include <QTableWidget>
#include <QSplitter>

#include <parser.h>
#include <codeeditor.h>
#include <treemodel.h>
#include <machine.h>

using namespace std;

class OperatingWindow : public QMainWindow
{
  Q_OBJECT
public:
  OperatingWindow();
  void setMachine(QVector<int>, int &, vector<int *> states);
  void setAsmCode(vector<string>);
  void setArrsAndVars(vector<Array> *, vector<Variable> *, int, vector<int>, int);
  ~OperatingWindow();
private:
  void redraw();
  void refreshVariablesValue();
  void createMenus();
  void setActions();
protected:
  void resizeEvent(QResizeEvent *);
  void closeEvent(QCloseEvent *event);
private slots:
  void makeMoveOnStateSlot();
  void makeMoveOnAsmCommSlot();
  void toEndOfProgrammSlot();
private:
  QGraphicsScene *tapeViewScene;
  QGraphicsView	*tapeView;
  QToolBar *toolBar;
  QMenu *fileMenu;
  QMenu *controlMenu;
  QTableWidget *assemblyCodeView;
  QTableWidget *statesView;
  TreeModel *variablesTree;
  QTreeView *variablesTreeView;
  QSplitter *splitter;
  QSpinBox *numberOfMove;
  QSpinBox *numberOfCommands;
  QAction *makeMoveOnState;
  QAction *makeMoveOnAsmComm;
  QAction *toEndOfProgramm;
  QAction *exitAct;
private:
  int beginOfOutput, endOfOutput;
  Machine *machine;
  vector<int> complianceTeamToState;
};

#endif // OPERATINGWINDOW_H
