/*
 *
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "widget.h"

#include <QBoxLayout>
#include <QTabWidget>
#include <QSlider>
#include "ktabbar.h"

Widget::Widget(QWidget *parent)
    : KWidget(parent)
{
    QTabWidget* tabWidget = new QTabWidget(this);
    //horizontal
    QWidget *widget1 = new QWidget(this);

    QVBoxLayout* vLayout1 = new QVBoxLayout;
    QHBoxLayout* hLayout = new QHBoxLayout;

    QLabel*label1 = new QLabel(this);
    label1->setText("并列关系:");
    m_pSlider1 = new KSlider(this);     //构建一个滑动条
    m_pSlider1->setFocusPolicy(Qt::ClickFocus);
    //设置是否显示节点
//    m_pSlider1->setNodeVisible(false);
    m_pSlider1->setSliderType(KSliderType::SingleSelectSlider); //设置滑动条的类型，包含四种SmoothSlider，StepSlider,NodeSlider,SingleSelectSlider
    m_pSlider1->setRange(0,4);      //设置滑动条值的范围
    m_pSlider1->setMaximum(2);      //设置滑动条的最大值
    m_pSlider1->setValue(1);        //设置滑动条的初始值
//    m_pSlider1->setTickInterval(1);
    m_pSlider1->setFixedWidth(200);

    hLayout->addWidget(label1);
    hLayout->addWidget(m_pSlider1);
    QLabel* numLabel1 = new QLabel(this);
    numLabel1->setFixedWidth(30);
    numLabel1->setText(QString::number(m_pSlider1->value()));  //获取滑动条当前的值
    hLayout->addWidget(numLabel1);
    hLayout->addStretch();
    vLayout1->addLayout(hLayout);

    QLabel*label2 = new QLabel(this);
    label2->setText("步数关系:");
    m_pSlider2 = new KSlider(this);
    m_pSlider2->setSliderType(KSliderType::StepSlider); //设置滑动条的类型StepSlider
    m_pSlider2->setFocusPolicy(Qt::ClickFocus);
    //设置刻度间隔
    m_pSlider2->setTickInterval(10);
    //设置步长
    m_pSlider2->setSingleStep(20);
    //设置取值范围
    m_pSlider2->setRange(-50,50);
    //设置初始值
    m_pSlider2->setValue(20);
    qDebug()<<m_pSlider2->value();
    hLayout = new QHBoxLayout;
    hLayout->addWidget(label2);
    hLayout->addWidget(m_pSlider2);
    QLabel* numLabel2 = new QLabel(this);
    numLabel2->setText(QString::number(m_pSlider2->value()));
    numLabel2->setFixedWidth(30);
    hLayout->addWidget(numLabel2);
    vLayout1->addLayout(hLayout);

    QLabel*label3 = new QLabel(this);
    label3->setText("节点关系:");
    m_pSlider3 = new KSlider(this);
    m_pSlider3->setSliderType(KSliderType::NodeSlider);
    m_pSlider3->setFocusPolicy(Qt::ClickFocus);
    m_pSlider3->setTickInterval(10);
    m_pSlider3->setSingleStep(20);
    m_pSlider3->setRange(-50,50);
    m_pSlider3->setValue(20);
    hLayout = new QHBoxLayout;
    hLayout->addWidget(label3);
    hLayout->addWidget(m_pSlider3);
    QLabel* numLabel3 = new QLabel(this);
    numLabel3->setText(QString::number(m_pSlider3->value()));
    numLabel3->setFixedWidth(30);
    hLayout->addWidget(numLabel3);
    vLayout1->addLayout(hLayout);

    connect(m_pSlider1,&QSlider::valueChanged,this,[=](){
        numLabel1->setText(QString::number(m_pSlider1->value()));
    });
    connect(m_pSlider2,&KSlider::valueChanged,this,[=](){
        numLabel2->setText(QString::number(m_pSlider2->value()));
    });
    connect(m_pSlider3,&KSlider::valueChanged,this,[=](){
        numLabel3->setText(QString::number(m_pSlider3->value()));
    });

    widget1->setLayout(vLayout1);
    tabWidget->addTab(widget1,"horizontal");

    //vertical
    QWidget *widget2 = new QWidget(this);

    QHBoxLayout* hLayout2 = new QHBoxLayout;
    QVBoxLayout* vLayout2 = new QVBoxLayout;

    QLabel*label4 = new QLabel(this);
    label4->setText("非步数关系:");
    m_pSlider4 = new KSlider(Qt::Vertical,this);            //设置滑动条为垂直方向
    m_pSlider4->setSliderType(KSliderType::SmoothSlider);
    m_pSlider4->setFocusPolicy(Qt::ClickFocus);
    m_pSlider4->setTickInterval(20);
    m_pSlider4->setSingleStep(10);
    m_pSlider4->setValue(10);
    m_pSlider4->setEnabled(false);
    QLabel* numLabel4 = new QLabel(this);
    numLabel4->setFixedWidth(30);
    numLabel4->setText(QString::number(m_pSlider4->value()));
    vLayout2->addWidget(numLabel4);
    vLayout2->addWidget(m_pSlider4);
    vLayout2->addWidget(label4);
    hLayout2->addLayout(vLayout2);

    QLabel*label5 = new QLabel(this);
    label5->setText("步数关系:");
    m_pSlider5 = new KSlider(Qt::Vertical,this);
    m_pSlider5->setSliderType(KSliderType::StepSlider);
    m_pSlider5->setFocusPolicy(Qt::ClickFocus);
    m_pSlider5->setTickInterval(20);
    m_pSlider5->setSingleStep(10);
    m_pSlider5->setValue(20);
    QLabel* numLabel5 = new QLabel(this);
    numLabel5->setFixedWidth(30);
    numLabel5->setText(QString::number(m_pSlider5->value()));
    vLayout2 = new QVBoxLayout;
    vLayout2->addWidget(numLabel5);
    vLayout2->addWidget(m_pSlider5);
    vLayout2->addWidget(label5);
    hLayout2->addLayout(vLayout2);

    QLabel*label6 = new QLabel(this);
    label6->setText("节点关系:");
    m_pSlider6 = new KSlider(this);
    m_pSlider6->setSliderType(KSliderType::NodeSlider);
    m_pSlider6->setFocusPolicy(Qt::ClickFocus);
    m_pSlider6->setTickInterval(20);
    m_pSlider6->setSingleStep(10);
    m_pSlider6->setValue(30);
    m_pSlider6->setOrientation(Qt::Vertical);
    QLabel* numLabel6 = new QLabel(this);
    numLabel6->setFixedWidth(30);
    numLabel6->setText(QString::number(m_pSlider6->value()));
    vLayout2 = new QVBoxLayout;
    vLayout2->addWidget(numLabel6);
    vLayout2->addWidget(m_pSlider6);
    vLayout2->addWidget(label6);
    hLayout2->addLayout(vLayout2);

    connect(m_pSlider4,&QSlider::valueChanged,this,[=](){
        numLabel4->setText(QString::number(m_pSlider4->value()));
    });
    connect(m_pSlider5,&KSlider::valueChanged,this,[=](){
        numLabel5->setText(QString::number(m_pSlider5->value()));
    });
    connect(m_pSlider6,&KSlider::valueChanged,this,[=](){
        numLabel6->setText(QString::number(m_pSlider6->value()));
    });

    widget2->setLayout(hLayout2);
    tabWidget->addTab(widget2,"vertical");

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(tabWidget);
    this->baseBar()->setLayout(mainLayout);

}

Widget::~Widget()
{
}

