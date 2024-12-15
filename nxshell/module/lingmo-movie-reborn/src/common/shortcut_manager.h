// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef _DMR_SHORTCUT_MANAGER
#define _DMR_SHORTCUT_MANAGER 

#include <memory>
#include <vector>
#include <QtCore>
#include "actions.h"

#include <DSettingsOption>
#include <DSettingsGroup>
#include <DSettings>

namespace dmr {

using namespace std;

using BindingMap = QHash<QKeySequence, ActionFactory::ActionKind>;
using ActionMap = QHash<QString, ActionFactory::ActionKind>;

    // keys comes from profiles, user configurations etc
class ShortcutManager: public QObject {
    Q_OBJECT
    public:
        static ShortcutManager& get();
        virtual ~ShortcutManager();
        BindingMap& map() { return _map; }
        const BindingMap& map() const { return _map; }
        vector<QAction*> actionsForBindings();
        void buildBindingsFromSettings();
        QString toJson();

    public slots:
        void buildBindings();

    signals:
        void bindingsChanged();

    private:
        ShortcutManager();

        BindingMap _map;
        ActionMap _keyToAction;

        void toggleGroupShortcuts(Dtk::Core::GroupPtr grp, bool on);
};

}

#endif /* ifndef _DMR_SHORTCUT_MANAGER */

