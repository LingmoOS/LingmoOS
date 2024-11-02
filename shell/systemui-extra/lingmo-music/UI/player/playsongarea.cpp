#include <lingmo-log4qt.h>
#include "UIControl/global/global.h"
#include "playsongarea.h"
#include "UI/base/widgetstyle.h"
#include "UI/base/xatom-helper.h"
#include "UIControl/base/musicDataBase.h"
#include "lingmostylehelper/lingmostylehelper.h"
#include "windowmanager/windowmanager.h"

#define VOLUME 5
#define PT_9 9

PlaySongArea::PlaySongArea(QWidget *parent) : QWidget(parent)
{
    resize(750,88);
    setAttribute(Qt::WA_StyledBackground,true);
    this->setObjectName("PlaySongArea");
    initWidget();
    initConnect();
    playcolor();
}

void PlaySongArea::initWidget()
{
    QVBoxLayout *m_vmainLayout = new QVBoxLayout(this);
    QHBoxLayout *m_hmainLayout = new QHBoxLayout();
    m_mainLayout = new QHBoxLayout();

    preBtn = new QPushButton;
    preBtn->setFixedSize(18,18);
    preBtn->setCursor(Qt::PointingHandCursor);
    preBtn->setToolTip(tr("Previous"));

    playBtn = new QPushButton(this);
    playBtn->setFixedSize(36,36);
    playBtn->setCursor(Qt::PointingHandCursor);

    nextBtn = new QPushButton;
    nextBtn->setFixedSize(18,18);
    nextBtn->setCursor(Qt::PointingHandCursor);
    nextBtn->setToolTip(tr("Next"));

    volumeBtn = new QPushButton(this);
    volumeBtn->setFixedSize(36, 36);
    volumeBtn->setToolTip(tr("Volume"));    //音量
    int volume = playController::getInstance().getVolume();
    if(volume == 0)
    {
        volumeBtn->setIcon(QIcon::fromTheme("audio-volume-muted-symbolic"));
        volumeBtn->setProperty("isWindowButton", 0x1);
        volumeBtn->setProperty("useIconHighlightEffect", 0x2);
        volumeBtn->setFlat(true);
    }
    else if(volume > 0 && volume <= 33)
    {
        volumeBtn->setIcon(QIcon::fromTheme("audio-volume-low-symbolic"));
        volumeBtn->setProperty("isWindowButton", 0x1);
        volumeBtn->setProperty("useIconHighlightEffect", 0x2);
        volumeBtn->setFlat(true);
    }
    else if(volume > 33 && volume <= 77)
    {
        volumeBtn->setIcon(QIcon::fromTheme("audio-volume-medium-symbolic"));
        volumeBtn->setProperty("isWindowButton", 0x1);
        volumeBtn->setProperty("useIconHighlightEffect", 0x2);
        volumeBtn->setFlat(true);
    }
    else
    {
        volumeBtn->setIcon(QIcon::fromTheme("audio-volume-high-symbolic"));
        volumeBtn->setProperty("isWindowButton", 0x1);
        volumeBtn->setProperty("useIconHighlightEffect", 0x2);
        volumeBtn->setFlat(true);
    }

    m_volSliderWid = new SliderWidget((QWidget *)parent());

    if (! WidgetStyle::isWayland) {
        MotifWmHints hint;
        hint.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
        hint.functions = MWM_FUNC_ALL;
        hint.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(m_volSliderWid->winId(), hint);
    }
    kdk::LingmoUIStyleHelper::self()->removeHeader((QWidget *)m_volSliderWid);
    m_volSliderWid->hide();
    moveVolSliderWid();
    kdk::WindowManager::setGeometry(m_volSliderWid->windowHandle(), QRect(volumePos, m_volSliderWid->size()));

    m_playBackModeWid = new PlayBackModeWidget(this);
    if (! WidgetStyle::isWayland) {
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(m_playBackModeWid->winId(), hints);
    }
    m_playBackModeWid->hide();

    hSlider = new MusicSlider(this);
    hSlider->setValue(0);
    hSlider->setDisabled(true);
    favBtn = new QPushButton;
    favBtn->setFixedSize(36, 36);
    favBtn->setFocusPolicy(Qt::NoFocus);
    favBtn->setToolTip(tr("Favourite"));
    favBtn->setProperty("isWindowButton", 0x1);
    favBtn->setProperty("useIconHighlightEffect", 0x2);
    favBtn->setFlat(true);

    //播放模式
    playModeBtn = new QToolButton;
    playModeBtn->setFixedSize(36, 36);
    playModeBtn->setPopupMode(QToolButton::InstantPopup);
    playModeBtn->setProperty("isWindowButton", 0x1);
    playModeBtn->setProperty("useIconHighlightEffect", 0x2);
    playModeBtn->setAutoRaise(true);

    switch (playController::getInstance().mode()) {
    case 1:
        playModeBtn->setIcon(QIcon::fromTheme("media-playlist-repeat-one-symbolic"));
        //playModeBtn->setProperty("isWindowButton", 0x1);
        //playModeBtn->setProperty("useIconHighlightEffect", 0x2);
        //playModeBtn->setFlat(true);
        playModeBtn->setToolTip(tr("CurrentItemInLoop"));
        playController::getInstance().setPlaymode(playController::CurrentItemInLoop);
        break;
    case 3:
        playModeBtn->setIcon(QIcon::fromTheme("media-playlist-repeat-symbolic"));
        //playModeBtn->setProperty("isWindowButton", 0x1);
        //playModeBtn->setProperty("useIconHighlightEffect", 0x2);
        //playModeBtn->setFlat(true);
        playModeBtn->setToolTip(tr("Loop"));
        playController::getInstance().setPlaymode(playController::Loop);
        break;
    case 4:
        playModeBtn->setIcon(QIcon::fromTheme("media-playlist-shuffle-symbolic"));
        //playModeBtn->setProperty("isWindowButton", 0x1);
        //playModeBtn->setProperty("useIconHighlightEffect", 0x2);
        //playModeBtn->setFlat(true);
        playModeBtn->setToolTip(tr("Random"));
        playController::getInstance().setPlaymode(playController::Random);
        break;
    default:
        break;
    }

    m_playMode = new QMenu(this);
    loopAct = new QAction(tr("Loop"));
    loopAct->setIcon(QIcon::fromTheme("media-playlist-repeat-symbolic"));
    randomAct = new QAction(tr("Random"));
    randomAct->setIcon(QIcon::fromTheme("media-playlist-shuffle-symbolic"));
    currentItemInLoopAct = new QAction(tr("CurrentItemInLoop"));
    currentItemInLoopAct->setIcon(QIcon::fromTheme("media-playlist-repeat-one-symbolic"));
    m_playMode->addAction(loopAct);
    m_playMode->addAction(randomAct);
    m_playMode->addAction(currentItemInLoopAct);
//    playModeBtn->setMenu(m_playMode);

    //历史播放列表
    listBtn = new QPushButton;
    listBtn->setFixedSize(36, 36);
//    listBtn->setCheckable(true); //按钮是否是可点击状态，默认不点击,
    listBtn->setIcon(QIcon::fromTheme("lingmo-play-list-symbolic"));
    listBtn->setProperty("isWindowButton", 0x1);
    listBtn->setProperty("useIconHighlightEffect", 0x2);
    listBtn->setFlat(true);
    listBtn->setToolTip(tr("Play List"));

    coverPhotoLabel = new QLabel(this);
    coverPhotoLabel->setFixedSize(40,40);

    playingLabel = new MyLabel(this);
    playingLabel->setFixedHeight(28);
//    playingLabel->setText(tr("Music Player"));

    timeLabel = new QLabel(this);
//    timeLabel->setText(tr("00:00/00:00"));

    QString playPath = playController::getInstance().getPath();
    if(playPath != "")
    {
        slotSongInfo(playPath);
        slotPositionChanged(0);
    }
    else
    {
        playingLabel->setText(tr("Music Player"));
        timeLabel->setText(tr("00:00/00:00"));
    }

    QWidget *letfWid = new QWidget(this);
    QWidget *centreWid = new QWidget(this);
    QWidget *rightWid = new QWidget(this);

    QHBoxLayout *leftLayout = new QHBoxLayout(letfWid);

    QWidget *vWidget = new QWidget(letfWid);
    vWidget->setFixedHeight(44);
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addStretch();
    vLayout->addWidget(playingLabel,0,Qt::AlignVCenter);
//    vLayout->addSpacing(8);
    vLayout->addWidget(timeLabel,0,Qt::AlignVCenter);
    vLayout->addStretch();
    vLayout->setContentsMargins(8,3,0,3);
    vLayout->setMargin(0);
    vWidget->setLayout(vLayout);


    leftLayout->addWidget(coverPhotoLabel,Qt::AlignVCenter);
    leftLayout->addWidget(vWidget,Qt::AlignVCenter);
    leftLayout->setMargin(0);

    QHBoxLayout *playLayout = new QHBoxLayout(centreWid);
    playLayout->addStretch(0);
    playLayout->addWidget(preBtn,Qt::AlignHCenter);
    playLayout->addSpacing(12);
    playLayout->addWidget(playBtn,Qt::AlignHCenter);
    playLayout->addSpacing(12);
    playLayout->addWidget(nextBtn,Qt::AlignHCenter);
    playLayout->addStretch();
    playLayout->setMargin(0);

    QHBoxLayout *rightLayout = new QHBoxLayout(rightWid);
    rightLayout->addStretch(0);
    rightLayout->addWidget(volumeBtn,Qt::AlignRight);
    rightLayout->addSpacing(8);
    rightLayout->addWidget(favBtn,Qt::AlignRight);
    rightLayout->addSpacing(8);
    rightLayout->addWidget(playModeBtn,Qt::AlignRight);
    rightLayout->addSpacing(8);
    rightLayout->addWidget(listBtn,Qt::AlignRight);
    rightLayout->setContentsMargins(0,14,30,14);
    rightLayout->setMargin(0);

    letfWid->setLayout(leftLayout);
//    letfWid->setStyleSheet("background: red;");
    centreWid->setLayout(playLayout);
//    centreWid->setStyleSheet("background: blue;");
    rightWid->setLayout(rightLayout);
//    rightWid->setStyleSheet("background: green;");

    m_hmainLayout->addWidget(hSlider);
    m_hmainLayout->setMargin(0);
    m_hmainLayout->setSpacing(0);

    m_mainLayout->addWidget(letfWid);
//    m_mainLayout->addStretch(0);
    m_mainLayout->addWidget(centreWid);
//    m_mainLayout->addStretch(0);
    m_mainLayout->addWidget(rightWid);
//    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);

    m_mainLayout->setStretchFactor(letfWid,1);
    m_mainLayout->setStretchFactor(centreWid,1);
    m_mainLayout->setStretchFactor(rightWid,1);
    m_mainLayout->setContentsMargins(25,0,25,0);


    m_vmainLayout->addLayout(m_hmainLayout);
    m_vmainLayout->addLayout(m_mainLayout);
    m_vmainLayout->setContentsMargins(0,0,0,8);
    m_vmainLayout->setSpacing(0);

    this->setLayout(m_vmainLayout);


    //限制应用字体不随着主题变化
//    QFont sizeFont;
//    sizeFont.setPixelSize(14);
//    playingLabel->setFont(sizeFont);
//    timeLabel->setFont(sizeFont);
}

