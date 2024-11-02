#include "custommessagebox.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVariant>
#include <QDebug>
#include <QDialog>
#include <QPainterPath>
#include <QPainter>
#include <KWindowEffects>
#include <KWindowSystem>
#include <QDesktopWidget>
#include <QApplication>
#include <QGuiApplication>
#include <QScreen>
//#include "kborderlessbutton.h"
#include "windowmanager/windowmanager.h"
#include <QGraphicsDropShadowEffect>
#include <QGSettings>
#include <QToolButton>
#define SHADOW_WIDTH 15    // 窗口阴影宽度;
#define TRIANGLE_WIDTH 15  // 小三角的宽度;
#define TRIANGLE_HEIGHT 10 // 小三角的高度;
#define BORDER_RADIUS 5    // 窗口边角的弧度;
#define THEME_SCHAME "org.lingmo.style"
#define COLOR_THEME "styleName"
CustomMessageBox::CustomMessageBox(int type,QWidget *parent) : QDialog(parent) {
    m_type = type;

    QString buttonStyleSheet = "QPushButton {"
                                   "    background-color: #E6E6E6;"
                                  "    color: black;"
                                   "}";
    const QByteArray style_id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(style_id)) {
       styleGsettings = new QGSettings(style_id, QByteArray(), this);
       QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
       if(currentTheme == "lingmo-dark"){
           buttonStyleSheet = "QPushButton {"
                                          "   background-color: #333333;"
                                         "    color: white;"
                                          "}";
       }
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog); //加个QDialog
    QVBoxLayout *layout = new QVBoxLayout(this);
    QHBoxLayout *titlelayout = new QHBoxLayout();

    QLabel *m_appPicture = new QLabel(this); //窗体左上角标题图片
    QIcon appIcon = QIcon::fromTheme("lingmo-calendar");
    QPixmap apppixmap = appIcon.pixmap(QSize(25,25));
    m_appPicture->setPixmap(apppixmap);
    m_appPicture->setStyleSheet("QPushButton{background:transparent;}");
    m_appPicture->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_appPicture->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    titlelayout->addWidget(m_appPicture);
    titlelayout->addSpacing(2);

    QLabel *titleLabel = new QLabel(tr("Schedule"), this); //日历
    titlelayout->addWidget(titleLabel);


    //关闭按钮
    QToolButton *m_closeButton = new QToolButton(this);
    m_closeButton->setFixedSize(30, 30);
    m_closeButton->setToolTip(tr("Close"));
    m_closeButton->setIcon(QIcon::fromTheme("window-close-symbolic")); //主题库的叉子图标
    m_closeButton->setProperty("isWindowButton", 0x2);
    m_closeButton->setProperty("useIconHighlightEffect", 0x8);
    m_closeButton->setAutoRaise(true);
    connect(m_closeButton, &QToolButton::clicked, [=]() {
        this->deleteLater();
    });
    titlelayout->addWidget(m_closeButton);
    layout->addLayout(titlelayout);

     QLabel *warning = new QLabel(this);
     QIcon icon = QIcon::fromTheme("dialog-warning");
     QPixmap pixmap = icon.pixmap(QSize(25,25));
     warning->setPixmap(pixmap);
    QHBoxLayout *summury = new QHBoxLayout(this);
//    QString text = getText();

    if(type == 1){
    textLabel = new QLabel(tr("Are you sure you want to delete this event?"), this);
    textLabel->setWordWrap(true); // 启用自动换行
    textLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding); // 设置尺寸策略
    }
    else if(type == 0){
    textLabel = new QLabel(tr("The end time cannot be earlier than the start time, please re-enter it!"),this);
    textLabel->setWordWrap(true); // 启用自动换行
    textLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding); // 设置尺寸策略
    }
    else if(type ==2){
       textLabel = new QLabel( tr("The time entered is not in the correct format, please make sure that the hours are between 0-23 and the minutes are between 0-59."),this);
       textLabel->setWordWrap(true); // 启用自动换行
       textLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding); // 设置尺寸策略
    }
    else if(type ==3){
       textLabel = new QLabel(tr("text is empty."),this);
       textLabel->setWordWrap(true); // 启用自动换行
       textLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding); // 设置尺寸策略
    }
    warning->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    warning->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    summury->addSpacing(20);
//    summury->addStretch(10);
    summury->addWidget(warning);
    summury->addSpacing(2);
    summury->addWidget(textLabel);
//    summury->setContentsMargins(20,0,0,0);

    layout->addLayout(summury);
    QHBoxLayout *buttonLayout = new QHBoxLayout(this);
    QPushButton *yesButton = new QPushButton(tr("Yes"), this);
    yesButton->setFixedSize(100,30);
    buttonLayout->addStretch(5);
    buttonLayout->addWidget(yesButton,0,Qt::AlignRight| Qt::AlignBottom);
    if(type ==1){
        QPushButton *noButton = new QPushButton(tr("No"), this);
        noButton->setFixedSize(100,30);
    buttonLayout->addWidget(noButton,0,Qt::AlignRight| Qt::AlignBottom);
      connect(noButton, &QPushButton::clicked, this, &CustomMessageBox::noClicked);
       noButton->setStyleSheet(buttonStyleSheet);
    }

    buttonLayout->setContentsMargins(0,0,20,20);
    QWidget *buttonWidget = new QWidget();
    buttonWidget->setLayout(buttonLayout);
    layout->addWidget(buttonWidget);
    connect(yesButton, &QPushButton::clicked, this, &CustomMessageBox::yesClicked);


    if (styleGsettings!=nullptr) {
       QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
       if(currentTheme == "lingmo-default"){
           this->setStyleSheet("background-color:#F0F0F0;color:black;");
           yesButton->setStyleSheet(buttonStyleSheet);


      }
    setLayout(layout);
    setFixedSize(480,200);
    if(type == 2){
    setFixedSize(480,270);
    }
}
    }
}
//以上代码还没有生效，还需要继续分析，因为显卡问题只有支持OpenGL才能有圆角，除非自己画一个圆角，先写上以后补充
void CustomMessageBox::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
//    painter.setBrush(QColor(255, 255, 255));
//    if(isDark){
//        painter.setBrush(QColor(0, 0, 0));
//    }

    QPainterPath drawPath;
    drawPath.addRoundedRect(rect(),BORDER_RADIUS,BORDER_RADIUS);

    painter.drawPath(drawPath);
}
QString CustomMessageBox::getText()
{
    return m_text;
}
void CustomMessageBox::SetText(QString text)
{
    qDebug()<<"text:"<<text;
    m_text = text;
}
CustomMessageBox::~CustomMessageBox()
{
  if(styleGsettings){
      delete styleGsettings;
      styleGsettings = nullptr;
  }
}
void CustomMessageBox::showEvent(QShowEvent *event)
{
    QRect rect;
    QWidget::showEvent(event);
    QPoint screenCenter = QApplication::desktop()->availableGeometry().center();
//    QSize dialogSize = msgBox->size();
    rect.setRect(screenCenter.x() - this->width() / 2,screenCenter.y() - this->height() / 2,this->width(),this->height());
    kdk::WindowManager::setGeometry(this->windowHandle(),rect);


}
