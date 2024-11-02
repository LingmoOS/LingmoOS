/*
* Copyright (C) 2020 Tianjin LINGMO Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "widget.h"
#include "ui_widget.h"
#include "editPage.h"
#include "information_collector.h"
#include "windowmanage.hpp"
#include "paletteWidget.h"
#include <lingmosdk/diagnosetest/libkydatacollect.h>

/*!
 * 系统时间
 */
#define FORMAT_SCHEMA   "org.lingmo.control-center.panel.plugins"
#define TIME_FORMAT_KEY "hoursystem"
#define STYLE_ICON                "icon-theme-name"
#define STYLE_ICON_NAME           "iconThemeName"
QFont g_currentFont;
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

Widget::Widget(QWidget *parent) :
    CustomWidget(parent),
    ui(new Ui::Widget),
    m_noteExitWindow(new noteExitWindow(this, parent)),
    m_emptyNotes(Q_NULLPTR),
    m_autoSaveTimer(new QTimer(this)),
    m_settingsDatabase(Q_NULLPTR),
    m_searchLine(Q_NULLPTR),
    m_newKynote(Q_NULLPTR),
    m_viewChangeButton(Q_NULLPTR),
    m_noteView(Q_NULLPTR),
    m_noteTable(Q_NULLPTR),
    m_noteModel(new NoteModel(this)),
    m_deletedNotesModel(new NoteModel(this)),
    m_proxyModel(new QSortFilterProxyModel(this)),
    m_dbManager(Q_NULLPTR),
    m_dbThread(Q_NULLPTR),
    m_isContentModified(false),
    m_isColorModified(false),
    m_isOperationRunning(false),
    mousePressed(false),
    m_isTextCpNew(false),
    m_isThemeChanged(false)   // lingmo-default
{
    kabase::WindowManage::getWindowId(&m_windowId);
    kabase::WindowManage::removeHeader(this);


    ui->setupUi(this->baseBar());
    m_noteView = ui->listView;
    setupDatabases();
    listenToGsettings();
    kyNoteInit(parent);
    kyNoteConn();
    initData();
    m_noteView->setContextMenuPolicy(Qt::CustomContextMenu);

    g_currentFont.setFamily(QApplication::font().family());
    g_currentFont.setPointSizeF(QApplication::font().pointSizeF());

    m_noteView->setStyleSheet("QListView{border-radius:4px;}");
}

/*!
 * \brief Widget::~Widget
 * Deconstructor of the class
 */
Widget::~Widget()
{
    //在程序推出前调用，不需要传入参数
    kdk_dia_upload_program_exit();

    for (auto it = m_editors.begin(); it != m_editors.end(); it++) {
        (*it)->close();
        delete *it;
    }
    m_editors.clear();
    if(ui!=nullptr){
        delete ui;
        ui = nullptr;
    }
    if(m_dbManager!=nullptr) {
        delete m_dbManager;
        m_dbManager = nullptr;
    }
    if(m_dbThread!=nullptr)
    {
        m_dbThread->quit();
        m_dbThread->wait();
        delete m_dbThread;
        m_dbThread = nullptr;
    }
}

/*!
 * \brief Widget::initData
 * Init the data from database and select the first note if there is one
 * 初始化数据库中的数据并选中第一个便签（如果有）
 */
void Widget::initData()
{
    qDebug() << "kyNote initData";

    QFileInfo fi(m_settingsDatabase->fileName());
    QDir dir(fi.absolutePath());
    QString oldNoteDBPath(dir.path() + QStringLiteral("/Notes.ini"));
    QString oldTrashDBPath(dir.path() + QStringLiteral("/Trash.ini"));

    bool exist = (QFile::exists(oldNoteDBPath) || QFile::exists(oldTrashDBPath));

    if (exist) {
        QProgressDialog *pd = new QProgressDialog(QStringLiteral(
                                                      "Migrating database, please wait."),
                                                  QString(), 0, 0, this);
        pd->setCancelButton(Q_NULLPTR);
        pd->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        pd->setMinimumDuration(0);
        pd->show();

        // setButtonsAndFieldsEnabled(false);

        QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
        connect(watcher, &QFutureWatcher<void>::finished, this, [&, pd](){
            pd->deleteLater();
            // setButtonsAndFieldsEnabled(true);
            emit requestNotesList();
        });

        QFuture<void> migration = QtConcurrent::run(this, &Widget::checkMigration);
        watcher->setFuture(migration);
    } else {
        emit requestNotesList();
    }
    iniNoteModeRead();
}

/*!
 * \brief Widget::setupListModeModel
 *
 */
void Widget::setupListModeModel()
{
    m_proxyModel->setSourceModel(m_noteModel);          // 代理真正的数据模型，对数据进行排序和过滤
    m_proxyModel->setFilterKeyColumn(0);                // 此属性保存用于读取源模型内容的键的列,listview只有一列所以是0
    m_proxyModel->setFilterRole(NoteModel::NoteMdContent);// 此属性保留项目角色，该角色用于在过滤项目时查询源模型的数据
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);//
    m_plistDelegate = new listViewModeDelegate(m_noteView);
    m_noteView->setItemDelegate(m_plistDelegate);    // 安装定制delegate提供编辑功能
    m_noteView->setModel(m_proxyModel); // 设置view的model是proxyModel，proxyModel作为view和QAbstractListModel的桥梁
}

/*!
 * \brief Widget::setupIconModeModel
 *
 */
void Widget::setupIconModeModel()
{
    m_proxyModel->setSourceModel(m_noteModel);          // 代理真正的数据模型，对数据进行排序和过滤
    m_proxyModel->setFilterKeyColumn(0);                // 此属性保存用于读取源模型内容的键的列,listview只有一列所以是0
    m_proxyModel->setFilterRole(NoteModel::NoteMdContent);// 此属性保留项目角色，该角色用于在过滤项目时查询源模型的数据
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);//

    m_noteView->setItemDelegate(new iconViewModeDelegate(m_noteView));    // 安装定制delegate提供编辑功能
    m_noteView->setModel(m_proxyModel); // 设置view的model是proxyModel，proxyModel作为view和QAbstractListModel的桥
}

/*!
 * \brief Widget::initializeSettingsDatabase
 *
 */
void Widget::initializeSettingsDatabase()
{
    if (m_settingsDatabase->value(QStringLiteral("version"), "NULL") == "NULL")
        m_settingsDatabase->setValue(QStringLiteral("version"), qApp->applicationVersion());

    if (m_settingsDatabase->value(QStringLiteral("windowGeometry"), "NULL") == "NULL") {
        int initWidth = 704;
        int initHeight = 590;
        QPoint center = qApp->desktop()->geometry().center();
        QRect rect(center.x() - initWidth/2, center.y() - initHeight/2, initWidth, initHeight);
        setGeometry(rect);      // 设置窗口居中
        m_settingsDatabase->setValue(QStringLiteral("windowGeometry"), saveGeometry());     // 保存窗口的几何形状
    }
}

/*!
 * \brief Widget::setupDatabases
 *
 */
