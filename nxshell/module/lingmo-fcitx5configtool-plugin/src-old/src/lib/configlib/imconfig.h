/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _CONFIGLIB_ABSTRACTIMPAGE_H_
#define _CONFIGLIB_ABSTRACTIMPAGE_H_

#include <QAbstractItemModel>
#include <QDBusPendingCallWatcher>
#include <QObject>
#include <fcitxqtdbustypes.h>

namespace fcitx {
namespace kcm {

class IMConfigModelInterface;
class IMProxyModel;
class FilteredIMModel;
class DBusProvider;

class IMConfig : public QObject {
    Q_OBJECT
    Q_PROPERTY(fcitx::kcm::FilteredIMModel *currentIMModel READ currentIMModel
                   CONSTANT)
    Q_PROPERTY(
        fcitx::kcm::IMProxyModel *availIMModel READ availIMModel CONSTANT)
    Q_PROPERTY(QString defaultLayout READ defaultLayout WRITE setDefaultLayout
                   NOTIFY defaultLayoutChanged)
    Q_PROPERTY(QStringList groups READ groups NOTIFY groupsChanged)
    Q_PROPERTY(QString currentGroup READ currentGroup WRITE setCurrentGroup)
    Q_PROPERTY(bool needSave READ needSave)
    Q_PROPERTY(bool needUpdate READ needUpdate NOTIFY needUpdateChanged)
public:
    enum ModelMode { Tree, Flatten };

    IMConfig(DBusProvider *dbus, ModelMode mode, QObject *parent);
    ~IMConfig();

    FilteredIMModel *currentIMModel() const { return currentIMModel_; }
    IMProxyModel *availIMModel() const { return availIMModel_; }
    const QStringList &groups() const { return groups_; }
    const QString &currentGroup() const { return lastGroup_; }
    void setCurrentGroup(const QString &name);
    bool needSave() const { return needSave_; }
    bool needUpdate() const { return needUpdate_; }

    void addIM(const QModelIndex &index);
    void addIMs(const QModelIndexList &indexes);
    void removeIM(const QModelIndex &index);

    const auto &imEntries() const { return imEntries_; }
    void setIMEntries(const FcitxQtStringKeyValueList &imEntires) {
        imEntries_ = imEntires;
        updateIMList();
    }

    const QString &defaultLayout() const { return defaultLayout_; }
    void setDefaultLayout(const QString &l) {
        if (defaultLayout_ != l) {
            defaultLayout_ = l;
            emitChanged();
            Q_EMIT defaultLayoutChanged();
        }
    }

    Q_INVOKABLE void setLayout(const QString &im, const QString &layout) {
        for (auto &imEntry : imEntries_) {
            if (imEntry.key() == im) {
                imEntry.setValue(layout);
                emitChanged();
                updateIMList();
                return;
            }
        }
    }

    void emitChanged();

public Q_SLOTS:
    void addGroup(const QString &name);
    void deleteGroup(const QString &name);
    void save();
    void load();
    void defaults();
    void addIM(int index);
    void removeIM(int index);
    void move(int from, int to);
    void refresh();
    void restart();

Q_SIGNALS:
    void changed();
    void currentGroupChanged(const QString &group);
    void groupsChanged(const QStringList &groups);
    void imListChanged();
    void defaultLayoutChanged();
    void needUpdateChanged(bool);

private Q_SLOTS:
    void availabilityChanged();
    void fetchGroupInfoFinished(QDBusPendingCallWatcher *watcher);
    void fetchInputMethodsFinished(QDBusPendingCallWatcher *watcher);
    void fetchGroupsFinished(QDBusPendingCallWatcher *watcher);
    void checkUpdateFinished(QDBusPendingCallWatcher *watcher);

private:
    void reloadGroup();
    void updateIMList(bool excludeCurrent = false);

    DBusProvider *dbus_;
    IMProxyModel *availIMModel_;
    IMConfigModelInterface *internalAvailIMModel_ = nullptr;
    FilteredIMModel *currentIMModel_;
    QString defaultLayout_;
    FcitxQtStringKeyValueList imEntries_;
    FcitxQtInputMethodEntryList allIMs_;
    QStringList groups_;
    QString lastGroup_;
    bool needSave_ = false;
    bool needUpdate_ = false;
};
} // namespace kcm
} // namespace fcitx

#endif // _CONFIGLIB_ABSTRACTIMPAGE_H_
