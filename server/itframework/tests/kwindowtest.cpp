#include "kwindowtest.h"

#include <QDir>
#include <QMessageBox>
#include <QTest>
#include <QTextEdit>

#include <cstdlib>

#include <KActionMenu>
#include <KToggleAction>
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QComboBox>
#include <QLineEdit>
#include <kactioncollection.h>
#include <khelpmenu.h>
#include <kxmlguifactory.h>

TestWindow::TestWindow(QWidget *parent)
    : KXmlGuiWindow(parent)
{
    ena = false;
    exitB = true; // exit button is shown
    lineL = true; // LineEdit is enabled
    greenF = false; // Frame not inserted
    timer = nullptr;

    setCaption(QStringLiteral("test window"));

    // The xmlgui file defines the layout of the menus and toolbars.
    // We only need to create actions with the right name here.

    // First four  buttons
    fileNewAction = new QAction(QIcon::fromTheme(QStringLiteral("document-new")), QStringLiteral("Create.. (toggles upper button)"), this);
    actionCollection()->addAction(QStringLiteral("filenew"), fileNewAction);
    fileNewAction->setCheckable(true);
    connect(fileNewAction, &QAction::triggered, this, &TestWindow::slotNew);

    QAction *fileOpenAction = new QAction(QIcon::fromTheme(QStringLiteral("document-open")), QStringLiteral("Open"), this);
    actionCollection()->addAction(QStringLiteral("fileopen"), fileOpenAction);
    connect(fileOpenAction, &QAction::triggered, this, &TestWindow::slotOpen);

    KActionMenu *fileFloppyAction = new KActionMenu(QIcon::fromTheme(QStringLiteral("filefloppy")), QStringLiteral("Save (beep or delayed popup)"), this);
    actionCollection()->addAction(QStringLiteral("filefloppy"), fileFloppyAction);
    connect(fileFloppyAction, &QAction::triggered, this, &TestWindow::slotSave);

    QAction *filePrintAction = new QAction(QIcon::fromTheme(QStringLiteral("document-print")), QStringLiteral("Print (enables/disables open)"), this);
    actionCollection()->addAction(QStringLiteral("fileprint"), filePrintAction);
    filePrintAction->setToolTip(QStringLiteral("This tooltip does not work for menu items"));
    filePrintAction->setWhatsThis(QStringLiteral("This is the longer explanation of the action"));
    filePrintAction->setStatusTip(QStringLiteral("This action is supposed to print, but in fact enables/disables open"));
    connect(filePrintAction, &QAction::triggered, this, &TestWindow::slotPrint);

    // And a combobox
    // arguments: text (or strList), ID, writable, signal, object, slot, enabled,
    //            tooltiptext, size
    testComboBox = new QComboBox(toolBar());
    // K3WidgetAction* comboAction = new K3WidgetAction(testComboBox, QString(), 0, 0, 0, actionCollection(), "combobox");
    // connect(testComboBox, SIGNAL(activated(QString)), this, SLOT(slotList(QString)));

    // Then one line editor
    // arguments: text, id, signal, object (this), slot, enabled, tooltiptext, size
    testLineEdit = new QLineEdit(toolBar());
    testLineEdit->setText(QStringLiteral("ftp://ftp.kde.org/pub/kde"));
    //    K3WidgetAction* lineEditAction = new K3WidgetAction(testLineEdit, QString(), 0, 0, 0, actionCollection(), "location");
    //    connect(testLineEdit, SIGNAL(returnPressed()), this, SLOT(slotReturn()));

    // Now add another button and align it right
    exitAction = new QAction(QIcon::fromTheme(QStringLiteral("application-exit")), QStringLiteral("Exit"), this);
    actionCollection()->addAction(QStringLiteral("exit"), exitAction);
    connect(exitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    // Another toolbar

    QAction *fileNewAction2 = new QAction(QIcon::fromTheme(QStringLiteral("document-new")), QStringLiteral("Create new file2 (Toggle)"), this);
    actionCollection()->addAction(QStringLiteral("filenew2"), fileNewAction2);
    connect(fileNewAction2, &QAction::toggled, this, &TestWindow::slotToggle);
    fileNewAction2->setToolTip(QStringLiteral("Tooltip"));
    fileNewAction2->setStatusTip(QStringLiteral("Statustip"));
    fileNewAction2->setWhatsThis(QStringLiteral("WhatsThis"));

    QAction *fileOpenAction2 = new QAction(QIcon::fromTheme(QStringLiteral("document-open")), QStringLiteral("Open (starts progress in sb)"), this);
    actionCollection()->addAction(QStringLiteral("fileopen2"), fileOpenAction2);
    connect(fileOpenAction2, &QAction::triggered, this, &TestWindow::slotOpen);
    fileOpenAction2->setToolTip(QStringLiteral("This action starts a progressbar inside the statusbar"));

    QAction *fileFloppyAction2 = new QAction(QIcon::fromTheme(QStringLiteral("filefloppy")), QStringLiteral("Save file2 (autorepeat)"), this);
    actionCollection()->addAction(QStringLiteral("filefloppy2"), fileFloppyAction2);
    connect(fileFloppyAction2, &QAction::triggered, this, &TestWindow::slotSave);

    itemsMenu = new QMenu;
    itemsMenu->addAction(QStringLiteral("delete/insert exit button"), this, &TestWindow::slotExit);
    itemsMenu->addAction(QStringLiteral("enable/disable lineedit"), this, &TestWindow::slotLined);
    itemsMenu->addAction(QStringLiteral("Toggle fileNew"), this, &TestWindow::slotNew);
    itemsMenu->addAction(QStringLiteral("Combo: clear"), this, &TestWindow::slotClearCombo);
    itemsMenu->addAction(QStringLiteral("Combo: insert list"), this, &TestWindow::slotInsertListInCombo);
    itemsMenu->addAction(QStringLiteral("Combo: make item 3 current"), this, &TestWindow::slotMakeItem3Current);
    itemsMenu->addAction(QStringLiteral("Important msg in statusbar"), this, &TestWindow::slotImportant);

    QAction *filePrintAction2 = new QAction(QIcon::fromTheme(QStringLiteral("document-print")), QStringLiteral("Print (pops menu)"), this);
    actionCollection()->addAction(QStringLiteral("fileprint2"), filePrintAction2);
    filePrintAction2->setMenu(itemsMenu);

    // *** RADIO BUTTONS
    QActionGroup *radioGroup = new QActionGroup(this);
    radioGroup->setExclusive(true);

    KToggleAction *radioButton1 = new KToggleAction(QIcon::fromTheme(QStringLiteral("document-new")), QStringLiteral("Radiobutton1"), this);
    actionCollection()->addAction(QStringLiteral("radioButton1"), radioButton1);
    radioButton1->setActionGroup(radioGroup);

    KToggleAction *radioButton2 = new KToggleAction(QIcon::fromTheme(QStringLiteral("document-open")), QStringLiteral("Radiobutton2"), this);
    actionCollection()->addAction(QStringLiteral("radioButton2"), radioButton2);
    radioButton2->setActionGroup(radioGroup);

    KToggleAction *radioButton3 = new KToggleAction(QIcon::fromTheme(QStringLiteral("filefloppy")), QStringLiteral("Radiobutton3"), this);
    actionCollection()->addAction(QStringLiteral("radioButton3"), radioButton3);
    radioButton3->setActionGroup(radioGroup);

    KToggleAction *radioButton4 = new KToggleAction(QIcon::fromTheme(QStringLiteral("document-print")), QStringLiteral("Radiobutton4"), this);
    actionCollection()->addAction(QStringLiteral("radioButton4"), radioButton4);
    radioButton4->setActionGroup(radioGroup);

    connect(radioGroup, &QActionGroup::triggered, this, &TestWindow::slotToggled);

    /**************************************************/
    /*Now, we setup statusbar; order is not important. */
    /**************************************************/
    statusBar = new QStatusBar(this);
    // statusBar->insertItem("Hi there!                         ", 0);
    // statusBar->insertItem("Look for tooltips to see functions", 1);
    setStatusBar(statusBar);

    // DigitalClock *clk = new DigitalClock (statusBar);
    // clk->setFrameStyle(QFrame::NoFrame);
    // statusBar->insertWidget(clk, 70, 2);

    // Set main widget. In this example it is Qt's multiline text editor.
    widget = new QTextEdit(this);
    setCentralWidget(widget);

    // Setup is now complete

    setAutoSaveSettings();

    // This is not strictly related to toolbars, menubars or KMainWindow.
    // Setup popup for completions
    completions = new QMenu;

    completions->addAction(QStringLiteral("/"));
    completions->addAction(QStringLiteral("/usr/"));
    completions->addAction(QStringLiteral("/lib/"));
    completions->addAction(QStringLiteral("/var/"));
    completions->addAction(QStringLiteral("/bin/"));
    completions->addAction(QStringLiteral("/kde/"));
    completions->addAction(QStringLiteral("/home/"));
    completions->addAction(QStringLiteral("/vmlinuz :-)"));

    connect(completions, &QMenu::triggered, this, &TestWindow::slotCompletionsMenu);
    pr = nullptr;

    // KXMLGUIClient looks in the "data" resource for the .rc files
    // This line is for test programs only!
    qputenv("XDG_DATA_HOME", QFile::encodeName(QFileInfo(QFINDTESTDATA("kwindowtest.rc")).absolutePath()));
    setupGUI(QSize(400, 500), Default, QStringLiteral("kwindowtest.rc"));

    tb = toolBar();
    tb1 = toolBar(QStringLiteral("AnotherToolBar"));
}
/***********************************/
/*  Now slots for toolbar actions  */
/***********************************/
void TestWindow::slotToggled(QAction *)
{
    statusBar->showMessage(QStringLiteral("Button toggled"), 1500);
}

void TestWindow::slotInsertClock()
{
    // DigitalClock *clock = new DigitalClock(tb1);
    // clock->setFrameStyle(QFrame::NoFrame);
    // tb1->insertWidget(8, 70, clock);
}

void TestWindow::slotNew()
{
    // tb1->actions()[0]->toggle();
    // toolBar()->removeAction( fileNewAction );
}
void TestWindow::slotOpen()
{
    if (pr == nullptr) {
        pr = new QProgressBar(statusBar);
        pr->show();
    }
    //  statusBar->message(pr);
    if (!timer) {
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &TestWindow::slotGoGoGoo);
    }
    timer->start(100);
}

