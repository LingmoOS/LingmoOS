// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOPTICALDISCINFO_H
#define DOPTICALDISCINFO_H

#include <QObject>
#include <QString>
#include <QSharedDataPointer>

#include <DExpected>

#include "dtkburn_global.h"
#include "dtkburntypes.h"

DBURN_BEGIN_NAMESPACE

class DOpticalDiscInfo;
namespace DOpticalDiscManager {
DTK_CORE_NAMESPACE::DExpected<DOpticalDiscInfo *>
createOpticalDiscInfo(const QString &dev, QObject *parent);
}   // namespace DOpticalDiscManager

class DOpticalDiscInfoPrivate;
class DOpticalDiscInfo final : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool blank READ blank CONSTANT FINAL)
    Q_PROPERTY(QString device READ device CONSTANT FINAL)
    Q_PROPERTY(QString volumeName READ volumeName CONSTANT FINAL)
    Q_PROPERTY(quint64 usedSize READ usedSize CONSTANT FINAL)
    Q_PROPERTY(quint64 availableSize READ availableSize CONSTANT FINAL)
    Q_PROPERTY(quint64 totalSize READ totalSize CONSTANT FINAL)
    Q_PROPERTY(quint64 dataBlocks READ dataBlocks CONSTANT FINAL)
    Q_PROPERTY(MediaType mediaType READ mediaType CONSTANT FINAL)
    Q_PROPERTY(QStringList writeSpeed READ writeSpeed CONSTANT FINAL)

public:
    DOpticalDiscInfo(const DOpticalDiscInfo &info);
    DOpticalDiscInfo &operator=(const DOpticalDiscInfo &info);
    ~DOpticalDiscInfo();

    bool blank() const;
    QString device() const;
    QString volumeName() const;
    quint64 usedSize() const;
    quint64 availableSize() const;
    quint64 totalSize() const;
    quint64 dataBlocks() const;
    MediaType mediaType() const;
    QStringList writeSpeed() const;

private:
    explicit DOpticalDiscInfo(const QString &dev, QObject *parent = nullptr);

    friend DTK_CORE_NAMESPACE::DExpected<DOpticalDiscInfo *>
    DOpticalDiscManager::createOpticalDiscInfo(const QString &dev, QObject *parent);

private:
    QSharedDataPointer<DOpticalDiscInfoPrivate> d_ptr;
};

DBURN_END_NAMESPACE

#endif   // DOPTICALDISCINFO_H
