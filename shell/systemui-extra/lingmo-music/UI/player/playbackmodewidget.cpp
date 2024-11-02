#include "playbackmodewidget.h"
#include "UI/base/widgetstyle.h"

PlayBackModeWidget::PlayBackModeWidget(QWidget *parent) : QDialog(parent)
{
    installEventFilter(this);
    setAttribute(Qt::WA_StyledBackground,true);
    initWidget();
    playModecolor();
//    this->setAutoFillBackground(true);
//    this->setBackgroundRole(QPalette::Base);
}

void PlayBackModeWidget::initWidget()
{
    setFixedSize(118,90);
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setAlignment(Qt::AlignTop);
    QHBoxLayout *loopLayout = new QHBoxLayout();
    loopBtn = new QToolButton(this);
    loopBtn->setFixedSize(118,30);
    loopBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    loopBtn->setIcon(QIcon::fromTheme("media-playlist-repeat-symbolic"));
    loopBtn->setProperty("useIconHighlightEffect", 0x8);
//    loopBtn->setProperty("useIconHighlightEffect", 0x2);
//    loopBtn->setFocusPolicy(Qt::NoFocus);
//    loopBtn->setProperty("isImportant",true);
    loopBtn->setIconSize(QSize(16,16));
    loopBtn->setText(tr("Loop"));
    loopLayout->addWidget(loopBtn);
    loopLayout->setMargin(0);
    loopLayout->setSpacing(0);

    QHBoxLayout *randomLayout = new QHBoxLayout();
    randomBtn = new QToolButton(this);
    randomBtn->setFixedSize(118,30);
    randomBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    randomBtn->setIcon(QIcon::fromTheme("media-playlist-shuffle-symbolic"));
    randomBtn->setProperty("useIconHighlightEffect", 0x8);
//    randomBtn->setProperty("useIconHighlightEffect", 0x2);
//    randomBtn->setFocusPolicy(Qt::NoFocus);
//    randomBtn->setProperty("isImportant",true);
    randomBtn->setIconSize(QSize(16,16));
    randomBtn->setText(tr("Random"));
    randomLayout->addWidget(randomBtn);
    randomLayout->setMargin(0);
    randomLayout->setSpacing(0);

    QHBoxLayout *sequentialLayout = new QHBoxLayout();
    sequentialBtn = new QToolButton(this);
    sequentialBtn->hide();
    sequentialBtn->setFixedSize(118,30);
    sequentialBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    sequentialBtn->setIcon(QIcon::fromTheme("media-playlist-repeat-symbolic"));
    sequentialBtn->setProperty("useIconHighlightEffect", 0x8);
//    sequentialBtn->setProperty("useIconHighlightEffect", 0x2);
//    sequentialBtn->setFocusPolicy(Qt::NoFocus);
//    sequentialBtn->setProperty("isImportant",true);
    sequentialBtn->setIconSize(QSize(16,16));
    sequentialBtn->setText(tr("Sequential"));
    sequentialLayout->addWidget(sequentialBtn);
    sequentialLayout->setMargin(0);
    sequentialLayout->setSpacing(0);

    QHBoxLayout *currentItemInLoopLayout = new QHBoxLayout();
    currentItemInLoopBtn = new QToolButton(this);
    currentItemInLoopBtn->setFixedSize(118,30);
    currentItemInLoopBtn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    currentItemInLoopBtn->setIcon(QIcon::fromTheme("media-playlist-repeat-one-symbolic"));
    currentItemInLoopBtn->setProperty("useIconHighlightEffect", 0x8);
//    currentItemInLoopBtn->setProperty("useIconHighlightEffect", 0x2);
//    currentItemInLoopBtn->setFocusPolicy(Qt::NoFocus);
//    currentItemInLoopBtn->setProperty("isImportant",true);
    currentItemInLoopBtn->setIconSize(QSize(16,16));
    currentItemInLoopBtn->setText(tr("CurrentItemInLoop"));
    currentItemInLoopLayout->addWidget(currentItemInLoopBtn);
    currentItemInLoopLayout->setMargin(0);
    currentItemInLoopLayout->setSpacing(0);

    vLayout->addLayout(loopLayout,Qt::AlignTop);
    vLayout->addLayout(randomLayout,Qt::AlignVCenter);
//    vLayout->addLayout(sequentialLayout,Qt::AlignVCenter);
    vLayout->addLayout(currentItemInLoopLayout,Qt::AlignBottom);
    vLayout->setMargin(0);
    vLayout->setSpacing(0);
    this->setLayout(vLayout);

    //限制应用字体不随着主题变化
//    QFont sizeFont;
//    sizeFont.setPixelSize(14);
//    loopBtn->setFont(sizeFont);
//    randomBtn->setFont(sizeFont);
//    sequentialBtn->setFont(sizeFont);
//    currentItemInLoopBtn->setFont(sizeFont);

}

