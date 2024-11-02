#include <QMimeData>
#include <QDirIterator>
#include <explor-qt/file-operation-utils.h>
#include <lingmo-log4qt.h>
#include "tableone.h"
#include "UI/mainwidget.h"
#include "UI/base/widgetstyle.h"
#include "UI/globalsignal.h"
#include "lingmostylehelper/lingmostylehelper.h"
#include "windowmanager/windowmanager.h"
#include "UIControl/global/global.h"
#define PT_18 18

TableOne::TableOne(QString listName, QWidget *parent) : QWidget(parent)
{
    nowListName = listName;
    setObjectName("TableOne");
    setAcceptDrops(true);
    initUI();
    initRightMenu();
    initConnect();
    initStyle();
    tableView->setAlternatingRowColors(false);
    qDebug() << "歌曲列表初始化完成";
}

TableOne::~TableOne()
{

}

void TableOne::initStyle()
{
    listTitleLabel->setStyleSheet("font-weight: 600; line-height: 24px;");
    listTotalNumLabel->setStyleSheet("font-weight: 400;color:#8C8C8C; border:none; line-height: 14px;");

    tableView->setAlternatingRowColors(false);

    if (WidgetStyle::themeColor == 1)
    {
        nullPageIconLabel->setPixmap(QPixmap(":/img/default/defaultIconDark.svg").scaled(96,96));
        this->setStyleSheet("#TableOne{background:#252526;border:none;}");
        titleWid->setStyleSheet("#titleWid{background:#252526;}");
        nullPageWidget->setStyleSheet("#nullPageWidget{background:#252526;}");
        tableView->setStyleSheet("#tableView{background:#252526;border:none;}");
        //tableView->setAlternatingRowColors(false);
        nullPageTextLabel->setStyleSheet("font-size: 14px; color:#8F9399; opacity: 1;");
    }
    else if(WidgetStyle::themeColor == 0)
    {
        nullPageIconLabel->setPixmap(QPixmap(":/img/default/defaultIconLight.svg").scaled(96,96));
        this->setStyleSheet("#TableOne{background:#FFFFFF;border:none;}");
        titleWid->setStyleSheet("#titleWid{background:#FFFFFF;}");
        nullPageWidget->setStyleSheet("#nullPageWidget{background:#FFFFFF;}");
        tableView->setStyleSheet("#tableView{background:#FFFFFF;border:none;}");
        nullPageTextLabel->setStyleSheet("font-size: 14px; color:#8C8C8C; opacity: 1;");

    }
}

void TableOne::initTableViewStyle()
{
    if (!tableView->horizontalHeader()->length()) {
        return;
    }
    tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    tableView->setColumnWidth(3, 80);
    tableView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    tableView->verticalHeader()->setVisible(false);// 垂直不可见
    tableView->verticalHeader()->setDefaultSectionSize(40);

    tableView->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);
    tableView->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Stretch);
    tableView->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Stretch);
//    tableView->horizontalHeader()->setSectionResizeMode(3,QHeaderView::setFixedWidth(25));
    tableView->setAlternatingRowColors(false); // 奇偶行颜色变化


    horizonHeader = tableView->horizontalHeader();
    horizonHeader->setEnabled(false);
    //m_model->m_model.setHorizontalHeaderLabels(m_model->titleList);
    horizonHeader->setHighlightSections(false);
    horizonHeader->setFixedHeight(36);

    // 表头内容样式
    if(WidgetStyle::themeColor == 0) {
        horizonHeader->setStyleSheet("QHeaderView::section,QTableCornerButton::section {padding-left: 25px; \
                                                                border: none ;border-bottom: 1px solid white;\
                                                                border-right: 1px solid white;border-bottom: 1px transparent;\
                                                                background-color:white;color:#8F9399;}");
    } else {
        horizonHeader->setStyleSheet("QHeaderView::section,QTableCornerButton::section {padding-left:25px; \
                                                        border: none;border-bottom: 1px solid #252526;\
                                                        border-right: 1px solid #252526;border-bottom: 1px transparent;\
                                                        background-color:#252526;color:#8F9399;}");
    }

    horizonHeader->setDefaultAlignment(Qt::AlignLeft);

    m_model->m_model.setHorizontalHeaderLabels(m_model->titleList);
    //列表刷新，时长右对齐需加到默认表头左对齐之后
    m_model->m_model.horizontalHeaderItem(m_model->titleList.length()-1)->setTextAlignment(Qt::AlignRight);

    tableView->verticalScrollBar()->setStyleSheet("width:25px;");
    tableView->verticalScrollBar()->setProperty("drawScrollBarGroove",false);
    if (m_model->count() <= showScrollbarNumber) {
        tableView->verticalScrollBar()->setVisible(false);
    } else {
        tableView->verticalScrollBar()->setVisible(true);
    }
}