void TestWindow::slotGoGoGoo()
{
    pr->setValue(pr->value() + 1);
    if (pr->value() == 100) {
        timer->stop();
        statusBar->clearMessage();
        delete pr;
        pr = nullptr;
    }
}

void TestWindow::slotSave()
{
    qApp->beep();
    statusBar->showMessage(QStringLiteral("Saving properties..."));
}

void TestWindow::slotPrint()
{
    statusBar->showMessage(QStringLiteral("Print file pressed"));
    ena = !ena;
    qobject_cast<QAction *>(sender())->setEnabled(ena);
}
void TestWindow::slotReturn()
{
    QString s = QStringLiteral("You entered ");
    s = s + testLineEdit->text();
    statusBar->showMessage(s);
}
void TestWindow::slotList(const QString &str)
{
    QString s = QStringLiteral("You chose ");
    s = s + str;
    statusBar->showMessage(s);
}

void TestWindow::slotCompletion()
{
    // Now do a completion
    // Call your completing function and set that text in lineedit
    // QString s = tb->getLinedText(/* ID */ 4)
    // QString completed = complete (s);
    // tb->setLinedText(/* ID */ 4, completed.data())

    // for now this:

    completions->popup(QCursor::pos()); // This popup should understunf keys up and down

    /* This is just an example. QLineEdit automatically sets cursor at end of string
     when ctrl-d or ctrl-s is pressed. KToolBar will also put cursor at end of text in LineEdit
     after inserting text with setLinedText (...).
    */
}

