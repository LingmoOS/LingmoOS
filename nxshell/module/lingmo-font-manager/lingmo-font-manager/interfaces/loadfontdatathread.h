// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOADFONTDATATHREAD_H
#define LOADFONTDATATHREAD_H
#include "dfontinfomanager.h"
#include "dfontpreviewlistdatathread.h"
#include "dfmdbmanager.h"
#include "dcopyfilesmanager.h"

#include <QThread>

class LoadFontDataThread : public QThread
{
    Q_OBJECT
public:
    explicit LoadFontDataThread(QList<QMap<QString, QString>> &list, QObject *parent = nullptr);

protected:
    void run();


signals:
    void dataLoadFinish(QList<DFontPreviewItemData> &);

private:
    QList<DFontPreviewItemData> m_delFontInfoList;
    QList<DFontPreviewItemData> m_startModelList;
    QList<QMap<QString, QString>> m_list;

    QThread mThread;
};



#endif // LOADFONTDATATHREAD_H
