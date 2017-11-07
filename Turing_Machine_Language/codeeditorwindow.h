#ifndef CODEEDITORWINDOW_H
#define CODEEDITORWINDOW_H

#include <operatingwindow.h>
#include <codeeditor.h>
#include <translator.h>

class CodeEditorWindow : public QMainWindow
{
	Q_OBJECT
public:
	CodeEditorWindow();

protected:
	void closeEvent(QCloseEvent *event);

private slots:
	void newFile();
	void open();
	bool save();
	bool saveAs();
	void documentWasModified();
	void start();

private:
	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();
	void readSettings();
	void writeSettings();
	bool maybeSave();
	void loadFile(const QString &fileName);
	bool saveFile(const QString &fileName);
	void setCurrentFile(const QString &fileName);
	QString strippedName(const QString &fullFileName);

private:
	CodeEditor *textEdit;
	Highlighter *highlighter;
    QString currentFileName;
	QMenu *fileMenu;
	QMenu *editMenu;
	QMenu *launchMenu;
    QToolBar *toolBar;
	QAction *newAct;
	QAction *openAct;
	QAction *saveAct;
	QAction *saveAsAct;
	QAction *exitAct;
	QAction *cutAct;
	QAction *copyAct;
	QAction *pasteAct;
	QAction *startAct;
	OperatingWindow *operatingWindow;
    Translator *translator;
};

#endif // CODEEDITORWINDOW_H