void Widget::setupDatabases()
{
    // QSettings::IniFormat 将设置存储在INI文件中。从INI文件读取设置时不会保留类型信息。所有值将作为QString返回
    // QSettings::UserScope 将设置存储在特定于当前用户的位置
    m_settingsDatabase = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                                       QStringLiteral("lingmo-note"), QStringLiteral(
                                           "Settings"), this);
    m_settingsDatabase->setFallbacksEnabled(false);     // 禁用回退
    initializeSettingsDatabase();

    bool doCreate = false;
    QFileInfo fi(m_settingsDatabase->fileName());
    QDir dir(fi.absolutePath());
    bool folderCreated = dir.mkpath(QStringLiteral("."));
    if (!folderCreated)
        qFatal("ERROR: Can't create settings folder : %s",
               dir.absolutePath().toStdString().c_str());

    QString noteDBFilePath(dir.path() + QDir::separator() + QStringLiteral("kyNotes.db"));

    if (!QFile::exists(noteDBFilePath)) {
        QFile noteDBFile(noteDBFilePath);
        if (!noteDBFile.open(QIODevice::WriteOnly))
            qFatal("ERROR : Can't create database file");

        noteDBFile.close();
        doCreate = true;
    }

    m_dbManager = new DBManager;
    m_dbThread = new QThread;
    m_dbThread->setObjectName(QStringLiteral("dbThread"));
    m_dbManager->moveToThread(m_dbThread);
    connect(m_dbThread, &QThread::started, [=](){
        emit requestOpenDBManager(noteDBFilePath, doCreate);
    });
    connect(this, &Widget::requestOpenDBManager, m_dbManager, &DBManager::onOpenDBManagerRequested);
    connect(m_dbThread, &QThread::finished, m_dbManager, &QObject::deleteLater);
    m_dbThread->start();
}

/*!
 * \brief Widget::kyNoteInit
 *
 */
void Widget::kyNoteInit(QWidget *parent)
{
    sortflag = 1;           // 排序
    m_listflag = 1;         // 平铺\列表


    m_searchLine = ui->SearchLine;
    m_newKynote = ui->newKynote;
    m_viewChangeButton = ui->viewChangeButton;

    initListMode();
//    setFixedSize(714, 590);
    setMouseTracking(true);               // 设置鼠标追踪


    // 弹出位置
    m_pSreenInfo = new adaptScreenInfo(this);
    move((m_pSreenInfo->m_screenWidth - this->width() + m_pSreenInfo->m_nScreen_x)/2,
         (m_pSreenInfo->m_screenHeight - this->height())/2);
    // 标题
    setWidgetName(tr("Notes"));
    // 任务栏图标
    //setWindowIcon(QIcon::fromTheme("lingmo-notebook"));

    // 钮
    btnInit();
    // 搜索
    searchInit();


    // 退出框
    m_noteExitWindow = new noteExitWindow(this, parent);
    //清除便签页弹框
    m_emptyNotes = new emptyNotes(this);
}

/*!
 * \brief Widget::kyNoteConn
 *
 */
void Widget::kyNoteConn()
{
    qDebug() << "init connect slot";
    // 主界面退出按钮
    connect(this->windowButtonBar()->closeButton(), SIGNAL(clicked()), this, SLOT(exitSlot()));
    // 主界面最小化按钮
//    connect(this->windowButtonBar()->minimumButton(), SIGNAL(clicked()), this, SLOT(miniSlot()));
    // 新建按钮
    connect(m_newKynote, &QPushButton::clicked, this, &Widget::newSlot);
    // 除按钮
    connect(this, &Widget::switchSortTypeRequest, this, &Widget::sortSlot);
    // 清空便签
    connect(m_menuActionEmpty, &QAction::triggered, this, &Widget::trashSlot);
    connect(m_emptyNotes, &emptyNotes::requestEmptyNotes, this, &Widget::clearNoteSlot);
    // 菜单按钮退出便签本
    connect(m_menuExit, &QAction::triggered, this, &Widget::exitSlot);
    // 列表平铺切换
    connect(m_viewChangeButton, &QPushButton::clicked, this, &Widget::changePageSlot);
    // 搜索栏文本输入
    connect(m_searchLine, &QLineEdit::textChanged, this, &Widget::onSearchEditTextChanged);
    // 退出弹窗
    connect(m_noteExitWindow, &noteExitWindow::requestSetNoteNull, this, &Widget::setNoteNullSlot);
    // listview单击事件
    connect(m_noteView, &NoteView::pressed, this, &Widget::listClickSlot);
    // listview双击事件
    connect(m_noteView, &NoteView::doubleClicked, this, &Widget::listDoubleClickSlot);
    connect(m_noteView, &NoteView::customContextMenuRequested, this, &Widget::ShowContextMenu);
    connect(m_noteView, &NoteView::viewportPressed, this, [this](){
        if (m_noteModel->rowCount() > 0) {
            QModelIndex index = m_noteView->currentIndex();
            if(index!=m_currentSelectedNoteProxy) {
                m_currentSelectedNoteProxy = index;
                m_noteView->selectionModel()->select(m_currentSelectedNoteProxy,
                                                     QItemSelectionModel::ClearAndSelect);
                m_noteView->setCurrentIndex(m_currentSelectedNoteProxy);
                m_noteView->scrollTo(m_currentSelectedNoteProxy);
            }
        }
    });

    // auto save timer
    connect(m_autoSaveTimer, &QTimer::timeout, [this](){
        m_autoSaveTimer->stop();
        saveNoteToDB(m_currentSelectedNoteProxy);
    });
    // 指定传递信号的方式Qt::BlockingQueuedConnection
    // 这种类型类似于QueuedConnection，但是它只能应用于跨线程操作即发送者和接收者处于不同的线程中的情况
    // 并且信号发送者线程会阻塞等待接收者的槽函数执行结束
    connect(this, &Widget::requestNotesList,
            m_dbManager, &DBManager::onNotesListRequested, Qt::BlockingQueuedConnection);
    connect(this, &Widget::requestCreateUpdateNote,
            m_dbManager, &DBManager::onCreateUpdateRequested, Qt::BlockingQueuedConnection);
    connect(this, &Widget::requestDeleteNote,
            m_dbManager, &DBManager::onDeleteNoteRequested);
    connect(this, &Widget::requestClearNote,
            m_dbManager, &DBManager::permanantlyRemoveAllNotes);
    connect(this, &Widget::requestMigrateNotes,
            m_dbManager, &DBManager::onMigrateNotesRequested, Qt::BlockingQueuedConnection);
    connect(this, &Widget::requestMigrateTrash,
            m_dbManager, &DBManager::onMigrateTrashRequested, Qt::BlockingQueuedConnection);
    connect(this, &Widget::requestForceLastRowIndexValue,
            m_dbManager, &DBManager::onForceLastRowIndexValueRequested,
            Qt::BlockingQueuedConnection);

    connect(m_dbManager, &DBManager::notesReceived, this, &Widget::loadNotes);
    // 快捷键
    new QShortcut(QKeySequence(Qt::Key_F1), this, SLOT(onF1ButtonClicked()));
}

/*!
 * \brief Widget::listenToGsettings
 *
 */