void TableOne::initUI()
{
    //QVBoxLayout *tableLayout = new QVBoxLayout();
    //this->setLayout(tableLayout);

    listTitleLabel = new QLabel(this);
    listTitleLabel->setAlignment(Qt::AlignBottom);
    listTitleLabel->setMaximumWidth(192);
    listTitleLabel->setFixedHeight(45);
    listTotalNumLabel = new QLabel(this);
    listTotalNumLabel->setAlignment(Qt::AlignBottom);
    listTotalNumLabel->setFixedHeight(42);
    addMusicButton = new QToolButton(this);
    addMusicButton->setFixedSize(125,36);
    listTitleHBoxLayout = new QHBoxLayout();
    playAllButton = new QPushButton(this);
    playAllButton->setFixedSize(125,36);


    titleWid = new QWidget(this);
    titleWid->setObjectName("titleWid");
    titleWid->setLayout(listTitleHBoxLayout);
    listTitleHBoxLayout->setMargin(0);
    listTitleHBoxLayout->setSpacing(0);
    listTitleHBoxLayout->setAlignment(Qt::AlignLeft);
    listTitleHBoxLayout->addWidget(listTitleLabel);
    listTitleHBoxLayout->addSpacing(16);
    listTitleHBoxLayout->addWidget(listTotalNumLabel);
    listTitleHBoxLayout->addStretch();
    listTitleHBoxLayout->addWidget(addMusicButton, 1, Qt::AlignRight);
    listTitleHBoxLayout->addSpacing(16);
    listTitleHBoxLayout->addWidget(playAllButton, 0, Qt::AlignRight);
    listTitleHBoxLayout->setContentsMargins(30,20,30,30);
    titleWid->setFixedHeight(95);

    playAllButton->setText(tr("Play All"));
    playAllButton->setProperty("isImportant", true);

    addMusicButton->setText(tr("Add Music"));
    //addMusicButton->setProperty("isImportant",true);
    addMusicButton->setPopupMode(QToolButton::InstantPopup);
    add_menu = new QMenu(this);
    addMusicFileAction = new QAction(this);
    addDirMusicAction = new QAction(this);
    addMusicFileAction->setText(tr("Add local songs"));
    addDirMusicAction->setText(tr("Add local folders"));
    add_menu->addAction(addMusicFileAction);
    add_menu->addAction(addDirMusicAction);
    addMusicButton->setMenu(add_menu);


    tableView = new TableBaseView();
    tableView->setObjectName("tableView"); // 可以通过 #tableView 设置样式表
    tableView->setFocusPolicy(Qt::NoFocus);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers); // 使用右键菜单，需启用该属性
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);//设置选中模式为选中行
    tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);//设置按ctrl键选中多个
    QList<musicDataStruct> resList;
    int ret;
    ret = g_db->getSongInfoListFromDB(resList,nowListName);
    if(ret == DB_OP_SUCC)
    {
        m_model = new MusicListModel;
        m_model->add(resList);
        m_model->setView(*tableView);
    }


    nullPageWidget = new QWidget(this);
    nullPageWidget->setObjectName("nullPageWidget");

    nullPageHLayout = new QHBoxLayout();
    nullPageVLayout = new QVBoxLayout();
    nullPageIconLabel = new QLabel(this);
    nullPageTextLabel = new QLabel(this);
    n_addDirMusicButton = new QPushButton(this);
    n_addDirMusicButton->setFixedSize(190,36);
    n_addMusicButton = new QPushButton(this);
    n_addMusicButton->setFixedSize(190,36);
    nullPageTextLabel->setText(tr("There are no songs!"));
    n_addMusicButton->setText(tr("Add Local Songs"));
    n_addDirMusicButton->setText(tr("Add Local Folder"));

    if (WidgetStyle::themeColor == 1) {
        // black theme
        nullPageIconLabel->setPixmap(QPixmap(":/img/default/defaultIconDark.svg").scaled(96,96));
    } else {
        nullPageIconLabel->setPixmap(QPixmap(":/img/default/defaultIconLight.svg").scaled(96,96));
    }

    nullPageWidget->setLayout(nullPageVLayout);
    nullPageVLayout->addStretch();
    nullPageVLayout->addWidget(nullPageIconLabel);
    nullPageIconLabel->setAlignment(Qt::AlignHCenter);
    nullPageVLayout->addSpacing(0);
    nullPageVLayout->addWidget(nullPageTextLabel);
    nullPageVLayout->addSpacing(24);
    nullPageTextLabel->setAlignment(Qt::AlignHCenter);
    nullPageVLayout->addLayout(nullPageHLayout);
    nullPageVLayout->addStretch();

    nullPageHLayout->addWidget(n_addMusicButton);
    nullPageHLayout->setSpacing(16);
    nullPageHLayout->addWidget(n_addDirMusicButton);

    nullPageVLayout->addStretch();
    nullPageVLayout->setAlignment(Qt::AlignCenter);
    nullPageVLayout->setSpacing(0);
    nullPageVLayout->setMargin(0);

    QVBoxLayout *tableLayout = new QVBoxLayout();
    tableLayout->addWidget(titleWid, Qt::AlignTop);
    tableLayout->addWidget(tableView);
    tableLayout->addWidget(nullPageWidget);
    tableLayout->setMargin(0);
    tableLayout->setSpacing(0);
    this->setLayout(tableLayout);

    initTableViewStyle();
//    listTitleLabel->setFixedWidth(120);
    listTotalNumLabel->setFixedWidth(120);
    showTitleText(nowListName);
    changeNumber();
}

void TableOne::slotLableSetFontSize(int size)
{
    //默认大小12px,换算成pt为9
    double lableBaseFontSize = PT_18;//魔鬼数字，自行处理
    double nowFontSize = lableBaseFontSize * double(size) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    font.setPointSizeF(nowFontSize);
    listTitleLabel->setFont(font);

    selectListChanged(listTitleLabel->text());

    if (infoDialog != nullptr) {
        infoDialog->slotLableSetFontSize(size);
    }
}

void TableOne::showTitleText(QString listName)
{
    QFontMetrics fontWidth(listTitleLabel->font());//得到每个字符的宽度
    QString elideNote = fontWidth.elidedText(listName, Qt::ElideRight, 192);//最大宽度150像素
    if(listName == ALLMUSIC) {
        listTitleLabel->setText(tr("Song List"));
    } else if(listName == FAV){
        listTitleLabel->setText(tr("I Love"));
    } else if(listName == SEARCH) {
        listTitleLabel->setText(tr("Search Result"));
    } else {
        listTitleLabel->setText(elideNote);
    }
    if(elideNote != listName) {
        listTitleLabel->setToolTip(listName);
    }
    else{
        listTitleLabel->setToolTip("");
    }
}
void TableOne::initConnect()
{
    connect(tableView,&TableBaseView::doubleClicked,this,&TableOne::playSongs);
    connect(tableView,&TableBaseView::customContextMenuRequested,this,&TableOne::showRightMenu);
    connect(addMusicFileAction,&QAction::triggered,this,&TableOne::addMusicSlot);
    connect(addDirMusicAction,&QAction::triggered,this,&TableOne::addDirMusicSlot);
    connect(this,&TableOne::countChanges,this,&TableOne::changeNumber);
    connect(&playController::getInstance(),&playController::currentIndexAndCurrentList,this,&TableOne::getHightLightIndex);
    connect(n_addMusicButton,&QPushButton::clicked,this,&TableOne::addMusicSlot);
    connect(n_addDirMusicButton,&QPushButton::clicked,this,&TableOne::addDirMusicSlot);
    connect(playAllButton,&QPushButton::clicked,this,&TableOne::playAllButtonClicked);
}

