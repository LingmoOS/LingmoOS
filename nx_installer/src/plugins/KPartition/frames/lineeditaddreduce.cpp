#include "lineeditaddreduce.h"
#include <QHBoxLayout>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QRegExpValidator>
namespace KInstaller {

LineEditAddReduce::LineEditAddReduce(QWidget *parent):QWidget(parent)
{
    initUI();
    initAllConnect();
}
LineEditAddReduce::~LineEditAddReduce()
{}


void LineEditAddReduce::setValue(QString sizestr)
{
    m_sizeStr = sizestr;

    sizevalue = m_sizeStr.toULong();
    maxSizevalue = m_sizeStr.toULong();
    QRegExp regx("[0-9]+$");
    QValidator *validator = new QRegExpValidator(regx, this );
    m_ledit->setValidator( validator );
    m_ledit->setText(sizestr);
}

qint64 LineEditAddReduce::getValue()
{
    return sizevalue;
}

void LineEditAddReduce::initUI()
{
    QHBoxLayout *hlayout = new QHBoxLayout();
    this->setObjectName("sizeEdit");
    hlayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(hlayout);
    hlayout->setSpacing(2);
    sizeAddBtn = new QPushButton();

    sizeAddBtn->setIcon(QIcon(":/res/pic/add.svg"));
    sizeAddBtn->setFlat(true);
    sizeAddBtn->setObjectName("sizeAddBtn");
    sizeAddBtn->setEnabled(false);

    m_ledit = new QLineEdit;
    m_ledit->setObjectName("ledit");

    sizeReduceBtn = new QPushButton();
    sizeReduceBtn->setIcon(QIcon(":/res/pic/delete.svg"));
    sizeReduceBtn->setFlat(true);
    sizeAddBtn->setCursor(QCursor(Qt::ArrowCursor));
    sizeReduceBtn->setCursor(QCursor(Qt::ArrowCursor));
    sizeReduceBtn->setObjectName("sizeReduceBtn");



    hlayout->addWidget(sizeReduceBtn, Qt::AlignLeft);
    hlayout->addWidget(m_ledit, Qt::AlignCenter);
    m_ledit->setAlignment(Qt::AlignHCenter);
    hlayout->addWidget(sizeAddBtn, Qt::AlignRight);

}
void LineEditAddReduce::initAllConnect()
{
//    connect(this, SIGNAL(signalAdd), this, SLOT(clickAddBtn));
//    connect(this, SIGNAL(signalReduce), this, SLOT(clickReduceBtn));

    connect(sizeAddBtn, &QPushButton::clicked, this, &LineEditAddReduce::clickAddBtn);
    connect(sizeReduceBtn, &QPushButton::clicked, this, &LineEditAddReduce::clickReduceBtn);
    connect(m_ledit, &QLineEdit::textChanged, this, &LineEditAddReduce::getTextChanged);
}
void LineEditAddReduce::addStyleSheet()
{

}

void LineEditAddReduce::getTextChanged()
{
    sizevalue = m_ledit->text().toULong();
    if(sizevalue < maxSizevalue) {
        sizeReduceBtn->setEnabled(true);
        sizeAddBtn->setEnabled(true);
    } else if(sizevalue == 0) {
        sizeReduceBtn->setEnabled(false);
        sizeAddBtn->setEnabled(true);
    } else if(sizevalue >= maxSizevalue) {
        m_ledit->setText(QString("%1").arg(maxSizevalue));
        sizeReduceBtn->setEnabled(true);
        sizeAddBtn->setEnabled(false);
    }
}

void LineEditAddReduce::clickAddBtn()
{
    qDebug() << "clickAddBtn:" << m_ledit->text();
    sizevalue = m_ledit->text().toULong();
    if(sizevalue < maxSizevalue) {
        sizeReduceBtn->setEnabled(true);
        sizeAddBtn->setEnabled(true);
        sizevalue += 1;
    } else {
        sizeReduceBtn->setEnabled(true);
        sizeAddBtn->setEnabled(false);
    }
    m_sizeStr = QString::number(sizevalue);
    m_ledit->setText(m_sizeStr);
}
void LineEditAddReduce::clickReduceBtn()
{
    qDebug() << "clickReduceBtn:" << m_ledit->text();
    sizevalue = m_ledit->text().toLong();
    if(sizevalue == 0) {
        sizeReduceBtn->setEnabled(false);
        sizeAddBtn->setEnabled(true);
    } else {
        sizeReduceBtn->setEnabled(true);
        sizeAddBtn->setEnabled(true);
        sizevalue -= 1;
    }
    m_sizeStr = QString::number(sizevalue);
    m_ledit->setText(m_sizeStr);
}
bool LineEditAddReduce::eventFilter(QObject *object, QEvent *event)
{
    if(object->objectName() == "sizeReduceBtn") {
        if(event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseevent = (QMouseEvent*)event;
            if(mouseevent->button() == Qt::LeftButton) {
                emit signalReduce();
                return true;
            }
        }
    }
    if(object->objectName() == "sizeAddBtn") {
        if(event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseevent = (QMouseEvent*)event;
            if(mouseevent->button() == Qt::LeftButton) {
                emit signalAdd();
                return true;
            }
        }
    }
    return QWidget::eventFilter(object,event);
}

}