void Widget::listenToGsettings()
{
    // 监听主题改变
    const QByteArray id(THEME_QT_SCHEMA);

    if (QGSettings::isSchemaInstalled(id)) {
        QGSettings *styleSettings = new QGSettings(id, QByteArray(), this);
        m_currentTheme = styleSettings->get("styleName").toString();
        if(m_currentTheme.compare("lingmo-dark")==0 || m_currentTheme.compare("lingmo-black")==0){
            m_isThemeChanged = 1;
        }
        connect(styleSettings, &QGSettings::changed, this, [=](const QString &key){
            auto style = styleSettings->get(key).toString();
            if (key == "styleName") {
                m_currentTheme = styleSettings->get(MODE_QT_KEY).toString();
                if (m_currentTheme == "lingmo-default" || m_currentTheme == "lingmo-white"
                    || m_currentTheme == "lingmo-light" || m_currentTheme == "lingmo") {
                    m_isThemeChanged = 0;
                    //edit-find-symbolic
                    //m_searchAction->setIcon(QIcon::fromTheme("edit-find-symbolic"));
                } else if (style == "lingmo-dark" || m_currentTheme == "lingmo-black") {
                    m_isThemeChanged = 1;
                }
            }
            if(key==STYLE_ICON_NAME || key==STYLE_ICON){
            //主题框架不能更新 titleIcon
            }
        });
        connect(styleSettings,&QGSettings::changed,this, [=] (const QString &key) {
            if(key == FONT_STYLE || key == FONT_SIZE)
            {
                //获取字号的值
                QString fontStyle = styleSettings->get(FONT_STYLE).toString();
                auto fontSizeKey = styleSettings->get(FONT_SIZE).toString().toFloat();

                //发送改变信号
                g_currentFont.setFamily(fontStyle);
                g_currentFont.setPointSizeF(fontSizeKey);
            }
        });

        QString fontStyle = styleSettings->get(FONT_STYLE).toString();
        auto fontSizeKey = styleSettings->get(FONT_SIZE).toString().toFloat();

        //发送改变信号
        g_currentFont.setFamily(fontStyle);
        g_currentFont.setPointSizeF(fontSizeKey);
    }

    // 监听控制面板字体变化
    const QByteArray idd(PERSONALISE_SCHEMA);

    if (QGSettings::isSchemaInstalled(idd)) {
        QGSettings *opacitySettings = new QGSettings(idd, QByteArray(), this);
        connect(opacitySettings, &QGSettings::changed, this, [=](const QString &key){
            if (key == "transparency") {
                QStringList keys = opacitySettings->keys();
                if (keys.contains("transparency")) {
                    m_transparency = opacitySettings->get("transparency").toDouble();
                }
            }
            repaint();
        });
        m_transparency = opacitySettings->get("transparency").toDouble();
    }

    // 用户手册
    QString serviceName = "com.lingmoUserGuide.hotel"
                          + QString("%1%2").arg("_").arg(QString::number(getuid()));
    userGuideInterface = new QDBusInterface(serviceName,
                                            "/",
                                            "com.guide.hotel",
                                            QDBusConnection::sessionBus(), this);
    qDebug() << "connect to lingmoUserGuide" << userGuideInterface->isValid();
    if (!userGuideInterface->isValid()) {
        qDebug() << "fail to connect to lingmoUserGuide";
        qDebug() << qPrintable(QDBusConnection::sessionBus().lastError().message());
        return;
    }

    // 监听时区变化
    const QByteArray iddd(FORMAT_SCHEMA);

    if (QGSettings::isSchemaInstalled(iddd)) {
        QGSettings *m_formatsettings = new QGSettings(iddd, QByteArray(), this);

        connect(m_formatsettings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "hoursystem") {
                QString value = m_formatsettings->get(TIME_FORMAT_KEY).toString();
                qDebug() << "hoursystem changed" << value;
                m_noteView->viewport()->update();
            }
        });
    }
    // 时间格式
   QDBusInterface *iface = new QDBusInterface("com.lingmo.lingmosdk.DateServer",
                                              "/com/lingmo/lingmosdk/Date",
                                              "com.lingmo.lingmosdk.DateInterface",
                                              QDBusConnection::sessionBus(),
                                              this);
    if(iface->isValid()){
        connect(iface,SIGNAL(ShortDateSignal(QString)),this,SLOT(ShortDateChangeSlot(QString)));
    }else{
        qDebug()<<"fail to connect to lingmosdk.DateInterface";
    }
}

/*!
 * \brief Widget::checkMigration
 *
 */
void Widget::checkMigration()
{
    qDebug() << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
    QFileInfo fi(m_settingsDatabase->fileName());
    QDir dir(fi.absolutePath());

    QString oldNoteDBPath(dir.path() + QDir::separator() + "Notes.ini");
    if (QFile::exists(oldNoteDBPath))
        migrateNote(oldNoteDBPath);

    QString oldTrashDBPath(dir.path() + QDir::separator() + "Trash.ini");
    if (QFile::exists(oldTrashDBPath)) {
        // migrateTrash(oldTrashDBPath);
        emit requestForceLastRowIndexValue(m_noteCounter);
    }
}

/*!
 * \brief Widget::migrateNote
 *
 */
void Widget::migrateNote(QString notePath)
{
    qDebug() << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
    QSettings notesIni(notePath, QSettings::IniFormat);
    QStringList dbKeys = notesIni.allKeys();

    m_noteCounter = notesIni.value(QStringLiteral("notesCounter"), "0").toInt();
    QList<NoteData *> noteList;

    auto it = dbKeys.begin();
    for (; it < dbKeys.end() - 1; it += 3) {
        QString noteName = it->split(QStringLiteral("/"))[0];
        int id = noteName.split(QStringLiteral("_"))[1].toInt();

        // sync db index with biggest notes id
        m_noteCounter = m_noteCounter < id ? id : m_noteCounter;

        NoteData *newNote = new NoteData(this);
        newNote->setId(id);

        QString createdDateDB
            = notesIni.value(noteName + QStringLiteral("/dateCreated"), "Error").toString();
        newNote->setCreationDateTime(QDateTime::fromString(createdDateDB, Qt::ISODate));
        QString lastEditedDateDB
            = notesIni.value(noteName + QStringLiteral("/dateEdited"), "Error").toString();
        newNote->setLastModificationDateTime(QDateTime::fromString(lastEditedDateDB, Qt::ISODate));
        QString contentText
            = notesIni.value(noteName + QStringLiteral("/content"), "Error").toString();
        newNote->setContent(contentText);
        QString firstLine = contentText;
        newNote->setFullTitle(firstLine);

        noteList.append(newNote);
    }

    if (!noteList.isEmpty())
        emit requestMigrateNotes(noteList);

    QFile oldNoteDBFile(notePath);
    oldNoteDBFile.rename(QFileInfo(notePath).dir().path() + QDir::separator()
                         + QStringLiteral("oldNotes.ini"));
}

/*!
 * \brief Widget::openMemoWithId
 *
 */
void Widget::openMemoWithId(int noteId)
{
    qDebug() << "openMemoWithId" << noteId;

    QTimer::singleShot(300, this, [=]() {
        m_notebook = new EditPage(this, noteId);
        m_editors.push_back(m_notebook);
        m_notebook->m_id = m_editors.size() - 1;

        for (int count = 0; count <= m_proxyModel->rowCount(); count++) {
            QModelIndex m_tmpIndex = m_proxyModel->index(count, 0);

            if (m_tmpIndex.data(NoteModel::NoteID).toInt() == noteId) {
                m_currentSelectedNoteProxy = m_proxyModel->mapToSource(m_tmpIndex);

                showNoteInEditor(m_currentSelectedNoteProxy);
                m_noteView->selectionModel()->select(m_currentSelectedNoteProxy,
                                                     QItemSelectionModel::ClearAndSelect);
                m_noteView->setCurrentIndex(m_currentSelectedNoteProxy);
                m_noteView->scrollTo(m_currentSelectedNoteProxy);
                break;
            }
        }

        connect(m_notebook->m_noteHeadMenu, &noteHeadMenu::requestNewNote, this, [=](){
            newSlot();
        });
        connect(m_notebook->m_noteHeadMenu, &noteHeadMenu::requestShowNote, this, [=] {
            // 添加窗管协议
            kabase::WindowManage::removeHeader(this);
            if(isHidden()||!isActiveWindow()){
                setWindowState(windowState() & ~Qt::WindowMinimized);
                this->show();
                kdk::WindowManager::activateWindow(m_windowId);
            }
            kabase::WindowManage::setMiddleOfScreen(this);
        });
        connect(m_editors[m_editors.size() - 1], &EditPage::requestDel, this, [=](int noteId){
            for (int count = 0; count <= m_proxyModel->rowCount(); count++) {
                QModelIndex m_tmpIndex = m_proxyModel->index(count, 0);
                if (m_tmpIndex.data(NoteModel::NoteID).toInt() == noteId) {
                    QModelIndex sourceIndex = m_proxyModel->mapToSource(m_tmpIndex);
                    deleteNote(m_tmpIndex, true);
                    break;
                }
            }
        });
        connect(m_editors[m_editors.size() - 1], SIGNAL(texthasChanged(int,int)), this,
                SLOT(onTextEditTextChanged(int,int)));
        connect(m_editors[m_editors.size() - 1], SIGNAL(colorhasChanged(QColor,int)), this,
                SLOT(onColorChanged(QColor,int)));

        // 设置鼠标焦点
        m_notebook->ui->textEdit->setFocus();
        // 移动光标至行末
        m_notebook->ui->textEdit->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
        m_notebook->show();
    });
}

