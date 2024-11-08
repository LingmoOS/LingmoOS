#include "messagebox.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>
#include <QBitmap>
#include <QStyleOption>
#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QEvent>
#include <QAction>
#include <QDebug>
#include <QHoverEvent>
#include <QDesktopWidget>
#include <QFile>
#include <QEvent>
#include <QTextStream>
#include "../../KUserRegister/checkedlineedit.h"
#include "../sysInfo/kpasswordcheck.h"
using namespace KInstaller;
namespace KServer {
MessageBox::MessageBox(QWidget *parent) : QDialog(parent)
  ,m_tTip(nullptr)
{
//    this->setFixedSize(520, 260);
    this->setFixedWidth(520);
    this->setMinimumHeight(164);
    this->setObjectName("MessageBox");
//    this->setWindowModality(Qt::ApplicationModal);
//    this->setAttribute(Qt::WA_ShowModal, true);
    this->setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint |
                   Qt::WindowDoesNotAcceptFocus |
                   Qt::Dialog);
    
    initUI();
    addStyleSheet();
    initAllConnect();
    translateStr();
//    this->setStyleSheet("QFrame{border-radius: 10px;}");
    qDebug()<< Q_FUNC_INFO << this->styleSheet();
}

void MessageBox::initUI()
{
    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(24, 14, 14, 24);

    this->setLayout(vlayout);
    vlayout->setSpacing(16);
    QHBoxLayout* hlayout = new QHBoxLayout;

    m_close = new QPushButton;
    m_close->setObjectName("close");
    m_close->setFlat(true);
    m_close->setIcon(QIcon(":/res/svg/close.svg"));
    m_close->setIconSize(QSize(36, 36));
    m_close->installEventFilter(this);

    hlayout->addSpacerItem(new QSpacerItem(20, 2, QSizePolicy::Expanding, QSizePolicy::Fixed));
    hlayout->addWidget(m_close);
    vlayout->addLayout(hlayout);

    m_info = new QLabel;
    m_info->setObjectName("info");
    m_info->setWordWrap(true);
    QGridLayout *hlayout1 = new QGridLayout;
    hlayout1->setHorizontalSpacing(6);
    hlayout1->setVerticalSpacing(16);
    hlayout1->setContentsMargins(0, 0, 10, 0);
    QLabel *label = new QLabel;

    //添加svg图片，并设置其大小
    label->setPixmap(QPixmap(QIcon(":/data/svg/tip.svg").pixmap(QSize(24, 24))));
    label->setFixedHeight(24);
    label->setFixedWidth(24);
    hlayout1->addWidget(label,0,0,Qt::AlignLeft | Qt::AlignTop);
    hlayout1->addWidget(m_info,0,1,Qt::AlignLeft | Qt::AlignTop);
    hlayout1->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed),0,2);
//    vlayout->addLayout(hlayout1);

    m_infoBelow = new QLabel;
    m_infoBelow->setObjectName("infoBelow");
    m_infoBelow->setWordWrap(true);
//    QHBoxLayout *hlayoutBelow = new QHBoxLayout;
//    hlayoutBelow->addSpacerItem(new QSpacerItem(24, 24, QSizePolicy::Fixed, QSizePolicy::Fixed));
    hlayout1->addWidget(m_infoBelow,1,1, Qt::AlignLeft | Qt::AlignTop);
    hlayout1->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Fixed),1,2);
//    hlayout1->addSpacerItem(new QSpacerItem(24, 4, QSizePolicy::Fixed, QSizePolicy::Expanding));
//    vlayout->addLayout(hlayoutBelow);
    vlayout->addLayout(hlayout1);

    vlayout->addStretch(2);
    QHBoxLayout *hlayout2 = new QHBoxLayout;
    hlayout2->addStretch();
    hlayout2->setContentsMargins(0, 0, 10, 0);
    hlayout2->setSpacing(20);
    m_cancel = new QPushButton;
    m_cancel->setObjectName("cancel");
    hlayout2->addWidget(m_cancel);

    m_OK = new QPushButton;
    m_OK->setObjectName("OK");
    m_OK->setDefault(true);
//    hlayout2->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed));
    hlayout2->addWidget(m_OK);
    vlayout->addLayout(hlayout2, Qt::AlignLeft);
}