void PlaySongArea::slotLableSetFontSize(int size)
{
    //默认大小12px,换算成pt为9
    double lableBaseFontSize = PT_9;//魔鬼数字，自行处理
    double nowFontSize = lableBaseFontSize * double(size) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    font.setPointSizeF(nowFontSize);
    timeLabel->setFont(font);
}

void PlaySongArea::initConnect()
{
    //
    connect(preBtn,&QPushButton::clicked,this,&PlaySongArea::slotPrevious);
    connect(playBtn,&QPushButton::clicked,this,&PlaySongArea::slotPlayClicked);
    connect(nextBtn,&QPushButton::clicked,this,&PlaySongArea::slotNext);
    connect(listBtn,&QPushButton::clicked,this,&PlaySongArea::listBtnClicked);
    connect(m_volSliderWid->vSlider,&QSlider::valueChanged,this,&PlaySongArea::slotVolumeChanged);
    connect(volumeBtn,&QPushButton::clicked,this,&PlaySongArea::slotVolSliderWidget);
    connect(favBtn,&QPushButton::clicked,this,&PlaySongArea::slotFav);
    //connect(playModeBtn,&QPushButton::clicked,this,&PlaySongArea::slotPlayBackModeChanged);
    connect(playModeBtn,&QToolButton::clicked,this,&PlaySongArea::slotPlayBackModeChanged);
//    connect(m_playBackModeWid->loopBtn,&QToolButton::clicked,this,&PlaySongArea::slotLoopClicked);
//    connect(m_playBackModeWid->randomBtn,&QToolButton::clicked,this,&PlaySongArea::slotRandomClicked);
//    connect(m_playBackModeWid->sequentialBtn,&QToolButton::clicked,this,&PlaySongArea::slotSequentialClicked);
//    connect(m_playBackModeWid->currentItemInLoopBtn,&QToolButton::clicked,this,&PlaySongArea::slotCurrentItemInLoopClicked);
    connect(loopAct,&QAction::triggered,this,&PlaySongArea::slotLoopClicked);
    connect(randomAct,&QAction::triggered,this,&PlaySongArea::slotRandomClicked);
    connect(currentItemInLoopAct,&QAction::triggered,this,&PlaySongArea::slotCurrentItemInLoopClicked);
    connect(&playController::getInstance(),&playController::singalChangePath,this,&PlaySongArea::slotSongInfo);
    connect(&playController::getInstance(),&playController::playerStateChange,this,&PlaySongArea::playerStateChange);
    connect(playController::getInstance().getPlayer(),SIGNAL(positionChanged(qint64)),this,SLOT(slotPositionChanged(qint64)));
    connect(playController::getInstance().getPlayer(),SIGNAL(durationChanged(qint64)),this,SLOT(slotDurationChanged(qint64)));
    connect(hSlider,SIGNAL(sliderPressed()),this,SLOT(slotSlidePressd()));
    connect(hSlider,SIGNAL(sliderReleased()),this,SLOT(slotSlideReleased()));
    connect(hSlider,&MusicSlider::valueChanged,this,&PlaySongArea::setPosition);
    connect(&playController::getInstance(),&playController::signalPlayMode,this,&PlaySongArea::setPlayMode);
    connect(&playController::getInstance(),&playController::signalNotPlaying,this,&PlaySongArea::slotNotPlaying);
    connect(&playController::getInstance(),&playController::signalSetValue,this,&PlaySongArea::slotSetValue);
    connect(&playController::getInstance(),&playController::signalVolume,this,&PlaySongArea::slotVolume);
    connect(&playController::getInstance(),&playController::signalMute,this,&PlaySongArea::slotMute);
}

