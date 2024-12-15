// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dfiledragserver.h"
#include "private/dfiledragcommon_p.h"

#include <DObjectPrivate>

#include <QMimeData>
#include <QUuid>
#include <QDBusServer>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusVariant>
#include <QDBusError>
#include <QCoreApplication>
#include <QSharedPointer>

DGUI_BEGIN_NAMESPACE

class DDndSourceInterface;

/*!
 @private
 */
class DFileDragServerPrivate : public DCORE_NAMESPACE::DObjectPrivate
{
    QMap<QString, QVariant> data;
    QUuid uuid;

    explicit DFileDragServerPrivate(DFileDragServer *q);
    ~DFileDragServerPrivate();

    void writeMimeData(QMimeData *dest);

    QSharedPointer<DDndSourceInterface> dbusif;
    static QHash<QString, DFileDragServer*> servermap;
    static QWeakPointer<DDndSourceInterface> dbusifinst;

    D_DECLARE_PUBLIC(DFileDragServer)
    friend class DDndSourceInterface;
};

QWeakPointer<DDndSourceInterface> DFileDragServerPrivate::dbusifinst;
QHash<QString, DFileDragServer*> DFileDragServerPrivate::servermap;

class DDndSourceInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", DND_INTERFACE)
public:
    explicit DDndSourceInterface(QObject *parent = nullptr) : QObject(parent) {}

Q_SIGNALS:
    void serverDestroyed(QString uuid);
    void stateChanged(QString uuid, int state);
    void progressChanged(QString uuid, int progress);

public Q_SLOTS:
    void setData(QString uuid, QString key, QString value) {
        DFileDragServer *srv = DFileDragServerPrivate::servermap.value(uuid);
        Q_ASSERT(srv);

        DFileDragServerPrivate *d = srv->d_func();

        if (d->data.value(key) != value) {
            d->data[key] = value;
            Q_EMIT srv->targetDataChanged(key);
        }
    }
    int state(QString uuid) const {return m_statemap.value(uuid);}
    int progress(QString uuid) const {return m_progressmap.value(uuid);}

private:
    QHash<QString, int> m_statemap;
    QHash<QString, int> m_progressmap;

    friend class DFileDragServer;
};

/*!
  \class Dtk::Gui::DFileDragServer
  \inmodule dtkgui
  \brief 提供拖拽文件时与文件接收方交互的接口.
 */

/*!
  \fn void DFileDragServer::targetDataChanged(const QString &key)

  \brief 信号会在接收方调用 setData 变化时被发送， \a key 为改变的键值.
 */

DFileDragServer::DFileDragServer(QObject *parent)
    : QObject(parent)
    , DCORE_NAMESPACE::DObject(*new DFileDragServerPrivate(this))
{
    D_D(DFileDragServer);
    DFileDragServerPrivate::servermap[d->uuid.toString()] = this;
}

DFileDragServer::~DFileDragServer()
{
    D_D(DFileDragServer);
    Q_EMIT d->dbusif->serverDestroyed(d->uuid.toString());
    DFileDragServerPrivate::servermap.remove(d->uuid.toString());
}

/*!
  \brief DFileDragServer::targetData.
  \a key
  \return 返回文件接收方设置数据 \a key 对应的 value
 */
QVariant DFileDragServer::targetData(const QString &key) const
{
    D_D(const DFileDragServer);

    return d->data.value(key);
}

/*!
  \brief DFileDragServer::setProgress.
  \a progress 当前进度
  \brief 拖拽进度更新，接收方会受到 progressChanged 信号.
 */
void DFileDragServer::setProgress(int progress)
{
    D_D(DFileDragServer);

    if (d->dbusif && progress != d->dbusif->m_progressmap.value(d->uuid.toString())) {
        d->dbusif->m_progressmap[d->uuid.toString()] = progress;
        Q_EMIT d->dbusif->progressChanged(d->uuid.toString(), progress);
    }
}

/*!
  \brief DFileDragServer::setState.
  \a state
  \brief 改变状态，接收方会受到 stateChanged 信号.
 */
void DFileDragServer::setState(DFileDragState state)
{
    D_D(DFileDragServer);

    if (d->dbusif && state != d->dbusif->m_statemap.value(d->uuid.toString())) {
        d->dbusif->m_statemap[d->uuid.toString()] = state;
        Q_EMIT d->dbusif->stateChanged(d->uuid.toString(), state);
    }
}

DFileDragServerPrivate::DFileDragServerPrivate(DFileDragServer *q)
    : DCORE_NAMESPACE::DObjectPrivate(q)
    , uuid(QUuid::createUuid())
{
    if (dbusifinst.isNull()) {
        dbusif = QSharedPointer<DDndSourceInterface>(new DDndSourceInterface(), [](DDndSourceInterface *intf){
            QDBusConnection::sessionBus().unregisterObject(DND_OBJPATH);
            intf->deleteLater();
        });
        dbusifinst = dbusif.toWeakRef();
        QDBusConnection::sessionBus().registerObject(DND_OBJPATH, DND_INTERFACE, dbusif.data(), QDBusConnection::RegisterOption::ExportAllContents);
    } else {
        dbusif = dbusifinst.toStrongRef();
    }
}

DFileDragServerPrivate::~DFileDragServerPrivate()
{
}

void DFileDragServerPrivate::writeMimeData(QMimeData *dest)
{
    dest->setData(DND_MIME_SERVICE, QDBusConnection::sessionBus().baseService().toUtf8());
    // qApp->applicationPid() not right in ll-box
    auto pid = QDBusConnection::sessionBus().interface()->servicePid(QDBusConnection::sessionBus().baseService());
    dest->setData(DND_MIME_PID, QString::number(pid).toUtf8());
    dest->setData(DND_MIME_UUID, uuid.toString().toUtf8());
}

DGUI_END_NAMESPACE

#include "dfiledragserver.moc"
