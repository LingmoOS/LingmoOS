// Copyright (C) 2020 ~ 2021, Deepin Technology Co., Ltd. <support@deepin.org>
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shortcut_manager.h"
#include "dmr_settings.h"

#include <DSettingsOption>
#include <DSettingsGroup>
#include <DSettings>

namespace dmr {
using namespace Dtk::Core;
static ShortcutManager *_shortcutManager = nullptr;

ShortcutManager &ShortcutManager::get()
{
    if (!_shortcutManager) {
        _shortcutManager = new ShortcutManager();
    }
    return *_shortcutManager;
}
ShortcutManager::~ShortcutManager()
{
}
ShortcutManager::ShortcutManager()
    : QObject(nullptr), _keyToAction({
    {"pause_play", ActionFactory::ActionKind::TogglePause},
    {"seek_forward", ActionFactory::ActionKind::SeekForward},
    {"seek_forward_large", ActionFactory::ActionKind::SeekForwardLarge},
    {"seek_backward", ActionFactory::ActionKind::SeekBackward},
    {"seek_backward_large", ActionFactory::ActionKind::SeekBackwardLarge},
    {"fullscreen", ActionFactory::ActionKind::ToggleFullscreen},
    {"exitfullscreen", ActionFactory::ActionKind::QuitFullscreen},
    {"playlist", ActionFactory::ActionKind::TogglePlaylist},
    {"accel", ActionFactory::ActionKind::AccelPlayback},
    {"decel", ActionFactory::ActionKind::DecelPlayback},
    {"reset", ActionFactory::ActionKind::ResetPlayback},
    {"delete_from_playlist", ActionFactory::ActionKind::PlaylistRemoveItem},
    {"movie_info", ActionFactory::ActionKind::MovieInfo},

    {"mini", ActionFactory::ActionKind::ToggleMiniMode},
    {"vol_up", ActionFactory::ActionKind::VolumeUp},
    {"vol_down", ActionFactory::ActionKind::VolumeDown},
    {"mute", ActionFactory::ActionKind::ToggleMute},

    {"open_file", ActionFactory::ActionKind::OpenFileList},
    {"playlist_next", ActionFactory::ActionKind::GotoPlaylistNext},
    {"playlist_prev", ActionFactory::ActionKind::GotoPlaylistPrev},

    {"sub_forward", ActionFactory::ActionKind::SubForward},
    {"sub_backward", ActionFactory::ActionKind::SubDelay},

    {"screenshot", ActionFactory::ActionKind::Screenshot},
    {"burst_screenshot", ActionFactory::ActionKind::BurstScreenshot},

    {"next_frame", ActionFactory::ActionKind::NextFrame},
    {"previous_frame", ActionFactory::ActionKind::PreviousFrame},
})
{
    connect(&Settings::get(), &Settings::shortcutsChanged, [ = ](QString sk, const QVariant & val) {
        if (sk.endsWith(".enable")) {
            auto grp_key = sk.left(sk.lastIndexOf('.'));
            qInfo() << "update group binding" << grp_key;

            QPointer<DSettingsGroup> shortcuts = Settings::get().shortcuts();
            auto grps = shortcuts->childGroups();
            auto grp_ptr = std::find_if(grps.begin(), grps.end(), [ = ](GroupPtr grp) {
                return grp->key() == grp_key;
            });

            toggleGroupShortcuts(*grp_ptr, val.toBool());
            emit bindingsChanged();
            return;
        }
        sk.remove(0, sk.lastIndexOf('.') + 1);
        qInfo() << "update binding" << sk << QKeySequence(val.toStringList().at(0));
        QString strKey = QKeySequence(val.toStringList().at(0)).toString();
        if (strKey.contains("Return")) {
            strKey = QString("%1Return").arg(strKey.remove("Return"));
            _map[strKey] = _keyToAction[sk];
            if (QString("Return") == strKey) {
                _map.remove(QString("Enter"));
            }
            qInfo() << val << QKeySequence(strKey) << strKey;
        } else if (strKey.contains("Num+Enter")) {
            strKey = QString("%1Enter").arg(strKey.remove("Num+Enter"));
            _map[strKey] = _keyToAction[sk];
            if (QString("Enter") == strKey) {
                _map.remove(QString("Return"));
            }
            qInfo() << val << QKeySequence(strKey) << strKey;
        } else {
            _map.remove(_map.key(_keyToAction[sk]));
            _map[QKeySequence(val.toStringList().at(0))] = _keyToAction[sk];
        }
        emit bindingsChanged();
    });
}

void ShortcutManager::buildBindings()
{
    buildBindingsFromSettings();
    emit bindingsChanged();
}

void ShortcutManager::toggleGroupShortcuts(GroupPtr grp, bool on)
{
    auto sub = grp->childOptions();
    std::for_each(sub.begin(), sub.end(), [ = ](OptionPtr opt) {
        if (opt->viewType() != "shortcut") return;

        QString sk = opt->key();
        sk.remove(0, sk.lastIndexOf('.') + 1);
        QString strKey = QKeySequence(opt->value().toStringList().at(0)).toString();

        if (strKey.contains("Return")) {
            strKey = QString("%1Return").arg(strKey.remove("Return"));
            _map[strKey] = _keyToAction[sk];
            qInfo() << opt->name() << QKeySequence(strKey) << strKey;
        } else if (strKey.contains("Num+Enter")) {
            strKey = QString("%1Enter").arg(strKey.remove("Num+Enter"));
            _map[strKey] = _keyToAction[sk];
            qInfo() << opt->name() << QKeySequence(strKey) << strKey;
        }

        if (on) {
            _map[strKey] = _keyToAction[sk];
            _map[QKeySequence(opt->value().toStringList().at(0))] = _keyToAction[sk];
        } else {
            _map.remove(QKeySequence(opt->value().toStringList().at(0)));
        }
    });
}

void ShortcutManager::buildBindingsFromSettings()
{
    _map.clear();
    _map.insert(QKeySequence(Qt::Key_Slash + Qt::CTRL + Qt::SHIFT), ActionFactory::ViewShortcut);

    QPointer<DSettingsGroup> shortcuts = Settings::get().shortcuts();

    auto subgroups = shortcuts->childGroups();
    std::for_each(subgroups.begin(), subgroups.end(), [ = ](GroupPtr grp) {
        auto enabled = Settings::get().settings()->option(grp->key() + ".enable");
        qInfo() << grp->name() << enabled->value();
        Q_ASSERT(enabled && enabled->viewType() == "checkbox");
        if (!enabled->value().toBool())
            return;

        toggleGroupShortcuts(grp, true);
    });
}

QString ShortcutManager::toJson()
{
    QJsonObject shortcutObj;
    QJsonArray jsonGroups;

    QPointer<DSettingsGroup> shortcuts = Settings::get().shortcuts();
    auto subgroups = shortcuts->childGroups();
    std::for_each(subgroups.begin(), subgroups.end(), [&](GroupPtr grp) {
        qInfo() << grp->name();

        QJsonObject jsonGroup;
        jsonGroup.insert("groupName", qApp->translate("QObject", grp->name().toUtf8().data()));
        QJsonArray jsonItems;

        auto sub = grp->childOptions();
        std::for_each(sub.begin(), sub.end(), [&](OptionPtr opt) {
            if (opt->viewType() != "shortcut") return;

            QStringList keyseqs = opt->value().toStringList();
            QJsonObject jsonItem;
            jsonItem.insert("name", qApp->translate("QObject", opt->name().toUtf8().data()));
            jsonItem.insert("value", QKeySequence(opt->value().toStringList().at(0)).toString(QKeySequence::PortableText));
            jsonItems.append(jsonItem);
        });

        jsonGroup.insert("groupItems", jsonItems);
        jsonGroups.append(jsonGroup);
    });

    QJsonObject jsonGroup;
    jsonGroup.insert("groupName", qApp->translate("QObject", "Settings"));
    QJsonArray jsonItems;
    QJsonObject jsonItem;
    jsonItem.insert("name", qApp->translate("QObject", "Help"));
    jsonItem.insert("value", "F1");
    jsonItems.append(jsonItem);

    QJsonObject jsonItem_show;
    jsonItem.insert("name", qApp->translate("QObject", "Display shortcuts"));
    jsonItem.insert("value", "Ctrl+Shift+?");
    jsonItems.append(jsonItem);
    jsonGroup.insert("groupItems", jsonItems);
    jsonGroups.append(jsonGroup);

    shortcutObj.insert("shortcut", jsonGroups);

    QJsonDocument doc(shortcutObj);
    return doc.toJson().data();
}

vector<QAction *> ShortcutManager::actionsForBindings()
{
    vector<QAction *> actions;
    auto p = _map.constBegin();
    while (p != _map.constEnd()) {
        auto *act = new QAction(this);
        switch (p.value()) {
        case ActionFactory::ActionKind::SeekForward:
        case ActionFactory::ActionKind::SeekForwardLarge:
        case ActionFactory::ActionKind::SeekBackward:
        case ActionFactory::ActionKind::SeekBackwardLarge:
        case ActionFactory::ActionKind::VolumeUp:
        case ActionFactory::ActionKind::VolumeDown:
        case ActionFactory::ActionKind::AccelPlayback:
        case ActionFactory::ActionKind::DecelPlayback:
            act->setAutoRepeat(true);
            break;
        default:
            act->setAutoRepeat(false);
            break;
        }
        act->setShortcut(p.key());
        //act->setShortcutContext(Qt::ApplicationShortcut);
        act->setProperty("kind", p.value());
        act->setProperty("origin", "shortcut");
        actions.push_back(act);
        ++p;
    }
    return actions;
}
}

