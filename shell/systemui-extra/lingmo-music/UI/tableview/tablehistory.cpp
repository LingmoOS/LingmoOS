#include "tablehistory.h"
#include "UI/mainwidget.h"
#include "UI/base/widgetstyle.h"
#include "UIControl/global/global.h"
#include "lingmostylehelper/lingmostylehelper.h"
#include "windowmanager/windowmanager.h"

#define PT_15 15

TableHistory::TableHistory(QWidget *parent) : QDialog(parent)
{
    if (WidgetStyle::isWayland) {
        // wayland 下move位置设置不生效，需要添加如下代码
        setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    }

    initSetModel();
    initStyle();
    initConnect();
    initRightMenu();
    initTableStyle();
    qDebug() << "播放历史列表初始化完成.";
}

void TableHistory::initSetModel()
{
    //禁用界面拖拽
    this->setProperty("useStyleWindowManager", false);
    mainVLayout = new QVBoxLayout();
    m_tableHistory = new TableBaseView;
    m_tableHistory->setObjectName("m_tableHistory");

    m_tableHistory->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tableHistory->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableHistory->setShowGrid(false);
    m_tableHistory->verticalHeader()->setDefaultSectionSize(40);
    m_model = new MusicListModel;
    QList<musicDataStruct> resList;
    g_db->getSongInfoListFromHistoryMusic(resList);
    m_model->add(resList);
    m_model->setView(*m_tableHistory);

    historyTitileLabel = new QLabel(this);
    historyTitileLabel->setText(tr("History"));
    listCountLabel = new QLabel(this);
    historyTitileWidget = new QWidget(this);
    historyTitileWidget->setFixedHeight(64);
    deleteAllBtn = new QToolButton(this);
    deleteAllBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    deleteAllBtn->setIconSize(QSize(16,16));
    deleteAllBtn->setIcon(QIcon::fromTheme("edit-delete-symbolic"));
    deleteAllBtn->setText(tr("Empty"));
    titleHBoxLayout = new QHBoxLayout();
    historyTitileWidget->setLayout(titleHBoxLayout);
    titleHBoxLayout->addWidget(historyTitileLabel,0,Qt::AlignLeft);
    titleHBoxLayout->addWidget(listCountLabel,1,Qt::AlignBottom);
    titleHBoxLayout->addWidget(deleteAllBtn,Qt::AlignRight);
    titleHBoxLayout->setContentsMargins(16,16,16,12);

    nullPageWidget = new QWidget(this);
    nullIconLabel = new QLabel(this);
    if (WidgetStyle::themeColor == 1) {
        // black theme
        nullIconLabel->setPixmap(QPixmap(":/img/default/defaultIconDark.svg").scaled(96, 96));
    } else {
        nullIconLabel->setPixmap(QPixmap(":/img/default/defaultIconLight.svg").scaled(96, 96));
    }
    nullTextLabel = new QLabel(this);
    //nullTextLabel->setFixedHeight(30);
    nullTextLabel->setText(tr("The playlist has no songs"));
    nullPageLayout = new QVBoxLayout();
    nullPageLayout->addStretch(1);
    nullPageLayout->addWidget(nullIconLabel);
    nullIconLabel->setAlignment(Qt::AlignHCenter);
    nullPageLayout->addWidget(nullTextLabel);
    nullTextLabel->setAlignment(Qt::AlignHCenter);
    nullPageLayout->addStretch(1);
    nullPageLayout->setSpacing(0);
    nullPageLayout->setAlignment(Qt::AlignCenter);
    nullPageWidget->setLayout(nullPageLayout);

    mainVLayout->addWidget(historyTitileWidget,Qt::AlignTop);
//    mainVLayout->addStretch(0);
    mainVLayout->addWidget(m_tableHistory);
    mainVLayout->addWidget(nullPageWidget);
    mainVLayout->setMargin(0);
    mainVLayout->setSpacing(0);
    this->setLayout(mainVLayout);
    this->setFixedWidth(320);
    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);
//    this->setAutoFillBackground(true);
//    this->setBackgroundRole(QPalette::Base);
    changeNumber();


}
void TableHistory::initStyle()
{


    m_tableHistory->setStyleSheet("#m_tableHistory{border:none;}");
    historyTitileLabel->setStyleSheet("height:20px;\
                                 font-weight: 600;\
                                 line-height: 20px;");

    listCountLabel->setStyleSheet("font-weight: 400;color:#8C8C8C;\
                                  line-height: 14px;");
    deleteAllBtn->setStyleSheet("color:#8F9399;background:transparent;");
    if (WidgetStyle::themeColor == 1) {
        // black theme
        nullTextLabel->setStyleSheet("font-size: 14px; color:#8F9399; opacity: 1;");
    } else {
        nullTextLabel->setStyleSheet("font-size: 14px; color:#8C8C8C; opacity: 1;");
    }
}

