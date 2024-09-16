// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "widgets/dialog/drawdialog.h"
#include "application.h"
#include "toptoolbar.h"
#include "utils/shortcut.h"
#include "utils/global.h"
#include "cundoredocommand.h"
#include "DataHanderInterface.h"
#include "cexportimagedialog.h"
#include "dataHander/datahander.h"
#include "attribution/attributionwidget.h"
#include "ccuttool.h"
#include "groupbuttonwidget.h"
#include "rotateattriwidget.h"
#include "pageview.h"
#include "orderwidget.h"
#include "iattributionregister.h"
#include "attributionregistermanager.h"
#include "setting.h"

#include <DTitlebar>
#include <DFileDialog>
#include <DMessageManager>

#include <QVBoxLayout>
#include <QCheckBox>
#include <QDebug>
#include <QApplication>
#include <QFileInfo>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QProcess>
#include <QStandardPaths>
#include <QSettings>
#include <QScrollBar>
#include <QMimeData>
#include <QtConcurrent>
#include <QDBusReply>
#include <QDBusUnixFileDescriptor>

static void notifySystemBlocked(bool block)
{
    static QDBusReply<QDBusUnixFileDescriptor> m_reply;
    static QDBusInterface *m_pLoginManager = nullptr;
    static QList<QVariant> m_arg;

    if (m_arg.isEmpty()) {

        m_pLoginManager = new QDBusInterface("org.freedesktop.login1",
                                             "/org/freedesktop/login1",
                                             "org.freedesktop.login1.Manager",
                                             QDBusConnection::systemBus());

        m_arg << QString("shutdown")                // what
              << qApp->productName()                // who
              << QObject::tr("File not saved")      // why
              << QString("block");                  // mode

        m_reply = m_pLoginManager->callWithArgumentList(QDBus::Block, "Inhibit", m_arg);

        (void)m_reply.value().fileDescriptor();
    }

    if (!block) {
        QDBusReply<QDBusUnixFileDescriptor> tmp = m_reply;
        m_reply = QDBusReply<QDBusUnixFileDescriptor>();
    } else {
        m_reply = m_pLoginManager->callWithArgumentList(QDBus::Block, "Inhibit", m_arg);//阻止关机
    }
}

MainWindow::MainWindow(QStringList filePaths)
{
    initUI();
    if (filePaths.isEmpty())
        initAttributions();
    initConnection();
    setAcceptDrops(true);

}