void TestWindow::slotListCompletion()
{
    /*
     Combo is not behaving good and it is ugly. I will see how it behaves in Qt-1.3,
     and then decide should I make a new combobox.
     */
    QString s(testComboBox->currentText()); // get text in combo
    s += QStringLiteral("(completing)");
    // tb->getCombo(4)->changeItem(s.data());   // setTextIncombo
}

void TestWindow::slotCompletionsMenu(QAction *action)
{
    // Now set text in lined
    QString s = action->text();
    testLineEdit->setText(s); // Cursor is automatically at the end of string after this
}

TestWindow::~TestWindow()
{
    qDebug() << "kwindowtest finished";
}

void TestWindow::beFixed()
{
    widget->setFixedSize(400, 200);
}

void TestWindow::beYFixed()
{
    widget->setMinimumSize(400, 200);
    widget->setMaximumSize(9999, 200);
}

void TestWindow::slotImportant()
{
    statusBar->showMessage(QStringLiteral("This important message will go away in 15 seconds"), 15000);
}

void TestWindow::slotExit()
{
    if (exitB == true) {
        tb->removeAction(exitAction);
        exitB = false;
    } else {
        if (tb->actions().count() >= 7) {
            tb->insertAction(tb->actions().at(6), exitAction);
        } else {
            tb->addAction(exitAction);
        }
        exitB = true;
    }
}

