// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef DIMPLATFORMINPUTCONTEXT_H
#define DIMPLATFORMINPUTCONTEXT_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "common/common.h"
#include "wl/client/ConnectionBase.h"

#include <qpa/qplatforminputcontext.h>
#include <xkbcommon/xkbcommon-compose.h>

#include <QPointer>

namespace QtWaylandClient {
class QWaylandTextInputInterface;
}

class XdgSurface;
class XdgToplevel;
class Keyboard;

class DIMPlatformInputContext : public QPlatformInputContext
{
    Q_OBJECT

public:
    explicit DIMPlatformInputContext();
    ~DIMPlatformInputContext() override;

    bool isValid() const override;

    void reset() override;
    void commit() override;
    void update(Qt::InputMethodQueries) override;

    void invokeAction(QInputMethod::Action, int cursorPosition) override;

    void showInputPanel() override;
    void hideInputPanel() override;
    bool isInputPanelVisible() const override;
    QRectF keyboardRect() const override;

    void setFocusObject(QObject *object) override;

    bool filterEvent(const QEvent *event) override;

private:
    std::shared_ptr<XdgSurface> xdgSurface_;
    std::shared_ptr<XdgToplevel> xdgToplevel_;
    std::shared_ptr<Keyboard> keyboard_;
    std::shared_ptr<QtWaylandClient::QWaylandTextInputInterface> textInput_;
    const std::shared_ptr<QtWaylandClient::QWaylandTextInputInterface> &textInput() const;

    std::unique_ptr<wl::client::ConnectionBase> wl_;

    void ensureInitialized();

    bool m_initialized = false;
    QObject *m_focusObject = nullptr;
    xkb_compose_table *m_composeTable = nullptr;
    xkb_compose_state *m_composeState = nullptr;
    std::unique_ptr<xkb_context, Deleter<xkb_context_unref>> m_XkbContext;
};

#endif // DIMPLATFORMINPUTCONTEXT_H
