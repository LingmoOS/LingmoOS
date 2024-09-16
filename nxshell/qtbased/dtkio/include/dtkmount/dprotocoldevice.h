// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPROTOCOLDEVICE_H
#define DPROTOCOLDEVICE_H

#include <DtkMountGlobal>

#include <functional>

#include <QObject>
#include <QVariantMap>

#include <DExpected>

DMOUNT_BEGIN_NAMESPACE

enum PasswdSaveMode {
    Never = 0,
    SaveInSession = 1,
    SaveForever = 2
};

namespace MountPasswdInfoParamKeys {
inline constexpr char kName[] { "name" };
inline constexpr char kPasswd[] { "passwd" };
inline constexpr char kDomain[] { "domain" };
inline constexpr char kAnonymous[] { "anonymous" };
inline constexpr char kCancel[] { "cancel" };
inline constexpr char kSaveMode[] { "mode" };
}   // namespace MountPasswdInfoParamKeys

// TODO(xust): error handle in callback
using OperateCallback = std::function<void(bool)>;
using OperateCallbackWithInfo = std::function<void(bool, const QString &)>;

using AskForPasswd = std::function<QVariantMap(const QString &msg, const QString &user, const QString &domain)>;
using AskForChoice = std::function<int(const QString &msg, const QStringList &choices)>;

class DProtocolDevice;
namespace DDeviceManager {
DCORE_NAMESPACE::DExpected<DProtocolDevice *> createProtocolDevice(const QString &, QObject *);
}   // namespace DDeviceManager

class DProtocolDevicePrivate;
class DProtocolDevice : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(DProtocolDevice)
    friend DCORE_NAMESPACE::DExpected<DProtocolDevice *> DDeviceManager::createProtocolDevice(const QString &, QObject *);

    Q_PROPERTY(QString path READ path CONSTANT FINAL)
    Q_PROPERTY(QString mountPoint READ mountPoint CONSTANT FINAL)
    Q_PROPERTY(QString displayName READ displayName CONSTANT FINAL)
    Q_PROPERTY(QString fileSystem READ fileSystem CONSTANT FINAL)
    Q_PROPERTY(quint64 sizeTotal READ sizeTotal CONSTANT FINAL)
    Q_PROPERTY(quint64 sizeFree READ sizeFree CONSTANT FINAL)
    Q_PROPERTY(quint64 sizeUsage READ sizeUsage CONSTANT FINAL)
    Q_PROPERTY(QStringList iconNames READ iconNames CONSTANT FINAL)

public:
    ~DProtocolDevice() override;

    QString path() const;
    QString mountPoint() const;
    QString displayName() const;
    QString fileSystem() const;
    quint64 sizeTotal() const;
    quint64 sizeFree() const;
    quint64 sizeUsage() const;
    QStringList iconNames() const;

    QString mount(const QVariantMap &opts = {});
    void mountAsync(const QVariantMap &opts = {}, OperateCallbackWithInfo callback = nullptr);
    bool unmount(const QVariantMap &opts = {});
    void unmountAsync(const QVariantMap &opts = {}, OperateCallback callback = nullptr);

    void setOperateTimeout(int secs = 3);
    void setAskForPasswd(AskForPasswd callback);
    void setAskForChoice(AskForChoice callback);

private:
    explicit DProtocolDevice(const QString &devicePath, QObject *parent = nullptr);

    QScopedPointer<DProtocolDevicePrivate> d_ptr;
};

DMOUNT_END_NAMESPACE

#endif   // DPROTOCOLDEVICE_H