/*!
 * \brief Widget::btnInit
 *
 */
void Widget::btnInit()
{
    m_menu = new QMenu(this->windowButtonBar()->menuButton());
    m_menu->setProperty("fillIconSymbolicColor", true);

    m_menuActionEmpty = new QAction(m_menu);
    QAction *m_helpAction = new QAction(m_menu);
    QAction *m_aboutAction = new QAction(m_menu);
    // m_menuActionSet = new QAction(m_menu);
    m_menuExit = new QAction(m_menu);

    m_helpAction->setText(tr("Help"));
    m_aboutAction->setText(tr("About"));
    m_menuActionEmpty->setText(tr("Empty Note"));
    // m_menuActionSet->setText(tr("Set Note"));
    m_menuExit->setText(tr("Exit"));

    m_menu->addAction(m_menuActionEmpty);
    m_menu->addAction(m_helpAction);
    m_menu->addAction(m_aboutAction);
    // m_menu->addAction(m_menuActionSet);
    m_menu->addAction(m_menuExit);
    this->windowButtonBar()->menuButton()->setMenu(m_menu);

    connect(m_helpAction, &QAction::triggered, this, [=](){
        qDebug() << "help clicked";
        userGuideInterface->call(QString("showGuide"), "lingmo-notebook");
    });
    connect(m_aboutAction, &QAction::triggered, this, [=](){
        About dialog(this);
        kabase::WindowManage::removeHeader(&dialog);
        dialog.exec();
    });


    m_newKynote->setIcon(QIcon::fromTheme ("list-add-symbolic"));
    m_newKynote->setButtonType(kdk::KPushButton::CircleType);
    m_newKynote->setFlat(true);
    m_newKynote->setIconSize(QSize(28,28));
    m_newKynote->setIconColor(Qt::white);
    m_newKynote->setBackgroundColorHighlight(true);

    m_viewChangeButton->setIcon(QIcon::fromTheme("view-grid-symbolic"));


    m_viewChangeButton->setProperty("useIconHighlightEffect", true);
    m_viewChangeButton->setProperty("iconHighlightEffectMode", 1);


    m_newKynote->setToolTip(tr("Create New Note"));
    m_viewChangeButton->setToolTip(tr("Switch View"));
}

/*!
 * \brief Widget::getListFlag
 *
 */
int Widget::getListFlag() const
{
    return m_listflag;
}

/*!
 * \brief Widget::setListFlag
 *
 */
void Widget::setListFlag(const int &listflag)
{
    m_listflag = listflag;
}

/*!
 * \brief Widget::initIconMode
 *
 */
void Widget::initIconMode()
{
    InformationCollector::getInstance().addPoint(InformationCollector::IconMode);
    qDebug() << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
    m_noteView->setViewMode(QListView::IconMode);
    m_noteView->setSelectionMode(QListView::ExtendedSelection);
    m_noteView->setEditTriggers(QListView::NoEditTriggers);
    m_noteView->setResizeMode(QListView::Adjust);
    // m_noteView->setMovement(QListView::Snap);
    m_noteView->setContextMenuPolicy(Qt::CustomContextMenu);
    // m_noteView->setGridSize(QSize(227, 246));
    // m_noteView->setIconSize(QSize(227, 246));
    setupIconModeModel();
}

/*!
 * \brief Widget::initListMode
 *
 */
void Widget::initListMode()
{
    InformationCollector::getInstance().addPoint(InformationCollector::ListMode);
    qDebug() << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
    // 列表模式
    m_noteView->setViewMode(QListView::ListMode);
    // 禁用双击编辑
    m_noteView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // 竖向滑动条
    m_noteView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    // 隐藏横向滑动条
    m_noteView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_noteView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    // 视图将一次滚动一个像素的内容
    m_noteView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_noteView->setFrameShape(QListView::NoFrame);
    // 启用项的拖动
    // m_noteView->setDragEnabled(true);
    // 允许用户将内部或外部项拖放到视图中
    // m_noteView->setAcceptDrops(true);
    // 显示当前拖动的项将被放在什么地方
    // m_noteView->setDropIndicatorShown(true);
    setupListModeModel();
}

/*!
 * \brief Widget::deleteNote
 *
 */
void Widget::deleteNote(const QModelIndex &noteIndex, bool isFromUser)
{
    InformationCollector::getInstance().addPoint(InformationCollector::Delete);
    qDebug() << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
    if (noteIndex.isValid()) {
        // delete from model
        QModelIndex indexToBeRemoved = m_proxyModel->mapToSource(noteIndex);
        NoteData *noteTobeRemoved = m_noteModel->removeNote(indexToBeRemoved);

        noteTobeRemoved->setDeletionDateTime(QDateTime::currentDateTime());
        qDebug() << "emit requestDeleteNote";
        // delete from sql
        emit requestDeleteNote(noteTobeRemoved);
        if (isFromUser) {
            if (m_noteModel->rowCount() > 0) {
                QModelIndex index = m_noteView->currentIndex();
                m_currentSelectedNoteProxy = index;
                m_noteView->selectionModel()->select(m_currentSelectedNoteProxy,
                                                     QItemSelectionModel::ClearAndSelect);
                m_noteView->setCurrentIndex(m_currentSelectedNoteProxy);
                m_noteView->scrollTo(m_currentSelectedNoteProxy);
            } else {
                // 创建新的空模型索引 此类型的模型索引用于指示模型中的位置无效
                m_currentSelectedNoteProxy = QModelIndex();
            }
        }
    } else {
        qDebug() << "Widget::deleteNote noteIndex is not valid";
    }

    m_noteView->setFocus();
}

/*!
 * \brief Widget::deleteSelectedNote
 *
 */
void Widget::deleteSelectedNote()
{
    InformationCollector::getInstance().addPoint(InformationCollector::DeleteCurrent);
    qDebug() << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
    if (!m_isOperationRunning) {
        m_isOperationRunning = true;
        if (m_noteModel->rowCount() > 0 && m_currentSelectedNoteProxy.isValid()) {
            int noteId = m_currentSelectedNoteProxy.data(NoteModel::NoteID).toInt();
            for (auto it = m_editors.begin(); it != m_editors.end(); it++) {
                if ((*it)->m_noteId == noteId) {
                    m_notebook = *it;
                    m_notebook->close();
                    delete m_notebook;
                    m_editors.erase(it);
                    break;
                }
            }
            // --m_noteCounter;
            deleteNote(m_currentSelectedNoteProxy, true);
            // int row = m_currentSelectedNoteProxy.row();
            // m_noteView->animateRemovedRow(QModelIndex(),row, row);
        }
        m_isOperationRunning = false;
    }
}