void PlaySongArea::slotVolumeChanged(int values)
{
    playController::getInstance().setVolume(values);
    int volume = playController::getInstance().getVolume();
    KyInfo() << "volume = " << volume;

    if(volume == 0)
    {
        volumeBtn->setIcon(QIcon::fromTheme("audio-volume-muted-symbolic"));
    }
    else if(volume > 0 && volume <= 33)
    {
        volumeBtn->setIcon(QIcon::fromTheme("audio-volume-low-symbolic"));
    }
    else if(volume > 33 && volume <= 77)
    {
        volumeBtn->setIcon(QIcon::fromTheme("audio-volume-medium-symbolic"));
    }
    else
    {
        volumeBtn->setIcon(QIcon::fromTheme("audio-volume-high-symbolic"));
    }
}

void PlaySongArea::volumeIncrease()
{
    int value = m_volSliderWid->vSlider->value() + VOLUME;
    if(value > 100)
        value = 100;
    m_volSliderWid->vSlider->setValue(value);
}

void PlaySongArea::volumeReduce()
{
    int value = m_volSliderWid->vSlider->value() - VOLUME;
    if(value < 0)
        value = 0;
    m_volSliderWid->vSlider->setValue(value);
}

void PlaySongArea::slotVolSliderWidget()
{
    if(m_volSliderWid->isVisible())
    {
        kdk::LingmoUIStyleHelper::self()->removeHeader((QWidget *)m_volSliderWid);
        m_volSliderWid->hide();
        moveVolSliderWid();
        kdk::WindowManager::setGeometry(m_volSliderWid->windowHandle(), QRect(volumePos, m_volSliderWid->size()));
    }
    else
    {
        kdk::LingmoUIStyleHelper::self()->removeHeader((QWidget *)m_volSliderWid);
        moveVolSliderWid();
        kdk::WindowManager::setGeometry(m_volSliderWid->windowHandle(), QRect(volumePos, m_volSliderWid->size()));
        if (Global::isWayland) {
            m_volSliderWid->show();
            m_volSliderWid->raise();
            volumeBtn->clearFocus();
            m_volSliderWid->vSlider->setFocus();
            m_volSliderWid->vSlider->setVisible(true);
            volumeBtn->setProperty("checked",false);


        }
        else{
            m_volSliderWid->show();
            m_volSliderWid->raise();

        }
    }
}

