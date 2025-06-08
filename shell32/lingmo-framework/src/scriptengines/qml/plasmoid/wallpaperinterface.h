/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WALLPAPERINTERFACE_H
#define WALLPAPERINTERFACE_H

#include <QQmlEngine>
#include <QQuickItem>

#include <KPackage/Package>
#include <kdeclarative/kdeclarative_export.h>

class KConfigLoader;
class KActionCollection;
class KConfigPropertyMap;

class ContainmentInterface;

namespace KDeclarative
{
#if KDECLARATIVE_BUILD_DEPRECATED_SINCE(5, 89)
class ConfigPropertyMap;
#endif
class QmlObject;
}

/**
 * @class WallpaperInterface
 *
 * @brief This class is exposed to wallpapers as the attached property "wallpaper"
 *
 *
 */
class WallpaperInterface : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QString pluginName READ pluginName NOTIFY packageChanged)
#if KDECLARATIVE_BUILD_DEPRECATED_SINCE(5, 89)
    Q_PROPERTY(KDeclarative::ConfigPropertyMap *configuration READ configuration NOTIFY configurationChanged)
#else
    Q_PROPERTY(KConfigPropertyMap *configuration READ configuration NOTIFY configurationChanged)
#endif
    Q_PROPERTY(bool loading MEMBER m_loading NOTIFY isLoadingChanged)

public:
    explicit WallpaperInterface(ContainmentInterface *parent = nullptr);
    ~WallpaperInterface() override;

    /**
     * Returns a list of all known wallpapers that can accept the given mimetype
     * @param mimetype the mimetype to search for
     * @param formFactor the format of the wallpaper being search for (e.g. desktop)
     * @return list of wallpapers
     */
    static QList<KPluginMetaData> listWallpaperMetadataForMimetype(const QString &mimetype, const QString &formFactor = QString());

    KPackage::Package kPackage() const;

    QString pluginName() const;

#if KDECLARATIVE_BUILD_DEPRECATED_SINCE(5, 89)
    KDeclarative::ConfigPropertyMap *configuration() const;
#else
    KConfigPropertyMap *configuration() const;
#endif

    KConfigLoader *configScheme();

    QList<QAction *> contextualActions() const;

    bool supportsMimetype(const QString &mimetype) const;

    void setUrl(const QUrl &urls);

    Q_INVOKABLE void setAction(const QString &name, const QString &text, const QString &icon = QString(), const QString &shortcut = QString());

    Q_INVOKABLE void removeAction(const QString &name);

    Q_INVOKABLE QAction *action(QString name) const;

    static WallpaperInterface *qmlAttachedProperties(QObject *object);

    bool isLoading() const;

Q_SIGNALS:
    void packageChanged();
    void configurationChanged();
    void isLoadingChanged();
    void repaintNeeded(const QColor &accentColor = Qt::transparent);

private Q_SLOTS:
    void syncWallpaperPackage();
    void executeAction(const QString &name);
    void loadFinished();

private:
    QString m_wallpaperPlugin;
    ContainmentInterface *m_containmentInterface;
    KDeclarative::QmlObject *m_qmlObject;
    KPackage::Package m_pkg;
#if KDECLARATIVE_BUILD_DEPRECATED_SINCE(5, 89)
    KDeclarative::ConfigPropertyMap *m_configuration;
#else
    KConfigPropertyMap *m_configuration;
#endif
    KConfigLoader *m_configLoader;
    KActionCollection *m_actions;
    bool m_loading = false;

    static QHash<QObject *, WallpaperInterface *> s_rootObjects;
};

QML_DECLARE_TYPEINFO(WallpaperInterface, QML_HAS_ATTACHED_PROPERTIES)

#endif
