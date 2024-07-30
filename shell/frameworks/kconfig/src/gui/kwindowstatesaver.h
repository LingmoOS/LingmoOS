/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KWINDOWSTATESAVER_H
#define KWINDOWSTATESAVER_H

#include <kconfiggroup.h>
#include <kconfiggui_export.h>

#include <QObject>

class QWindow;
class KWindowStateSaverPrivate;

/**
 * Saves and restores a window size and (when possible) position.
 *
 * This is useful for retrofitting persisting window geometry on existing windows or dialogs,
 * without having to modify those classes themselves, or having to inherit from them.
 * For this, create a new instance of KWindowStateSaver for every window that should have it's
 * state persisted, and pass it the window or widget as well as the config group the state
 * should be stored in. The KWindowStateSaver will restore an existing state and then monitor
 * the window for subsequent changes to persist. It will delete itself once the window is
 * deleted.
 *
 * @code
 * QPrintPreviewDialog dlg = ...
 * new KWindowStateSaver(&dlg, "printPreviewDialogState");
 * ...
 * dlg.exec();
 * @endcode
 *
 * Note that freshly created top-level QWidgets (such as the dialog in the above example)
 * do not have an associated QWindow yet (ie. windowHandle() return @c nullptr). KWindowStateSaver
 * supports this with its QWidget constructors which will monitor the widget for having
 * its associated QWindow created before continuing with that.
 *
 * When implementing your own windows/dialogs, using KWindowConfig directly can be an
 * alternative.
 *
 * @see KWindowConfig
 * @since 5.92
 */
class KCONFIGGUI_EXPORT KWindowStateSaver : public QObject
{
    Q_OBJECT
public:
    /**
     * Create a new window state saver for @p window.
     * @param configGroup A KConfigGroup that holds the window state.
     */
    explicit KWindowStateSaver(QWindow *window, const KConfigGroup &configGroup);
    /**
     * Create a new window state saver for @p window.
     * @param configGroupName The name of a KConfigGroup in the default state
     * configuration (see KSharedConfig::openStateConfig) that holds the window state.
     */
    explicit KWindowStateSaver(QWindow *window, const QString &configGroupName);

    /**
     * Create a new window state saver for @p widget.
     * Use this for widgets that aren't shown yet and would still return @c nullptr from windowHandle().
     * @param configGroup A KConfigGroup that holds the window state.
     */
    template<typename Widget>
    explicit inline KWindowStateSaver(Widget *widget, const KConfigGroup &configGroup);
    /**
     * Create a new window state saver for @p widget.
     * Use this for widgets that aren't shown yet and would still return @c nullptr from windowHandle().
     * @param configGroupName The name of a KConfigGroup in the default state
     * configuration (see KSharedConfig::openStateConfig) that holds the window state.
     */
    template<typename Widget>
    explicit inline KWindowStateSaver(Widget *widget, const QString &configGroupName);

    ~KWindowStateSaver();

private:
    void timerEvent(QTimerEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

    // API used by template code, so technically part of the ABI
    void initWidget(QObject *widget, const std::function<QWindow *()> &windowHandleCallback, const KConfigGroup &configGroup);
    void initWidget(QObject *widget, const std::function<QWindow *()> &windowHandleCallback, const QString &configGroupName);

    // cannot use std::unique_ptr due to the template ctors
    // not seeing the full private class
    KWindowStateSaverPrivate *d = nullptr;
};

template<typename Widget>
KWindowStateSaver::KWindowStateSaver(Widget *widget, const KConfigGroup &configGroup)
    : QObject(widget)
{
    initWidget(
        widget,
        [widget]() {
            return widget->windowHandle();
        },
        configGroup);
}

template<typename Widget>
KWindowStateSaver::KWindowStateSaver(Widget *widget, const QString &configGroupName)
    : QObject(widget)
{
    initWidget(
        widget,
        [widget]() {
            return widget->windowHandle();
        },
        configGroupName);
}

#endif // KWINDOWSTATESAVER_H