void PlaySongArea::slotVolume(int volume)
{
    m_volSliderWid->vSlider->setValue(volume);
}

void PlaySongArea::slotMute(bool mute)
{
    if (mute) {
        volumeBtn->setIcon(QIcon::fromTheme("audio-volume-muted-symbolic"));
    }
}

void PlaySongArea::slotText(QString btnText)
{
    listName = btnText;
}

void PlaySongArea::slotFav()
{
    if(g_db->checkSongIsInFav(filePath))
    {
        QList<musicDataStruct> resList;
        int ref = g_db->getSongInfoListFromDB(resList, FAV);
        int ret = g_db->delMusicFromPlayList(filePath,FAV);
        if(ref == DB_OP_SUCC)
        {
//            Q_EMIT signalAddFromFavButton("我喜欢");
            //根据歌单名title值查询对应歌单列表
//            int ref = g_db->getSongInfoListFromDB(resList, "我喜欢");
            if(ret == DB_OP_SUCC)
            {
                for(int i = 0; i < resList.size(); i++)
                {
                    if(resList.at(i).filepath == filePath)
                    {
                        playController::getInstance().removeSongFromCurList(FAV, i);
                        //刷新我喜欢界面
                        if(listName == tr("I Love"))
                        {
                            Q_EMIT signalRefreshFav(FAV);
                        }
                        break;
                    }
                }
            }
        }
    }
    else
    {
        int ref = g_db->addMusicToPlayList(filePath,FAV);
        if(ref == DB_OP_SUCC)
        {
            playController::getInstance().addSongToCurList(FAV, filePath);
//            Q_EMIT signalDelFromFavButton("我喜欢");
            if(listName == tr("I Love"))
            {
                Q_EMIT signalRefreshFav(FAV);
            }
        }
//        Q_EMIT signalRefreshFav("我喜欢");
    }
    slotFavExixts();
//    if(favBtn->isVisible())
//    {
////        favBtn->setStyleSheet("QPushButton{border-image:url(:/img/clicked/love1.png);}");
//        favBtn->setIcon(QIcon(":/img/clicked/love1.png"));
//    }
//    else
//    {
//        favBtn->setIcon(QIcon(":/img/default/loveblack2.png"));
//    }

////    favBtn->setIcon(QIcon(":/img/clicked/love1.png"));
}

void PlaySongArea::slotPlayBackModeChanged()
{
    movePlayMenu();
//    if(m_playBackModeWid->isVisible())
//    {
//        m_playBackModeWid->hide();
//    }
//    else
//    {
//        movePlayModeWid();
//        m_playBackModeWid->show();
//        m_playBackModeWid->raise();
//    }
}

void PlaySongArea::slotLoopClicked()
{
    playController::getInstance().setMode(playController::Loop);
    playModeBtn->setIcon(QIcon::fromTheme("media-playlist-repeat-symbolic"));
    playModeBtn->setToolTip(tr("Loop"));
    playController::getInstance().setPlaymode(playController::Loop);
    m_playBackModeWid->hide();
}

void PlaySongArea::slotRandomClicked()
{
    playController::getInstance().setMode(playController::Random);
    playModeBtn->setIcon(QIcon::fromTheme("media-playlist-shuffle-symbolic"));
    playModeBtn->setToolTip(tr("Random"));
    playController::getInstance().setPlaymode(playController::Random);
    m_playBackModeWid->hide();
}

void PlaySongArea::slotSequentialClicked()
{
//    playModeBtn->setIcon(QIcon::fromTheme("media-playlist-repeat-symbolic"));
//    playModeBtn->setToolTip(tr("Sequential"));
//    playController::getInstance().setPlaymode(playController::Sequential);
//    m_playBackModeWid->hide();
}

void PlaySongArea::slotCurrentItemInLoopClicked()
{
    playController::getInstance().setMode(playController::CurrentItemInLoop);
    playModeBtn->setIcon(QIcon::fromTheme("media-playlist-repeat-one-symbolic"));
    playModeBtn->setToolTip(tr("CurrentItemInLoop"));
    playController::getInstance().setPlaymode(playController::CurrentItemInLoop);
    m_playBackModeWid->hide();
}