void TableOne::initRightMenu()
{
    m_menu = new QMenu(tableView);

    playRow = new QAction(tr("Play"));
    removeRow = new QAction(tr("Delete from list"));
    removeLocalRow = new QAction(tr("Remove from local"));
    showInfoRow = new QAction(tr("View song information"));
    addToOtherListMenu = new QMenu(tr("Add to songlist"));
    connect(playRow,&QAction::triggered,this,&TableOne::playSongs);
    connect(removeRow,&QAction::triggered,this,&TableOne::isDeleteSongs);
    connect(removeLocalRow,&QAction::triggered,this,&TableOne::isDeleteLocalSongs);
    connect(showInfoRow,&QAction::triggered,this,&TableOne::showInfo);
    connect(addToOtherListMenu,&QMenu::triggered,this,&TableOne::addToOtherList);

    //限制应用字体不随着主题变化
//    QFont sizeFont;
//    sizeFont.setPixelSize(14);
//    m_menu->setFont(sizeFont);
//    addToOtherListMenu->setFont(sizeFont);

    tableView->installEventFilter(this);
}

void TableOne::showRightMenu(const QPoint &pos)
{
    QModelIndex index = tableView->indexAt(pos);
    if(index.row() < 0)
    {
        return;
    }
//    m_menu->move(0,0);
    m_menu->addAction(playRow);      //添加动作到菜单
    m_menu->addMenu(addToOtherListMenu);
    m_menu->addAction(removeRow);  //添加动作到菜单
    m_menu->addAction(removeLocalRow);
    m_menu->addAction(showInfoRow); //添加动作到菜单
    QMap<int,QString> map1 = getSelectedTaskIdList();
    addToOtherListMenu->clear();
    QStringList allList;
    int ret = g_db->getPlayList(allList);
    if(ret == 0)
    {
        for(int i = 0 ;i < allList.size() ;i++)
        {
            if(allList[i] == SEARCH)
            {
                continue;
            }
            QAction *listNameAct = new QAction;
            addToOtherListMenu->addAction(listNameAct);
            if(allList[i] != FAV)
            {
                listNameAct->setText(allList[i]);
            }
            else
            {
                listNameAct->setText(tr("I Love"));
            }
        }
    }
    else
    {
        qDebug() << "获取所有歌单失败";
    }
    if(map1.size() > 1)
    {
        m_menu->removeAction(playRow);
        m_menu->removeAction(showInfoRow);
    }
    if(nowListName == SEARCH)
    {
        m_menu->removeAction(removeRow);
        m_menu->removeAction(removeLocalRow);
    }
    m_menu->exec(QCursor::pos());
}

void TableOne::isDeleteSongs()
{
    //确认将选中的歌曲从歌单中删除？
    QMessageBox *warn = new QMessageBox(QMessageBox::Warning,tr("Prompt information"),tr("Confirm that the selected song will be deleted from the song list?"),QMessageBox::Yes | QMessageBox::No,Widget::mutual);
    kdk::LingmoUIStyleHelper::self()->removeHeader(warn);
    warn->setWindowTitle(tr("lingmo-music"));
    QRect availableGeometry = parentWidget()->geometry();
    warn->move((availableGeometry.center() - warn->rect().center()));
    warn->button(QMessageBox::Yes)->setText(tr("Yes"));
    warn->button(QMessageBox::No)->setText(tr("No"));
    int result = warn->exec();
    if(result == QMessageBox::Yes)
    {
        deleteSongs();
        qDebug() << "QMessageBox::Yes";
    }
    else
    {
        qDebug() << "QMessageBox::No";
        return;
    }
}

void TableOne::isDeleteLocalSongs()
{

    if (! WidgetStyle::isWayland) {
        deleteLocalSongs();

        return;
    }

    //歌曲从本地删除后不可恢复，是否确定删除？
    QMessageBox *warn = new QMessageBox(QMessageBox::Warning,tr("Prompt information"),tr("After the song is deleted from the local, it cannot be resumed. Is it sure to delete?"),QMessageBox::Yes | QMessageBox::No,Widget::mutual);
    kdk::LingmoUIStyleHelper::self()->removeHeader(warn);
    QRect availableGeometry = parentWidget()->geometry();
    warn->move(availableGeometry.center() - warn->rect().center());
    warn->button(QMessageBox::Yes)->setText(tr("Yes"));
    warn->button(QMessageBox::No)->setText(tr("No"));
    kdk::WindowManager::setGeometry(warn->windowHandle(),warn->geometry());
    int result = warn->exec();

    if(result == QMessageBox::Yes) {
        deleteLocalSongsWayland();
        qDebug() << "QMessageBox::Yes";
    } else {
        qDebug() << "QMessageBox::No";
        return;
    }
}

void TableOne::deleteSongs()
{
    QMap<int,QString> map1 = getSelectedTaskIdList();
    while (!map1.empty()) {
        QMap<int,QString>::iterator iter;
        iter = map1.end();
        iter--;
        int ret;
        int index = -1;
        if(nowListName != nowPlayListName && nowListName == ALLMUSIC) {
            index = MusicFileInformation::getInstance().findIndexFromPlayList(nowPlayListName,iter.value());
        }
        if(nowListName != ALLMUSIC)
        {
           ret = g_db->delMusicFromPlayList(iter.value(),nowListName);
        }
        else
        {
           ret = g_db->delSongFromEveryWhere(iter.value());
        }
        if(ret == 0)
        {
//            m_model->remove(iter.key());
            if((nowListName != nowPlayListName) && nowListName == ALLMUSIC && index != -1) {
                playController::getInstance().removeSongFromCurList(nowPlayListName,index);
            } else {
                playController::getInstance().removeSongFromCurList(nowListName,iter.key());
            }
//            if(nowListName == tr("I Love"))

            if(nowListName == FAV)
            {
                Q_EMIT removeILoveFilepathSignal(iter.value());
            }
//            Q_EMIT countChanges();

            map1.remove(iter.key());
        }
        else
        {
            qDebug() << "delete failed" << iter.value();
        }
    }
    selectListChanged(nowListName);
    Q_EMIT refreshHistoryListSignal();
}

