/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOLORSCHEMEMANAGER_H
#define KCOLORSCHEMEMANAGER_H

#include <kconfigwidgets_export.h>

#include <QObject>
#include <memory>

class QAbstractItemModel;
class QModelIndex;
class QIcon;

class KActionMenu;
class KColorSchemeManagerPrivate;

/**
 * @class KColorSchemeManager kcolorschememanager.h KColorSchemeManager
 *
 * A small helper to get access to all available color schemes and activating a scheme in the
 * QApplication. This is useful for applications which want to provide a selection of custom color
 * schemes to their user. For example it is very common for photo and painting applications to use
 * a dark color scheme even if the default is a light scheme. Since version 5.67 it also allows
 * going back to following the system color scheme.
 *
 * The KColorSchemeManager provides access to a QAbstractItemModel which holds all the available
 * schemes. A possible usage looks like the following:
 *
 * @code
 * KColorSchemeManager *schemes = new KColorSchemeManager(this);
 * QListView *view = new QListView(this);
 * view->setModel(schemes->model());
 * connect(view, &QListView::activated, schemes, &KColorSchemeManager::activateScheme);
 * @endcode
 *
 * In addition the KColorSchemeManager also provides the possibility to create a KActionMenu populated
 * with all the available color schemes in an action group. If one of the actions is selected the
 * scheme is applied instantly:
 *
 * @code
 * QToolButton *button = new QToolButton();
 * KColorSchemeManager *schemes = new KColorSchemeManager(this);
 * KActionMenu *menu = schemes->createSchemeSelectionMenu(QStringLiteral("Oxygen"), button);
 * button->setMenu(menu->menu());
 * @endcode
 *
 * @since 5.0
 */
class KCONFIGWIDGETS_EXPORT KColorSchemeManager : public QObject
{
    Q_OBJECT
public:
    explicit KColorSchemeManager(QObject *parent = nullptr);
    ~KColorSchemeManager() override;

    /**
     * A QAbstractItemModel of all available color schemes.
     *
     * The model provides the name of the scheme in Qt::DisplayRole, a preview
     * in Qt::DelegateRole and the full path to the scheme file in Qt::UserRole. The system theme
     * has an empty Qt::UserRole.
     *
     * @return Model of all available color schemes.
     */
    QAbstractItemModel *model() const;
    /**
     * Returns the model index for the scheme with the given @p name. If no such
     * scheme exists an invalid index is returned. If you pass an empty
     * string the index that is equivalent to going back to following the system scheme is returned
     * for versions 5.67 and newer.
     * @see model
     */
    QModelIndex indexForScheme(const QString &name) const;

    /**
     * Creates a KActionMenu populated with all the available color schemes.
     * All actions are in an action group and when one of the actions is triggered the scheme
     * referenced by this action is activated.
     *
     * The color scheme with the same name as @p selectedSchemeName will be checked. If none
     * of the available color schemes has the same name, the system theme entry will be checked.
     *
     * The KActionMenu will not be updated in case the installed color schemes change. It's the
     * task of the user of the KActionMenu to monitor for changes if required.
     *
     * @param icon The icon to use for the KActionMenu
     * @param text The text to use for the KActionMenu
     * @param selectedSchemeName The name of the color scheme to select
     * @param parent The parent of the KActionMenu
     * @return KActionMenu populated with all available color schemes.
     * @see activateScheme
     */
    KActionMenu *createSchemeSelectionMenu(const QIcon &icon, const QString &text, const QString &selectedSchemeName, QObject *parent);
    /**
     * Overload for createSchemeSelectionMenu(const QIcon &icon, const QString &text, const QString &selectedSchemeName, QObject *parent).
     *
     * Since 5.67 sets the icon to theme id "preferences-desktop-color", before set a null icon.
     */
    KActionMenu *createSchemeSelectionMenu(const QString &text, const QString &selectedSchemeName, QObject *parent);
    /**
     * Overload for createSchemeSelectionMenu(const QIcon &icon, const QString &text, const QString &selectedSchemeName, QObject *parent).
     *
     * Since 5.67 sets the icon to theme id "preferences-desktop-color" and the text to "Color Scheme", before set a null icon and an empty string.
     */
    KActionMenu *createSchemeSelectionMenu(const QString &selectedSchemeName, QObject *parent);
    /**
     * Overload for createSchemeSelectionMenu(const QIcon &icon, const QString &text, const QString &selectedSchemeName, QObject *parent).
     *
     * Sets the icon to theme id "preferences-desktop-color" and the text to "Color Scheme".
     * Since 5.93 the selectedSchemeName is set to the value previously saved (if any).
     * Before that it was set to an empty string.
     * @since 5.67
     */
    KActionMenu *createSchemeSelectionMenu(QObject *parent);
    /**
     * Saves the color scheme to config file. The scheme is saved by default whenever it's changed.
     * Use this method when autosaving is turned off, see setAutosaveChanges().
     *
     * @since 5.89
     */
    void saveSchemeToConfigFile(const QString &schemeName) const;
    /**
     * Sets color scheme autosaving. Default value is @c true.
     * If this is set to @c false, the scheme is not going to be remembered when the
     * application is restarted.
     *
     * @param autosaveChanges Enables/Disables autosaving of the color scheme.
     * @since 5.89
     */
    void setAutosaveChanges(bool autosaveChanges);

public Q_SLOTS:
    /**
     * @brief Activates the KColorScheme identified by the provided @p index.
     *
     * Installs the KColorScheme as the QApplication's QPalette.
     *
     * @param index The index for the KColorScheme to activate.
     * The index must reference the QAbstractItemModel provided by @link model @endlink. Since
     * version 5.67 passing an invalid index activates the system scheme.
     * @see model()
     */
    void activateScheme(const QModelIndex &index);

private:
    std::unique_ptr<KColorSchemeManagerPrivate> const d;
};

#endif
