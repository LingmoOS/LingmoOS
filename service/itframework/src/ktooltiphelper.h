/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Felix Ernst <fe.a.ernst@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later OR BSD-2-Clause
*/

#ifndef KTOOLTIPHELPER_H
#define KTOOLTIPHELPER_H

#include <kxmlgui_export.h>

#include <QObject>

#include <memory>

class KToolTipHelperPrivate;

/**
 * @class KToolTipHelper ktooltiphelper.h KToolTipHelper
 *
 * @short An event filter used to enhance tooltips
 *
 * Example:
 * Without this class, a tooltip of a QToolButton of a "New" action will read something like
 * "New File". Using this class, the tooltip can be enhanced to read something like
 * "New File (Ctrl+N)" and in the next line smaller "Press Shift for help.".
 * Pressing Shift will open the "What's This" context help for that widget. If a hyperlink in
 * that help is clicked, the corresponding event will also be filtered by this class and open
 * the linked location.
 *
 * The extra text added to tooltips is only shown when available and where it makes sense. If a
 * QToolButton has no associated shortcut and an empty QWidget::whatsThis(), this class won't
 * tamper with the requested tooltip at all.
 *
 * This class also activates tooltips for actions in QMenus but only when it makes sense like when
 * the tooltip isn't equal to the already displayed text.
 *
 * If you want the "Press Shift for help." line to be displayed for a widget that has whatsThis()
 * but no toolTip() take a look at KToolTipHelper::whatsThisHintOnly().
 *
 * The enhanced tooltips can be enabled at any time after the QApplication was constructed with
 * \code
 * qApp->installEventFilter(KToolTipHelper::instance());
 * \endcode
 * Therefore, to de-activate them you can call
 * \code
 * qApp->removeEventFilter(KToolTipHelper::instance());
 * \endcode
 * any time later.
 *
 * If you want KToolTipHelper to not tamper with certain QEvents, e.g. you want to handle some
 * tooltips differently or you want to change what happens when a QWhatsThisClickedEvent is
 * processed, first remove KToolTipHelper as an event filter just like in the line of code above.
 * Then create your own custom EventFilter that handles those QEvents differently and for all
 * cases that you don't want to handle differently call
 * \code
 * return KToolTipHelper::instance()->eventFilter(watched, event);
 * \endcode
 *
 * KMainWindow will have this EventFilter installed by default from framework version 5.84 onward
 * so if you want to opt out of that, remove the EventFilter in the constructor of your MainWindow
 * class inheriting from KMainWindow.
 *
 * @see QToolTip
 * @since 5.84
 */
class KXMLGUI_EXPORT KToolTipHelper : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(KToolTipHelper)

public:
    static KToolTipHelper *instance();

    /**
     * Filters QEvent::ToolTip if an enhanced tooltip is available for the widget.
     * Filters the QEvent::KeyPress that is used to expand an expandable tooltip.
     * Filters QEvent::WhatsThisClicked so hyperlinks in whatsThis() texts work.
     *
     * @see QObject::eventFilter()
     * @see QHelpEvent
     */
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

    /**
     * Use this to have a widget show "Press Shift for help." as its tooltip.
     * \code
     * widget->setToolTip(KToolTipHelper::whatsThisHintOnly());
     * \endcode
     * KToolTipHelper won't show that tooltip if the widget's whatsThis() is empty.
     *
     * @return a QString that is interpreted by this class to show the expected tooltip.
     */
    static const QString whatsThisHintOnly();

private:
    explicit KToolTipHelper(QObject *parent);

    ~KToolTipHelper() override;

private:
    std::unique_ptr<KToolTipHelperPrivate> const d;

    friend class KToolTipHelperPrivate;
};

#endif // KTOOLTIPHELPER_H