void TableHistory::slotLableSetFontSize(int size)
{
    //默认大小12px,换算成pt为9
    double lableBaseFontSize = PT_15;//魔鬼数字，自行处理
    double nowFontSize = lableBaseFontSize * double(size) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    font.setPointSizeF(nowFontSize);
    historyTitileLabel->setFont(font);
}

void TableHistory::deleteAllClicked()
{
    QMessageBox msg(QMessageBox::Warning,tr("Prompt information"),tr("Clear the playlist?"),QMessageBox::Yes|QMessageBox::No,Widget::mutual);
    msg.setWindowTitle(tr("lingmo-music"));
    kdk::LingmoUIStyleHelper::self()->removeHeader(&msg);

    int ret = msg.exec();
    if(ret == QMessageBox::Yes)
    {
        for(int i = 0;i < m_model->count();i++)
        {
            playController::getInstance().removeSongFromCurList(HISTORY, 0);
        }
        int result = g_db->emptyHistoryMusic();
        if(result == DB_OP_SUCC) {
            m_model->clear();
            changeNumber();
        }
        else {
            return;
        }

    }
    else
    {
        return;
    }

}
void TableHistory::slotPlayIndexChanged(int index, QString listname)
{
    if(listname == HISTORY) {
        nowPlayListName = HISTORY;
        nowPlayIndex = index;
        noRefreshHistory();
    }
    else {
        nowPlayListName = listname;
    }
}

void TableHistory::slotPlayPathChanged(QString songPath)
{
    if(nowPlayListName != HISTORY && songPath != "") {
        int ret = g_db->addMusicToHistoryMusic(songPath);
        if(ret == DB_OP_SUCC) {
            nowPlayIndex = 0;
            refreshHistoryTable();
        }
        else {
            return;
        }
    }
    else if(songPath == "")
    {
        nowPlayIndex = -1;
        noRefreshHistory();
    }
    else
    {
        return;
    }
}

bool TableHistory::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(result);
    if(eventType != "xcb_generic_event_t")
    {
        return false;
    }

    xcb_generic_event_t *event = (xcb_generic_event_t*)message;
    switch (event->response_type & ~0x80)
    {
        case XCB_FOCUS_OUT:
            // -playHistoryPosWidth (负数原因：因为计算按钮的右下角点)
            QRect rect(playHistoryPosX, playHistoryPosY, -playHistoryPosWidth, playHistoryPosHeight);
            if(rect.contains(QCursor::pos(), false)) {
                return false;
            }

            if (this->geometry().contains(QCursor::pos(), false)) {
                return false;
            }

            this->hide();
            Q_EMIT signalHistoryBtnChecked(false);
            break;
    }
    return false;
}

void TableHistory::initTableStyle()
{
    m_tableHistory->hideColumn(2);
    m_tableHistory->setColumnWidth(0,150);
    m_tableHistory->setColumnWidth(1,80);
    m_tableHistory->setColumnWidth(3,40);
    m_tableHistory->setAutoFillBackground(true);
    m_tableHistory->verticalHeader()->setVisible(false);// 垂直不可见
    m_tableHistory->horizontalHeader()->setVisible(false);
    m_tableHistory->setAlternatingRowColors(false);

}

