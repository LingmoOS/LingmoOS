// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "ddiskinfo.h"
#include "ddiskinfo_p.h"
#include "dpartinfo_p.h"
#include "helper.h"
#include "ddevicediskinfo.h"
#include "dfilediskinfo.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>
#include <QDebug>

DDiskInfoPrivate::DDiskInfoPrivate(DDiskInfo *qq)
    : q(qq)
{

}

void DDiskInfoPrivate::initFromJson(const QByteArray &json)
{
    DDiskInfo::fromJson(json, this);
}

qint64 DDiskInfoPrivate::maxReadableDataSize() const
{
    return size;
}

bool DDiskInfoPrivate::setTotalWritableDataSize(qint64 size)
{
    Q_UNUSED(size)

    return false;
}

void DDiskInfoPrivate::setErrorString(const QString &error)
{
    this->error = error;
}

QString DDiskInfoPrivate::errorString() const
{
    return error;
}

QString DDiskInfoPrivate::scopeString(DDiskInfo::DataScope scope)
{
    switch (scope) {
    case DDiskInfo::Headgear:
        return "Headgear";
    case DDiskInfo::PartitionTable:
        return "PartitionTable";
    case DDiskInfo::Partition:
        return "Partition";
    case DDiskInfo::JsonInfo:
        return "JsonInfo";
    case DDiskInfo::NullScope:
        return "NullScope";
    default:
        break;
    }

    return QString();
}

QString DDiskInfoPrivate::modeString(DDiskInfo::ScopeMode mode)
{
    if (mode == DDiskInfo::Read)
        return "Read";

    return "Write";
}

DDiskInfo::DDiskInfo()
{

}

DDiskInfo::DDiskInfo(const DDiskInfo &other)
    : d(other.d)
{
    if (d)
        d->q = this;
}

DDiskInfo::~DDiskInfo()
{

}

void DDiskInfo::swap(DDiskInfo &other)
{
    qSwap(d, other.d);

    if (d)
        d->q = this;
}

DDiskInfo::DataScope DDiskInfo::currentScope() const
{
    return d->currentScope;
}

bool DDiskInfo::hasScope(DDiskInfo::DataScope scope, ScopeMode mode, int index) const
{
    if (scope == NullScope)
        return true;

    return d->hasScope(scope, mode, index);
}

bool DDiskInfo::beginScope(DDiskInfo::DataScope scope, ScopeMode mode, int index)
{
    endScope();

    d->error.clear();

    if (!d->hasScope(scope, mode, index)) {
        dCError("Device \"%s\" not support scope: \"%s\" mode: \"%s\", index: %d", qPrintable(filePath()), qPrintable(d->scopeString(scope)), qPrintable(d->modeString(mode)), index);

        return false;
    }

    d->currentScope = scope;
    d->currentMode = mode;

    dCDebug("Try open data stream(this=%llx): scope=%d, mode=%d, index=%d", this, scope, mode, index);

    return d->openDataStream(index);
}

bool DDiskInfo::endScope()
{
    if (d->currentScope != NullScope) {
        dCDebug("Try close data stream(this=%llx): scope=%d, mode=%d", this, d->currentScope, d->currentMode);

        d->closeDataStream();

        dCDebug("Close data stream finished");
    }

    d->currentScope = NullScope;

    return d->errorString().isEmpty();
}

qint64 DDiskInfo::readableDataSize(DDiskInfo::DataScope scope) const
{
    return d->readableDataSize(scope);
}

qint64 DDiskInfo::totalReadableDataSize() const
{
    return d->totalReadableDataSize();
}

qint64 DDiskInfo::maxReadableDataSize() const
{
    return d->maxReadableDataSize();
}

qint64 DDiskInfo::totalWritableDataSize() const
{
    return d->totalWritableDataSize();
}

bool DDiskInfo::setTotalWritableDataSize(qint64 size)
{
    return d->setTotalWritableDataSize(size);
}

qint64 DDiskInfo::read(char *data, qint64 maxSize)
{
    return d->read(data, maxSize);
}

qint64 DDiskInfo::write(const char *data, qint64 maxSize)
{
    return d->write(data, maxSize);
}

qint64 DDiskInfo::write(const char *data)
{
    return write(data, qstrlen(data));
}

bool DDiskInfo::atEnd() const
{
    return d->atEnd();
}

QString DDiskInfo::filePath() const
{
    return d->filePath();
}

DDiskInfo &DDiskInfo::operator=(const DDiskInfo &other)
{
    d = other.d;
    return *this;
}

QString DDiskInfo::model() const
{
    return d->model;
}

QString DDiskInfo::name() const
{
    return d->name;
}

QString DDiskInfo::kname() const
{
    return d->kname;
}