void MainWindow::initUI()
{
    m_drawBoard = new DrawBoard(this);
    m_drawBoard->setAutoClose(true);
    m_drawBoard->setToolManager(new DrawBoardToolMgr(m_drawBoard, this));

    bool   darkTheme = false;
#ifdef USE_DTK
    darkTheme = (DGuiApplicationHelper::instance()->themeType()  == 2);
#endif
    QColor color  = !darkTheme ? QColor(255, 255, 255) : QColor(0, 0, 0, int(0.05 * 255));

    auto attriManager = new AttributionWidget(m_drawBoard);
    QPalette palette_attri;
    palette_attri.setColor(QPalette::Window, color);
    attriManager->setPalette(palette_attri);

    attriManager->setDisplayWidget(attriManager);
    m_drawBoard->setAttributionManager(attriManager);
    QWidget *w = new QWidget(this);
    setCentralWidget(w);
    QHBoxLayout *l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    l->setMargin(0);

    m_toolManagerScrollArea = new DScrollArea(this);
    m_toolManagerScrollArea->setWidget(m_drawBoard->toolManager());
    m_toolManagerScrollArea->setContentsMargins(0, 0, 0, 0);
    m_toolManagerScrollArea->setFixedWidth(58);
    QPalette palette_tool;

    palette_tool.setColor(QPalette::Window, color);
    m_toolManagerScrollArea->setPalette(palette_tool);
    m_toolManagerScrollArea->setFrameShape(QFrame::NoFrame);

    l->addWidget(m_toolManagerScrollArea);
    l->addWidget(m_drawBoard);
    w->setLayout(l);

    DrawToolFactory::registTool(cut, CCutTool::createTool);
    m_drawBoard->toolManager()->addTool(cut);
    m_drawBoard->toolManager()->addTool(EUndoTool);
    //设置左侧裁剪窗口隐藏
    m_drawBoard->toolManager()->tool(cut)->toolButton()->hide();
    setWgtAccesibleName(this, "MainWindow");
    drawApp->setWidgetAllPosterityNoFocus(titlebar());
    setWindowTitle(tr("Draw"));

    m_attriRegisterMgr = new AttributionRegisterManager(m_drawBoard);
    m_attriRegisterMgr->registe();

    //根据屏幕分辨率进行最小化窗口的设置
//    QDesktopWidget *desktopWidget = QApplication::desktop();
//    QRect screenRect = desktopWidget->screenGeometry();
//    int screenWidth;
//    screenWidth = screenRect.width();

    setMinimumSize(QSize(600, 300));

    if (drawApp->isTabletSystemEnvir())
        setMinimumWidth(1080);

    m_drawBoard->setFocusPolicy(Qt::StrongFocus);
    setContentsMargins(QMargins(0, 0, 0, 0));
    //setCentralWidget(m_drawBoard);

    //设置菜单样式
    DIconButton *pWidget = titlebar()->findChild<DIconButton *>("DTitlebarDWindowOptionButton"); //如果dtk修改了object名字，记得修改
    pWidget->setIcon(QIcon::fromTheme("menu"));
    pWidget->setCheckable(true);
    m_topToolbar = new TopTilte(titlebar());

    m_topToolbar->setFrameShape(DFrame::NoFrame);

    titlebar()->installEventFilter(this);
    titlebar()->setMenu(m_topToolbar->mainMenu());

    //titlebar()->setFocusPolicy(Qt::NoFocus);
    titlebar()->setIcon(QIcon::fromTheme("deepin-draw"));
    //ESC快捷键功能
    m_quitMode = new QAction(this);
    m_quitMode->setShortcut(QKeySequence(Qt::Key_Escape));
    this->addAction(m_quitMode);

    m_showCut = new QAction(this);
    m_showCut->setObjectName("shortCutManPannel");
    m_showCut->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Slash));
    this->addAction(m_showCut);
}

void MainWindow::initAttributions()
{
    //初始化显示属性
    QMetaObject::invokeMethod(this, [ = ] {
        m_drawBoard->currentPage()->showAttributions(m_drawBoard->currentPage()->scene()->currentAttris(), AttributionManager::ForceShow);
    }, Qt::QueuedConnection);
}