bool PlayBackModeWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
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
            QRect rect(playModePosX, playModePosY, playModePosWidth, playModePosHeight);
            if(rect.contains(QCursor::pos(), false))
            {
                return 0;
            }
            else
            {
                this->hide();
                break;
            }
    }
    return false;
}

void PlayBackModeWidget::changePlayModePos(int posX, int posY, int width, int height)
{
    playModePosX = posX;
    playModePosY = posY;
    playModePosWidth = width;
    playModePosHeight = height;
}

void PlayBackModeWidget::playModecolor()
{
//
//    loopBtn->setAutoRaise(true);
//    randomBtn->setAutoRaise(true);
//    sequentialBtn->setAutoRaise(true);
//    currentItemInLoopBtn->setAutoRaise(true);

//    if(WidgetStyle::themeColor == 1)
//    {
//        loopBtn->setStyleSheet("QToolButton{padding-left:7px;background-color:#1F2022;color:#F9F9F9;border-radius:4px;}"
//                               "QToolButton::hover{background-color:#303032;border-radius:4px;}");
//        randomBtn->setStyleSheet("QToolButton{padding-left:7px;background-color:#1F2022;color:#F9F9F9;border-radius:4px;}"
//                                 "QToolButton::hover{background-color:#303032;border-radius:4px;}");
//        sequentialBtn->setStyleSheet("QToolButton{padding-left:7px;background-color:#1F2022;color:#F9F9F9;border-radius:4px;}"
//                                     "QToolButton::hover{background-color:#303032;border-radius:4px;}");
//        currentItemInLoopBtn->setStyleSheet("QToolButton{padding-left:7px;background-color:#1F2022;color:#F9F9F9;border-radius:4px;}"
//                                            "QToolButton::hover{background-color:#303032;border-radius:4px;}");
//    }
//    else if(WidgetStyle::themeColor == 0)
//    {
//        loopBtn->setStyleSheet("QToolButton{padding-left:7px;\
//                               background-color:#FFFFFF;color:#303133;border-radius:4px;}"
//                               "QToolButton::hover{background-color:#EEEEEE;border-radius:4px;}");
//        randomBtn->setStyleSheet("QToolButton{padding-left:7px;\
//                                 background-color:#FFFFFF;color:#303133;border-radius:4px;}"
//                                 "QToolButton::hover{background-color:#EEEEEE;border-radius:4px;}");
//        sequentialBtn->setStyleSheet("QToolButton{padding-left:7px;\
//                                     background-color:#FFFFFF;color:#303133;border-radius:4px;}"
//                                     "QToolButton::hover{background-color:#EEEEEE;border-radius:4px;}");
//        currentItemInLoopBtn->setStyleSheet("QToolButton{padding-left:7px;\
//                                            background-color:#FFFFFF;color:#303133;border-radius:4px;}"
//                                            "QToolButton::hover{background-color:#EEEEEE;border-radius:4px;}");
//    }
    if(WidgetStyle::themeColor == 1)
    {
        this->setStyleSheet("background-color:#252526;");
    }
    else if(WidgetStyle::themeColor == 0)
    {
        this->setStyleSheet("background-color:#FFFFFF;");
    }
}