qint64 DDiskInfo::totalSize() const
{
    return d->size;
}

qint64 DDiskInfo::usedSize() const
{
    qint64 size = 0;

    for (const DPartInfo &part : childrenPartList())
        size += part.usedSize();

    return size;
}

QString DDiskInfo::typeName() const
{
    return d->typeName;
}

DDiskInfo::Type DDiskInfo::type() const
{
    return d->type;
}

DDiskInfo::PTType DDiskInfo::ptType() const
{
    return d->ptType;
}

QString DDiskInfo::transport() const
{
    return d->transport;
}

QString DDiskInfo::serial() const
{
    return d->serial;
}

bool DDiskInfo::isReadonly() const
{
    return d->readonly;
}

bool DDiskInfo::isRemoveable() const
{
    return d->removeable;
}

QList<DPartInfo> DDiskInfo::childrenPartList() const
{
    return d->children;
}

void DDiskInfo::refresh()
{
    d->refresh();
}

QByteArray DDiskInfo::toJson() const
{
    QJsonObject root
    {
        {"totalReadableDataSize", QString::number(totalReadableDataSize())},
        {"maxReadableDataSize", QString::number(maxReadableDataSize())},
        {"totalWritableDataSize", QString::number(totalWritableDataSize())},
        {"filePath", filePath()},
        {"model", model()},
        {"name", name()},
        {"kname", kname()},
        {"totalSize", QString::number(totalSize())},
        {"typeName", typeName()},
        {"type", type()},
        {"ptTypeName", d->ptTypeName},
        {"ptType", ptType()},
        {"readonly", isReadonly()},
        {"removeable", isRemoveable()},
        {"transport", transport()},
        {"serial", serial()}
    };

    QJsonArray children;

    for (const DPartInfo &part : childrenPartList())
        children.append(QJsonDocument::fromJson(part.toJson()).object());

    root.insert("childrenPartList", children);

    QJsonDocument doc(root);

    return doc.toJson();
}

QString DDiskInfo::errorString() const
{
    return d->errorString();
}

DDiskInfo DDiskInfo::getInfo(const QString &file)
{
    DDiskInfo info;

    if (Helper::isBlockSpecialFile(file)) {
        info = DDeviceDiskInfo(file);
    } else {
        QFileInfo file_info(file);

        if (file_info.suffix() != "dim") {
            return info;
        }

        if (file_info.exists()) {
            if (file_info.isFile())
                info = DFileDiskInfo(file);
        } else {
            QFile touch(file);

            if (touch.open(QIODevice::WriteOnly)) {
                touch.close();
                info = DFileDiskInfo(file);
            }
        }
    }

    return info;
}

DDiskInfo::DDiskInfo(DDiskInfoPrivate *dd)
    : d(dd)
{
    if (dd)
        dd->q = this;
}

const DPartInfo &DDiskInfo::getPartByNumber(int index)
{
    for (const DPartInfo &info : d->children) {
        if (info.indexNumber() == index)
            return info;
    }

    static DPartInfo global_null_info;

    return global_null_info;
}

void DDiskInfo::fromJson(const QByteArray &json, DDiskInfoPrivate *dd)
{
    const QJsonDocument &doc = QJsonDocument::fromJson(json);
    const QJsonObject &root = doc.object();

    dd->model = root.value("model").toString();
    dd->name = root.value("name").toString();
    dd->kname = root.value("kname").toString();
    dd->size = root.value("totalSize").toString().toLongLong();
    dd->typeName = root.value("typeName").toString();
    dd->type = (Type)root.value("type").toInt();
    dd->ptTypeName = root.value("ptTypeName").toString();
    dd->ptType = (PTType)root.value("ptType").toInt();
    dd->havePartitionTable = dd->type == Disk && !dd->ptTypeName.isEmpty();
    dd->readonly = root.value("readonly").toBool();
    dd->removeable = root.value("removeable").toBool();
    dd->transport = root.value("transport").toString();
    dd->serial = root.value("serial").toString();

    for (const QJsonValue &v : root.value("childrenPartList").toArray()) {
        DPartInfoPrivate *part_dd = new DPartInfoPrivate(0);
        DPartInfo::fromJson(v.toObject(), part_dd);

        dd->children << DPartInfo(part_dd);
    }
}

QT_BEGIN_NAMESPACE
#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug deg, const DDiskInfo &info)
{
    QDebugStateSaver saver(deg);
    Q_UNUSED(saver)

    deg.space() << "name:" << info.name()
                << "size:" << Helper::sizeDisplay(info.totalSize());
    deg << "partitions:" << info.childrenPartList();

    return deg;
}
#endif
QT_END_NAMESPACE
