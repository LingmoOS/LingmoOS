#include "widget.h"
#include <QStringList>
#include <QStringListModel>
#include <QComboBox>
#include <QDebug>
#include <QPainter>
#include <QPaintEvent>
#include <QFont>
#include <QPen>
#include <QStyleOptionFrame>
#include <QPushButton>
#include <QVBoxLayout>
#include "kdragwidget.h"
#include <QFileDialog>
#include "ktoolbutton.h"
#include "kiconbar.h"

using namespace kdk;
Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainlayout = new QVBoxLayout(this);

    QLabel* label1 = new QLabel();
    label1->setText("添加过滤文件");
    KDragWidget *widget1 = new KDragWidget();
    widget1->iconButton()->setIcon(QIcon::fromTheme("list-add.symbolic"));
    widget1->textLabel()->setText("选择或者拖放文件夹识别路径");
    QStringList list;
    list<<"JPEG Files(*.jpg)"<<"PNG Files(*.png)"<<"PGM Files(*.pgm)";
    widget1->setNameFilters(list);

    mainlayout->addWidget(label1);
    mainlayout->addWidget(widget1);

    QLabel* label2 = new QLabel();
    label2->setText("添加所有文件");
    KDragWidget *widget2 = new KDragWidget();
    widget2->iconButton()->setIcon(QIcon::fromTheme("list-add.symbolic"));
    widget2->textLabel()->setText("选择或者拖放文件夹识别路径");

    mainlayout->addWidget(label2);
    mainlayout->addWidget(widget2);

    connect(widget1,&KDragWidget::getPath,[=](QString str){
        qDebug()<< "widget1 "<<str;
    });
    connect(widget2,&KDragWidget::getPath,[=](QString str){
        qDebug()<< "widget2 "<<str;
    });

    setMinimumSize(600,480);
}

Widget::~Widget()
{
}

