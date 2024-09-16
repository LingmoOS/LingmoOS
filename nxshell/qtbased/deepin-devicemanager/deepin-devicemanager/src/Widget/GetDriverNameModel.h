// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GETDRIVERNAMEMODEL_H
#define GETDRIVERNAMEMODEL_H

#include <QObject>
#include <QStandardItemModel>

class GetDriverNameModel : public QObject
{
    Q_OBJECT
public:
    explicit GetDriverNameModel(QObject *parent = nullptr);

    /**
     * @brief stopLoadingDrivers 停止加载设备
     */
    void stopLoadingDrivers();

public slots:
    /**
     * @brief startLoadDrivers 通过给的路径，查找路径下的所有驱动文件
     * @param includeSub 是否查找目录下的子目录
     * @param path 给定的目录
     */
    void startLoadDrivers(QStandardItemModel* model, bool includeSub, const QString &path);

signals:
    void finishLoadDrivers();

private:
    /**
     * @brief traverseFolders 遍历目录下的文件
     * @param path 给定的目录
     */
    void traverseFolders(const QString &path, bool recursion = false);

private:
    QStringList             mp_driverPathList;   //驱动路径列表
    QStringList             mp_driversList;      //驱动名列表
    bool                    m_Stop = false;      //停止加载
};

#endif // GETDRIVERNAMEMODEL_H