/*!
 * \brief Widget::selectNote
 *
 */
// 双击选中笔记
void Widget::selectNote(const QModelIndex &noteIndex)
{
    qDebug() << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
    if (noteIndex.isValid()) {
        // show the content of the pressed note in the text editor
        QModelIndex indexSrc = m_proxyModel->mapToSource(noteIndex);
        showNoteInEditor(indexSrc);

        m_currentSelectedNoteProxy = noteIndex;

        m_noteView->selectionModel()->select(m_currentSelectedNoteProxy,
                                             QItemSelectionModel::ClearAndSelect);
        m_noteView->setCurrentIndex(m_currentSelectedNoteProxy);
        m_noteView->scrollTo(m_currentSelectedNoteProxy);
    } else {
        qDebug() << "Widget::selectNote() : noteIndex is not valid";
    }
}

/*!
 * \brief Widget::showNoteInEditor
 *
 */
void Widget::showNoteInEditor(const QModelIndex &noteIndex)
{
    qDebug() << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
    QString content = noteIndex.data(NoteModel::NoteContent).toString();
    QDateTime dateTime = noteIndex.data(NoteModel::NoteLastModificationDateTime).toDateTime();
    // int scrollbarPos = noteIndex.data(NoteModel::NoteScrollbarPos).toInt();
    int noteColor = noteIndex.data(NoteModel::NoteColor).toInt();
    // QString mdContent = noteIndex.data(NoteModel::NoteMdContent).toString();

//    const listViewModeDelegate delegate(m_currentFont);
//    QColor m_color = delegate.intToQcolor(noteColor);
    QColor m_color = m_plistDelegate->intToQcolor(noteColor);
    // set text and date
    m_notebook->ui->textEdit->setText(content);
    m_notebook->m_noteHead->colorWidget = QColor(m_color);
    m_notebook->m_noteHeadMenu->colorWidget = QColor(m_color);
    m_notebook->update();

    QString noteDate = dateTime.toString(Qt::ISODate);
    QString noteDateEditor = getNoteDateEditor(noteDate);
    // set scrollbar position
    // m_textEdit->verticalScrollBar()->setValue(scrollbarPos);
}

/*!
 * \brief Widget::selectFirstNote
 *
 */
void Widget::selectFirstNote()
{
    if (m_proxyModel->rowCount() > 0) {
        QModelIndex index = m_proxyModel->index(0, 0);
        m_noteView->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
        // 设置索引index所在的页面为当前页面
        m_noteView->setCurrentIndex(index);

        m_currentSelectedNoteProxy = index;
    }
}

/*!
 * \brief Widget::createNewNoteIfEmpty
 *
 */
void Widget::createNewNoteIfEmpty()
{
    // if(m_proxyModel->rowCount() == 0)
    createNewNote();
}

/*!
 * \brief Widget::createNewNote
 * add note
 * save to db
 */
void Widget::createNewNote()
{
    InformationCollector::getInstance().addPoint(InformationCollector::NewNote);
    qDebug() << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
    if (!m_isOperationRunning) {
        m_isOperationRunning = true;

        m_noteView->scrollToTop();

        ++m_noteCounter;
        qDebug() << "创建便签 m_noteCounter:" << m_noteCounter;
        NoteData *tmpNote = generateNote(m_noteCounter);

        // insert the new note to NoteModel
        QModelIndex indexSrc = m_noteModel->insertNote(tmpNote, 0);

        // update the editor header date label
        QString dateTimeFromDB = tmpNote->lastModificationdateTime().toString(Qt::ISODate);
        QString dateTimeForEditor = getNoteDateEditor(dateTimeFromDB);
        // 从排序过滤器模型返回与给定 indexSrc 对应的源模型索引。
        m_currentSelectedNoteProxy = m_proxyModel->mapFromSource(indexSrc);
        saveNoteToDB(m_currentSelectedNoteProxy);
        // int row = m_currentSelectedNoteProxy.row();
        // m_noteView->animateAddedRow(QModelIndex(),row, row);

        // 设置索引 m_currentSelectedNoteProxy 所在的页面为当前页面
        m_noteView->setCurrentIndex(m_currentSelectedNoteProxy);
        m_isOperationRunning = false;
    }
    int noteId = m_currentSelectedNoteProxy.data(NoteModel::NoteID).toInt();
    m_notebook = new EditPage(this, noteId);
    m_editors.push_back(m_notebook);
    m_notebook->m_id = m_editors.size() - 1;

    if (sender() != Q_NULLPTR) {
        // 获取当前选中item下标
        // QModelIndex indexInProxy = m_proxyModel->index(index.row(), 0);
        // 加载便签
        selectNote(m_currentSelectedNoteProxy);
        m_noteView->setCurrentRowActive(false);
    }
    connect(m_editors[m_editors.size() - 1], &EditPage::isEmptyNote, this, [=](int noteId){
        // m_editors.erase(m_editors[m_editors.size() - 1]);
        qDebug() << "receive signal isEmptyNote" << noteId;
        for (int count = 0; count <= m_proxyModel->rowCount(); count++) {
            QModelIndex m_tmpIndex = m_proxyModel->index(count, 0);
            if (m_tmpIndex.data(NoteModel::NoteID).toInt() == noteId) {
                QModelIndex sourceIndex = m_proxyModel->mapToSource(m_tmpIndex);
                deleteNote(m_tmpIndex, true);
                break;
            }
        }
    });
    connect(m_editors[m_editors.size() - 1], &EditPage::requestDel, this, [=](int noteId){
        for (int count = 0; count <= m_proxyModel->rowCount(); count++) {
            QModelIndex m_tmpIndex = m_proxyModel->index(count, 0);
            if (m_tmpIndex.data(NoteModel::NoteID).toInt() == noteId) {
                QModelIndex sourceIndex = m_proxyModel->mapToSource(m_tmpIndex);
                deleteNote(m_tmpIndex, true);
                break;
            }
        }
    });
    connect(m_notebook->m_noteHeadMenu, &noteHeadMenu::requestNewNote, this, [=](){
        newSlot();
    });
    connect(m_notebook->m_noteHeadMenu, &noteHeadMenu::requestShowNote, this, [=] {
        // 添加窗管协议
        kabase::WindowManage::removeHeader(this);
        if(isHidden()||!isActiveWindow()){
            setWindowState(windowState() & ~Qt::WindowMinimized);
            this->show();
            kdk::WindowManager::activateWindow(m_windowId);
        }
        kabase::WindowManage::setMiddleOfScreen(this);
    });
    connect(m_editors[m_editors.size() - 1], SIGNAL(texthasChanged(int,int)), this,
            SLOT(onTextEditTextChanged(int,int)));
    connect(m_editors[m_editors.size() - 1], SIGNAL(colorhasChanged(QColor,int)), this,
            SLOT(onColorChanged(QColor,int)));
    //将选择的内容复制到新的便签页
    connect(m_notebook, SIGNAL(textForNewEditpage()), this, SLOT(textForNewEditpageSigReceived()));
    // 设置鼠标焦点
    m_notebook->ui->textEdit->setFocus();
    // 移动光标至行末
    m_notebook->ui->textEdit->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
    if(m_isTextCpNew)
    {
        QClipboard *clipboard = QApplication::clipboard();   //获取系统剪贴板指针
        QString originalText  = clipboard->text();       //获取剪贴板上文本信息
        m_notebook->ui->textEdit->setText(originalText);
        m_isTextCpNew = false;
    }
    m_notebook->show();
    m_notebook->m_noteHead->hide();
    m_notebook->m_noteHeadMenu->show();
}

