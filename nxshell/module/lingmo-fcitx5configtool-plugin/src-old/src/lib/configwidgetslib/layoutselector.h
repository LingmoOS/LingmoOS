/*
 * SPDX-FileCopyrightText: 2017~2017 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KCM_FCITX_LAYOUTWIDGET_H_
#define _KCM_FCITX_LAYOUTWIDGET_H_

#include "iso639.h"
#include "layoutmodel.h"
#include "layoutprovider.h"
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QWidget>
#include <fcitxqtdbustypes.h>
#include <memory>

class QDBusPendingCallWatcher;

namespace Ui {
class LayoutSelector;
}

namespace fcitx {
namespace kcm {

class DBusProvider;
class KeyboardLayoutWidget;
class LanguageFilterModel;
class LayoutInfoModel;
class VariantInfoModel;

class LayoutSelector : public QWidget {
    Q_OBJECT
public:
    LayoutSelector(DBusProvider *dbus, QWidget *parent = nullptr);
    ~LayoutSelector();
    void setLayout(const QString &layout, const QString &variant);

    static QPair<QString, QString>
    selectLayout(QWidget *parent, DBusProvider *dbus, const QString &title,
                 const QString &layout, const QString &variant,
                 bool *ok = nullptr);

    QPair<QString, QString> layout() const;

private Q_SLOTS:
    void languageComboBoxChanged();
    void layoutComboBoxChanged();
    void variantComboBoxChanged();

private:
    std::unique_ptr<Ui::LayoutSelector> ui_;
    DBusProvider *dbus_;
    LayoutProvider *layoutProvider_;
    KeyboardLayoutWidget *keyboardLayoutWidget_ = nullptr;
    Iso639 iso639_;

    QString preSelectLayout_;
    QString preSelectVariant_;
};

} // namespace kcm
} // namespace fcitx

#endif // _KCM_FCITX_LAYOUTWIDGET_H_
