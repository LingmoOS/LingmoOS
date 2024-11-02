#include "custom_push_button_group.h"
#include "ui_custom_push_button_group.h"
#include <QFrame>
#include <QFile>
#include <QDebug>

QString CustomPushButtonGroup::m_buttonGroupQSS = QString();

CustomPushButtonGroup::CustomPushButtonGroup(QWidget *parent)
    : QFrame(parent)
    , ui(new Ui::CustomPushButtonGroup)
{
    loadQSS();
    ui->setupUi(this);
    setProperty("state", "background");
    setStyleSheet(m_buttonGroupQSS);
}

CustomPushButtonGroup::~CustomPushButtonGroup()
{
    delete ui;
}

void CustomPushButtonGroup::addButton(QPushButton *button, BtnState state)
{
    button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    switch (state)
    {
    case (BtnState::Left) :
        button->setProperty("state", "left");
        break;
    case (BtnState::Right) :
        addSeparator();
        button->setProperty("state", "right");
        break;
    case (BtnState::Mid) :
        addSeparator();
        button->setProperty("state", "mid");
        break;
    default:
        break;
    }
    ui->buttonLayout->addWidget(button);
    button->setStyleSheet(m_buttonGroupQSS);
}



void CustomPushButtonGroup::addSeparator()
{
    QFrame *frame = new QFrame(this);
    frame->setStyleSheet("QFrame{background-color: palette(Button);}");
    frame->setFixedSize(QSize(1, 21));
    frame->setFrameShape(QFrame::VLine);
    frame->setFrameShadow(QFrame::Raised);

    ui->buttonLayout->addWidget(frame);
}

void CustomPushButtonGroup::loadQSS()
{
    if (!m_buttonGroupQSS.isEmpty()) {
        return;
    }
    QFile file(":/custom_ui/custom_push_button_group.qss");
	file.open(QFile::ReadOnly);

	m_buttonGroupQSS = file.readAll();

    file.close();
}

void CustomPushButtonGroup::frameUpdate(bool isClassical)
{
    setProperty("state", "background");
    setProperty("isClassical", isClassical);
    setStyleSheet(m_buttonGroupQSS);
}
