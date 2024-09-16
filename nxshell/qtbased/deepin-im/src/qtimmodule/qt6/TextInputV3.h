// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef DIM_TEXTINPUTV3_H
#define DIM_TEXTINPUTV3_H

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

#include "QWaylandInputMethodEventBuilder.h"
#include "QWaylandTextInputInterface.h"
#include "wl/client/ZwpTextInputV3.h"

#include <QLoggingCategory>

struct wl_callback;
struct wl_callback_listener;

Q_DECLARE_LOGGING_CATEGORY(qLcQpaWaylandTextInput)

class QWaylandDisplay;

class TextInputV3 : public wl::client::ZwpTextInputV3,
                    public QtWaylandClient::QWaylandTextInputInterface
{
public:
    explicit TextInputV3(struct ::zwp_text_input_v3 *text_input);
    ~TextInputV3() override;

    void reset() override;
    void commit() override;
    void updateState(Qt::InputMethodQueries queries, uint32_t flags) override;

    QRectF keyboardRect() const override;

    void enable() override;
    void disable() override;

protected:
    void zwp_text_input_v3_enter(struct wl_surface *surface) override;
    void zwp_text_input_v3_leave(struct wl_surface *surface) override;
    void zwp_text_input_v3_preedit_string(const char *text,
                                          int32_t cursor_begin,
                                          int32_t cursor_end) override;
    void zwp_text_input_v3_commit_string(const char *text) override;
    void zwp_text_input_v3_delete_surrounding_text(uint32_t before_length,
                                                   uint32_t after_length) override;
    void zwp_text_input_v3_done(uint32_t serial) override;

private:
    QWaylandInputMethodEventBuilder m_builder;

    wl_surface *m_surface = nullptr;

    struct PreeditInfo
    {
        QString text;
        int cursorBegin = 0;
        int cursorEnd = 0;

        void clear()
        {
            text.clear();
            cursorBegin = 0;
            cursorEnd = 0;
        }
    };

    PreeditInfo m_pendingPreeditString;
    PreeditInfo m_currentPreeditString;
    QString m_pendingCommitString;
    uint m_pendingDeleteBeforeText = 0;
    uint m_pendingDeleteAfterText = 0;

    QString m_surroundingText;
    int m_cursorPos = 0; // cursor position in wayland index
    int m_anchorPos = 0; // anchor position in wayland index
    uint32_t m_contentHint = 0;
    uint32_t m_contentPurpose = 0;
    QRect m_cursorRect;
    bool m_entered = false;

    uint m_currentSerial = 0;

    bool m_condReselection = false;
};

#endif // DIM_TEXTINPUTV3_H