/*!
 * \brief Widget::loadNotes
 *
 */
void Widget::loadNotes(QList<NoteData *> noteList, int noteCounter)
{
    if (!noteList.isEmpty()) {
        m_noteModel->addListNote(noteList);
        // Qt::AscendingOrder 升序排序
        m_noteModel->sort(1, Qt::AscendingOrder);
    }

    m_noteCounter = noteCounter;

    createNewNoteIfEmpty();
    selectFirstNote();
}

/*!
 * \brief Widget::saveNoteToDB
 *
 */
// 将当前便笺保存到数据库
void Widget::saveNoteToDB(const QModelIndex &noteIndex)
{
    if (noteIndex.isValid()) {
        // 从排序过滤器模型返回与给定 noteIndex 对应的源模型索引。
        QModelIndex indexInSrc = m_proxyModel->mapToSource(noteIndex);
        NoteData *note = m_noteModel->getNote(indexInSrc);
        if (note != Q_NULLPTR)
            emit requestCreateUpdateNote(note);

        m_isContentModified = false;
    } else if (noteIndex.isValid() && m_isColorModified) {
        // 从排序过滤器模型返回与给定 noteIndex 对应的源模型索引。
        QModelIndex indexInSrc = m_proxyModel->mapToSource(noteIndex);
        NoteData *note = m_noteModel->getNote(indexInSrc);
        if (note != Q_NULLPTR)
            emit requestCreateUpdateNote(note);
        m_isColorModified = false;
    }
}

/*!
 * \brief Widget::getQDateTime
 *
 */
QDateTime Widget::getQDateTime(QString date)
{
    QDateTime dateTime = QDateTime::fromString(date, Qt::ISODate);
    return dateTime;
}

/*!
 * \brief Widget::getNoteDateEditor
 *
 */
QString Widget::getNoteDateEditor(QString dateEdited)
{
    QDateTime dateTimeEdited(getQDateTime(dateEdited));
    QLocale usLocale = QLocale::system();

    return usLocale.toString(dateTimeEdited, QStringLiteral("yyyy/MM/dd hh:mm"));
}

/*!
 * \brief Widget::generateNote
 * 初始化一个笔记
 */
NoteData *Widget::generateNote(const int noteID)
{
    NoteData *newNote = new NoteData(this);
    newNote->setId(noteID);

    QDateTime noteDate = QDateTime::currentDateTime();
    newNote->setCreationDateTime(noteDate);
    newNote->setLastModificationDateTime(noteDate);
    QColor color(PaletteWidget::LINGMO_BLUE);
    newNote->setNoteColor(color.blue() << 16 | color.green() << 8 | color.red());
    newNote->m_note = nullptr;
    return newNote;
}

/*!
 * \brief Widget::moveNoteToTop
 *
 */
void Widget::moveNoteToTop()
{
    qDebug() << "movenote to top";
    // check if the current note is note on the top of the list
    // if true move the note to the top
    if (m_tmpIndex.isValid()) {
        m_noteView->scrollToTop();

        // NoteData* note = m_noteModel->getNote(index);

        // move the current selected note to the top
        // 当前要移动到顶端的item QSortFilterProxyModel
        QModelIndex sourceIndex = m_proxyModel->mapToSource(m_tmpIndex);

        // 目前顶端的item QAbstractListModel
        QModelIndex destinationIndex = m_noteModel->index(0);

        // 将 sourceIndex.row() 移动到第0行,第0行变第一行
        m_noteModel->moveRow(sourceIndex, sourceIndex.row(), destinationIndex, 0);

        // 更新当前 最顶端QAbstractListModel item 并添加代理
        m_tmpIndex = m_proxyModel->mapFromSource(destinationIndex);
        m_currentSelectedNoteProxy = m_tmpIndex;
        // 修改当前选中
        m_noteView->setCurrentIndex(m_tmpIndex);
    } else {
        qDebug() << "Widget::moveNoteTop : m_currentSelectedNoteProxy not valid";
    }
}

/*!
 * \brief Widget::getFirstLine
 *
 */
QString Widget::getFirstLine(const QString &str)
{
    if (str.simplified().isEmpty()) {
        return QString();
    }

    QString text = str.trimmed();
    QTextStream ts(&text);
    return ts.readLine(FIRST_LINE_MAX);
}

/*!
 * \brief Widget::findNotesContain
 *
 */
void Widget::findNotesContain(const QString &keyword)
{
    // 将用于过滤源模型内容的固定字符串设置为给定模式
    m_proxyModel->setFilterFixedString(keyword);

    // 如果匹配到不止一行
    if (m_proxyModel->rowCount() > 0) {
        selectFirstNote();
    } else {
        m_currentSelectedNoteProxy = QModelIndex();
    }
}

/*!
 * \brief Widget::searchInit
 *
 */
void Widget::searchInit()
{
    m_searchLine->setContextMenuPolicy(Qt::NoContextMenu);  // 禁用右键菜单
    m_searchLine->setPlaceholderText(tr("Enter search content"));         // 设置详细输入框的提示信息
}

/*!
 * \brief Widget::clearSearch
 *
 */
void Widget::clearSearch()
{
    m_noteView->setFocusPolicy(Qt::StrongFocus);

    m_proxyModel->setFilterFixedString(QString());
}

/*!
 * \brief Widget::paintEvent
 *
 */
void Widget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    // QStyleOption opt;
    // opt.init(this);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    QPainterPath rectPath;
    rectPath.addRect(this->rect());

    p.fillPath(rectPath, palette().color(QPalette::Base));
    // p.restore();
}

/********************Slots************************/

/*!
 * \brief Widget::onTextEditTextChanged
 *
 */
