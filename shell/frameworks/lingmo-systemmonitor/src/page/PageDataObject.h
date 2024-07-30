/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <QQmlListProperty>
#include <QQmlPropertyMap>
#include <qqmlregistration.h>

#include <KSharedConfig>

class FaceLoader;

class PageDataObject : public QQmlPropertyMap
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<PageDataObject> children READ childrenProperty NOTIFY childrenChanged)
    Q_PROPERTY(bool dirty READ dirty NOTIFY dirtyChanged)
    Q_PROPERTY(QString fileName READ fileName CONSTANT)
    QML_ELEMENT
    QML_UNCREATABLE("Used for data storage")

public:
    explicit PageDataObject(const KSharedConfig::Ptr &config, QObject *parent = nullptr);

    QQmlListProperty<PageDataObject> childrenProperty() const;
    QList<PageDataObject *> children() const;

    KSharedConfig::Ptr config() const;

    void setValue(const QString &name, const QVariant &value);

    Q_INVOKABLE bool resetPage();
    Q_INVOKABLE bool savePage();
    Q_INVOKABLE void saveAs(const QUrl &destination);

    bool load(const KConfigBase &config, const QString &groupName);
    Q_SIGNAL void loaded();
    bool save(KConfigBase &config, const QString &groupName, const QStringList &ignoreProperties = {QStringLiteral("children")});
    Q_SIGNAL void saved();

    void reset();

    Q_INVOKABLE PageDataObject *insertChild(int index, const QVariantMap &properties);
    Q_INVOKABLE void removeChild(int index);
    Q_INVOKABLE void moveChild(int from, int to);

    PageDataObject *childAt(int index) const;
    int childCount() const;

    Q_SIGNAL void childrenChanged();
    Q_SIGNAL void childInserted(int index);
    Q_SIGNAL void childRemoved(int index);
    Q_SIGNAL void childMoved(int from, int to);

    bool dirty() const;
    Q_INVOKABLE void markDirty();
    Q_INVOKABLE void markClean();
    Q_SIGNAL void dirtyChanged();

    FaceLoader *faceLoader();
    void setFaceLoader(FaceLoader *faceLoader);

    QString fileName() const;

private:
    bool isGroupEmpty(const KConfigGroup &group);
    void updateNames();

    QQmlListProperty<PageDataObject> m_childrenProperty;
    QList<PageDataObject *> m_children;
    KSharedConfig::Ptr m_config;
    bool m_dirty = false;
    FaceLoader *m_faceLoader = nullptr;
};
