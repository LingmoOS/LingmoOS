// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loadiconsworker.h"
#include "common/vnotedatamanager.h"

#include <QPixmap>
#include <QPainter>

QVector<QPixmap> VNoteDataManager::m_defaultIcons[IconsType::MaxIconsType];
QReadWriteLock VNoteDataManager::m_iconLock;

/**
 * @brief LoadIconsWorker::LoadIconsWorker
 * @param parent
 */
LoadIconsWorker::LoadIconsWorker(QObject *parent)
    : VNTask(parent)
{
    //Hold the lock at constructor, this can void
    //other thread acess the icons before that being
    //loaded.
    VNoteDataManager::m_iconLock.lockForWrite();
}

/**
 * @brief LoadIconsWorker::greyPix
 * @param pix
 * @return 置灰后的图标
 */
QPixmap LoadIconsWorker::greyPix(QPixmap pix)
{
    QPixmap temp(pix.size());
    temp.fill(Qt::transparent);

    QPainter iconPainer(&temp);
    iconPainer.setCompositionMode(QPainter::CompositionMode_Source);
    iconPainer.drawPixmap(0, 0, pix);
    iconPainer.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    iconPainer.fillRect(temp.rect(), QColor(0, 0, 0, 64));
    iconPainer.end();

    return temp;
}

/**
 * @brief LoadIconsWorker::run
 */
void LoadIconsWorker::run()
{
    QString defaultIconPathFmt(":/icons/deepin/builtin/default_folder_icons/%1.svg");

    for (int i = 0; i < DEFAULTICONS_COUNT; i++) {
        QString iconPath = defaultIconPathFmt.arg(i + 1);
        QPixmap bitmap(iconPath);
        VNoteDataManager::m_defaultIcons[IconsType::DefaultIcon].push_back(bitmap);
        VNoteDataManager::m_defaultIcons[IconsType::DefaultGrayIcon].push_back(greyPix(bitmap));
    }

    VNoteDataManager::m_iconLock.unlock();
}