void TableHistory::initConnect()
{
    connect(deleteAllBtn,&QToolButton::clicked,this,&TableHistory::deleteAllClicked);
    connect(m_tableHistory,&TableHistory::customContextMenuRequested,this,&TableHistory::showRightMenu);
    connect(m_tableHistory,&TableBaseView::doubleClicked,this,&TableHistory::playSongs);
    connect(&playController::getInstance(),&playController::currentIndexAndCurrentList,this,&TableHistory::slotPlayIndexChanged);
    connect(&playController::getInstance(),&playController::singalChangePath,this,&TableHistory::slotPlayPathChanged);


}
void TableHistory::showHistroryPlayList()
{
    if(this->isVisible()) {
        this->hide();
        signalHistoryBtnChecked(false);
    } else {
        Widget::mutual->movePlayHistoryWid();
        if (Global::isWayland) {
            this->exec();
        }
        else{
            this->show();
        }
        signalHistoryBtnChecked(true);
        this->raise();
    }
    m_tableHistory->setAlternatingRowColors(false);
}
void TableHistory::changeNumber()
{
    int num = m_model->count();
    listCountLabel->setText(QString::number(num) + tr(" songs"));
    if(num == 0) {
        nullPageWidget->show();
        m_tableHistory->hide();
        isHightLight = false;
    } else{
        nullPageWidget->hide();
        m_tableHistory->show();
    }
}
void TableHistory::refreshHistoryTable()
{
    m_model->clear();
    QList<musicDataStruct> resList;
    g_db->getSongInfoListFromHistoryMusic(resList);
    m_model->add(resList);
    changeNumber();
    initTableStyle();
    setHighlight(nowPlayIndex);
}
void TableHistory::noRefreshHistory()
{
    changeNumber();
    initTableStyle();
    setHighlight(nowPlayIndex);
}
void TableHistory::initRightMenu()
{
    m_menu = new QMenu;
    playRow = new QAction(tr("Play"));
    removeRow = new QAction(tr("Delete"));
    playNextRow = new QAction(tr("Play the next one"));

    m_menu->addAction(playRow);
    m_menu->addAction(removeRow);
//    m_menu->addAction(playNextRow);
    connect(playRow,&QAction::triggered,this,&TableHistory::playSongs);
    connect(removeRow,&QAction::triggered,this,&TableHistory::deleteSongs);
    connect(playNextRow,&QAction::triggered,this,&TableHistory::playNextRowClicked);

//    m_tableHistory->installEventFilter(this);
}
void TableHistory::showRightMenu(const QPoint &pos)
{
    QModelIndex index = m_tableHistory->indexAt(pos);
    if(index.row() < 0)
    {
        return;
    }
    m_menu->exec(QCursor::pos());
}

//加入移除列表 removeList
void TableHistory::slotAddToRList(QString path,QString owner){
    QStringList  pathList = m_model->getPathList(nowListName); //历史列表
    //判断删除的歌曲是否在历史列表
    if(pathList.contains(path)){
        if( removeList.contains(path)){
            removeList[path] = owner;
        }else{
            removeList.insert(path,owner);
        }
    }
}
//移除移除列表 removeList
void TableHistory::slotRemoveFromRList(QString path){
    if(removeList.contains(path)){
        removeList.remove(path);
    }
}

/**
 * @brief TableHistory::playSongs 播放音乐
 */
void TableHistory::playSongs()
{
    int index = m_tableHistory->currentIndex().row();
    musicDataStruct date = m_model->getItem(index);
    QStringList pathList;
    pathList = m_model->getPathList(nowListName);

    //历史列表选择播放的歌曲的路径
    QString selectPath = m_model->getItem(index).filepath;

    //选择播放歌曲是否在 removeList
    while ( removeList.contains(selectPath) )
    {
        //选择播放歌曲是否在 对应歌单列表(删除时所在的歌单列表)
        QStringList removePathList = m_model->getPathList(removeList.value(selectPath));
        if( !removePathList.contains(selectPath)) {
            //不在 则提示、删除、播放历史列表下一首
            QMessageBox::information(Widget::mutual,tr("Prompt information"),tr("The file path does not exist or has been deleted"));
            int ret = g_db->delMusicFromHistoryMusic(selectPath);
            slotRemoveFromRList(selectPath);
            if (ret == 0){
                playController::getInstance().removeSongFromCurList(nowListName,index);
                if((index+1) < pathList.size()){
                    index = index+1;
                }else{
                    index = 0;
                }
            }

        }
        selectPath = m_model->getItem(index).filepath;
    }

    //歌曲不存在则播放歌曲列表下一首
    QString lastListName = playController::getInstance().getPlayListName();
    QStringList lastPathList = m_model->getPathList(lastListName);
    int m_lastindex = lastPathList.indexOf(selectPath,0); //已删除音乐的索引
    playController::getInstance().setCurPlaylist(lastListName,lastPathList);
    playController::getInstance().play(lastListName,m_lastindex);
    refreshHistoryTable();
    return;

    refreshHistoryTable();

    QString filepath = pathList.at(index);
    //将历史表对应的歌曲重新添加 实现置顶功能
    MusicDataBase* database = MusicDataBase::getInstance();
    database->delMusicFromHistoryMusic(filepath);
    database->addMusicToHistoryMusic(filepath);
    //数据模型和视图分别进行更新
    QList<musicDataStruct> resList;
    g_db->getSongInfoListFromHistoryMusic(resList);
//    for(int i =0; i<resList.length();i++){
//        qDebug()<<"reList的第"<<i<<"项是"<<resList[i].title;
//    }
    m_model->clear();
    m_model->add(resList);
    m_tableHistory->update();
    QStringList afterUpdatePathList = m_model->getPathList(nowListName);
    //播放歌曲
    playController::getInstance().setCurPlaylist(nowListName,afterUpdatePathList);
    playController::getInstance().play(nowListName,0);
    //以下是高亮看看能不能置顶

    QItemSelectionModel *model = m_tableHistory->selectionModel();
    //获取目前点击的项index取消掉选中状态
    QModelIndex m_firstIndex = model->model()->index(index,0);
    QModelIndex m_lastIndex = model->model()->index(index,m_model->m_model.columnCount()-1);
    QItemSelection m_selection(m_firstIndex,m_lastIndex);
    model->select(m_selection,QItemSelectionModel::Deselect);
    //将第一行设置为选中状态
    QModelIndex firstIndex =model->model()->index(0,0);
    qDebug()<<"m_lastIndex"<<m_lastIndex.data();
    QModelIndex lastIndex = model->model()->index(0,m_model->m_model.columnCount()-1);
    QItemSelection selection(firstIndex,lastIndex);
    model->select(selection,QItemSelectionModel::Select);
    m_tableHistory->update();
    changeNumber();
    initTableStyle();
    setHighlight(0);

    Q_EMIT signalHistoryPlaying();
}
void TableHistory::deleteSongs()
{
    int index = m_tableHistory->currentIndex().row();
//    if(index == nowPlayIndex && nowPlayListName != HISTORY) {
//        QMessageBox::warning(Widget::mutual,tr("Prompt information"),tr("歌曲列表或歌单中正在播放此歌曲，无法删除~"));
//        return;
//    }
    musicDataStruct date = m_model->getItem(index);
//    g_db->delMusicFromPlayList(date.filepath,nowListName);
    playController::getInstance().removeSongFromCurList(nowListName,index);
    g_db->delMusicFromHistoryMusic(date.filepath);
    if(index == nowPlayIndex && nowPlayListName != HISTORY) {
        nowPlayIndex = -1;
    }
    refreshHistoryTable();
}