void PlaySongArea::setPlayMode(int playModel)
{
    switch (playModel) {
    case 1:
        playModeBtn->setIcon(QIcon::fromTheme("media-playlist-repeat-one-symbolic"));
        playModeBtn->setToolTip(tr("CurrentItemInLoop"));
        playController::getInstance().setPlaymode(playController::CurrentItemInLoop);
        break;
//    case 2:
//        playModeBtn->setIcon(QIcon::fromTheme("media-playlist-repeat-symbolic"));
//        playModeBtn->setToolTip(tr("Sequential"));
//        playController::getInstance().setPlaymode(playController::Sequential);
//        break;
    case 3:
        playModeBtn->setIcon(QIcon::fromTheme("media-playlist-repeat-symbolic"));
        playModeBtn->setToolTip(tr("Loop"));
        playController::getInstance().setPlaymode(playController::Loop);
        break;
    case 4:
        playModeBtn->setIcon(QIcon::fromTheme("media-playlist-shuffle-symbolic"));
        playModeBtn->setToolTip(tr("Random"));
        playController::getInstance().setPlaymode(playController::Random);
        break;
    default:
        break;
    }

    playController::getInstance().setMode(static_cast<playController::PlayMode>(playModel));
}

void PlaySongArea::slotSongInfo(QString path)
{
    musicDataStruct musicStruct;
    if(!path.isEmpty()) {
        filePath = path.remove("file://");
        g_db->getSongInfoFromDB(filePath, musicStruct);
    }
    //使用库解析总时间
    m_time = musicStruct.time;
    if(musicStruct.title == "")
    {
        playingLabel->setText(tr("Music Player"));
        coverPhotoLabel->setPixmap(QIcon(":/img/fengmian.png").pixmap(QSize(40,40)));
        Q_EMIT signalPlayingLab(tr("Music Player"));
    }
    else
    {
        hSlider->isPlaying(true);
        playingLabel->setText(musicStruct.title);
        QPixmap pix = MusicFileInformation::getInstance().getCoverPhotoPixmap(filePath);
        setCoverPhotoPixmap(pix);
        Q_EMIT signalPlayingLab(musicStruct.title);
    }
    slotFavExixts();
}