void MainWindow::initConnection()
{
    connect(m_drawBoard, &DrawBoard::modified, this, [ = ](bool modified) {
        notifySystemBlocked(modified);
    });

    connect(m_drawBoard, &DrawBoard::toClose, drawApp, &Application::quitApp);
    connect(m_topToolbar, &TopTilte::editProportion, m_drawBoard, [ = ]() {
        if (m_drawBoard->currentTool() != cut)
            m_drawBoard->setCurrentTool(cut);
        else
            m_drawBoard->setCurrentTool(selection);
    });

    connect(m_drawBoard, &DrawBoard::zoomValueChanged, m_topToolbar, &TopTilte::slotSetScale);
    connect(m_topToolbar, &TopTilte::zoomTo, m_drawBoard, [ = ](qreal total) {
        QSignalBlocker blocker(m_drawBoard);
        m_drawBoard->zoomTo(total);
    });

    connect(m_drawBoard, qOverload<Page *>(&DrawBoard::currentPageChanged), m_topToolbar, [ = ](Page * page) {
        m_topToolbar->setTitleName(page->title());
    });
    connect(drawApp, &Application::quitRequest, this, [ = ] {
        if (drawBoard()->close())
        {
            drawApp->quitApp();
            _Exit(0);//确保正常退出
        }
    });

    connect(m_topToolbar, &TopTilte::toOpen, this, &MainWindow::slotShowOpenFileDialog);

    connect(m_quitMode, &QAction::triggered, this, &MainWindow::slotOnEscButtonClick);

    connect(m_showCut, &QAction::triggered, this, &MainWindow::onViewShortcut);

    connect(m_topToolbar, &TopTilte::creatOnePage, m_drawBoard, [ = ]() {
        m_drawBoard->addPage();
    });

    connect(m_topToolbar, &TopTilte::toExport, this, [ = ]() {
        if (m_drawBoard->currentPage() != nullptr) {
            auto currentTool = m_drawBoard->currentTool_p();

            bool refuse = currentTool != nullptr ? currentTool->blockPageBeforeOutput(m_drawBoard->currentPage()) : false;
            if (refuse) {
                return;
            }

            CExportImageDialog dialog(m_drawBoard);
            int ret = dialog.execFor(m_drawBoard->currentPage());
            if (ret != -1) {
                bool success = (ret == 1);

                if (pDFloatingMessage == nullptr) {
                    pDFloatingMessage = new DFloatingMessage(DFloatingMessage::MessageType::TransientType, drawApp->topMainWindow());
                }
                pDFloatingMessage->show();
                pDFloatingMessage->setBlurBackgroundEnabled(true);
                // Export success应改为Export successful
                pDFloatingMessage->setMessage(success ? tr("Export successful") : tr("Export failed"));
                pDFloatingMessage->setIcon(!success ? QIcon::fromTheme(":/icons/deepin/builtin/texts/warning_new_32px.svg") :
                                           QIcon(":/icons/deepin/builtin/texts/notify_success_32px.svg"));
                pDFloatingMessage->setDuration(2000); //set 2000ms to display it
                DMessageManager::instance()->sendMessage(drawApp->topMainWindow(), pDFloatingMessage);
            }
        }
    });

    //cut按钮置灰时，mainwindow上的按钮也同样置灰
    connect(m_drawBoard->tool(cut)->toolButton(), &QAbstractButton::toggled, this, [ = ](bool checked) {
        topTitle()->editSceneRectButton()->setChecked(checked);
    });

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [ = ](DGuiApplicationHelper::ColorType themeType) {
        QColor color = themeType == DGuiApplicationHelper::LightType ? QColor(255, 255, 255) : QColor(0, 0, 0, int(0.05 * 255));
        QPalette palette_attri;
        palette_attri.setColor(QPalette::Window, color);
        auto attriManager = m_drawBoard->attributionManager();
        if (attriManager && attriManager->displayWidget())
            attriManager->displayWidget()->setPalette(palette_attri);

        QPalette palette_tool;
        palette_tool.setColor(QPalette::Window, color);
        m_toolManagerScrollArea->setPalette(palette_tool);
    });
}

void MainWindow::activeWindow()
{
    window()->show();
    window()->raise();
    window()->activateWindow();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    DMainWindow::resizeEvent(event);
    this->update();
}

void MainWindow::slotShowOpenFileDialog()
{
    DFileDialog dialog(this);
    dialog.setWindowTitle(tr("Open"));//设置文件保存对话框的标题
    dialog.setAcceptMode(QFileDialog::AcceptOpen);//设置文件对话框为保存模式
    dialog.setViewMode(DFileDialog::List);
    dialog.setFileMode(DFileDialog::ExistingFiles);
    if (Application::isTabletSystemEnvir())
        dialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    else
        dialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    QStringList nameFilters;

    auto formatsList = Setting::instance()->readableFormats();
    auto formats = QString(" *.") + formatsList.join(" *.");
    nameFilters << formats;
    dialog.setNameFilters(nameFilters);//设置文件类型过滤器
    dialog.setDirectory(Setting::instance()->defaultFileDialogPath());
    QStringList picturePathList;


    if (dialog.exec()) {
        QStringList tempfilePathList = dialog.selectedFiles();
        int ret = m_drawBoard->execPicturesLimit(tempfilePathList.size());
        if (ret == 0) {
            drawBoard()->loadFiles(tempfilePathList);
        }
    }
}


