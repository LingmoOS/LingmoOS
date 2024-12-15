/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include "imconfig.h"
#include "dbusprovider.h"
#include "model.h"

namespace fcitx {
namespace kcm {

IMConfig::IMConfig(DBusProvider *dbus, ModelMode mode, QObject *parent)
    : QObject(parent), dbus_(dbus), availIMModel_(new IMProxyModel(this)),
      currentIMModel_(new FilteredIMModel(FilteredIMModel::CurrentIM, this)) {
    connect(dbus, &DBusProvider::availabilityChanged, this,
            &IMConfig::availabilityChanged);
    availabilityChanged();

    if (mode == Flatten) {
        auto flattenAvailIMModel =
            new FilteredIMModel(FilteredIMModel::AvailIM, this);
        availIMModel_->setSourceModel(flattenAvailIMModel);
        internalAvailIMModel_ = flattenAvailIMModel;
    } else {
        auto availIMModel = new AvailIMModel(this);
        availIMModel_->setSourceModel(availIMModel);
        internalAvailIMModel_ = availIMModel;
    }

    connect(currentIMModel_, &FilteredIMModel::imListChanged, this,
            [this](const FcitxQtInputMethodEntryList &list) {
                auto old = imEntries_;
                FcitxQtStringKeyValueList newEntries;
                for (const auto &item : list) {
                    auto iter = std::find_if(
                        old.begin(), old.end(),
                        [&item](const FcitxQtStringKeyValue &entry) {
                            return entry.key() == item.uniqueName();
                        });
                    if (iter != old.end()) {
                        newEntries.push_back(*iter);
                    }
                }
                imEntries_ = newEntries;
                updateIMList(true);
                emitChanged();
            });
}

IMConfig::~IMConfig() {}

void IMConfig::save() {
    if (!dbus_->controller()) {
        return;
    }
    if (needSave_) {
        dbus_->controller()->SetInputMethodGroupInfo(lastGroup_, defaultLayout_,
                                                     imEntries_);
        needSave_ = false;
    }
}

void IMConfig::load() { availabilityChanged(); }

void IMConfig::defaults() {}

void IMConfig::addIM(int idx) {
    auto index = availIMModel_->index(idx, 0);
    addIM(index);
}

void IMConfig::addIM(const QModelIndex &index) {
    if (!index.isValid()) {
        return;
    }
    auto uniqueName = index.data(FcitxIMUniqueNameRole).toString();
    FcitxQtStringKeyValue imEntry;
    imEntry.setKey(uniqueName);
    imEntries_.push_back(imEntry);
    updateIMList();
    emitChanged();
}

void IMConfig::addIMs(const QModelIndexList &indexes) {
    for (const auto &index : indexes) {
        if (!index.isValid()) {
            continue;
        }
        auto uniqueName = index.data(FcitxIMUniqueNameRole).toString();
        FcitxQtStringKeyValue imEntry;
        imEntry.setKey(uniqueName);
        imEntries_.push_back(imEntry);
    }
    updateIMList();
    emitChanged();
}

void IMConfig::removeIM(int idx) { currentIMModel_->remove(idx); }

void IMConfig::removeIM(const QModelIndex &index) {
    currentIMModel_->remove(index.row());
}

void IMConfig::move(int from, int to) { currentIMModel_->move(from, to); }

void IMConfig::reloadGroup() {
    if (!dbus_->controller()) {
        return;
    }
    auto call = dbus_->controller()->InputMethodGroups();
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *watcher) {
                fetchGroupsFinished(watcher);
            });
}

void IMConfig::fetchGroupsFinished(QDBusPendingCallWatcher *watcher) {
    QDBusPendingReply<QStringList> groups = *watcher;
    watcher->deleteLater();

    if (groups.isValid()) {
        groups_ = groups.value();
        Q_EMIT groupsChanged(groups_);
    }

    if (!groups_.empty()) {
        setCurrentGroup(groups_.front());
    }
}

