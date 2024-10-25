/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LINGMOQUICKCONTAINMENTVIEW_H
#define LINGMOQUICKCONTAINMENTVIEW_H

#include "lingmo/containment.h"
#include "lingmo/corona.h"
#include <lingmoquick/lingmoquick_export.h>
#include <lingmoquick/quickviewsharedengine.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the public Lingmo API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

namespace LingmoQuick
{
class ContainmentViewPrivate;

class LINGMOQUICK_EXPORT ContainmentView : public LingmoQuick::QuickViewSharedEngine
{
    Q_OBJECT
    Q_PROPERTY(Lingmo::Types::Location location READ location WRITE setLocation NOTIFY locationChanged)
    Q_PROPERTY(Lingmo::Types::FormFactor formFactor READ formFactor NOTIFY formFactorChanged)
    Q_PROPERTY(QRectF screenGeometry READ screenGeometry NOTIFY screenGeometryChanged)

public:
    /**
     * @param corona the corona of this view
     * @param parent the QWindow this ContainmentView is parented to
     **/
    explicit ContainmentView(Lingmo::Corona *corona, QWindow *parent = nullptr);
    ~ContainmentView() override;

    /**
     * Unassign any containment UI from this view, then delete it
     */
    void destroy();

    /**
     * @return the corona of this view
     **/
    Lingmo::Corona *corona() const;

    /**
     * @return the KConfigGroup of this view
     **/
    virtual KConfigGroup config() const;

    /**
     * sets the containment for this view
     * @param cont the containment of this view
     **/
    void setContainment(Lingmo::Containment *cont);

    /**
     * @return the containment of this ContainmentView
     **/
    Lingmo::Containment *containment() const;

    /**
     * @return the location of this ContainmentView
     **/
    Lingmo::Types::Location location() const;

    /**
     * Sets the location of the ContainmentView
     * @param location the location of the ContainmentView
     **/
    void setLocation(Lingmo::Types::Location location);

    /**
     * @return the formfactor of the ContainmentView
     **/
    Lingmo::Types::FormFactor formFactor() const;

    /**
     * @return the screenGeometry of the ContainmentView
     **/
    QRectF screenGeometry();

protected Q_SLOTS:
    /**
     * It will be called when the configuration is requested
     */
    virtual void showConfigurationInterface(Lingmo::Applet *applet);

Q_SIGNALS:
    /**
     * emitted when the location is changed
     **/
    void locationChanged(Lingmo::Types::Location location);

    /**
     * emitted when the formfactor is changed
     **/
    void formFactorChanged(Lingmo::Types::FormFactor formFactor);

    /**
     * emitted when the containment is changed
     **/
    void containmentChanged();

    /**
     * emitted when the screenGeometry is changed
     **/
    void screenGeometryChanged();

private:
    ContainmentViewPrivate *const d;
    Q_PRIVATE_SLOT(d, void updateDestroyed(bool))
    friend class ContainmentViewPrivate;
};

}

#endif // CONTAINMENTVIEW_H