void MessageBox::initAllConnect()
{
    connect(m_close, &QPushButton::clicked, [=]{execid = 1;reject();});
    connect(m_cancel, &QPushButton::clicked, [=]{ execid = 1;
        reject();
//            this->close();
    });
    connect(m_OK, &QPushButton::clicked, [=]{
        execid = 1;
        accept();
//        this->close();
    });
}

void MessageBox::addStyleSheet()
{
    QFile file(":/data/qss/MessageBox.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}

void MessageBox::translateStr()
{
    QString strLan = ReadSettingIni("config", "language");
    if("kk_KZ" == strLan || "ug_CN" == strLan || "ky_KG" == strLan){
        m_cancel->setFixedWidth(130);
        m_OK->setFixedWidth(130);
    }else{
        m_cancel->setFixedWidth(96);
        m_OK->setFixedWidth(96);
    }

//    m_title->setText(tr("Close"));
    m_cancel->setText(tr("Cancel"));
    m_OK->setText(tr("OK"));
//    m_info->setText(tr("the installer is about to exit and the computer will be shut down."));
}

void MessageBox::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}

void MessageBox::resizeEvent(QResizeEvent *event)
{
//    InstallerMainWidget::resizeEvent(event);
    // Your code here    
    QRect screenRect = QApplication::desktop()->screenGeometry();

    this->move(screenRect.width() / 2 - this->width() / 2, screenRect.height() / 2 - this->height() / 2);
}

void MessageBox::setMessageInfo(QString info)
{
    QString strInfoBelow;
    setMessageInfo(info, strInfoBelow);
}

void MessageBox::setMessageInfo(QString info, QString infoBelow)
{
    m_info->setText(info);
//    m_info->adjustSize();
    if(!infoBelow.isEmpty())
    {
        m_infoBelow->setText(infoBelow);
//        m_infoBelow->adjustSize();
    }
}

void MessageBox::setCancleHidden()
{
    m_cancel->setHidden(true);
    m_cancel->setEnabled(false);
}

void MessageBox::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;

    QPixmap pixmap = QPixmap(":/data/png/bg_message.jpg");
//    QPalette pal(this->palette());
//    pal.setBrush(backgroundRole(), QBrush(pixmap));
//    this->setAutoFillBackground(true);
//    this->setPalette(pal);


    painter.setBrush(QBrush(pixmap));
    painter.setPen(Qt::transparent);
    QRect rect = this->rect();
    rect.setWidth(rect.width() );
    rect.setHeight(rect.height() );
    painter.drawRoundedRect(rect, 6, 6);
//    也可用QPainterPath 绘制代替
//    painter.drawRoundedRect(rect, 15, 15);
    {
        QPainterPath painterPath;
        painterPath.addRoundedRect(rect, 6, 6);
        painter.drawPath(painterPath);
    }
    QDialog::paintEvent(event);
}

bool MessageBox::eventFilter(QObject *watched, QEvent *event)
{
    if(watched->objectName() == "close") {
        QHoverEvent *eve = (QHoverEvent*)event;
        if(eve->type() == QEvent::Enter) {

            if(m_tTip != nullptr) {
                m_tTip->close();
            }
            m_tTip = new ArrowWidget(true, this);
            m_tTip->setObjectName("ArrowWidget");
            m_tTip->setDerection(ArrowWidget::Derection::down);
            m_tTip->setText(tr("close"));

//            m_tTip->move(m_close->x() + (m_close->width() - m_tTip->width()) / 2, m_close->y() + m_close->height() + 4);
            m_tTip->move(x()+width()-m_close->width()-13, y()+m_close->height()+15);
//            w->move(pos.x() - 20, pos.y() - w->height());
            m_tTip->show();
            connect(m_close, &QPushButton::clicked, [=]{m_tTip->close();});

        } else if(event->type() == QEvent::Leave){
            if(m_tTip != nullptr) {
                m_tTip->close();
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

void MessageBox::keyPressEvent(QKeyEvent *event)
{
    this->setFocus();
    qDebug() << event->key() << "key pressed!";
    qDebug() << Q_FUNC_INFO << "keyPressEvent";

    if(event->key() == Qt::Key_Up)
    {

    }
    else if(event->key() == Qt::Key_Down)
    {

    }
    else if(event->key() == Qt::Key_Enter)
    {

    }
    else if(event->key() == Qt::Key_Return)
    {

    }
    else if(event->key() == Qt::Key_Backspace)
    {

    }
    else if(event->key() == Qt::Key_Escape)
    {
        execid = 1;
        reject();
    }
        QWidget::keyPressEvent(event);
}

}