void TableOne::deleteLocalSongs()
{
    m_map = getSelectedTaskIdList();

    QStringList trashFilenames;
    for (int key: m_map.keys()) {
        QString value = m_map.value(key);
        QString urlFile = "file://" + value;
        qDebug() << "filename: " << urlFile;
        trashFilenames << urlFile;
    }
    qDebug() << "trashFilenames: " << trashFilenames;


    for (int key : m_map.keys()) {
        QString value = m_map.value(key);

        QFile file(value);
        if(file.exists())
        {
            QString uri = "file://" + value;
            Peony::FileOperation *trash = Peony::FileOperationUtils::trash(QStringList(uri),true);
            connect(trash,&Peony::FileOperation::operationFinished,this,[=]{
                if (!trash->hasError()) {
                    int ret;
                    int index = -1;
                    if(nowListName != nowPlayListName && nowListName == ALLMUSIC)
                    {
                        index = MusicFileInformation::getInstance().findIndexFromPlayList(nowPlayListName,value);
                    }
                    if(nowListName != ALLMUSIC)
                    {
                       ret = g_db->delMusicFromPlayList(value,nowListName);
                    }
                    else
                    {
                       ret = g_db->delSongFromEveryWhere(value);
                    }
                    if(ret == 0)
                    {
                        if((nowListName != nowPlayListName) && nowListName == ALLMUSIC && index != -1)
                        {
                            playController::getInstance().removeSongFromCurList(nowPlayListName,index);
                        }
                        else
                        {
                            playController::getInstance().removeSongFromCurList(nowListName,key);
                        }

                        if(nowListName == FAV)
                        {
                            Q_EMIT removeILoveFilepathSignal(value);
                        }

                        m_map.remove(key);
                    }
                    else
                    {
                        qDebug() << "delete failed" << value;
                    }
                    selectListChanged(nowListName);
                    Q_EMIT refreshHistoryListSignal();
                }
            },Qt::BlockingQueuedConnection);
        } else {
                QMessageBox *warn = new QMessageBox(QMessageBox::Warning,tr("Prompt information"),tr("After the song is deleted from the local, it cannot be resumed. Is it sure to delete?"),QMessageBox::Yes | QMessageBox::No);
                kdk::LingmoUIStyleHelper::self()->removeHeader(warn);
                warn->setWindowTitle(tr("lingmo-music"));
                warn->button(QMessageBox::Yes)->setText(tr("Yes"));
                warn->button(QMessageBox::No)->setText(tr("No"));
                warn->move(warn->parentWidget()->width()/2-warn->width()/2+warn->parentWidget()->x(),warn->parentWidget()->height()/2-warn->height()/2+warn->parentWidget()->y());
                kdk::WindowManager::setGeometry(warn->windowHandle(),warn->geometry());
                int result = warn->exec();
                if(result == QMessageBox::Yes)
                {
                    int ret;
                    int index = -1;
                    if(nowListName != nowPlayListName && nowListName == ALLMUSIC) {
                        index = MusicFileInformation::getInstance().findIndexFromPlayList(nowPlayListName,value);
                    }

                    if(nowListName != ALLMUSIC) {
                       ret = g_db->delMusicFromPlayList(value,nowListName);
                    } else {
                       ret = g_db->delSongFromEveryWhere(value);
                    }

                    if(ret == 0) {
                        if((nowListName != nowPlayListName) && nowListName == ALLMUSIC && index != -1)
                        {
                            playController::getInstance().removeSongFromCurList(nowPlayListName,index);
                        } else {
                            playController::getInstance().removeSongFromCurList(nowListName,key);
                        }

                        if(nowListName == FAV) {
                            Q_EMIT removeILoveFilepathSignal(value);
                        }

                        m_map.remove(key);
                    } else {
                        qDebug() << "delete failed" << value;
                    }
                    selectListChanged(nowListName);
                    Q_EMIT refreshHistoryListSignal();
                } else {
                    return;
                }
        }
    }

}

/**
 * @brief TableOne::deleteLocalSongsWayland
 */
void TableOne::deleteLocalSongsWayland()
{
    m_map = getSelectedTaskIdList();

    QStringList trashFilenames;
    for (int key: m_map.keys()) {
        QString value = m_map.value(key);
        QString urlFile = "file://" + value;
        qDebug() << "filename: " << urlFile;
        trashFilenames << urlFile;
    }
    qDebug() << "trashFilenames: " << trashFilenames;


    for (int key : m_map.keys()) {
        QString value = m_map.value(key);
        QFile file(value);

        if(file.exists()) {
            file.remove();

            int ret;
            int index = -1;
            if(nowListName != nowPlayListName && nowListName == ALLMUSIC) {
                index = MusicFileInformation::getInstance().findIndexFromPlayList(nowPlayListName,value);
            }

            if(nowListName != ALLMUSIC) {
                ret = g_db->delMusicFromPlayList(value,nowListName);
            } else {
                ret = g_db->delSongFromEveryWhere(value);
            }

            if(ret == 0) {
                if((nowListName != nowPlayListName) && nowListName == ALLMUSIC && index != -1)
                {
                    playController::getInstance().removeSongFromCurList(nowPlayListName,index);
                } else {
                    playController::getInstance().removeSongFromCurList(nowListName,key);
                }

                if(nowListName == FAV) {
                    Q_EMIT removeILoveFilepathSignal(value);
                }

                m_map.remove(key);
            } else {
                qDebug() << "delete failed" << value;
            }
            selectListChanged(nowListName);
            Q_EMIT refreshHistoryListSignal();

        } else {
                QMessageBox *warn = new QMessageBox(QMessageBox::Warning,tr("Prompt information"),tr("After the song is deleted from the local, it cannot be resumed. Is it sure to delete?"),QMessageBox::Yes | QMessageBox::No);
                kdk::LingmoUIStyleHelper::self()->removeHeader(warn);
                warn->setWindowTitle(tr("lingmo-music"));
                warn->button(QMessageBox::Yes)->setText(tr("Yes"));
                warn->button(QMessageBox::No)->setText(tr("No"));
                warn->move(warn->parentWidget()->width()/2-warn->width()/2+warn->parentWidget()->x(),warn->parentWidget()->height()/2-warn->height()/2+warn->parentWidget()->y());
                kdk::WindowManager::setGeometry(warn->windowHandle(),warn->geometry());
                int result = warn->exec();

                if(result == QMessageBox::Yes) {
                    int ret;
                    int index = -1;
                    if(nowListName != nowPlayListName && nowListName == ALLMUSIC) {
                        index = MusicFileInformation::getInstance().findIndexFromPlayList(nowPlayListName,value);
                    }

                    if(nowListName != ALLMUSIC) {
                       ret = g_db->delMusicFromPlayList(value,nowListName);
                    } else {
                       ret = g_db->delSongFromEveryWhere(value);
                    }

                    if(ret == 0) {
                        if((nowListName != nowPlayListName) && nowListName == ALLMUSIC && index != -1) {
                            playController::getInstance().removeSongFromCurList(nowPlayListName,index);
                        } else {
                            playController::getInstance().removeSongFromCurList(nowListName,key);
                        }

                        if(nowListName == FAV) {
                            Q_EMIT removeILoveFilepathSignal(value);
                        }

                        m_map.remove(key);
                    } else {
                        qDebug() << "delete failed" << value;
                    }
                    selectListChanged(nowListName);
                    Q_EMIT refreshHistoryListSignal();
                } else {
                    return;
                }
        }
    }
}

