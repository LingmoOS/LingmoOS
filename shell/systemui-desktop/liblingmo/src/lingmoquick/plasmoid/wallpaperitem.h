/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WALLPAPERITEM_H
#define WALLPAPERITEM_H

#include <QQmlEngine>
#include <QQuickItem>

#include <KPackage/Package>

class KConfigLoader;
class KConfigPropertyMap;

class ContainmentItem;

namespace Lingmo
{
class Containment;
}

namespace LingmoQuick
{
class SharedQmlEngine;
}

/**
 * @brief This class is exposed to wallpapers as the WallpaperItem root qml item
 *
 * <b>Import Statement</b>
 * @code import org.kde.lingmo.plasmoid @endcode
 * @version 2.0
 */
class WallpaperItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QString pluginName READ pluginName CONSTANT)
    Q_PROPERTY(KConfigPropertyMap *configuration READ configuration CONSTANT)
    /**
     * Actions to be added in the desktop context menu. To instantiate QActions in a declarative way,
     * LingmoCore.Action {} can be used
     */
    Q_PROPERTY(QQmlListProperty<QAction> contextualActions READ qmlContextualActions NOTIFY contextualActionsChanged)
    Q_PROPERTY(bool loading MEMBER m_loading NOTIFY isLoadingChanged)

    /*
     * The accent color manually set by the wallpaper plugin.
     * By default it's transparent, which means either the dominant color is used
     * when "Accent Color From Wallpaper" is enabled, or the theme color is used.
     *
     * @since 6.0
     */
    Q_PROPERTY(QColor accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged RESET resetAccentColor)

public:
    explicit WallpaperItem(QQuickItem *parent = nullptr);
    ~WallpaperItem() override;

    void classBegin() override;
    void componentComplete() override;

    /**
     * Returns a list of all known wallpapers that can accept the given mimetype
     * @param mimetype the mimetype to search for
     * @param formFactor the format of the wallpaper being search for (e.g. desktop)
     * @return list of wallpapers
     */
    static QList<KPluginMetaData> listWallpaperMetadataForMimetype(const QString &mimetype, const QString &formFactor = QString());

    /**
     * Instantiate the WallpaperItem for a given containment, using the proper plugin
     */
    static WallpaperItem *loadWallpaper(ContainmentItem *ContainmentItem);

    KPackage::Package kPackage() const;

    QString pluginName() const;

    KConfigPropertyMap *configuration() const;

    KConfigLoader *configScheme();

    void requestOpenUrl(const QUrl &url);

    QList<QAction *> contextualActions() const;

    QML_LIST_PROPERTY_ASSIGN_BEHAVIOR_REPLACE
    QQmlListProperty<QAction> qmlContextualActions();

    bool supportsMimetype(const QString &mimetype) const;

    bool isLoading() const;

    QColor accentColor() const;
    void setAccentColor(const QColor &newColor);
    void resetAccentColor();

Q_SIGNALS:
    void isLoadingChanged();
    void openUrlRequested(const QUrl &url);
    void contextualActionsChanged(const QList<QAction *> &actions);
    void accentColorChanged();

private:
    static void contextualActions_append(QQmlListProperty<QAction> *prop, QAction *action);
    static qsizetype contextualActions_count(QQmlListProperty<QAction> *prop);
    static QAction *contextualActions_at(QQmlListProperty<QAction> *prop, qsizetype idx);
    static void contextualActions_clear(QQmlListProperty<QAction> *prop);
    static void contextualActions_replace(QQmlListProperty<QAction> *prop, qsizetype idx, QAction *action);
    static void contextualActions_removeLast(QQmlListProperty<QAction> *prop);

    QString m_wallpaperPlugin;
    Lingmo::Containment *m_containment = nullptr;
    LingmoQuick::SharedQmlEngine *m_qmlObject = nullptr;
    KPackage::Package m_pkg;
    KConfigPropertyMap *m_configuration = nullptr;
    KConfigLoader *m_configLoader = nullptr;
    QList<QAction *> m_contextualActions;
    bool m_loading = false;
    std::optional<QColor> m_accentColor;
};

#endif
