// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DGIOMOUNTOPERATION_H
#define DGIOMOUNTOPERATION_H

#include <QObject>

namespace  Glib{
template <class T_CppObject>
class RefPtr;
}
namespace  Gio{
class MountOperation;
}

enum DGioAskPasswordFlag
{
    ASK_PASSWORD_NEED_PASSWORD = (1 << 0),
    ASK_PASSWORD_NEED_USERNAME = (1 << 1),
    ASK_PASSWORD_NEED_DOMAIN = (1 << 2),
    ASK_PASSWORD_SAVING_SUPPORTED = (1 << 3),
    ASK_PASSWORD_ANONYMOUS_SUPPORTED = (1 << 4)
};
Q_DECLARE_FLAGS(DGioAskPasswordFlags, DGioAskPasswordFlag)

//! Indicated the mount operation result
/*!
 *  Wrapper of GMountOperationResult.
 */
enum DGioMountOperationResult
{
    MOUNT_OPERATION_HANDLED, /*!< The request was fulfilled and the user specified data is now available */
    MOUNT_OPERATION_ABORTED, /*!< The user requested the mount operation to be aborted */
    MOUNT_OPERATION_UNHANDLED /*!< The request was unhandled (i.e. not implemented) */
};
Q_ENUMS(DGioMountOperationResult);

enum DGioPasswordSave
{
  PASSWORD_SAVE_NEVER,
  PASSWORD_SAVE_FOR_SESSION,
  PASSWORD_SAVE_PERMANENTLY
};
Q_ENUMS(DGioPasswordSave);

class DGioMountOperationPrivate;
class DGioMountOperation : public QObject
{
    Q_OBJECT
public:
    explicit DGioMountOperation(QObject *parent);
    ~DGioMountOperation();

    QString username() const;
    void setUsername(QString name);
    QString password() const;
    void setPassword(QString password);
    DGioPasswordSave passwordSave();
    void setPasswordSave(DGioPasswordSave save);
    QString domain() const;
    void setDomain(QString domain);

    bool anonymous()const;
    void setAnonymous(bool anonymous =  true);
    int choice()const;
    void setChoice(int choice);

    void reply(DGioMountOperationResult result);

    // do not release the ptr returned
    Glib::RefPtr<Gio::MountOperation> getGIOMountOperationObj();

Q_SIGNALS:
    void askPassword(QString message, QString defaultUser, QString defaultDomain, DGioAskPasswordFlags flags);
    void askQuestion(QString message, QStringList choices);
    void showUnmountProgress(QString message, qint64 timeLeftMs, qint64 bytesLeft);

private:
    QScopedPointer<DGioMountOperationPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DGioMountOperation)
};

#endif // DGIOMOUNTOPERATION_H