void TableOne::playSongs()
{
    int index = tableView->currentIndex().row();
    m_model->getItem(index);
    playMusicforIndex(nowListName,index);
}
void TableOne::playMusicforIndex(QString listName, int index)
{
    QStringList pathList;
    pathList = m_model->getPathList(listName);
    playController::getInstance().setCurPlaylist(listName,pathList);
    playController::getInstance().play(listName,index);

    playController::getInstance().delayMsecondSetVolume();
}
void TableOne::showInfo()
{
    int index = tableView->currentIndex().row();
    musicDataStruct date = m_model->getItem(index);
    infoDialog = new MusicInfoDialog(Widget::mutual,date);
    //infoDialog->setMusicDataStruct(date);
//    connect(infoDialog,&MusicInfoDialog::accepted,infoDialog,&MusicInfoDialog::deleteLater);
//    connect(infoDialog,&MusicInfoDialog::rejected,infoDialog,&MusicInfoDialog::deleteLater);
    //将弹窗应用居中显示
    if(!Global::isWayland){
        infoDialog->move(Widget::mutual->width()/2-infoDialog->width()/2+Widget::mutual->x(),Widget::mutual->height()/2-infoDialog->height()/2-20+Widget::mutual->y());
        kdk::WindowManager::setGeometry(infoDialog->windowHandle(),infoDialog->geometry());
    }else{
        QRect availableGeometry = this->parentWidget()->parentWidget()->geometry();
        infoDialog->move(availableGeometry.center() - infoDialog->rect().center());
    }
    infoDialog->exec();
}

void TableOne::addToOtherList(QAction *listNameAction)
{
    int successCount = 0;
    int failCount = 0;
    QString listName = listNameAction->text();
    if(listName == tr("I Love")) {
        listName = FAV;
    }
    QMap<int,QString> map1 = getSelectedTaskIdList();
    int allCount = map1.size();
    QMap<int,QString>::Iterator it = map1.begin();
    while(it != map1.end())
    {
        int ret = g_db->addMusicToPlayList(it.value(),listName);
        if(ret == DB_OP_SUCC)
        {
            successCount++;
            playController::getInstance().addSongToCurList(listName,it.value());
//            if(listName == tr("I Love"))
            if(listName == FAV)
            {
                Q_EMIT addILoveFilepathSignal(it.value());
            }
        }else{
            failCount++;
        }
        it++;
    }
    importFailed(successCount, failCount, allCount);
}
void TableOne::addMusicSlot()
{
    KyInfo() << "addMusicSlot";

    QFileDialog *fileDialog = new QFileDialog(Widget::mutual);
    fileDialog->setWindowTitle(tr("lingmo-music"));
    kdk::LingmoUIStyleHelper::self()->removeHeader(fileDialog);
    fileDialog->move(fileDialog->parentWidget()->width()/2-fileDialog->width()/2+fileDialog->parentWidget()->x(),fileDialog->parentWidget()->height()/2-fileDialog->height()/2+38+fileDialog->parentWidget()->y());
    kdk::WindowManager::setGeometry(fileDialog->windowHandle(),fileDialog->geometry());

//    fileDialog->setParent(Widget::mutual);
    QList<QUrl> list = fileDialog->sidebarUrls();
    int sidebarNum = 8;//最大添加U盘数，可以自己定义
    QString home = QDir::homePath().section("/", -1, -1);
    QString mnt = "/media/" + home + "/";
    QDir mntDir(mnt);
    mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList filist = mntDir.entryInfoList();
    QList<QUrl> mntUrlList;
    for (int i = 0; i < sidebarNum && i < filist.size(); ++i) {
        QFileInfo fi = filist.at(i);
        //华为990、9a0需要屏蔽最小系统挂载的目录
        if (fi.fileName() == "2691-6AB8")
             continue;
        mntUrlList << QUrl("file://" + fi.filePath());
    }

    QFileSystemWatcher fsw(fileDialog);
    fsw.addPath("/media/" + home + "/");
    connect(&fsw, &QFileSystemWatcher::directoryChanged, fileDialog,
    [=, &sidebarNum, &mntUrlList, &list, fileDialog](const QString path) {
        QDir wmntDir(path);
        wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList wfilist = wmntDir.entryInfoList();
        mntUrlList.clear();
        for (int i = 0; i < sidebarNum && i < wfilist.size(); ++i) {
                       QFileInfo fi = wfilist.at(i);
                       //华为990、9a0需要屏蔽最小系统挂载的目录
                       if (fi.fileName() == "2691-6AB8")
                            continue;
                 mntUrlList << QUrl("file://" + fi.filePath());
             }
             fileDialog->setSidebarUrls(list + mntUrlList);

             fileDialog->update();
        });

    connect(fileDialog, &QFileDialog::finished, fileDialog, [=, &list, fileDialog]() {
        QList<QUrl> lists;
        QString homePath = "file://" + QDir::homePath();
        lists << QUrl("file://");
        lists << QUrl(homePath);
        fileDialog->setSidebarUrls(lists);
    });
    fileDialog->setSidebarUrls(list + mntUrlList);
    QString setFilter = tr("Audio File") + (" (%1) ");
    setFilter = setFilter.arg(MusicFileInformation::getInstance().getMusicType().join(" "));
    fileDialog->setNameFilter(setFilter);
    //设置视图模式
    fileDialog->setViewMode(QFileDialog::Detail);
    //设置可以选择多个文件,默认为只能选择一个文件QFileDialog::ExistingFiles
    fileDialog->setFileMode(QFileDialog::ExistingFiles);
    fileDialog->setOption(QFileDialog::HideNameFilterDetails);
    QStringList songFiles;
    if(QFileDialog::Accepted == fileDialog->exec())
    {
        songFiles = fileDialog->selectedFiles();
    }
    else
    {
        return;
    }

    for( int i = 0; i < songFiles.length(); i++ )
    {
        songFiles.at(i);
    }
    addMusicToDatebase(songFiles);
}

