/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyher@yahoo.com>

    based on ideas from Martijn and Simon

    SPDX-License-Identifier: LGPL-2.0-only

    Many thanks to Simon tronical Hausmann
*/

#ifndef kguiitem_h
#define kguiitem_h

#include <kwidgetsaddons_export.h>

#include <QIcon>
#include <QSharedDataPointer>
#include <QString>

class QPushButton;

/**
 * @class KGuiItem kguiitem.h KGuiItem
 *
 * @short An abstract class for setting the text, icon, tooltip and WhatsThis data
 * on a GUI item (e.g.\ a QPushButton).
 *
 * @author Holger Freyther <freyher@yahoo.com>
 * @see KStandardGuiItem
 */

class KWIDGETSADDONS_EXPORT KGuiItem
{
public:
    /**
     * Constructs an empty KGuiItem. You can use the various methods provided by
     * this class to set the text, icon... etc.
     */
    KGuiItem();

    // This is explicit because it's easy to get subtle bugs otherwise. The
    // icon name, tooltip and whatsthis text get changed behind your back if
    // you do 'setButtonFoo( "Bar" );' It gives the wrong impression that you
    // just change the text.
    /**
     * Constructs a KGuiItem with the provided arguments.
     *
     * @param text the text to use with the GUI item
     * @param iconName the name of the icon to display next to the text on the item;
     *             QIcon::fromTheme() is used to get a icon with that name from
     *             the icon themes available on the system
     * @param tooltip the tooltip to use for this item
     * @param whatsThis the text to use for the WhatThis help message
     */
    explicit KGuiItem(const QString &text, const QString &iconName = QString(), const QString &toolTip = QString(), const QString &whatsThis = QString());
    /**
     * Constructs a KGuiItem with the provided arguments.
     *
     * @param text the text to use with the GUI item
     * @param icon the QIcon object used to get an icon to display next to the text
     *         on this item
     * @param tooltip the tooltip to use for this item
     * @param whatsThis the text to use for the WhatThis help message
     */
    KGuiItem(const QString &text, const QIcon &icon, const QString &toolTip = QString(), const QString &whatsThis = QString());

    /**
     * Constructs a copy of @p other.
     */
    KGuiItem(const KGuiItem &other);

    /**
     * Assigns @p other to this KGuiItem object and returns a reference to this object.
     */
    KGuiItem &operator=(const KGuiItem &other);

    /**
     * Destructor.
     */
    ~KGuiItem();

    /**
     * Sets the text to use for this GUI item.
     */
    void setText(const QString &text);

    /**
     * Returns the text used by this GUI item.
     *
     * This may contain '&' characters which denote a keyboard accelerator shortcut that
     * can be used to invoke the GUI item, e.g. Alt + 'O' for button "&OK".
     * (Note that the '&' is not visible to the user).
     *
     * You can get the plain text without the accelerator denoting character '&', by
     * using plainText().
     *
     */
    QString text() const;

    /**
     * Returns the text used by this GUI item after stripping all existing '&'
     * characters which denote keyboard accelerators.
     *
     * @see text()
     */
    QString plainText() const;

    /**
     * Sets the icon to be shown next to the text of this GUI item.
     */
    void setIcon(const QIcon &iconset);

    /**
     * Returns the icon used by this GUI item.
     *
     * This will return a null QIcon if no icon was previously set for this item.
     */
    QIcon icon() const;

    /**
     * Sets the name of the icon that will be shown next to the text of this
     * GUI item. The actual QIcon will be obtained by using QIcon::fromTheme().
     */
    void setIconName(const QString &iconName);

    /**
     * Returns the name of the icon used by this GUI item.
     *
     * This will return an empty string if no icon was previously set for this item.
     */
    QString iconName() const;

    /**
     * Returns @c true if this GUI item has an icon set for it and @c false otherwise.
     */
    bool hasIcon() const;

    /**
     * Sets the tooltip text.
     */
    void setToolTip(const QString &tooltip);

    /**
     * Returns the tooltip used for this GUI item.
     *
     * This will return an empty string if no tooltip was previously set for this item.
     */
    QString toolTip() const;

    /**
     * Sets the WhatThis text.
     */
    void setWhatsThis(const QString &whatsThis);

    /**
     * Returns the WhatThis text used for this GUI item.
     *
     * This will return an empty string if no WhatThis text was previously set for
     * this item.
     */
    QString whatsThis() const;

    /**
     * Toggles the enabled property of this GUI item.
     *
     * @see QWidget::setEnabled()
     */
    void setEnabled(bool enable);

    /**
     * Returns @c true if this GUI item is enabled and @c false otherwise.
     *
     * @see QWidget::isEnabled()
     */
    bool isEnabled() const;

    /**
     * A static method that can be used to set the text, icon, tooltip and WhatThis
     * properties from @p item on @p button.
     *
     * @code
     * // Create a QDialogButtonBox with two buttons, of Yes and No standard type
     * auto *buttonBox = new QDialogButtonBox({QDialogButtonBox::Yes | QDialogButtonBox::No}, this);
     *
     * // Assign the text and icon from KStandardGuiItem::quit()/continue() to the buttons in the
     * // button dialog box
     * KGuiItem::assign(buttonBox->button(QDialogButtonBox::Yes), KStandardGuiItem::quit());
     * KGuiItem::assign(buttonBox->button(QDialogButtonBox::No), KStandardGuiItem::continue());
     * @endcode
     */
    static void assign(QPushButton *button, const KGuiItem &item);

private:
    QSharedDataPointer<class KGuiItemPrivate> d;
};

#endif
