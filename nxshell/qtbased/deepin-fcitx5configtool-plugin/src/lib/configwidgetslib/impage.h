/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _FCITX_IMPAGE_H_
#define _FCITX_IMPAGE_H_

#include "imconfig.h"
#include <QDBusPendingCallWatcher>
#include <QWidget>
#include <fcitxqtdbustypes.h>
#include <memory>

namespace Ui {
class IMPage;
}

namespace fcitx {
namespace kcm {

class AvailIMModel;
class IMProxyModel;
class CurrentIMModel;
class DBusProvider;

class IMPage : public QWidget {
    Q_OBJECT
public:
    IMPage(DBusProvider *dbus, QWidget *parent);
    ~IMPage();
Q_SIGNALS:
    void changed();
public Q_SLOTS:
    void save();
    void load();
    void defaults();

private Q_SLOTS:
    void selectedGroupChanged();

    void availIMSelectionChanged();
    void currentIMCurrentChanged();
    void selectCurrentIM(const QModelIndex &index);
    void doubleClickCurrentIM(const QModelIndex &index);
    void doubleClickAvailIM(const QModelIndex &index);
    void selectDefaultLayout();
    void selectLayout();

    void selectAvailIM(const QModelIndex &index);
    void clickAddIM();
    void clickRemoveIM();
    void moveDownIM();
    void moveUpIM();
    void configureIM();
    void addGroup();
    void deleteGroup();

private:
    void addIM(const QModelIndex &index);
    void removeIM(const QModelIndex &index);
    void checkDefaultLayout();
    void emitChanged();

    std::unique_ptr<Ui::IMPage> ui_;
    DBusProvider *dbus_;
    IMConfig *config_;
};
} // namespace kcm
} // namespace fcitx

#endif // _FCITX_IMPAGE_H_