void TableOne::addDirMusicSlot()
{
    KyInfo() << "addDirMusicSlot";

    QFileDialog *fileDialog = new QFileDialog(Widget::mutual);
    fileDialog->setWindowTitle(tr("lingmo-music"));
    kdk::LingmoUIStyleHelper::self()->removeHeader(fileDialog);
    fileDialog->move(fileDialog->parentWidget()->width()/2-fileDialog->width()/2+fileDialog->parentWidget()->x(),fileDialog->parentWidget()->height()/2-fileDialog->height()/2+38+fileDialog->parentWidget()->y());
    kdk::WindowManager::setGeometry(fileDialog->windowHandle(),fileDialog->geometry());

    QList<QUrl> list = fileDialog->sidebarUrls();
    int sidebarNum = 8;//最大添加U盘数，可以自己定义
    QString home = QDir::homePath().section("/", -1, -1);
    QString mnt = "/media/" + home + "/";
    QDir mntDir(mnt);
    mntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfoList filist = mntDir.entryInfoList();
    QList<QUrl> mntUrlList;
    for (int i = 0; i < sidebarNum && i < filist.size(); ++i) {
        QFileInfo fi = filist.at(i);
        //华为990、9a0需要屏蔽最小系统挂载的目录
        if (fi.fileName() == "2691-6AB8")
             continue;
        mntUrlList << QUrl("file://" + fi.filePath());
    }

    QFileSystemWatcher fsw(fileDialog);
    fsw.addPath("/media/" + home + "/");
    connect(&fsw, &QFileSystemWatcher::directoryChanged, fileDialog,
    [=, &sidebarNum, &mntUrlList, &list, fileDialog](const QString path) {
        QDir wmntDir(path);
        wmntDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
        QFileInfoList wfilist = wmntDir.entryInfoList();
        mntUrlList.clear();
        for (int i = 0; i < sidebarNum && i < wfilist.size(); ++i) {
                       QFileInfo fi = wfilist.at(i);
                       //华为990、9a0需要屏蔽最小系统挂载的目录
                       if (fi.fileName() == "2691-6AB8")
                            continue;
                 mntUrlList << QUrl("file://" + fi.filePath());
             }
             fileDialog->setSidebarUrls(list + mntUrlList);

             fileDialog->update();
        });

        connect(fileDialog, &QFileDialog::finished, fileDialog, [=, &list, fileDialog]() {
            QList<QUrl> lists;
            QString homePath = "file://" + QDir::homePath();
            lists << QUrl("file://");
            lists << QUrl(homePath);
            fileDialog->setSidebarUrls(lists);
        });

    //自己QFileDialog的用法，这里只是列子
//    fileDialog->setNameFilter(QLatin1String("*.mp3 *.ogg *.wma *.amr *.flac *.wav *.ape *.m4a *.ac3 *.aac"));

    fileDialog->setSidebarUrls(list + mntUrlList);

    fileDialog->setFileMode(QFileDialog::Directory);
    QStringList m_fileNames;
    if (QFileDialog::Accepted == fileDialog->exec())
    {
        m_fileNames = fileDialog->selectedFiles();
    }
    else
    {
        return;
    }
//    foreach (QString dirPath, m_fileNames) {
//        QDir dir(dirPath);
//        QStringList fileNames = dir.entryList(MusicFileInformation::getInstance().getMusicType(), QDir::Files|QDir::Readable, QDir::Name);
//        for(int i = 0; i < fileNames.count(); i++)
//        {
//            fileNames[i] = QString(dirPath + "/" + fileNames[i]);
//        }

//    }

    addMusicToDatebase(m_fileNames);
}

void TableOne::addMusicToDatebase(QStringList fileNames)
{
    //成功数量
    int successCount = 0;
    //失败数量
    int failCount = 0;
    MusicFileInformation::getInstance().addFile(fileNames);
    int allCount = MusicFileInformation::getInstance().getCount();
    int fail_count = MusicFileInformation::getInstance().getFailCount();
    QList<musicDataStruct> resList;
    resList = MusicFileInformation::getInstance().resList;

    int ret;
    foreach (const musicDataStruct date, resList)
    {
        if(nowListName == ALLMUSIC)
        {
            ret = g_db->addMusicToLocalMusic(date);
        } else if (nowListName == SEARCH) {
            return;
        } else {
            ret = g_db->addNewSongToPlayList(date,nowListName);   //数据库接口
        }
        if(ret == DB_OP_SUCC)
        {
            successCount++;
            playController::getInstance().addSongToCurList(nowListName,date.filepath);
        }
//        else
//        {
//            failCount++;
//        }
    }
    selectListChanged(nowListName);
    importFinished(successCount, fail_count, allCount);
    playController::getInstance().delayMsecondSetVolume();
}

void TableOne::importFinished(int successCount, int m_failCount, int allCount)
{

    if(m_failCount > 0) {
        Widget::mutual->setCreatFinishMsg(tr("This format file is not supported"));
        return;
    }
    if(successCount > 0)
    {
        if(allCount == 1)
        {
            return;
        }
        QMessageBox *warn = new QMessageBox(QMessageBox::Warning,tr("Prompt information"),tr("Success add %1 songs").arg(successCount),QMessageBox::Ok,Widget::mutual);
        warn->setWindowTitle(tr("lingmo-music"));
        warn->exec();
    }
    else
    {
        //此处逻辑待优化，双击打开不应该提示错误
        //if(allCount == 0)
        {
            return;
        }
        QMessageBox *warn = new QMessageBox(QMessageBox::Warning,tr("Prompt information"),tr("Add failed"),QMessageBox::Yes,Widget::mutual);
        warn->setWindowTitle(tr("lingmo-music"));
        warn->button(QMessageBox::Yes)->setText(tr("Yes"));
        kdk::LingmoUIStyleHelper::self()->removeHeader(warn);
        warn->move(warn->parentWidget()->width()/2-warn->width()/2+warn->parentWidget()->x(),warn->parentWidget()->height()/2-warn->height()/2+warn->parentWidget()->y());
        kdk::WindowManager::setGeometry(warn->windowHandle(),warn->geometry());
        warn->exec();
    }
}