void Widget::onTextEditTextChanged(int noteId, int i)
{
    qDebug() << "receive signal textchange" << noteId;
    for (int count = 0; count <= m_proxyModel->rowCount(); count++) {
        m_tmpIndex = m_proxyModel->index(count, 0);
        if (m_tmpIndex.data(NoteModel::NoteID).toInt() == noteId) {
            qDebug() << m_tmpIndex.data(NoteModel::NoteID).toInt();
            break;
        }
    }

    if (m_tmpIndex.isValid()) {
        QString content = m_tmpIndex.data(NoteModel::NoteContent).toString();
        QString mdContent = m_tmpIndex.data(NoteModel::NoteMdContent).toString();

        if (m_editors[i]->ui->textEdit->document()->isEmpty()) {
            m_editors[i]->m_isInsImg = false;
        }

        if (m_editors[i]->ui->textEdit->toHtml().toUtf8() != content) {
            // move note to the top of the list
            QModelIndex sourceIndex = m_proxyModel->mapToSource(m_tmpIndex);

            qDebug() << m_currentSelectedNoteProxy;
            if (m_tmpIndex.row() != 0) {
                // m_noteView->setAnimationEnabled(false);
                moveNoteToTop();
                // m_noteView->setAnimationEnabled(true);
            } else if (!m_searchLine->text().isEmpty() && sourceIndex.row() != 0) {
                m_noteView->setAnimationEnabled(false);
                moveNoteToTop();
                m_noteView->setAnimationEnabled(true);
            }

            // QModelIndex firstIndex = m_proxyModel->index(0,0);
            // Get the new data
            QString firstline;
            if(m_editors[i]->m_isInsImg) {
                firstline = getFirstLine(("[picture]"));
            }
            else {
                firstline = getFirstLine(m_editors[i]->ui->textEdit->toPlainText());
            }
            QDateTime dateTime = QDateTime::currentDateTime();
            // QString noteDate = dateTime.toString(Qt::ISODate);

            // update model
            QMap<int, QVariant> dataValue;
            dataValue[NoteModel::NoteContent] = QVariant::fromValue(
                m_editors[i]->ui->textEdit->toHtml().toUtf8());
            dataValue[NoteModel::NoteMdContent] = QVariant::fromValue(
                m_editors[i]->ui->textEdit->toPlainText());
            dataValue[NoteModel::NoteFullTitle] = QVariant::fromValue(firstline);
            dataValue[NoteModel::NoteLastModificationDateTime] = QVariant::fromValue(dateTime);

            QModelIndex currentIndex = m_proxyModel->mapToSource(m_tmpIndex);
            m_noteModel->setItemData(currentIndex, dataValue);

            m_isContentModified = true;
            // m_autoSaveTimer->start(500);
            saveNoteToDB(m_tmpIndex);
        }
    } else {
        qDebug() << "Widget::onTextEditTextChanged() : index is not valid";
    }
}

/*!
 * \brief Widget::onColorChanged
 *
 */
void Widget::onColorChanged(const QColor &color, int noteId)
{
    InformationCollector::getInstance().addMessage(QString("set widget color to %1.").arg(color.name()));
    qDebug() << "receive signal onColorChanged";
    for (int count = 0; count <= m_proxyModel->rowCount(); count++) {
        m_tmpColorIndex = m_proxyModel->index(count, 0);
        if (m_tmpColorIndex.data(NoteModel::NoteID).toInt() == noteId) {
            break;
        }
    }

    if (m_tmpColorIndex.isValid()) {
//        const listViewModeDelegate delegate(m_currentFont);
//        int m_color = delegate.qcolorToInt(color);
        int m_color = m_plistDelegate->qcolorToInt(color);
        QMap<int, QVariant> dataValue;
        dataValue[NoteModel::NoteColor] = QVariant::fromValue(m_color);

        QModelIndex index = m_proxyModel->mapToSource(m_tmpColorIndex);
        m_noteModel->setItemData(index, dataValue);
        m_isColorModified = true;
        saveNoteToDB(m_tmpColorIndex);
    }
}

/*!
 * \brief Widget::exitSlot
 *
 */
void Widget::exitSlot()
{
//    closeAllEditors();
//    this->close();
    this->hide();
}

/*!
 * \brief Widget::trashSlot
 *
 */
void Widget::trashSlot()
{
    if(!m_emptyNotes->m_isDontShow) {
//        kabase::WindowManage::removeHeader(m_emptyNotes);
        m_emptyNotes->exec();
    } else{
        emit m_emptyNotes->requestEmptyNotes();
    }
}

/*!
 * \brief Widget::miniSlot
 *
 */
void Widget::miniSlot()
{
    this->showNormal();
    this->showMinimized();
}

void Widget::textForNewEditpageSigReceived()
{
    m_isTextCpNew = true;
    newSlot();
}

void Widget::closeAllEditors()
{
    for (auto it = m_editors.begin(); it != m_editors.end(); it++) {
        (*it)->close();
        delete *it;
    }
    m_editors.clear();
}

/*!
 * \brief Widget::newSlot
 *
 */
void Widget::newSlot()
{
    // 如果搜索栏有内容,则在新建便签时清空
    if (!m_searchLine->text().isEmpty()) {
        clearSearch();
    }
    this->createNewNote();
}

/*!
 * \brief Widget::onTrashButtonClicked
 *
 */
void Widget::onTrashButtonClicked()
{
    deleteSelectedNote();
}

/*!
 * \brief Widget::listClickSlot
 * listview单击事件
 */
void Widget::listClickSlot(const QModelIndex &index)
{
    qDebug() << "listClickSlot" << index;
    if (sender() != Q_NULLPTR) {
        QModelIndex indexInProxy = m_proxyModel->index(index.row(), 0);
        if (indexInProxy.isValid()) {
            m_currentSelectedNoteProxy = indexInProxy;

            m_noteView->selectionModel()->select(m_currentSelectedNoteProxy,
                                                 QItemSelectionModel::ClearAndSelect);
            m_noteView->setCurrentIndex(m_currentSelectedNoteProxy);
            m_noteView->scrollTo(m_currentSelectedNoteProxy);
        } else {
            qDebug() << "Widget::selectNote() : indexInProxy is not valid";
            m_currentSelectedNoteProxy = QModelIndex();
        }
        m_noteView->setCurrentRowActive(false);
    }
}

/*!
 * \brief Widget::listDoubleClickSlot
 * 在滚动区域中单击便笺时：
 * 取消突出显示上一个选定的便笺
 * 如果在临时便笺存在时选择便笺，请删除临时便笺
 * 突出显示所选便笺
 * 将所选便笺内容加载到textedit
 */
void Widget::listDoubleClickSlot(const QModelIndex &index)
{
    int noteId = index.data(NoteModel::NoteID).toInt();
    int isExistInMeditors = 0;
    qDebug() << "list double click" << noteId << index;
    for (auto it = m_editors.begin(); it != m_editors.end(); it++) {
        if ((*it)->m_noteId == noteId) {
            isExistInMeditors = 1;
            m_notebook = *it;
            m_notebook->raise();
            m_notebook->activateWindow();
            break;
        }
    }
    if (isExistInMeditors == 0) {
        m_notebook = new EditPage(this, noteId);
        m_editors.push_back(m_notebook);
        m_notebook->m_id = m_editors.size() - 1;

        if (sender() != Q_NULLPTR) {
            // 获取当前选中item下标
            // QModelIndex indexInProxy = m_proxyModel->index(index.row(), 0);
            // 加载便签
            selectNote(index);
            m_noteView->setCurrentRowActive(false);
        }
        connect(m_notebook->m_noteHeadMenu, &noteHeadMenu::requestNewNote, this, [=](){
            newSlot();
        });
        connect(m_notebook->m_noteHeadMenu, &noteHeadMenu::requestShowNote, this, [=] {
            // 添加窗管协议
            kabase::WindowManage::removeHeader(this);
            if(isHidden()||!isActiveWindow()){
                setWindowState(windowState() & ~Qt::WindowMinimized);
                this->show();
                kdk::WindowManager::activateWindow(m_windowId);
            }
            kabase::WindowManage::setMiddleOfScreen(this);
        });
        connect(m_editors[m_editors.size() - 1], &EditPage::requestDel, this, [=](int noteId){
            for (int count = 0; count <= m_proxyModel->rowCount(); count++) {
                QModelIndex m_tmpIndex = m_proxyModel->index(count, 0);
                if (m_tmpIndex.data(NoteModel::NoteID).toInt() == noteId) {
                    QModelIndex sourceIndex = m_proxyModel->mapToSource(m_tmpIndex);
                    deleteNote(m_tmpIndex, true);
                    break;
                }
            }
        });
        connect(m_editors[m_editors.size() - 1], SIGNAL(texthasChanged(int,int)), this,
                SLOT(onTextEditTextChanged(int,int)));
        connect(m_editors[m_editors.size() - 1], SIGNAL(colorhasChanged(QColor,int)), this,
                SLOT(onColorChanged(QColor,int)));
    }
    // 设置鼠标焦点
    m_notebook->ui->textEdit->setFocus();
    // 移动光标至行末
    m_notebook->ui->textEdit->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
    m_notebook->setHints();
    m_notebook->show();
}

