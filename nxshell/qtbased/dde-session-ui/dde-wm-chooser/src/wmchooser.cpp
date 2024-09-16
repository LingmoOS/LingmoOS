// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wmchooser.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QApplication>
#include <QPushButton>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QLabel>

WMChooser::WMChooser(QWidget *parent)
    : QWidget(parent)
{
    const QString btnStyle = "QPushButton {"
                             "background-color: rgba(255, 255, 255, 0.2);"
                             "border-radius: 6px;"
                             "color: #ffffff;"
                             "outline: none;"
                             "font-size: 16px;"
                             "font-weight: 500;"
                             "}"
                             "QPushButton:hover{"
                             "background-color: rgba(255, 255, 255, 0.3);"
                             "}"
                             "QPushButton:pressed{"
                             "background-color: rgba(255, 255, 255, 0.1);"
                             "color: #2ca7f8;"
                             "}";

    m_gorgeousBtn = new QPushButton(tr("Effect Mode"));
    m_gorgeousBtn->setFixedSize(300, 50);
    m_gorgeousBtn->setStyleSheet(btnStyle);
    m_topSpeedBtn = new QPushButton(tr("Normal Mode"));
    m_topSpeedBtn->setFixedSize(m_gorgeousBtn->size());
    m_topSpeedBtn->setStyleSheet(btnStyle);

    QLabel *titleTip = new QLabel(tr("Friendly Reminder"));
    titleTip->setStyleSheet("font-size: 40px;"
                            "color: #ffffff;");

    QLabel*tip = new QLabel(tr("It has detected that you are using a virtual machine, which will affect the system performance and operation experience, for a smooth experience, Normal Mode is recommended"));
    tip->setWordWrap(true);

    QLabel *bottomTip = new QLabel(tr("Effect Mode: Have a delicate experience. Normal Mode: Enjoy the fast performance"));
    tip->setStyleSheet("line-height: 1.43;"
                       "color: #ffffff;"
                       "font-size: 14px;");

    bottomTip->setStyleSheet(tip->styleSheet());

    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addStretch();
    mainLayout->addWidget(titleTip, 0, Qt::AlignCenter);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(tip, 0, Qt::AlignCenter);
    mainLayout->addSpacing(40);
    mainLayout->addWidget(m_gorgeousBtn, 0, Qt::AlignCenter);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_topSpeedBtn, 0, Qt::AlignCenter);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(bottomTip, 0, Qt::AlignCenter);
    mainLayout->addStretch();

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addStretch();
    layout->addLayout(mainLayout);
    layout->addStretch();

    setLayout(layout);

    connect(m_gorgeousBtn, &QPushButton::clicked, this, &WMChooser::saveConfig);
    connect(m_topSpeedBtn, &QPushButton::clicked, this, &WMChooser::saveConfig);
}

void WMChooser::createParentDir(const QString &fullfilepath)
{
    QFileInfo info(fullfilepath);
    QDir dir(info.path());
    if (!dir.exists())
       dir.mkpath(info.path());
}

void WMChooser::setConfigPath(const QString &path)
{
    m_configPath = path;
}

void WMChooser::saveConfig()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());

    createParentDir(m_configPath);
    QFile file(m_configPath);

    if (!file.open(QFile::WriteOnly))
        exit(0);

    QJsonObject obj;
    obj.insert("allow_switch", true);

    if (button == m_gorgeousBtn) {
        obj.insert("last_wm", "deepin-wm");
    } else {
        obj.insert("last_wm", "deepin-metacity");
    }

    QJsonDocument doc(obj);
    file.write(doc.toJson());
    file.close();

    qApp->quit();
}

void WMChooser::leaveEvent(QEvent *)
{
    emit screenChanged();
}