void TableHistory::playNextRowClicked()
{

}

void TableHistory::changePlayHistoryPos(int posX, int posY, int width, int height)
{
    playHistoryPosX = posX;
    playHistoryPosY = posY;
    playHistoryPosWidth = width;
    playHistoryPosHeight = height;
}

void TableHistory::setHighlight(int index)
{
    if (WidgetStyle::themeColor == 1) {
        // black theme
        nullIconLabel->setPixmap(QPixmap(":/img/default/defaultIconDark.svg").scaled(96, 96));
    } else {
        nullIconLabel->setPixmap(QPixmap(":/img/default/defaultIconLight.svg").scaled(96, 96));
    }

    //播放历史模块高亮
    if(m_model->count() == 0 ||  index >= m_model->count()) {
        return;
    }

    QColor highlightColor(QApplication::palette().highlight().color());

    if(WidgetStyle::themeColor == 0) {
        for (int m = 0; m<3 ; m++) {
            for(int k = 0; k < m_model->count() ; k++) {
                QStringList pathList;
                pathList = m_model->getPathList(nowListName);

                for(int i = 0; i < pathList.size(); i++) {
                    QFileInfo info(pathList[i]);
                    if(!info.exists()) {
                        for (int j=0; j<4; j++){
                            m_model->m_model.item(i, j)->setForeground(QBrush(QColor(Qt::gray)));
                            m_model->m_model.item(i, j)->setData(tr("path does not exist"),Qt::ToolTipRole);
                        }
                    }
               }

                m_model->m_model.item(k, m)->setForeground(QBrush(QColor(Qt::black)));
            }
        }

        if(index != -1) {
            for (int i = 0 ; i<4 ;i++ ) {
                m_model->m_model.item(index,i)->setData(QBrush(highlightColor),Qt::ForegroundRole);
            }
        }
    }

    if(WidgetStyle::themeColor == 1) {
        for (int m = 0; m < 3; m++ ) {
            for(int k = 0; k < m_model->count(); k++) {
                QStringList pathList;
                pathList = m_model->getPathList(nowListName);

                for(int i = 0; i < pathList.size(); i++ ) {
                    QFileInfo info(pathList[i]);

                    if(!info.exists()) {
                        for (int j=0; j<4; j++){
                            m_model->m_model.item(i,j)->setForeground(QBrush(QColor(Qt::gray)));
                            m_model->m_model.item(i,j)->setData(tr("path does not exist"),Qt::ToolTipRole);
                        }
                    }
               }

                m_model->m_model.item(k, m)->setForeground(QBrush(QColor(Qt::white)));
            }
        }

        if(index != -1) {
            for (int i = 0 ; i<4 ;i++ ) {
                m_model->m_model.item(index, i)->setData(QBrush(highlightColor),Qt::ForegroundRole);
            }
        }
    }
}