void IMConfig::availabilityChanged() {
    lastGroup_.clear();
    if (!dbus_->controller()) {
        return;
    }
    reloadGroup();
    auto imcall = dbus_->controller()->AvailableInputMethods();
    auto imcallwatcher = new QDBusPendingCallWatcher(imcall, this);
    connect(imcallwatcher, &QDBusPendingCallWatcher::finished, this,
            &IMConfig::fetchInputMethodsFinished);
    auto checkUpdate = dbus_->controller()->CheckUpdate();
    auto checkUpdateWatcher = new QDBusPendingCallWatcher(checkUpdate, this);
    connect(checkUpdateWatcher, &QDBusPendingCallWatcher::finished, this,
            &IMConfig::checkUpdateFinished);
}

void IMConfig::fetchInputMethodsFinished(QDBusPendingCallWatcher *watcher) {
    QDBusPendingReply<FcitxQtInputMethodEntryList> ims = *watcher;
    watcher->deleteLater();
    if (!ims.isError()) {
        allIMs_ = ims.value();
        updateIMList();
    }
}

void IMConfig::checkUpdateFinished(QDBusPendingCallWatcher *watcher) {
    QDBusPendingReply<bool> reply = *watcher;
    watcher->deleteLater();
    const bool needUpdate = !reply.isError() && reply.value();
    if (needUpdate_ != needUpdate) {
        needUpdate_ = needUpdate;
        Q_EMIT needUpdateChanged(needUpdate_);
    }
}

void IMConfig::setCurrentGroup(const QString &name) {
    if (dbus_->available() && !name.isEmpty()) {
        auto call = dbus_->controller()->InputMethodGroupInfo(name);
        lastGroup_ = name;
        Q_EMIT currentGroupChanged(lastGroup_);
        auto watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this,
                &IMConfig::fetchGroupInfoFinished);
    }
}

void IMConfig::fetchGroupInfoFinished(QDBusPendingCallWatcher *watcher) {
    watcher->deleteLater();
    needSave_ = false;
    QDBusPendingReply<QString, FcitxQtStringKeyValueList> reply = *watcher;
    if (!reply.isError()) {
        defaultLayout_ = reply.argumentAt<0>();
        imEntries_ = reply.argumentAt<1>();
    } else {
        defaultLayout_.clear();
        imEntries_.clear();
    }
    Q_EMIT defaultLayoutChanged();

    updateIMList();
}

void IMConfig::emitChanged() {
    needSave_ = true;
    Q_EMIT changed();
}

void IMConfig::updateIMList(bool excludeCurrent) {
    if (!excludeCurrent) {
        currentIMModel_->filterIMEntryList(allIMs_, imEntries_);
    }
    internalAvailIMModel_->filterIMEntryList(allIMs_, imEntries_);
    availIMModel_->filterIMEntryList(allIMs_, imEntries_);

    Q_EMIT imListChanged();
}

void IMConfig::addGroup(const QString &name) {
    if (!name.isEmpty() && dbus_->controller()) {
        auto call = dbus_->controller()->AddInputMethodGroup(name);
        auto watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this,
                [this](QDBusPendingCallWatcher *watcher) {
                    watcher->deleteLater();
                    if (!watcher->isError()) {
                        reloadGroup();
                    }
                });
    }
}

void IMConfig::deleteGroup(const QString &name) {
    if (!dbus_->controller()) {
        return;
    }
    auto call = dbus_->controller()->RemoveInputMethodGroup(name);
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *watcher) {
                watcher->deleteLater();
                if (!watcher->isError()) {
                    reloadGroup();
                }
            });
}

void IMConfig::refresh() {
    if (!dbus_->controller()) {
        return;
    }
    auto call = dbus_->controller()->Refresh();
    auto watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *watcher) {
                watcher->deleteLater();
                if (!watcher->isError()) {
                    load();
                }
            });
}

void IMConfig::restart() {
    if (!dbus_->controller()) {
        return;
    }
    dbus_->controller()->Restart();
}

} // namespace kcm
} // namespace fcitx
