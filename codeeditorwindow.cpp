#include "codeeditorwindow.h"

CodeEditorWindow::CodeEditorWindow()
{
    textEdit = new CodeEditor;
    setCentralWidget(textEdit);
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    readSettings();
    connect(textEdit->document(), SIGNAL(contentsChanged()), this, SLOT(documentWasModified()));
    setCurrentFile("");
    setUnifiedTitleAndToolBarOnMac(true);
    highlighter = new Highlighter(this->textEdit->document());
    translator = 0;
    operatingWindow = 0;
}

void CodeEditorWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
    {
        writeSettings();
        event->accept();
    }
    else
        event->ignore();
}
void CodeEditorWindow::newFile()
{
    if (maybeSave())
        textEdit->clear(), setCurrentFile("");
}
void CodeEditorWindow::open()
{
    if (maybeSave())
    {
        QString fileName = QFileDialog::getOpenFileName(this);
        if (!fileName.isEmpty())
            loadFile(fileName);
    }
}

bool CodeEditorWindow::save()
{
    if (currentFileName.isEmpty())
        return saveAs();
    else
        return saveFile(currentFileName);
}

bool CodeEditorWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this);
    if (fileName.isEmpty())
        return false;
    return saveFile(fileName);
}

void CodeEditorWindow::documentWasModified()
{
    setWindowModified(textEdit->document()->isModified());
}

void CodeEditorWindow::start()
{
    if (maybeSave())
        writeSettings();
    translator = new Translator(currentFileName.toStdString());
    translator->translate();
    int beginOfTape;
    operatingWindow = new OperatingWindow();
    operatingWindow->setMachine(translator->getTape(beginOfTape), beginOfTape, translator->getMachineCode());
    operatingWindow->setAsmCode(translator->getAssemblyCode());
    operatingWindow->setArrsAndVars(translator->getListOfArraies(), translator->getListOfVariables(),
                                    translator->getNumberOfTmpVariables(),
                                    translator->getComplianceTeamToState(), translator->getNumberOfDigits());
    operatingWindow->showMaximized();
    delete translator;
}

void CodeEditorWindow::createActions()
{
    startAct = new QAction(QIcon(":/icons/start.png"), tr("&Старт"), this);
    startAct->setStatusTip(tr("Запустить программу"));
    connect(startAct, SIGNAL(triggered()), this, SLOT(start()));
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    newAct = new QAction(QIcon(":/icons/new.png"), tr("&Новый"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Создать новый файл"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    openAct = new QAction(QIcon(":/icons/open.png"), tr("&Открыть..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Открыть существующий файл"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    saveAct = new QAction(QIcon(":/icons/save.png"), tr("&Сохранить..."),this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Сохранить документ на диск"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    saveAsAct = new QAction(tr("Сохранить &Как..."), this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Сохранить документ под заданным именем"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    exitAct = new QAction(QIcon(":/icons/exit.png"),tr("В&ыход"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Выйти из приложения"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    cutAct = new QAction(QIcon(":/icons/cut.png"), tr("Вы&резать"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Вырезать текущий выделенный текст в буффер обмена"));
    connect(cutAct, SIGNAL(triggered()), textEdit, SLOT(cut()));
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    copyAct = new QAction(QIcon(":/icons/copy.png"), tr("&Копировать"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Вырезать текущий выделенный текст в буффер обмена"));
    connect(copyAct, SIGNAL(triggered()), textEdit, SLOT(copy()));
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    pasteAct = new QAction(QIcon(":/icons/paste.png"), tr("&Вставить"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Вставить текст из буффера обмена"));
    connect(pasteAct, SIGNAL(triggered()), textEdit, SLOT(paste()));
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    cutAct->setEnabled(false);
    copyAct->setEnabled(false);
    connect(textEdit, SIGNAL(copyAvailable(bool)), cutAct, SLOT(setEnabled(bool)));
    connect(textEdit, SIGNAL(copyAvailable(bool)), copyAct, SLOT(setEnabled(bool)));
}

void CodeEditorWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&Файл"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    editMenu = menuBar()->addMenu(tr("&Редактирование"));
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    launchMenu = menuBar()->addMenu(tr("&Управление"));
    launchMenu->addAction(startAct);
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    menuBar()->addSeparator();
}

void CodeEditorWindow::createToolBars()
{
    toolBar = addToolBar(tr("Файл"));
    toolBar->addAction(newAct);
    toolBar->addAction(openAct);
    toolBar->addAction(saveAct);
    toolBar->addSeparator();
    toolBar->addAction(startAct);
    this->addToolBar(Qt::LeftToolBarArea, this->toolBar);
    toolBar->setMovable(false);
}

void CodeEditorWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void CodeEditorWindow::readSettings()
{
    QSettings settings("QtProject", "Application Example");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    resize(size);
    move(pos);
}

void CodeEditorWindow::writeSettings()
{
    QSettings settings("QtProject", "Application Example");
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}

bool CodeEditorWindow::maybeSave()
{
    if (textEdit->document()->isModified())
    {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Application"),
                                   tr("The document has been modified.\n"
                                      "Do you want to save your changes?"),
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}
void CodeEditorWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, tr("Application"), tr("Cannot read file %1:\n%2.").arg(fileName).arg(file.errorString()));
        return;
    }
    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    textEdit->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 2000);
}
bool CodeEditorWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }
    QTextStream out(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    out << textEdit->toPlainText();
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}
void CodeEditorWindow::setCurrentFile(const QString &fileName)
{
    currentFileName = fileName;
    textEdit->document()->setModified(false);
    setWindowModified(false);
    //////////////////////////////////////////
    QString shownName = currentFileName;
    if (currentFileName.isEmpty())
        shownName = "untitled.txt";
    setWindowFilePath(shownName);
}
QString CodeEditorWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}
