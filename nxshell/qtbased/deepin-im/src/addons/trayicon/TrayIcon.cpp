// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "TrayIcon.h"

#include "IconProvider.h"
#include "dimcore/Dim.h"
#include "dimcore/InputContext.h"

#include <DDBusSender>
#include <DGuiApplicationHelper>

#include <QQmlContext>

using namespace org::deepin::dim;

DIM_ADDON_FACTORY(TrayIcon)

// TODO:
static const QString DEFAULT_IM_ADDON_NAME = "none";

TrayIconInputMethodEntry::TrayIconInputMethodEntry(const QString &addon,
                                                   const QString &name,
                                                   const QString &description,
                                                   const QString &icon)
    : addon_(addon)
    , name_(name)
    , description_(description)
    , icon_(icon)
{
}

TrayIconInputMethodEntry::~TrayIconInputMethodEntry() { }

TrayIcon::TrayIcon(Dim *dim)
    : FrontendAddon(dim, "trayicon")
    , imAddonIcon_("org.fcitx.Fcitx5")
{
    engine.rootContext()->setContextProperty("trayicon", this);
    engine.addImageProvider(QLatin1String("icon"), new IconProvider);
    const QUrl url(QStringLiteral("qrc:/TrayIcon.qml"));
    loadTranslator();
    engine.load(url);

    connect(dim, &Dim::focusedInputContextChanged, this, &TrayIcon::onFocusedInputContextChanged);
    onFocusedInputContextChanged(dim->focusedInputContext());

    connect(dim,
            &Dim::inputMethodEntryChanged,
            this,
            [this, &imList = dim->activeInputMethodEntries(), &imEntries = dim->imEntries()]() {
                for (auto it = imList.begin(); it != imList.cend(); ++it) {
                    auto iter =
                        std::find_if(trayIconIMEntries_.cbegin(),
                                     trayIconIMEntries_.cend(),
                                     [&it](TrayIconInputMethodEntry *entry) {
                                         return entry->addon() == QString::fromStdString(it->first)
                                             && entry->name() == QString::fromStdString(it->second);
                                     });
                    if (iter != trayIconIMEntries_.cend()) {
                        // TODO:remove exist im
                        continue;
                    }

                    auto entryIter = std::find_if(imEntries.cbegin(),
                                                  imEntries.cend(),
                                                  [&it](const InputMethodEntry &entry) {
                                                      return entry.addonKey() == it->first
                                                          && entry.uniqueName() == it->second;
                                                  });
                    if (entryIter == imEntries.cend()) {
                        continue;
                    }

                    trayIconIMEntries_.append(new TrayIconInputMethodEntry{
                        QString::fromStdString(entryIter->addonKey()),
                        QString::fromStdString(entryIter->uniqueName()),
                        QString::fromStdString(entryIter->description()),
                        QString::fromStdString(entryIter->label()) });
                }

                updateTrayIconIMEntries();
            });
}

TrayIcon::~TrayIcon() { }

void TrayIcon::onFocusedInputContextChanged(int focusedInputContext)
{
    disconnect(oldConnection_);

    if (focusedInputContext == 0) {
        return;
    }

    auto *ic = dim()->getFocusedIC(focusedInputContext);
    if (ic) {
        oldConnection_ = connect(ic, &InputContext::imSwitch, this, &TrayIcon::onImSwitched);
    }
}

void TrayIcon::onImSwitched(const std::pair<std::string, std::string> &imIndex)
{
    const auto &imEntries = dim()->imEntries();
    auto entryIter = std::find_if(imEntries.cbegin(),
                                  imEntries.cend(),
                                  [&imIndex](const InputMethodEntry &entry) {
                                      return entry.addonKey() == imIndex.first
                                          && entry.uniqueName() == imIndex.second;
                                  });
    if (entryIter == imEntries.cend()) {
        qDebug() << "invalid input method" << QString::fromStdString(imIndex.second);
        return;
    }

    imAddonIcon_ = QString::fromStdString(entryIter->label());
    qDebug() << "imAddonIcon" << imAddonIcon_;
    emit imAddonIconChanged(imAddonIcon_);
}

void const TrayIcon::updateTrayIconIMEntries()
{
    QQmlListProperty<TrayIconInputMethodEntry> list{ this, &trayIconIMEntries_ };
    emit imEntriesChanged(list);
}

QQmlListProperty<TrayIconInputMethodEntry> const TrayIcon::getIMEntries()
{
    return QQmlListProperty<TrayIconInputMethodEntry>{ this, &trayIconIMEntries_ };
}

void TrayIcon::imEntryMenuTriggered(const QString &addon, const QString &name)
{
    qDebug() << "imEntryMenuTriggered" << addon << name;
    dim()->requestSwitchIM(addon.toStdString(), name.toStdString());
}

void TrayIcon::configureTriggered()
{
    qDebug() << "configureTriggered";

    DDBusSender()
        .service("org.deepin.dde.ControlCenter1")
        .interface("org.deepin.dde.ControlCenter1")
        .path("/org/deepin/dde/ControlCenter1")
        .method(QString("ShowPage"))
        .arg(QString("keyboard/Manage Input Methods"))
        .call();
}

void TrayIcon::toggle()
{
    qDebug() << "toggle";
    dim()->toggle();
}

bool TrayIcon::loadTranslator()
{
#ifdef TRANSLATE_DIR
    const QString translateDir(TRANSLATE_DIR);
    return Dtk::Gui::DGuiApplicationHelper::loadTranslator("trayicon",
                                                           QStringList{ translateDir },
                                                           QList<QLocale>() << QLocale::system());
#endif
}
