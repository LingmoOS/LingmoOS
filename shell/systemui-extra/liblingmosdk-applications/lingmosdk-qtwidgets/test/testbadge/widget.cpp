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
#include <QSpinBox>
#include <QLabel>

Widget::Widget(QWidget *parent)
    : KWidget(parent)
{
    QVBoxLayout* vLayout = new QVBoxLayout(this);

    QHBoxLayout* hLayout = new QHBoxLayout();
    m_pContainer1 = new KPixmapContainer(this);
    m_pContainer1->setPixmap(QIcon::fromTheme("edit-find-symbolic").pixmap(16,16));
    m_pContainer1->setFontSize(8);

    m_pContainer1->setColor(QColor(255,0,0)); //设置背景色
    m_pContainer2 = new KPixmapContainer(this);
    m_pContainer2->setPixmap(QIcon::fromTheme("edit-find-symbolic").pixmap(24,24));
    m_pContainer2->setFontSize(10);
    m_pContainer2->setValue(99); //设置值
    m_pContainer2->setValueVisiable(false); //设置值不可见
    m_pContainer3 = new KPixmapContainer(this);
    m_pContainer3->setPixmap(QIcon::fromTheme("edit-find-symbolic").pixmap(36,36));
    m_pContainer3->setFontSize(10);
    m_pContainer3->setValue(999);
    m_pContainer4 = new KPixmapContainer(this);
    m_pContainer4->setPixmap(QIcon::fromTheme("edit-find-symbolic").pixmap(48,48));
    m_pContainer4->setFontSize(12);  //设置字体大小
    m_pContainer4->setValue(1000);
    hLayout->addWidget(m_pContainer1);
    hLayout->addWidget(m_pContainer2);
    hLayout->addWidget(m_pContainer3);
    hLayout->addWidget(m_pContainer4);
    vLayout->addStretch();
    vLayout->addLayout(hLayout);

    QLabel*label = new QLabel("调整badge3大小：",this);
    QSpinBox*spinBox = new QSpinBox(this);
    spinBox->setValue(m_pContainer3->fontSize());
    connect(spinBox,static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),this,
            [=](int i){m_pContainer3->setFontSize(spinBox->value());});
    hLayout = new QHBoxLayout();
    hLayout->addStretch();
    hLayout->addWidget(label);
    hLayout->addWidget(spinBox);
    hLayout->addStretch();
    vLayout->addLayout(hLayout);
    m_pBadge1 = new KBadge(this);
    m_pBadge1->setValue(0);
    m_pBadge2 = new KBadge(this);
    m_pBadge2->setValue(99);
    //m_pBadge2->setValueVisiable(false);
    m_pBadge3 = new KBadge(this);
    m_pBadge3->setValue(999);
    m_pBadge4 = new KBadge(this);
    m_pBadge4->setValue(1000);
    hLayout = new QHBoxLayout(this);
    hLayout->addWidget(m_pBadge1);
    hLayout->addWidget(m_pBadge2);
    hLayout->addWidget(m_pBadge3);
    hLayout->addWidget(m_pBadge4);
    vLayout->addLayout(hLayout);
    vLayout->addStretch();
    baseBar()->setLayout(vLayout);
}

Widget::~Widget()
{
}