void MainWindow::loadFiles(const QStringList &filePaths)
{
    int pictureCount = 0;
    foreach (auto path, filePaths) {
        QFileInfo info(path);
        auto stuff = info.suffix().toLower();
        if (FileHander::supPictureSuffix().contains(stuff)) {
            pictureCount ++;
        }
    }

    int ret = m_drawBoard->execPicturesLimit(pictureCount);
    if (ret == 0) {
        m_drawBoard->loadFiles(filePaths, true, 0, true);
    }
}

void MainWindow::onViewShortcut()
{
    QRect rect = window()->geometry();
    QPoint pos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
    Shortcut sc;
    QStringList shortcutString;
    QString param1 = "-j=" + sc.toStr();
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    shortcutString /*<< "-b"*/ << param1 << param2;

    QProcess *shortcutViewProc = new QProcess(this);
    //shortcutViewProc->startDetached("killall deepin-shortcut-viewer");
    shortcutViewProc->startDetached("deepin-shortcut-viewer", shortcutString);

    connect(shortcutViewProc, SIGNAL(finished(int)), shortcutViewProc, SLOT(deleteLater()));
}

void MainWindow::slotOnEscButtonClick()
{
    drawApp->setCurrentTool(selection);
}

void MainWindow::setWindowTitleInfo()
{
    Page *current_page = m_drawBoard->currentPage();
    if (current_page) {
        qDebug() << __FUNCTION__ << "--" << current_page->context()->file();
        if (current_page->context()->file() != "") {
            this->setWindowTitle(current_page->context()->file());
        } else {
            this->setWindowTitle(current_page->context()->name());
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (event == nullptr)
        return;

//    QString fileName = Global::configPath() + "/config.conf";
//    QSettings settings(fileName, QSettings::IniFormat);
//    settings.setValue("geometry", saveGeometry());
//    settings.setValue("windowState", saveState());
//    settings.setValue("opened", "true");
    drawApp->saveSettings();
    emit drawApp->quitRequest();
    event->ignore();

    closeEvent(nullptr);
}

bool MainWindow::eventFilter(QObject *o, QEvent *e)
{
    if (o->isWidgetType()) {
        if (titlebar() == o) {
            //实现m_topToolbar的正确位置放置
            if (e->type() == QEvent::Resize) {
                if (m_topToolbar != nullptr) {
                    m_topToolbar->setGeometry(60, 0, titlebar()->width() - 60 - 200, titlebar()->height());
                }
            }
        }
    }
    return DMainWindow::eventFilter(o, e);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
        return;
    }
    DWidget::dragMoveEvent(event);
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->setDropAction(Qt::MoveAction);
        event->accept();
        return;
    }
    DWidget::dragMoveEvent(event);
}

void MainWindow::dropEvent(QDropEvent *e)
{
    if (e->mimeData()->hasText()) {
        QList<QUrl> urls  = e->mimeData()->urls();

        int ret = m_drawBoard->execPicturesLimit(urls.count());

        if (ret == 0) {
            QStringList fileLists;
            foreach (auto url, urls) {
                QString filePath = url.path();
                if (!filePath.isEmpty()) {
                    fileLists.append(filePath);
                }
            }
            drawBoard()->loadFiles(fileLists);
        }
    }
    DWidget::dropEvent(e);
}

void MainWindow::readSettings()
{
    drawApp->readSettings();
}

bool MainWindow::openFiles(QStringList filePaths, bool bAdapt)
{
    if (nullptr == m_drawBoard)
        return false;

    m_drawBoard->loadFiles(filePaths, true, bAdapt);
    return true;
}

DrawBoard *MainWindow::drawBoard() const
{
    return m_drawBoard;
}

TopTilte *MainWindow::topTitle() const
{
    return m_topToolbar;
}


MainWindow::~MainWindow()
{
//    foreach (auto tool, CDrawToolFactory::allTools()) {
//        delete tool;
//    }
//    CDrawToolFactory::allTools().clear();
//    CUndoRedoCommand::clearCommand();
}