void PlaySongArea::setCoverPhotoPixmap(QPixmap pixmap)
{
    if(pixmap.isNull())
    {
        pixmap = QIcon(":/img/fengmian.png").pixmap(QSize(40,40));
    }
    else
    {
        pixmap = pixmap.scaled(QSize(40,40),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    }
    coverPhotoLabel->setPixmap(pixmap);
}

void PlaySongArea::playerStateChange(playController::PlayState newState)
{
    QString playDefaultIcon(":/img/default/media-playback-start-symbolic.svg");
    QString pauseDefaultIcon(":/img/default/media-playback-pause-symbolic.svg");

    // 设置按钮高亮色的鼠标划过透明度
    QColor highlightColor(QApplication::palette().highlight().color());
    qreal r,g,b,a;
    r = highlightColor.redF();
    g = highlightColor.greenF();
    b = highlightColor.blueF();
    a = 0.8;
    QColor color = QColor::fromRgbF(r, g, b, a);
    QString hoverColor = QString("rgba(%1, %2, %3, %4)").arg(color.red()) .arg(color.green()) .arg(color.blue()) .arg(color.alpha());

    if(newState == playController::PlayState::PLAY_STATE) {
        // 这两个图标一样大小，都是QSize(12, 16)，但不清楚 media-playback-start-symbolic.svg 会变形，因此可以通过QPixmap设置大小，而不是直接setStyleSheet设置背景
        QPixmap pixmap(pauseDefaultIcon);
        QPixmap fitpixmap = pixmap.scaled(30, 30, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        playBtn->setIcon(QIcon(fitpixmap));
        playBtn->setIconSize(QSize(30, 30));
        playBtn->setToolTip(tr("Pause"));
        playBtn->setStyleSheet(QString("QPushButton{background-color: palette(highlight); border-radius:17px;}"
                                       "QPushButton::hover{background-color: %1;}"
                                       "QPushButton::pressed{background-color: palette(highlight);}").arg(hoverColor)
                               );
    } else if((newState == playController::PlayState::PAUSED_STATE) ||(newState == playController::PlayState::STOP_STATE)) {
        QPixmap pixmap(playDefaultIcon);
        QPixmap fitpixmap = pixmap.scaled(12 ,16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        playBtn->setIcon(QIcon(fitpixmap));
        playBtn->setIconSize(QSize(12, 16));
        playBtn->setToolTip(tr("Play"));
        playBtn->setStyleSheet(QString("QPushButton{background-color: palette(highlight); border-radius:17px;}"
                                       "QPushButton::hover{background-color: %1;}"
                                       "QPushButton::pressed{background-color: palette(highlight);}").arg(hoverColor)
                               );
    }
}


void PlaySongArea::slotPositionChanged(qint64 position)
{
    QString str_time;
    int pos = position / 1000;
    int hour = static_cast<int>(pos / 3600);
    int minutes = static_cast<int>(pos % 3600 / 60);
    int seconds = static_cast<int>(pos % 60);

    QTime duration(hour, minutes, seconds);
    QStringList s = m_time.split(":");
    if(s.size() == 3) {
        str_time = duration.toString("hh:mm:ss");
    }
    else {
        str_time = duration.toString("mm:ss");
    }

    QString length = str_time + "/" + m_time;
    if(m_time == "")
    {
        timeLabel->setText("00:00/00:00");
        Q_EMIT signalTimeLab("00:00/00:00");
    }
    else
    {
        timeLabel->setText(length);
        Q_EMIT signalTimeLab(length);
    }

    if(hSlider->signalsBlocked())
    {
        return;
    }
    hSlider->blockSignals(true);
    hSlider->setValue(static_cast<int>(position));

    hSlider->blockSignals(false);
    hSlider->update();
}

void PlaySongArea::slotNotPlaying()
{
    //滑块可以点击，无法跳转
    hSlider->isPlaying(false);
    //禁用
    hSlider->setDisabled(true);
    hSlider->setValue(0);
    playingLabel->setText(tr("Music Player"));
    timeLabel->setText("00:00/00:00");
}

void PlaySongArea::slotSetValue()
{
    hSlider->setValue(0);
}

void PlaySongArea::slotDurationChanged(qint64 duration)
{
    hSlider->setRange(0,static_cast<int>(duration));
    hSlider->setEnabled(duration>0);
    hSlider->setPageStep(static_cast<int>(duration)/10);
}

void PlaySongArea::slotSlidePressd()
{
    playController::getInstance().getPlayer()->pause();
}

void PlaySongArea::slotSlideReleased()
{
    playController::getInstance().getPlayer()->play();
}

void PlaySongArea::setPosition(int position)
{
//    qDebug() << "position" << position;
//    qDebug() << "playController::getInstance().getPlayer()->position()" << playController::getInstance().getPlayer()->position();
//    qDebug() << "playController::getInstance().getPlayer()->position() - position" << playController::getInstance().getPlayer()->position() - position;
    //判断播放的位置 - 滑块的位置是否大于0.1s
    if (qAbs(playController::getInstance().getPlayer()->position() - position) > 99)
       playController::getInstance().getPlayer()->setPosition(position);
}

void PlaySongArea::resizeEvent(QResizeEvent *event)
{
    movePlayModeWid();
    moveVolSliderWid();
    playingLabel->setFixedWidth(this->width()/3.6);
    QWidget::resizeEvent(event);
}

void PlaySongArea::moveVolSliderWid()
{
    if (m_volSliderWid){
        volumePos = volumeBtn->mapToGlobal(volumeBtn->rect().center());
        // qDebug() << "移动前的vplumePos是:" << volumePos;
               QSize size = m_volSliderWid->size();
               int newPosX = volumePos.x() - size.width() / 2;
               int newPosY = volumePos.y() - size.height() - 25;

               // Move the popup to the calculated position
               m_volSliderWid->move(newPosX, newPosY);
        // qDebug() << "移动后的的vplumePos是:" << volumePos;



//    m_volSliderWid->move(volumePos);
      }
}

void PlaySongArea::movePlayModeWid()
{
    QPoint modePos = playModeBtn->mapToGlobal(playModeBtn->rect().center());
    qDebug() << "modePos " << modePos;

    m_playBackModeWid->adjustSize();
    QSize size = m_playBackModeWid->size();
    modePos.setX(modePos.x() - size.width() / 2);
    modePos.setY(modePos.y() - size.height() - 25);
    QSize modeSize = playModeBtn->size();
    int newPosX = modePos.x() + size.width() / 2 - modeSize.width() / 2 + 1;
    int newPosY = modePos.y() + 25 + size.height() - modeSize.height() / 2 + 1;
    m_playBackModeWid->changePlayModePos(newPosX, newPosY, modeSize.width(), modeSize.height());
    m_playBackModeWid->move(modePos);
}

void PlaySongArea::movePlayMenu()
{
    QPoint modePos = playModeBtn->mapToGlobal(playModeBtn->rect().center());
    QSize size = m_playMode->sizeHint();
    modePos.setX(modePos.x() - size.width() + 70);
    modePos.setY(modePos.y() - size.height() - 15);
    m_playMode->move(modePos);
    m_playMode->show();
//    m_playMode->exec(modePos);
}

void PlaySongArea::slotPlayClicked()
{
    KyInfo() << QString("%1 %2")
                .arg("slotPlayCliked: ")
                .arg(playController::getInstance().getPlayer()->state());

    if(playController::getInstance().getPlayer()->state() == MMediaPlayer::PlayingState)
    {
        playController::getInstance().getPlayer()->pause();
    }
    else if(playController::getInstance().getPlayer()->state() == MMediaPlayer::PausedState)
    {
        playController::getInstance().getPlayer()->play();

        int currentVolume = playController::getInstance().getVolume();
        KyInfo() << "currentVolume = " << currentVolume
                 << "playState = " << playController::getInstance().getPlayer()->state();

        delayMsecond(100);
        playController::getInstance().setVolume(currentVolume);
    }
    else if(playController::getInstance().getPlayer()->state() == MMediaPlayer::StoppedState)
    {
        playMeta();

        int currentVolume = playController::getInstance().getVolume();
        KyInfo() << "currentVolume = " << currentVolume
                 << "playState = " << playController::getInstance().getPlayer()->state();

        delayMsecond(100);
        playController::getInstance().setVolume(currentVolume);
    }
}

void PlaySongArea::playMeta()
{
    QString playListName = playController::getInstance().getPlayListName();
    QString playPath = playController::getInstance().getPath();
    QList<musicDataStruct> resList;
    QStringList filePaths;
    int index;
    int ret = g_db->getSongInfoListFromDB(resList, playListName);
    if(playPath != "")
    {
        if(ret == DB_OP_SUCC)
        {
            if(resList.size() == 0)
            {
                return;
            }
            for(int i = 0; i < resList.size(); i++)
            {
                filePaths << resList.at(i).filepath;
                if(resList.at(i).filepath == playPath)
                {
                    index = i;
                }
            }
            playController::getInstance().setCurPlaylist(playListName, filePaths);
            playController::getInstance().play(playListName, index);
        }
    }
    else
    {
        QList<musicDataStruct> resList;
        int ref = g_db->getSongInfoListFromDB(resList,ALLMUSIC);
        if(ref == DB_OP_SUCC)
        {
            if(resList.size() == 0)
            {
                return;
            }
            if(resList.size() >= 1)
            {
                QStringList paths;
                for(int i = 0; i < resList.size(); i++)
                {
                    paths << resList.at(i).filepath;
                }
                playController::getInstance().setCurPlaylist(ALLMUSIC,paths);
                playController::getInstance().play(ALLMUSIC,0);
            }
        }
    }
}

void PlaySongArea::slotPrevious()
{
    playController::getInstance().onPreviousSong();
}

void PlaySongArea::delayMsecond(unsigned int msec)
{
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}

void PlaySongArea::slotNext()
{
    playController::getInstance().onNextSong();
}

void PlaySongArea::listBtnClicked()
{
    Q_EMIT showHistoryListBtnClicked();
}

void PlaySongArea::slotFavExixts()
{
    if (g_db->checkSongIsInFav(filePath)) {
        favBtn->setIcon(QIcon::fromTheme("favorite-new-symbolic"));
        favBtn->setProperty("useIconHighlightEffect", 0x4);
    } else {
        favBtn->setIcon(QIcon::fromTheme("lingmo-love-symbolic"));
        favBtn->setProperty("useIconHighlightEffect", 0x2);
    }

    Q_EMIT signalFavBtnChange(filePath);
}

void PlaySongArea::slotFavExixtsDark()
{
    if(g_db->checkSongIsInFav(filePath))
    {
        favBtn->setIcon(QIcon::fromTheme("favorite-new-symbolic"));
        favBtn->setProperty("useIconHighlightEffect", 0x4);
    }
    else
    {
        favBtn->setIcon(QIcon::fromTheme("lingmo-love-symbolic"));
        favBtn->setProperty("useIconHighlightEffect", 0x2);
    }
    Q_EMIT signalFavBtnChange(filePath);
}
void PlaySongArea::slotHistoryBtnChecked(bool checked)
{
//    listBtn->setChecked(checked);
}
void PlaySongArea::slotFavIsExixts(QString filePaths)
{
    if(g_db->checkSongIsInFav(filePaths))
    {
        if(playingLabel->text() == tr("Music Player"))
        {
            return;
        }
        //用于判断添加到我喜欢的歌曲是否是当前播放的歌曲， 如果是刷新， 如果不是不做操作
        if(filePath != filePaths)
        {
            return;
        }
        favBtn->setIcon(QIcon::fromTheme("favorite-new-symbolic"));
        favBtn->setProperty("useIconHighlightEffect", 0x4);
    }
    else
    {
        //用于判断添加到我喜欢的歌曲是否是当前播放的歌曲， 如果是刷新， 如果不是不做操作
        if(filePath != filePaths)
        {
            return;
        }
        favBtn->setIcon(QIcon::fromTheme("lingmo-love-symbolic"));
        favBtn->setProperty("useIconHighlightEffect", 0x2);
    }
}

void PlaySongArea::slotFavBtnChange(QString filePath)
{
    if(g_db->checkSongIsInFav(filePath))
    {
        if(playingLabel->text() == tr("Music Player"))
        {
            return;
        }
        favBtn->setIcon(QIcon::fromTheme("favorite-new-symbolic"));
        favBtn->setProperty("useIconHighlightEffect", 0x4);
    }
    else
    {
        favBtn->setIcon(QIcon::fromTheme("lingmo-love-symbolic"));
        favBtn->setProperty("useIconHighlightEffect", 0x2);
    }
}

void PlaySongArea::playcolor()
{

    QString playDefaultIcon(":/img/default/media-playback-start-symbolic.svg");
    QString pauseDefaultIcon(":/img/default/media-playback-pause-symbolic.svg");

    QColor highlightColor(QApplication::palette().highlight().color());
    qreal r,g,b,a;
    r = highlightColor.redF();
    g = highlightColor.greenF();
    b = highlightColor.blueF();
    a = 0.8;
    QColor color = QColor::fromRgbF(r, g, b, a);
    QString hoverColor = QString("rgba(%1, %2, %3, %4)").arg(color.red()) .arg(color.green()) .arg(color.blue()) .arg(color.alpha());


    if(WidgetStyle::themeColor == 1)
    {
        this->setStyleSheet("#PlaySongArea{background-color:#252526;}");
        preBtn->setStyleSheet("QPushButton{background:transparent;border-radius:15px;border-image:url(:/img/dark/lastsong.png);}"
                              "QPushButton::hover{border-image:url(:/img/hover/lastsong.png);}"
                              "QPushButton::pressed{border-image:url(:/img/clicked/lastsong.png);}");

        if(playController::getInstance().getState() == playController::PlayState::PLAY_STATE)
        {
            QPixmap pixmap(pauseDefaultIcon);
            QPixmap fitpixmap = pixmap.scaled(30, 30, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            playBtn->setIcon(QIcon(fitpixmap));
            playBtn->setIconSize(QSize(30, 30));
            playBtn->setToolTip(tr("Pause"));
            playBtn->setStyleSheet(QString("QPushButton{background-color: palette(highlight); border-radius:17px;}"
                                           "QPushButton::hover{background-color: %1;}"
                                           "QPushButton::pressed{background-color: palette(highlight);}").arg(hoverColor)
                                   );
        }
        else if(playController::getInstance().getState() == playController::PlayState::PAUSED_STATE)
        {
            QPixmap pixmap(playDefaultIcon);
            QPixmap fitpixmap = pixmap.scaled(12 ,16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            playBtn->setIcon(QIcon(fitpixmap));
            playBtn->setIconSize(QSize(12, 16));
            playBtn->setToolTip(tr("Play"));
            playBtn->setStyleSheet(QString("QPushButton{background-color: palette(highlight); border-radius:17px;}"
                                           "QPushButton::hover{background-color: %1;}"
                                           "QPushButton::pressed{background-color: palette(highlight);}").arg(hoverColor)
                                   );
        }
        else if(playController::getInstance().getState() == playController::PlayState::STOP_STATE)
        {
            QPixmap pixmap(playDefaultIcon);
            QPixmap fitpixmap = pixmap.scaled(12 ,16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            playBtn->setIcon(QIcon(fitpixmap));
            playBtn->setIconSize(QSize(12, 16));
            playBtn->setToolTip(tr("Play"));
            playBtn->setStyleSheet(QString("QPushButton{background-color: palette(highlight); border-radius:17px;}"
                                           "QPushButton::hover{background-color: %1;}"
                                           "QPushButton::pressed{background-color: palette(highlight);}").arg(hoverColor)
                                   );
        }

        nextBtn->setStyleSheet("QPushButton{background:transparent;border-radius:15px;border-image:url(:/img/dark/nextsong.png);}"
                               "QPushButton::hover{border-image:url(:/img/hover/nextsong.png);}"
                               "QPushButton::pressed{border-image:url(:/img/clicked/nextsong.png);}");

        slotFavExixtsDark();

        coverPhotoLabel->setStyleSheet("background:transparent;border-image:url(:/img/fengmian.png);");

        playingLabel->setStyleSheet("color:#F9F9F9;line-height:14px;");

        hSlider->initStyle();
    }
    else if(WidgetStyle::themeColor == 0)
    {
        this->setStyleSheet("#PlaySongArea{background-color:#FFFFFF;}");
        preBtn->setStyleSheet("QPushButton{background:transparent;border-radius:15px;border-image:url(:/img/default/lastsong.png);}"
                              "QPushButton::hover{border-image:url(:/img/hover/lastsong.png);}"
                              "QPushButton::pressed{border-image:url(:/img/clicked/lastsong.png);}");

        if(playController::getInstance().getState() == playController::PlayState::PLAY_STATE)
        {
            QPixmap pixmap(pauseDefaultIcon);
            QPixmap fitpixmap = pixmap.scaled(30, 30, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            playBtn->setIcon(QIcon(fitpixmap));
            playBtn->setIconSize(QSize(30, 30));
            playBtn->setToolTip(tr("Pause"));
            playBtn->setStyleSheet(QString("QPushButton{background-color: palette(highlight); border-radius:17px;}"
                                           "QPushButton::hover{background-color: %1;}"
                                           "QPushButton::pressed{background-color: palette(highlight);}").arg(hoverColor)
                                   );
        }
        else if(playController::getInstance().getState() == playController::PlayState::PAUSED_STATE)
        {
            QPixmap pixmap(playDefaultIcon);
            QPixmap fitpixmap = pixmap.scaled(12 ,16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            playBtn->setIcon(QIcon(fitpixmap));
            playBtn->setIconSize(QSize(12, 16));
            playBtn->setToolTip(tr("Play"));
            playBtn->setStyleSheet(QString("QPushButton{background-color: palette(highlight); border-radius:17px;}"
                                           "QPushButton::hover{background-color: %1;}"
                                           "QPushButton::pressed{background-color: palette(highlight);}").arg(hoverColor)
                                   );
        }
        else if(playController::getInstance().getState() == playController::PlayState::STOP_STATE)
        {
            QPixmap pixmap(playDefaultIcon);
            QPixmap fitpixmap = pixmap.scaled(12 ,16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            playBtn->setIcon(QIcon(fitpixmap));
            playBtn->setIconSize(QSize(12, 16));
            playBtn->setToolTip(tr("Play"));
            playBtn->setStyleSheet(QString("QPushButton{background-color: palette(highlight); border-radius:17px;}"
                                           "QPushButton::hover{background-color: %1;}"
                                           "QPushButton::pressed{background-color: palette(highlight);}").arg(hoverColor)
                                   );
        }

        nextBtn->setStyleSheet("QPushButton{background:transparent;border-radius:15px;border-image:url(:/img/default/nextsong.png);}"
                               "QPushButton::hover{border-image:url(:/img/hover/nextsong.png);}"
                               "QPushButton::pressed{border-image:url(:/img/clicked/nextsong.png);}");

        slotFavExixts();

        coverPhotoLabel->setStyleSheet("background:transparent;border-image:url(:/img/fengmian.png);");

        playingLabel->setStyleSheet("line-height:14px;color:#303133;");

        timeLabel->setStyleSheet("line-height:12px;color:#8F9399;");

        hSlider->initStyle();

    }
}
