// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once 

#include "dsglobal.h"

#include <QObject>
#include <QScreen>
#include <QWindow>
#include <QLoggingCategory>

DS_BEGIN_NAMESPACE

class DLayerShellWindowPrivate;

class DS_SHARE DLayerShellWindow : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Anchors anchors READ anchors WRITE setAnchors NOTIFY anchorsChanged)
    Q_PROPERTY(QString scope READ scope WRITE setScope)
    Q_PROPERTY(int leftMargin READ leftMargin WRITE setLeftMargin NOTIFY marginsChanged)
    Q_PROPERTY(int rightMargin READ rightMargin WRITE setRightMargin NOTIFY marginsChanged)
    Q_PROPERTY(int topMargin READ topMargin WRITE setTopMargin NOTIFY marginsChanged)
    Q_PROPERTY(int bottomMargin READ bottomMargin WRITE setBottomMargin NOTIFY marginsChanged)
    Q_PROPERTY(int32_t exclusionZone READ exclusionZone WRITE setExclusiveZone NOTIFY exclusionZoneChanged)
    Q_PROPERTY(Layer layer READ layer WRITE setLayer NOTIFY layerChanged)
    Q_PROPERTY(KeyboardInteractivity keyboardInteractivity READ keyboardInteractivity WRITE setKeyboardInteractivity NOTIFY keyboardInteractivityChanged)
    Q_PROPERTY(ScreenConfiguration screenConfiguration READ screenConfiguration WRITE setScreenConfiguration)
    
    Q_PROPERTY(bool closeOnDismissed READ closeOnDismissed WRITE setCloseOnDismissed)

public:
    ~DLayerShellWindow() override;

    /**
     * This enum type is used to specify the position where to anchor
     */
    enum Anchor {
        AnchorNone      = 0,
        AnchorTop       = 1,
        AnchorBottom    = 2,
        AnchorLeft      = 4,
        AnchorRight     = 8
    };

    Q_ENUM(Anchor)
    Q_DECLARE_FLAGS(Anchors, Anchor)

    /**
     * This enum type is used to specify the layer where a surface can be put in.
     */
    enum Layer {
        LayerBackground = 0,
        LayerButtom     = 1,
        LayerTop        = 2,
        LayerOverlay    = 3,
    };

    Q_ENUM(Layer)

    /**
     * This enum type is used to specify how the layer surface handles keyboard focus.
     */
    enum KeyboardInteractivity {
        // this surface is not interested in keyboard events and the compositor should never assign it the keyboard focus.
        KeyboardInteractivityNone = 0,
        // Request exclusive keyboard focus if this surface is above the shell surface layer.
        KeyboardInteractivityExclusive = 1,
        // This requests the compositor to allow this surface to be focused and unfocused by the user in an implementation-defined manner.
        KeyboardInteractivityOnDemand = 2,
    };

    /**
     * This enum type is used to specify which screen to place the surface on.
     * ScreenFromQWindow (the default) reads QWindow::screen() while ScreenFromCompositor
     * passes nil and lets the compositor decide.
     */
    enum ScreenConfiguration {
        ScreenFromQWindow = 0,
        ScreenFromCompositor = 1,
    };

    Q_ENUM(KeyboardInteractivity)

    void setAnchors(Anchors anchor);
    Anchors anchors() const;

    void setExclusiveZone(int32_t zone);
    int32_t exclusionZone() const;

    void setLeftMargin(const int &marginLeft);
    int leftMargin() const;

    void setRightMargin(const int &marginRight);
    int rightMargin() const;

    void setTopMargin(const int &marginTop);
    int topMargin() const;

    void setBottomMargin(const int &marginBottom);
    int bottomMargin() const;

    void setKeyboardInteractivity(KeyboardInteractivity interactivity);
    KeyboardInteractivity keyboardInteractivity() const;

    void setLayer(Layer layer);
    Layer layer() const;

    void setScreenConfiguration(ScreenConfiguration screenConfiguration);
    ScreenConfiguration screenConfiguration() const;

    /**
     * Sets a string based identifier for this window.
     * This may be used by a compositor to determine stacking
     * order within a given layer.
     *
     * May also be referred to as a role
     */
    void setScope(const QString &scope);
    QString scope() const;

    /**
     * Whether the QWindow should be closed when the layer surface is dismissed by the compositor.
     * For example, if the associated screen has been removed.
     *
     * This can be used to map the window on another screen.
     */
    void setCloseOnDismissed(bool close);
    bool closeOnDismissed() const;

    /**
     * Gets the LayerShell Window for a given Qt Window
     * Ownership is not transferred
     */
    static DLayerShellWindow* get(QWindow* window);

    static DLayerShellWindow *qmlAttachedProperties(QObject *object);

Q_SIGNALS:
    void anchorsChanged();
    void exclusionZoneChanged();
    void marginsChanged();
    void keyboardInteractivityChanged();
    void layerChanged();
    void scopeChanged();

private:
    DLayerShellWindow(QWindow* window);
    QScopedPointer<DLayerShellWindowPrivate> d;
};

DS_END_NAMESPACE
