#include "labedit.h"
#include <QRegExpValidator>

LabEdit::LabEdit()
{
    init();
}

void LabEdit::init()
{
    QHBoxLayout* mainLayout = new QHBoxLayout();
    lab = new QLabel(this);
    lab->setText(QString("%1").arg(leng));
    mainLayout->addWidget(lab);
    mainLayout->setAlignment(Qt::AlignRight);
    this->setLayout(mainLayout);

//    QRegExp rx = QRegExp("[^\\\\/:*?\"&<>|]*"); //限制以下特殊符号在lineEdit中的输入
//    QRegExpValidator* validator = new QRegExpValidator(rx);
//    this->setValidator(validator);
}

void LabEdit::setLabelNumber(int num)
{
    lab->setText(QString("%1").arg(num));

    if (num == 0) {
        lab->setStyleSheet("QLabel{color:#F44E50;}");
    } else {
        lab->setStyleSheet("QLabel{color:#000000;}");
    }
}

void LabEdit::slotLableSetFontSize(int size)
{
    //默认大小12px,换算成pt为9
    double lableBaseFontSize = PT_11;//魔鬼数字，自行处理
    double nowFontSize = lableBaseFontSize * double(size) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    font.setPointSizeF(nowFontSize);
    lab->setFont(font);
}