void TestWindow::slotLined()
{
    lineL = !lineL;
    testLineEdit->setEnabled(lineL); // enable/disable lined
}

void TestWindow::slotToggle(bool on)
{
    if (on == true) {
        statusBar->showMessage(QStringLiteral("Toggle is on"));
    } else {
        statusBar->showMessage(QStringLiteral("Toggle is off"));
    }
}

void TestWindow::slotFrame()
{
#if 0
    if (greenF == false) {
        tb1->insertFrame(10, 100);
        tb1->alignItemRight(10);  // this is pointless 'cause tb1 is not fullwidth

        QFrame *myFrame = tb1->getFrame(10); // get frame pointer

        if (myFrame == 0) {
            warning("bad frame ID");
            return;
        }

        //paint it green
        // Or do whatever you want with it, just don't change its height (height = hardcoded = 24)
        // And don't move it
        // If you want to have something right from your toolbar you can reduce its
        // max_width with setMaxWidth()
        myFrame->setBackgroundColor(QColor("green"));

        greenF = true;
    } else {
        tb1->removeItem(10);
        greenF = false;
    }
#endif
}

void TestWindow::slotMessage(int, bool boo)
{
    if (boo) {
        statusBar->showMessage(QStringLiteral("This button does this and that"), 1500);
    } else {
        statusBar->clearMessage();
    }
}
// Now few Combo slots, for Torben

void TestWindow::slotClearCombo()
{
    testComboBox->clear();
}

void TestWindow::slotInsertListInCombo()
{
    QStringList list;
    list.append(QStringLiteral("ListOne"));
    list.append(QStringLiteral("ListTwo"));
    list.append(QStringLiteral("ListThree"));
    list.append(QStringLiteral("ListFour"));
    list.append(QStringLiteral("ListFive"));
    list.append(QStringLiteral("ListSix"));
    list.append(QStringLiteral("ListSeven"));
    list.append(QStringLiteral("ListEight"));
    list.append(QStringLiteral("ListNine"));
    list.append(QStringLiteral("ListTen"));
    list.append(QStringLiteral("ListEleven"));
    list.append(QStringLiteral("ListAndSoOn"));
    testComboBox->addItems(list);
}

void TestWindow::slotMakeItem3Current()
{
    testComboBox->setCurrentIndex(3);
}

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setApplicationName(QStringLiteral("kwindowtest"));

    QApplication myApp(argc, argv);
    TestWindow *test = new TestWindow;

    myApp.setQuitOnLastWindowClosed(false); // don't quit after the messagebox!

#if 0
    int i = QMessageBox::information(0, "Select", "Select type of mainwidget",
                                     "Fixed", "Y-fixed", "Resizable");
    if (i == 0) {
        test->beFixed();
    } else if (i == 1) {
        test->beYFixed();
    }
#endif

    test->show();
    myApp.setQuitOnLastWindowClosed(true);
    int ret = myApp.exec();

    // delete test;
    return ret;
}
