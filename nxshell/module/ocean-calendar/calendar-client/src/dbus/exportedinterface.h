// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef EXPORTEDINTERFACE_H
#define EXPORTEDINTERFACE_H

#include "dschedule.h"

#include <DExportedInterface>

#include <QObject>

DCORE_USE_NAMESPACE
struct Exportpara {
    int viewType; //视图名称:year,month,week,day对应年、月、周、日视图类型
    QDateTime  viewTime; //表示需要查看视图上某个时间
    QString ADTitleName;
    QDateTime  ADStartTime;
    QDateTime  ADEndTime;
};
class ExportedInterface : public Dtk::Core::DUtil::DExportedInterface
{
public:
    explicit ExportedInterface(QObject *parent = nullptr);
    QVariant invoke(const QString &action, const QString &parameters) const override;
private:
    bool analysispara(QString &parameters, DSchedule::Ptr &info, Exportpara &para) const;

private:
    QObject *m_object = nullptr;
};

#endif // EXPORTEDINTERFACE_H