void TableOne::importFailed(int successCount, int m_failCount, int allCount)
{
    if (successCount > 0)
    {
        QMessageBox *warn = new QMessageBox(QMessageBox::Warning,tr("Prompt information"),tr("Success add %1 songs").arg(successCount),QMessageBox::Ok,Widget::mutual);
        warn->setWindowTitle(tr("lingmo-music"));
        kdk::LingmoUIStyleHelper::self()->removeHeader(warn);
        warn->move(warn->parentWidget()->width()/2-warn->width()/2+warn->parentWidget()->x(),warn->parentWidget()->height()/2-warn->height()/2+warn->parentWidget()->y());
        kdk::WindowManager::setGeometry(warn->windowHandle(),warn->geometry());
        warn->exec();
    }
    else
    {
        if (m_failCount > 0) {
            QMessageBox *warn = new QMessageBox(QMessageBox::Warning,tr("Prompt information"),tr("Repeat add"),QMessageBox::Yes,Widget::mutual);
            warn->setWindowTitle(tr("lingmo-music"));
            warn->button(QMessageBox::Yes)->setText(tr("Yes"));
            kdk::LingmoUIStyleHelper::self()->removeHeader(warn);
            warn->move(warn->parentWidget()->width()/2-warn->width()/2+warn->parentWidget()->x(),warn->parentWidget()->height()/2-warn->height()/2+warn->parentWidget()->y());
            kdk::WindowManager::setGeometry(warn->windowHandle(),warn->geometry());
            warn->exec();
        }
    }
}

QMap<int,QString> TableOne::getSelectedTaskIdList()
{
    QTableView *m_pTableView = tableView;
    QModelIndexList selected = m_pTableView->selectionModel()->selectedRows();
//    QList<int> rows;
    QMap<int,QString> map1;
    foreach( const QModelIndex & index, selected)
    {
//        rows.append( index.row() );
        musicDataStruct date = m_model->getItem(index.row());
        map1.insert(index.row(),date.filepath);
    }
    return map1;
}

void TableOne::setHightLightAndSelect()
{
    //列表中歌曲为空时，跳过高亮判断
    if(m_model->count() == 0 || ( heightLightIndex >= m_model->count() && nowListName == nowPlayListName)) {
        return;
    }
    QStringList pathList;
    pathList = m_model->getPathList(nowListName);
    for(int i = 0; i < pathList.size(); i++ ) {
        QFileInfo info(pathList[i]);
        if(!info.exists()) {
            for (int j=0; j<4; j++){
                m_model->m_model.item(i,j)->setForeground(QBrush(QColor(Qt::gray)));
                m_model->m_model.item(i,j)->setData(tr("path does not exist"),Qt::ToolTipRole);
                m_model->m_model.item(i,j)->setData(QString("nofile"),Qt::StatusTipRole);
            }
        } else {
            if (m_model->m_model.item(i,0)->data(Qt::StatusTipRole).toString() != ""  ) {
                for(int j=0; j<4; j++) {
                    if(WidgetStyle::themeColor == 0) {
                        m_model->m_model.item(i,j)->setForeground(QBrush(QColor(Qt::black)));
                    } else if (WidgetStyle::themeColor == 1) {
                        m_model->m_model.item(i,j)->setForeground(QBrush(QColor(Qt::white)));
                    }
                    m_model->m_model.item(i,j)->setData(m_model->m_model.item(i,j)->text(),Qt::ToolTipRole);
                    m_model->m_model.item(i,j)->setData(QString(),Qt::StatusTipRole);
                }
            } else {
                for(int j=0; j<4; j++) {
                    if(WidgetStyle::themeColor == 0) {
                        m_model->m_model.item(i,j)->setForeground(QBrush(QColor(Qt::black)));
                    } else if (WidgetStyle::themeColor == 1) {
                        m_model->m_model.item(i,j)->setForeground(QBrush(QColor(Qt::white)));
                    }
                }
            }
            if(heightLightIndex != -1 && nowListName == nowPlayListName) {
                for (int j=0; j<4; j++) {
                    // 高亮选择highlight
                    m_model->m_model.item(heightLightIndex,j)->setData(QBrush( QApplication::palette().highlight().color()),Qt::ForegroundRole);
                    m_model->m_model.item(heightLightIndex,j)->setData(QString("heightLight"),Qt::StatusTipRole);
                }
            }
        }
    }
}

void TableOne::changeNumber()
{
    int num = m_model->count();
    listTotalNumLabel->setText(QString::number(num) + tr(" songs"));
    if(num == 0) {
        tableView->hide();
        nullPageWidget->show();

        //搜索结果为空界面时，添加按钮设置为hide，在这里需要重新处理
        if (n_addMusicButton->isHidden()) {
            n_addMusicButton->show();
        }
        if (n_addDirMusicButton->isHidden()) {
            n_addDirMusicButton->show();
        }
        addMusicButton->hide();
        playAllButton->hide();
    } else {
        tableView->show();
        nullPageWidget->hide();
        addMusicButton->show();
        playAllButton->show();
    }
}

/**
 * @brief TableOne::getMusicList
 * 从数据库中获取歌单列表并显示
 */
void TableOne::getMusicList()
{
    if(nowListName == SEARCH)
    {
        QList<musicDataStruct> resList;
        int ret;
        ret = g_db->getSongInfoListFromLocalMusicByKeyword(resList, m_text);
        if (ret == DB_OP_SUCC)
        {
            m_model->clear();
            m_model->add(resList);
            for(int i = 0; i < resList.size(); i++)
            {
                g_db->addMusicToPlayList(resList.at(i).filepath, SEARCH);
            }
            initTableViewStyle();
        }
    }
    else
    {
        QList<musicDataStruct> resList;
        int ret;
        ret = g_db->getSongInfoListFromDB(resList, nowListName);
        if(ret == DB_OP_SUCC)
        {
            m_model->clear();
            m_model->add(resList);
            initTableViewStyle();
        }
    }
}

void TableOne::selectListChanged(QString listname)
{
    tableView->setNowPlayListName(listname);
    m_model->clear();
    if(listname == tr("Song List")) {
        nowListName = ALLMUSIC;
    } else if(listname == tr("I Love")) {
        nowListName = FAV;
    } else if(listname == tr("Search Result")) {
        Q_EMIT signalListSearch();
        nowListName = SEARCH;
//        return;
    } else {
        nowListName = listname;
    }
    getMusicList();
    changeNumber();
    showTitleText(listname);
    setHightLightAndSelect();
}

void TableOne::playListRenamed(QString oldName, QString newName)
{
    if(listTitleLabel->text() == oldName)
    {
        listTitleLabel->setText(newName);
        nowListName = newName;
    }
}