/*!
 * \brief Widget::onSearchEditTextChanged
 *
 */
void Widget::onSearchEditTextChanged(const QString &keyword)
{
    InformationCollector::getInstance().addPoint(InformationCollector::Search);
    qDebug() << "onSearchEditTextChanged";
    m_searchQueue.enqueue(keyword);

    if (!m_isOperationRunning) {
        m_isOperationRunning = true;

        // disable animation while searching
        m_noteView->setAnimationEnabled(false);

        while (!m_searchQueue.isEmpty()) {
            qApp->processEvents();
            QString str = m_searchQueue.dequeue();
            if (str.isEmpty()) {
                clearSearch();
            } else {
                m_noteView->setFocusPolicy(Qt::NoFocus);
                // 过滤
                findNotesContain(str);
            }
        }

        m_noteView->setAnimationEnabled(true);
        m_isOperationRunning = false;
    }
}

/*!
 * \brief Widget::changePageSlot
 *
 */
void Widget::changePageSlot()
{
    InformationCollector::getInstance().addPoint(InformationCollector::ModeChange);
    if (getListFlag() != 0) {
        initIconMode();
        m_viewChangeButton->setIcon(QIcon::fromTheme("view-list-symbolic"));
        setListFlag(0);
        m_settingsDatabase->setValue(QStringLiteral("iniNoteMode"), "icon");
    } else if (getListFlag() == 0) {
        initListMode();
        m_viewChangeButton->setIcon(QIcon::fromTheme("view-grid-symbolic"));
        setListFlag(1);
        m_settingsDatabase->setValue(QStringLiteral("iniNoteMode"), "list");
    }
    if (m_noteModel->rowCount() > 0) {
        QModelIndex index = m_noteView->currentIndex();
        m_currentSelectedNoteProxy = index;
        m_noteView->selectionModel()->select(m_currentSelectedNoteProxy,
                                             QItemSelectionModel::ClearAndSelect);
        m_noteView->setCurrentIndex(m_currentSelectedNoteProxy);
        m_noteView->scrollTo(m_currentSelectedNoteProxy);
    } else {
        // 创建新的空模型索引 此类型的模型索引用于指示模型中的位置无效
        m_currentSelectedNoteProxy = QModelIndex();
    }
}

/*!
 * \brief Widget::sortSlot
 *
 */
void Widget::sortSlot(int index)
{
    // Qt::AscendingOrder 升序排序
    // 参见 NoteModel::sort
    if (m_proxyModel->rowCount()) {
        if (sortflag) {
            m_noteModel->sort(index, Qt::DescendingOrder);
            sortflag = 0;
        } else {
            m_noteModel->sort(index, Qt::AscendingOrder);
            sortflag = 1;
        }
    }
}

/*!
 * \brief Widget::clearNoteSlot
 *
 */
void Widget::clearNoteSlot()
{
    qDebug() << "empty note" << m_editors.size();
    for (auto it = m_editors.begin(); it != m_editors.end(); it++) {
        (*it)->close();
        // m_editors.erase(it);
//        delete (*it);
    }
    m_editors.clear();
    qDebug() << "清空vector" << m_editors.size();
    m_noteModel->clearNotes();
    emit requestClearNote();
}

/*!
 * \brief Widget::setNoteNullSlot
 *
 */
void Widget::setNoteNullSlot()
{
    for (int i = 0; i < m_proxyModel->rowCount(); i++) {
        QModelIndex index = m_proxyModel->index(i, 0);
        NoteData *note = m_noteModel->getNote(index);
        note->m_note = nullptr;
    }
}

/*!
 * \brief Widget::onF1ButtonClicked
 *
 */
void Widget::onF1ButtonClicked()
{
    qDebug() << "onF1ButtonClicked";
    userGuideInterface->call(QString("showGuide"), "lingmo-notebook");
}

/*!
 * \brief Widget::sltMessageReceived
 *
 */
void Widget::sltMessageReceived(/*const QString &msg*/)
{
    int noteId = m_currentSelectedNoteProxy.data(NoteModel::NoteID).toInt();
    qDebug() << __FUNCTION__ << __LINE__ << "noteId  == " << noteId;

    if(this->isHidden())
    {
        this->m_notebook->show();
        this->m_notebook->activateWindow();
        this->m_notebook->raise();
    }
    else
    {
        this->raise();
        this->activateWindow();
        this->show();
    }

#if 0
    int noteId = m_currentSelectedNoteProxy.data(NoteModel::NoteID).toInt();

        if (m_noteModel->rowCount() > 0/* && m_currentSelectedNoteProxy.isValid()*/) {
            int noteId = m_currentSelectedNoteProxy.data(NoteModel::NoteID).toInt();
            for (auto it = m_editors.begin(); it != m_editors.end(); it++) {
                if ((*it)->m_noteId == noteId) {
                    (*it)->raise();
                    (*it)->activateWindow();
                    (*it)->show();
                    break;
                }
            }
        }
#endif
}

/*!
 * \brief Widget::readIniNoteMode
 *
 */
void Widget::iniNoteModeRead()
{
    qDebug() << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
    //读取配置文件，确定要不要变更视图
    if (m_settingsDatabase->value(QStringLiteral("iniNoteMode"), "NULL") == "icon")
        changePageSlot();
}

QString Widget::getWindowId(void)
{
    return m_windowId;
}

void Widget::handlePCMode()
{
//    this->showNormal();
    this->windowButtonBar()->maximumButton()->show();
}

void Widget::handlePADMode()
{
//    this->showMaximized();
    this->windowButtonBar()->maximumButton()->hide();
}

void Widget::ShowContextMenu(const QPoint& pos)
{
    MyMenu menu;

    if (!m_noteView->selectionModel()->currentIndex().isValid()) {
        return;
    }
    connect(&menu, &QMenu::triggered, this, &Widget::triggerMenu);

    QAction deleteAction(tr("delete this note"), &menu);
    QAction openAction(tr("open this note"), &menu);
    QAction clearAction(tr("clear note list"), &menu);

    menu.addAction(&deleteAction);
    menu.addAction(&openAction);
    menu.addAction(&clearAction);

    menu.exec(QCursor::pos());
}

void Widget::triggerMenu(QAction *action)
{
    QString str = action->text();
    if(tr("delete this note") == str) {
        deleteSelectedNote();
    }
    else if(tr("open this note") == str) {
        listDoubleClickSlot(m_noteView->selectionModel()->currentIndex());
    }
    else if(tr("clear note list") == str) {
        trashSlot();
    }
}

void Widget::ShortDateChangeSlot(QString date)
{
    qDebug() << "Widget::ShortDateChangeSlot changed" << date;
    m_noteView->viewport()->update();
}
void Widget::keyPressEvent(QKeyEvent *event)
{
    if(event->modifiers() == Qt::ControlModifier&&event->key()==Qt::Key_E){
        qDebug()<<__LINE__<<__func__;
        m_searchLine->setFocus();
    }
    if((event->modifiers() == Qt::ShiftModifier) && (event->key() == Qt::Key_F10)){
        if(!isActiveWindow()){
            //不作處理
        }else{
            ShowContextMenu(cursor().pos());
        }
    }
    return QWidget::keyPressEvent(event);
}
