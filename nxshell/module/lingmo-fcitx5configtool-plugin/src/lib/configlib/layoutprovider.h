/*
 * SPDX-FileCopyrightText: 2020~2020 CSSlayer <wengxt@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#ifndef _CONFIGLIB_LAYOUTPROVIDER_H_
#define _CONFIGLIB_LAYOUTPROVIDER_H_

#include "iso639.h"
#include "layoutmodel.h"
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStringListModel>
#include <fcitxqtdbustypes.h>

class QDBusPendingCallWatcher;

namespace fcitx {
namespace kcm {

class DBusProvider;
class LanguageFilterModel;
class LayoutInfoModel;
class VariantInfoModel;

class LayoutProvider : public QObject {
    Q_OBJECT
    Q_PROPERTY(
        fcitx::kcm::LanguageModel *languageModel READ languageModel CONSTANT)
    Q_PROPERTY(LanguageFilterModel *layoutModel READ layoutModel CONSTANT)
    Q_PROPERTY(LanguageFilterModel *variantModel READ variantModel CONSTANT)
public:
    LayoutProvider(DBusProvider *dbus, QObject *parent = nullptr);
    ~LayoutProvider();

    auto languageModel() const { return languageModel_; }
    auto layoutModel() const { return layoutFilterModel_; }
    auto variantModel() const { return variantFilterModel_; }

    Q_INVOKABLE int layoutIndex(const QString &layoutString);
    Q_INVOKABLE int variantIndex(const QString &layoutString);
    Q_INVOKABLE QString layoutDescription(const QString &layoutString);

    Q_INVOKABLE void setVariantInfo(const FcitxQtLayoutInfo &info) const {
        variantModel_->setVariantInfo(info);
    }

    Q_INVOKABLE QString layout(int layoutIdx, int variantIdx) const {
        auto layoutModelIndex = layoutFilterModel_->index(layoutIdx, 0);
        auto variantModelIndex = variantFilterModel_->index(variantIdx, 0);
        if (layoutModelIndex.isValid() && variantModelIndex.isValid()) {
            auto layout = layoutModelIndex.data(Qt::UserRole).toString();
            auto variant = variantModelIndex.data(Qt::UserRole).toString();
            if (layout.isEmpty()) {
                return QString();
            }
            if (variant.isEmpty()) {
                return layout;
            }
            return QString("%1-%2").arg(layout, variant);
        }
        return QString();
    }

    bool loaded() const { return loaded_; }

Q_SIGNALS:
    void loadedChanged();

private Q_SLOTS:
    void availabilityChanged();
    void fetchLayoutFinished(QDBusPendingCallWatcher *watcher);

private:
    void setLoaded(bool loaded) {
        if (loaded != loaded_) {
            loaded_ = loaded;
            Q_EMIT loadedChanged();
        }
    }

    DBusProvider *dbus_;
    bool loaded_ = false;
    LanguageModel *languageModel_;
    LayoutInfoModel *layoutModel_;
    VariantInfoModel *variantModel_;
    LanguageFilterModel *layoutFilterModel_;
    LanguageFilterModel *variantFilterModel_;
    Iso639 iso639_;
};

} // namespace kcm
} // namespace fcitx

#endif // _CONFIGLIB_LAYOUTPROVIDER_H_