void TableOne::getHightLightIndex(int index, QString listName)
{
    nowPlayListName = listName;
    heightLightIndex = index;
    if(listName == nowListName){
        setHightLightAndSelect();
    } else {
        heightLightIndex = -1;
        setHightLightAndSelect();
        heightLightIndex = index;
    }
}

void TableOne::playAll(QString listName)
{
    if(listName == tr("Song List")) {
        listName = ALLMUSIC;
    } else if(listName == tr("I Love")) {
        listName = FAV;
    } else if(listName == tr("Search Result")) {
        listName = SEARCH;
    }
    QStringList list = m_model->getPathList(listName);
    if(list.size() == 0) {
        return;
    }
    playMusicforIndex(listName,0);
}

void TableOne::playAllButtonClicked()
{
    playAll(nowListName);
}

void TableOne::slotReturnText(QString text)
{
    int ret = g_db->checkPlayListExist(SEARCH);
    if(ret == DB_OP_SUCC)
    {
        m_model->clear();
        int ret;
        QList<musicDataStruct> musicDateList;
        ret = g_db->getSongInfoListFromPlayList(musicDateList, SEARCH);
        if(ret == DB_OP_SUCC)
        {
            for(int i = 0; i < musicDateList.size(); i++)
            {
                playController::getInstance().removeSongFromCurList(SEARCH, 0);
            }
        }

        g_db->delPlayList(SEARCH);
    }

    int ref = g_db->createNewPlayList(SEARCH);
    if(ref != DB_OP_SUCC)
    {
        qDebug() << "创建搜索歌单失败";
        return;
    }

    m_text = text;
    getMusicList();
    changeNumber();
    showTitleText(tr("Search Result"));
    setHightLightAndSelect();

    addMusicButton->hide();
    n_addMusicButton->hide();
    n_addDirMusicButton->hide();
    listTitleHBoxLayout->setContentsMargins(25, 20, 9, 30);
}

void TableOne::slotFilePath(QString path)
{
    Q_EMIT signalSongListHigh();

    QList<musicDataStruct> resList;
    int ret = g_db->getSongInfoListFromLocalMusic(resList);
    if (ret == DB_OP_SUCC) {
        for (int i = 0; i < resList.size(); i++) {
            if(resList[i].filepath == path) {
                playMusicforIndex(ALLMUSIC, i);
            }
        }
    }

    selectListChanged(tr("Song List"));
}

void TableOne::slotSongListBySinger(QString singer)
{
    clearSearchList();

    m_model->clear();
    //取消侧边栏所有按钮的选中状态
    Q_EMIT signalListSearch();
    nowListName = SEARCH;
//        return;

    QList<musicDataStruct> resList;
    int ret = g_db->getSongInfoListBySinger(resList, singer);
    if (ret == DB_OP_SUCC) {
        m_model->clear();
        m_model->add(resList);
        for(int i = 0; i < resList.size(); i++)
        {
            g_db->addMusicToPlayList(resList.at(i).filepath, SEARCH);
        }
        initTableViewStyle();
    }

    changeNumber();
    showTitleText(tr("Search Result"));
    setHightLightAndSelect();

    addMusicButton->hide();
    listTitleHBoxLayout->setContentsMargins(25, 20, 9, 30);
}

void TableOne::slotSongListByAlbum(QString album)
{
    clearSearchList();

    m_model->clear();
    //取消侧边栏所有按钮的选中状态
    Q_EMIT signalListSearch();
    nowListName = SEARCH;

    QList<musicDataStruct> resList;
    int ret = g_db->getSongInfoListByAlbum(resList, album);
    if (ret == DB_OP_SUCC) {
        m_model->clear();
        m_model->add(resList);
        for(int i = 0; i < resList.size(); i++)
        {
            g_db->addMusicToPlayList(resList.at(i).filepath, SEARCH);
        }
        initTableViewStyle();
    }

    changeNumber();
    showTitleText(tr("Search Result"));
    setHightLightAndSelect();

    addMusicButton->hide();
    listTitleHBoxLayout->setContentsMargins(25, 20, 9, 30);
}

void TableOne::clearSearchList()
{
    int ret = g_db->checkPlayListExist(SEARCH);
    if(ret == DB_OP_SUCC)
    {
        m_model->clear();
        int ret;
        QList<musicDataStruct> musicDateList;
        ret = g_db->getSongInfoListFromPlayList(musicDateList, SEARCH);
        if(ret == DB_OP_SUCC)
        {
            for(int i = 0; i < musicDateList.size(); i++)
            {
                playController::getInstance().removeSongFromCurList(SEARCH, 0);
            }
        }

        g_db->delPlayList(SEARCH);
    }

    int ref = g_db->createNewPlayList(SEARCH);
    if(ref != DB_OP_SUCC)
    {
        qDebug() << "创建搜索歌单失败";
        return;
    }
}

void TableOne::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasFormat("text/uri-list"))
    {
        event->acceptProposedAction();
    }
    QWidget::dragEnterEvent(event);
}

void TableOne::dropEvent(QDropEvent *event)
{
    auto urls = event->mimeData()->urls();
    if(urls.isEmpty())
    {
        return;
    }
    QStringList localpath;
    for(auto &url : urls)
    {
        localpath << url.toLocalFile();
    }

    if(!localpath.isEmpty())
    {
        addMusicToDatebase(localpath);
    }

    playController::getInstance().delayMsecondSetVolume();

}

void TableOne::mouseMoveEvent(QMouseEvent *event)
{
    QModelIndex index = tableView->indexAt(event->pos());
    int row = index.row();
    Q_EMIT hoverIndexChanged(index);
}


void TableOne::mousePressEvent(QMouseEvent *event)
{
    g_user_signal->hideSearchResult();

    QWidget::mousePressEvent(event);
}

void TableOne::resizeEvent(QResizeEvent *event)
{
    // 使用变量保存tableView一页最多可以完全显示多少条目，实时刷新，实现滑动条的显示和隐藏
    showScrollbarNumber = tableView->height()/42;
    initTableViewStyle();
}

void TableOne::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Space)
    {
        return;
    }
    QWidget::keyPressEvent(event);
}

//搜索结果关键字通过LineEdit传参得到
void TableOne::slotSearchTexts(QString text)
{
    tableView->setSearchText(text);
}

//搜索歌单通过LineEdit传参得到
void TableOne::slotSearchReturnPressed(QString listName)
{
    tableView->setSearchListName(listName);
}
