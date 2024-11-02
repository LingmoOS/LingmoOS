#include "encryptsetframe.h"

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
#include "../../PluginService/sysInfo/kpasswordcheck.h"
using namespace KInstaller;
namespace KServer {
EncryptSetFrame::EncryptSetFrame(QDialog *parent) : QDialog(parent)
  ,m_tTip(nullptr)
  ,pwdIsOK(false)
  ,cpwdIsOK(false)
{
    this->setFixedSize(520, 340);
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
    KServer::KPasswordCheck::init();
    translateStr();
//    this->setStyleSheet("QFrame{border-radius: 10px;}");
    qDebug()<< Q_FUNC_INFO << this->styleSheet();

}

void EncryptSetFrame::initUI()
{
    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->setContentsMargins(24, 14, 14, 14);

    this->setLayout(vlayout);
    vlayout->setSpacing(4);
    QHBoxLayout* hlayout = new QHBoxLayout;


    m_close = new QPushButton;
    m_close->setObjectName("close");
    m_close->setFlat(true);
    m_close->setIcon(QIcon(":/res/svg/close.svg"));
    m_close->setIconSize(QSize(36, 36));
    m_close->installEventFilter(this);

    hlayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Expanding));
    hlayout->addWidget(m_close);
    vlayout->addLayout(hlayout);


    QLabel* pwdLabel = new QLabel(tr("password:"));
    QLabel* pwdLabel1 = new QLabel(tr("confirm password:"));
    m_pwd = new CLineEditLabel("", QIcon(""), this);
    m_pwd1 = new CLineEditLabel("", QIcon(""), this);
    m_pwd->m_lineEdit->setEchoMode(QLineEdit::Password);
    m_pwd1->m_lineEdit->setEchoMode(QLineEdit::Password);
    m_pwd->setTrailingPng(QIcon(":/res/svg/code.svg"));
    m_pwd1->setTrailingPng(QIcon(":/res/svg/code.svg"));

    checkpwd = new QLabel;
    checkpwd1 = new QLabel;
    checkpwd->setObjectName("checkpng");
    checkpwd1->setObjectName("checkpng");

    QGridLayout* glay = new QGridLayout;
    glay->setColumnStretch(0, 3);
    glay->setColumnStretch(1, 3);
    glay->setColumnStretch(2, 1);
    glay->setColumnStretch(3, 1);
    glay->setSpacing(5);
    glay->setContentsMargins(10, 10, 10, 10);
    glay->addWidget(pwdLabel, 0, 0, 1, 1, Qt::AlignLeft);
    glay->addLayout(m_pwd->layout, 0, 1, 1, 1,Qt::AlignLeft | Qt::AlignTop);
    glay->addItem(new QSpacerItem(24, 24, QSizePolicy::Fixed, QSizePolicy::Preferred), 1, 1, 1, 1);
    glay->addWidget(pwdLabel1, 2, 0, 1, 1,Qt::AlignLeft);
    glay->addLayout(m_pwd1->layout, 2, 1, 1, 1,Qt::AlignLeft | Qt::AlignTop);

    glay->addWidget(checkpwd, 0, 2, 1, 1, Qt::AlignLeft);
    glay->addWidget(checkpwd1, 2, 2, 1, 1, Qt::AlignLeft);

    m_info = new QLabel;
    glay->addWidget(m_info, 3, 1, 1, 3);

    QLabel* tip = new QLabel;
    glay->addItem(new QSpacerItem(34, 34, QSizePolicy::Fixed, QSizePolicy::Preferred), 4, 1, 1, 1);
    tip->setText(tr("Please keep your password properly.If you forget it,\n"
                    "you will not be able to access the disk data."));//请妥善保管您的密码，如忘记密码，将无法访问磁盘数据.
    QHBoxLayout *hlayout1 = new QHBoxLayout;
    QLabel *label = new QLabel;

    //添加svg图片，并设置其大小
    label->setPixmap(QPixmap(QIcon(":/res/svg/finger.png").pixmap(QSize(24, 24))));
    hlayout1->addWidget(label);
    hlayout1->addWidget(tip, Qt::AlignLeft | Qt::AlignTop);
    hlayout1->addStretch();
//        hlayout1->addSpacerItem(new QSpacerItem(24, 4, QSizePolicy::Fixed, QSizePolicy::Expanding));
    glay->addLayout(hlayout1, 5, 0, 1, 4);

    vlayout->addLayout(glay);
    vlayout->addStretch(10);

    QHBoxLayout *hlayout2 = new QHBoxLayout;
    hlayout2->addStretch();
    hlayout2->setContentsMargins(10, 10, 10, 10);
    hlayout2->setSpacing(30);
    m_cancel = new QPushButton;
    m_cancel->setObjectName("cancel");
    hlayout2->addWidget(m_cancel);


    m_OK = new QPushButton;
    m_OK->setObjectName("OK");
    m_OK->setDefault(true);
    hlayout2->addWidget(m_OK);
    vlayout->addLayout(hlayout2, Qt::AlignLeft);
    m_OK->setDisabled(true);

}

