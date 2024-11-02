/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2024 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TOOLTIP_DIALOG_H
#define TOOLTIP_DIALOG_H

#include <QQuickWindow>
#include <QTimer>
#include <QVariant>
#include "windows/dialog.h"
#include "shared-engine-component.h"

class QQuickItem;

namespace LingmoUIQuick {
/**
 * Internally used by Tooltip
 */
class TooltipDialog : public LingmoUIQuick::Dialog
{
    Q_OBJECT

public:
    explicit TooltipDialog(QQuickItem *parent = nullptr);
    ~TooltipDialog() override;

    QQuickItem *loadDefaultItem();

    void dismiss();
    void keepalive();

    bool interactive();
    void setInteractive(bool interactive);

    int hideTimeout() const;
    void setHideTimeout(int timeout);

    /**
     * Basically the last one who has shown the dialog
     */
    QObject *owner() const;
    void setOwner(QObject *owner);
    bool posFollowCursor() const;
    void setPosFollowCursor(bool follow);
    void setMargin(int margin);

protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void resizeEvent(QResizeEvent *re) override;
    bool event(QEvent *e) override;
    QPoint popupPosition(QQuickItem *item, const QSize &size) override;

private Q_SLOTS:
    void valueChanged(const QVariant &value);

private:
    QPoint posByCursor();
    SharedEngineComponent *m_qmlObject = nullptr;
    QTimer *m_showTimer;
    int m_hideTimeout;
    bool m_interactive;
    /**
     * HACK: prevent tooltips from being incorrectly dismissed (BUG439522)
     */
    enum m_extendTimeoutFlags {
        None = 0x0,
        Resized = 0x1,
        Moved = 0x2,
    };
    int m_extendTimeoutFlag;
    QObject *m_owner;
    bool m_posFollowCursor;
};
}
#endif
