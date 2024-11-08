/*
    SPDX-FileCopyrightText: 2019 Mark Nauwelaerts <mark.nauwelaerts@gmail.com>
    SPDX-FileCopyrightText: 2019 Christoph Cullmann <cullmann@kde.org>

    SPDX-License-Identifier: MIT
*/

#pragma once

#include "lspclientplugin.h"
#include "lspclientservermanager.h"

#include <QObject>

class LSPClientSymbolView
{
public:
    // only needs a factory; no other public interface
    static LSPClientSymbolView *new_(LSPClientPlugin *plugin, KTextEditor::MainWindow *mainWin, std::shared_ptr<LSPClientServerManager> manager);

    virtual ~LSPClientSymbolView();

    virtual class QAbstractItemModel *documentSymbolsModel() = 0;
};

class LSPClientViewTracker : public QObject
{
    Q_OBJECT
public:
    // factory method; private implementation by interface
    static LSPClientViewTracker *new_(LSPClientPlugin *plugin, KTextEditor::MainWindow *mainWin, int change_ms, int motion_ms);

    enum State {
        ViewChanged,
        TextChanged,
        LineChanged,
    };

Q_SIGNALS:
    void newState(KTextEditor::View *, State);
};
