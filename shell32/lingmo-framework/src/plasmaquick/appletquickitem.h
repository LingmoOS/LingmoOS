/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef APPLETQUICKITEM_H
#define APPLETQUICKITEM_H

#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QTimer>

#include <plasmaquick/plasmaquick_export.h>
#if PLASMAQUICK_ENABLE_DEPRECATED_SINCE(5, 94)
#include <Plasma/Package>
#endif

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public Plasma API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace Plasma
{
class Applet;
}

namespace KDeclarative
{
class QmlObject;
}

namespace PlasmaQuick
{
class AppletQuickItemPrivate;

class PLASMAQUICK_EXPORT AppletQuickItem : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(int switchWidth READ switchWidth WRITE setSwitchWidth NOTIFY switchWidthChanged)
    Q_PROPERTY(int switchHeight READ switchHeight WRITE setSwitchHeight NOTIFY switchHeightChanged)

    Q_PROPERTY(QQmlComponent *compactRepresentation READ compactRepresentation WRITE setCompactRepresentation NOTIFY compactRepresentationChanged)
    Q_PROPERTY(QQuickItem *compactRepresentationItem READ compactRepresentationItem NOTIFY compactRepresentationItemChanged)

    Q_PROPERTY(QQmlComponent *fullRepresentation READ fullRepresentation WRITE setFullRepresentation NOTIFY fullRepresentationChanged)
    Q_PROPERTY(QQuickItem *fullRepresentationItem READ fullRepresentationItem NOTIFY fullRepresentationItemChanged)

    /**
     * this is supposed to be either one between compactRepresentation or fullRepresentation
     */
    Q_PROPERTY(QQmlComponent *preferredRepresentation READ preferredRepresentation WRITE setPreferredRepresentation NOTIFY preferredRepresentationChanged)

    /**
     * True when the applet is showing its full representation. either as the main only view, or in a popup.
     * Setting it will open or close the popup if the plasmoid is iconified, however it won't have effect if the applet is open
     */
    Q_PROPERTY(bool expanded WRITE setExpanded READ isExpanded NOTIFY expandedChanged)

    /**
     * True when the applet wants the activation signal act in toggle mode, i.e. while being expanded
     * the signal shrinks the applet to its not expanded state instead of reexpanding it.
     */
    Q_PROPERTY(bool activationTogglesExpanded WRITE setActivationTogglesExpanded READ isActivationTogglesExpanded NOTIFY activationTogglesExpandedChanged)

    /**
     * the applet root QML item: sometimes is the same as fullRepresentationItem
     * if a fullrepresentation was not declared explicitly
     */
    Q_PROPERTY(QObject *rootItem READ rootItem CONSTANT)

public:
    AppletQuickItem(Plasma::Applet *applet, QQuickItem *parent = nullptr);
    ~AppletQuickItem() override;

    ////API NOT SUPPOSED TO BE USED BY QML
    Plasma::Applet *applet() const;

    // Make the constructor lighter and delay the actual instantiation of the qml in the applet
    virtual void init();

#if PLASMAQUICK_ENABLE_DEPRECATED_SINCE(5, 36)
    PLASMAQUICK_DEPRECATED_VERSION(5, 36, "No longer use")
    Plasma::Package appletPackage() const;
    PLASMAQUICK_DEPRECATED_VERSION(5, 36, "No longer use")
    void setAppletPackage(const Plasma::Package &package);

    PLASMAQUICK_DEPRECATED_VERSION(5, 36, "No longer use")
    Plasma::Package coronaPackage() const;
    PLASMAQUICK_DEPRECATED_VERSION(5, 36, "No longer use")
    void setCoronaPackage(const Plasma::Package &package);
#endif

    QQuickItem *compactRepresentationItem();
    QQuickItem *fullRepresentationItem();
    QObject *rootItem();
    QObject *testItem();

    ////PROPERTY ACCESSORS
    int switchWidth() const;
    void setSwitchWidth(int width);

    int switchHeight() const;
    void setSwitchHeight(int width);

    QQmlComponent *compactRepresentation();
    void setCompactRepresentation(QQmlComponent *component);

    QQmlComponent *fullRepresentation();
    void setFullRepresentation(QQmlComponent *component);

    QQmlComponent *preferredRepresentation();
    void setPreferredRepresentation(QQmlComponent *component);

    bool isExpanded() const;
    void setExpanded(bool expanded);

    bool isActivationTogglesExpanded() const;
    void setActivationTogglesExpanded(bool activationTogglesExpanded);

    ////NEEDED BY QML TO CREATE ATTACHED PROPERTIES
    static AppletQuickItem *qmlAttachedProperties(QObject *object);

Q_SIGNALS:
    // Property signals
    void switchWidthChanged(int width);
    void switchHeightChanged(int height);

    void expandedChanged(bool expanded);
    void activationTogglesExpandedChanged(bool activationTogglesExpanded);

    void compactRepresentationChanged(QQmlComponent *compactRepresentation);
    void fullRepresentationChanged(QQmlComponent *fullRepresentation);
    void preferredRepresentationChanged(QQmlComponent *preferredRepresentation);

    void compactRepresentationItemChanged(QObject *compactRepresentationItem);
    void fullRepresentationItemChanged(QObject *fullRepresentationItem);

protected:
    KDeclarative::QmlObject *qmlObject();

    // Reimplementation
    void childEvent(QChildEvent *event) override;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
#else
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
#endif
    void itemChange(ItemChange change, const ItemChangeData &value) override;

private:
    AppletQuickItemPrivate *const d;

    Q_PRIVATE_SLOT(d, void minimumWidthChanged())
    Q_PRIVATE_SLOT(d, void minimumHeightChanged())
    Q_PRIVATE_SLOT(d, void preferredWidthChanged())
    Q_PRIVATE_SLOT(d, void preferredHeightChanged())
    Q_PRIVATE_SLOT(d, void maximumWidthChanged())
    Q_PRIVATE_SLOT(d, void maximumHeightChanged())
    Q_PRIVATE_SLOT(d, void fillWidthChanged())
    Q_PRIVATE_SLOT(d, void fillHeightChanged())
};

}

QML_DECLARE_TYPEINFO(PlasmaQuick::AppletQuickItem, QML_HAS_ATTACHED_PROPERTIES)

#endif
