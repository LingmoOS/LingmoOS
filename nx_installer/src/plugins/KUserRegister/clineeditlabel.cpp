#include "clineeditlabel.h"
#include <QEvent>
#include <QMouseEvent>
#include <QToolTip>

namespace KInstaller {

CLineEditLabel::CLineEditLabel(QString strName, QIcon icon, QWidget *parent) : QWidget(parent)
  ,m_flag(true)
  ,leadingPng("")
  ,bltoopTip(false)
  ,w(nullptr)
  ,m_strName("")
{
    m_strName = strName;
    leadingPng = icon;
    initUI();

    this->installEventFilter(this);
}

void CLineEditLabel::initUI()
{
    layout = new QVBoxLayout;

    m_lineEdit = new QLineEdit();

    m_lineEdit->setTextMargins(10, 0, 0, 0);
    m_lineEdit->setObjectName("lineEdit");
//    m_lineEdit->setStyleSheet("padding-left: 8px;");
////    edit->setMinimumWidth(300);
////    m_lineEdit->setMinimumHeight(16);
    m_lineEdit->setPlaceholderText(m_strName);

//    QAction *edit_action = new QAction(m_lineEdit);
//    edit_action->setIcon(leadingPng);
//    edit_action->setToolTip(tr("ddddddddd"));
//    m_lineEdit->addAction(edit_action, QLineEdit::LeadingPosition);
    layout->addWidget(m_lineEdit);
}

void CLineEditLabel::initAllConnect()
{
    connect(action, &QAction::triggered, this, &CLineEditLabel::changeCodePng);
}

void CLineEditLabel::setTrailingPng(QIcon icon)
{
    action = new QAction(m_lineEdit);
    action->setIcon(icon);
    m_lineEdit->addAction(action, QLineEdit::TrailingPosition);
    initAllConnect();
}

void CLineEditLabel::changeCodePng()
{
    m_flag = !m_flag;
    if(!m_flag) {
        action->setIcon(QIcon(":/res/svg/plaincode.svg"));
        m_lineEdit->setEchoMode(QLineEdit::Normal);
    } else {
        action->setIcon(QIcon(":/res/svg/code.svg"));
        m_lineEdit->setEchoMode(QLineEdit::Password);
    }
}

void CLineEditLabel::setPlaceholderText(QString str)
{
    m_lineEdit->setPlaceholderText(str);

    //设置提示字体颜色
//    m_lineEdit->setStyleSheet("QLineEdit::placeholder{color:#cccccc;}");
    QPalette pale = m_lineEdit->palette();
    pale.setColor(QPalette::PlaceholderText, QColor("#cccccc"));
    m_lineEdit->setPalette(pale);
}

//void CLineEditLabel::mousePressEvent(QMouseEvent *event)
//{
//    if(event->button() == Qt::LeftButton) {
//        if(bltoopTip) {
//            emit signalToolTip();
//        }
//    }
//}

//bool CLineEditLabel::eventFilter(QObject *watched, QEvent *event)
//{
////    if( watched->objectName() == "lineEdit" ) {
//        if( event->type() == QEvent::MouseButtonPress ) {
//        //由于我需要传递参数，但是监控的便签有2个，所以要找到指定的qlineedit
////            QLineEdit *point = this->findChild<QLineEdit *>( watched->objectName() );
//        //处理鼠标点击事件
////            key->show();
//            if(bltoopTip) {
//                emit signalToolTip();
//            }

//            return true;
//        } else {
//            return false;
//        }
////    }
//    return QWidget::eventFilter(watched,event);
//}

}
