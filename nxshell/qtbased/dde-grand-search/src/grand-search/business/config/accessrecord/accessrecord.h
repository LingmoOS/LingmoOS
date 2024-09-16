// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ACCESSRECORD_H
#define ACCESSRECORD_H

#include "global/matcheditem.h"

namespace GrandSearch {

class AccessRecord : public QObject
{
    Q_OBJECT
public:
    static AccessRecord *instance();

    void startParseRecord();
    QHash<QString, QHash<QString, int>> getRecord();  //主线程获取数据  item:次数
    void updateRecord(const MatchedItem &matchedItem, qint64 time); // 主线程更新用户点击后的数据
public slots:
    void sync();   //主线程回写数据

signals:
    void sigParseFinished(const QJsonObject &recordObj, const QHash<QString, QHash<QString, int>> &recordHash, QPrivateSignal);
protected:
    explicit AccessRecord();
    ~AccessRecord();

private:
    static void parseRecord(QString recordPath);  // 子线程创建/解析文件
    static void parseJson(const QString &recordPath); //子线程解析文件数据并过滤数据

private:
    QHash<QString, QHash<QString, int>> m_recordHash; // 存放计算次数后的数据
    QJsonObject m_recordObj; // 存放文件中的数据
    QString m_recordPath; // 记录文件的路径
    bool m_finished = false;
    std::once_flag m_initFlag;
};

}

#endif // ACCESSRECORD_H