void EncryptSetFrame::initAllConnect()
{
    connect(m_close, &QPushButton::clicked, [=]{execid = 1;reject();});
    connect(m_cancel, &QPushButton::clicked, [=]{ execid = 1;
        reject();
    });
    connect(m_OK, &QPushButton::clicked, [=]{
        if(pwdIsOK & cpwdIsOK) {
            execid = 1;
            accept();
        }
    });

    QObject::connect(m_pwd->m_lineEdit, &QLineEdit::textChanged, this, &EncryptSetFrame::checkPasswd);
    QObject::connect(m_pwd1->m_lineEdit, &QLineEdit::textChanged, this, &EncryptSetFrame::checkPasswd);
    connect(this, &EncryptSetFrame::signalConfirmPasswd, [=]{
        QString pwd = m_pwd->getLineEditText();
        QString cpwd = m_pwd1->getLineEditText();
        if(!cpwd.isEmpty())
          checkCPasswd(cpwd);
    });
}


void EncryptSetFrame::addStyleSheet()
{
    QFile file(":/data/qss/MessageBox.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}


void EncryptSetFrame::translateStr()
{
    m_cancel->setText(tr("Cancel"));
    m_OK->setText(tr("OK"));
    markLineEditCorrect(m_LabelState, m_flag);
}
void EncryptSetFrame::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}

void EncryptSetFrame::resizeEvent(QResizeEvent *event)
{
//    InstallerMainWidget::resizeEvent(event);
    // Your code here
    QRect screenRect = QApplication::desktop()->screenGeometry();

    this->move(screenRect.width() / 2 - this->width() / 2, screenRect.height() / 2 - this->height() / 2);
}

void EncryptSetFrame::setMessageInfo(QString info)
{
    m_info->setText(info);
    m_info->adjustSize();
}
void EncryptSetFrame::setCancleHidden()
{
    m_cancel->setHidden(true);
    m_cancel->setEnabled(false);
}

void EncryptSetFrame::paintEvent(QPaintEvent *event)
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

bool EncryptSetFrame::eventFilter(QObject *watched, QEvent *event)
{
    if(watched->objectName() == "close") {
        QHoverEvent *eve = (QHoverEvent*)event;
        if(eve->type() == QEvent::Enter) {

            if(m_OK->isEnabled()==false)
                return false;

            if(m_tTip != nullptr) {
                m_tTip->close();
            }
            m_tTip = new ArrowWidget(true, this);
            m_tTip->setObjectName("ArrowWidget");
            m_tTip->setDerection(ArrowWidget::Derection::down);
            m_tTip->setText(tr("close"));

            m_tTip->move(m_close->x() + (m_close->width() - m_tTip->width()) / 2, m_close->y() + m_close->height() + 4);
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


void EncryptSetFrame::markLineEditCorrect(LabelState state, bool flag)
{


    switch (state) {
    case LabelState::Pwd:
        if(flag) {
            checkpwd->setPixmap(QPixmap(":/res/svg/correct.svg"));
            if(pwdIsOK==false) checkpwd->setVisible(false);
            else checkpwd->setVisible(true);
            m_pwd->m_lineEdit->setStyleSheet("QLineEdit{border: 0px solid #5BADFF; }"
                             "QLineEdit:hover { background: rgba(0, 0, 0, 0.15);border: 2px solid #5BADFF;}");
            if(cpwdIsOK==true)
            m_info->setText("");

        } else {
            checkpwd->setVisible(true);
            checkpwd->setPixmap(QPixmap(":/res/svg/error.svg"));
            m_pwd->m_lineEdit->setStyleSheet("QLineEdit{border-color: red }"
                             "QLineEdit:hover { background: rgba(0, 0, 0, 0.15);border: 2px solid #5BADFF;}");
            m_info->setText(errstr);
        }
        break;
    case LabelState::CPwd:
        if(flag) {
            checkpwd1->setPixmap(QPixmap(":/res/svg/correct.svg"));
            if(cpwdIsOK==false)checkpwd1->setVisible(false);
            else checkpwd1->setVisible(true);
            m_pwd1->m_lineEdit->setStyleSheet("QLineEdit{border: 0px solid #5BADFF; }"
                             "QLineEdit:hover { background: rgba(0, 0, 0, 0.15);border: 2px solid #5BADFF;}");
           if(pwdIsOK==true)
            m_info->setText("");
        } else {
            checkpwd1->setVisible(true);
            checkpwd1->setPixmap(QPixmap(":/res/svg/error.svg"));
            m_pwd1->m_lineEdit->setStyleSheet("QLineEdit{border-color: red }"
                             "QLineEdit:hover { background: rgba(0, 0, 0, 0.15);border: 2px solid #5BADFF;}");
            m_info->setText(QObject::tr("Two password entries are inconsistent!"));

        }
        break;
    default:

        break;
    }

    m_OK->setEnabled(pwdIsOK & cpwdIsOK);
}


void EncryptSetFrame::checkPasswd(QString pwd)
{
//    w->show();
    m_LabelState = LabelState::Pwd;
    pwd=m_pwd->getLineEditText();
    m_state = KCheckPassWD(pwd, errstr, "");
    if(m_state == CheckState::OK) {
        pwdIsOK = true;
        m_flag = true;
    }
    else if(pwd.isEmpty())
    {
       pwdIsOK = false;
       m_flag = true;
    }
    else {
        pwdIsOK = false;
        m_flag = false;
  //
    }
    translateStr();
    emit signalConfirmPasswd();
}

void EncryptSetFrame::checkCPasswd(QString cpwd)
{
    QString pwd = m_pwd->getLineEditText();
    m_LabelState = LabelState::CPwd;

    if(cpwd.isEmpty()==true){
        cpwdIsOK = false;
        m_flag = true;//不需要显示×
    }
    else if(cpwd.compare(pwd)==0)
    {
        cpwdIsOK = true;
        m_flag = true;


    }
    else
    {
        cpwdIsOK = false;
        m_flag = false;

    }
    translateStr();

}

}

