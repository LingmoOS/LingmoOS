/*
 * SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
#pragma once

#include "decorationshadow.h"
#include <kdecoration2/kdecoration2_export.h>

#include <QMargins>
#include <QObject>
#include <QRect>

class QHoverEvent;
class QMouseEvent;
class QPainter;
class QWheelEvent;

/**
 * @brief Framework for creating window decorations.
 *
 **/
namespace KDecoration2
{
class DecorationPrivate;
class DecoratedClient;
class DecorationButton;
class DecorationSettings;

/**
 * @brief Base class for the Decoration.
 *
 * To provide a Decoration one needs to inherit from this class. The framework will instantiate
 * an instance of the inherited class for each DecoratedClient.
 *
 * The main tasks of the Decoration is to provide borders around the DecoratedClient. For this
 * the Deocration provides border sizes: those should be adjusted depending on the state of the
 * DecoratedClient. E.g. commonly a maximized DecoratedClient does not have borders on the side,
 * only the title bar.
 *
 * Whenever the visual representation of the Decoration changes the slot @link Decoration::update @endlink
 * should be invoked to request a repaint. The framework will in return invoke the
 * @link Decoration::paint @endlink method. This method needs to be implemented by inheriting
 * classes.
 *
 * A Decoration commonly provides buttons for interaction. E.g. a close button to close the
 * DecoratedClient. For such actions the Decoration provides slots which should be connected to
 * the clicked signals of the buttons. For convenience the framework provides the @link DecorationButton @endlink
 * and the @link DecorationButtonGroup @endlink for easier layout. It is not required to use those,
 * if one uses different ways to represent the actions one needs to filter the events accordingly.
 *
 * @see DecoratedClient
 * @see DecorationButton
 * @see DecorationButtonGroup
 **/
class KDECORATIONS2_EXPORT Decoration : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QMargins borders READ borders NOTIFY bordersChanged)
    Q_PROPERTY(int borderLeft READ borderLeft NOTIFY bordersChanged)
    Q_PROPERTY(int borderRight READ borderRight NOTIFY bordersChanged)
    Q_PROPERTY(int borderTop READ borderTop NOTIFY bordersChanged)
    Q_PROPERTY(int borderBottom READ borderBottom NOTIFY bordersChanged)
    Q_PROPERTY(QMargins resizeOnlyBorders READ resizeOnlyBorders NOTIFY resizeOnlyBordersChanged)
    Q_PROPERTY(int resizeOnlyBorderLeft READ resizeOnlyBorderLeft NOTIFY resizeOnlyBordersChanged)
    Q_PROPERTY(int resizeOnlyBorderRight READ resizeOnlyBorderRight NOTIFY resizeOnlyBordersChanged)
    Q_PROPERTY(int resizeOnlyBorderTop READ resizeOnlyBorderTop NOTIFY resizeOnlyBordersChanged)
    Q_PROPERTY(int resizeOnlyBorderBottom READ resizeOnlyBorderBottom NOTIFY resizeOnlyBordersChanged)
    /**
     * This property denotes the part of the Decoration which is currently under the mouse pointer.
     * It gets automatically updated whenever a QMouseEvent or QHoverEvent gets processed.
     **/
    Q_PROPERTY(Qt::WindowFrameSection sectionUnderMouse READ sectionUnderMouse NOTIFY sectionUnderMouseChanged)
    /**
     * The titleBar is the area inside the Decoration containing all controls (e.g. Buttons)
     * and the caption. The titleBar is the main interaction area, while all other areas of the
     * Decoration are normally used as resize areas.
     **/
    Q_PROPERTY(QRect titleBar READ titleBar NOTIFY titleBarChanged)
    /**
     * Whether the Decoration is fully opaque. By default a Decoration is considered to
     * use the alpha channel and this property has the value @c false. But for e.g. a maximized
     * DecoratedClient it is possible that the Decoration is fully opaque. In this case the
     * Decoration should set this property to @c true.
     **/
    Q_PROPERTY(bool opaque READ isOpaque NOTIFY opaqueChanged)
public:
    ~Decoration() override;

    /**
     * The DecoratedClient for this Decoration.
     **/
    DecoratedClient *client() const;

    QMargins borders() const;
    int borderLeft() const;
    int borderRight() const;
    int borderTop() const;
    int borderBottom() const;
    QMargins resizeOnlyBorders() const;
    int resizeOnlyBorderLeft() const;
    int resizeOnlyBorderRight() const;
    int resizeOnlyBorderTop() const;
    int resizeOnlyBorderBottom() const;
    Qt::WindowFrameSection sectionUnderMouse() const;
    QRect titleBar() const;
    bool isOpaque() const;

    /**
     * DecorationShadow for this Decoration. It is recommended that multiple Decorations share
     * the same DecorationShadow. E.g one DecorationShadow for all inactive Decorations and one
     * for the active Decoration.
     **/
    std::shared_ptr<DecorationShadow> shadow() const;

    /**
     * The decoration's geometry in local coordinates.
     *
     * Basically the size of the DecoratedClient combined with the borders.
     **/
    QRect rect() const;
    QSize size() const;

    /**
     * The decoration's blur region in local coordinates
     */
    QRegion blurRegion() const;

    /**
     * Invoked by the framework to set the Settings for this Decoration before
     * init is invoked.
     * @internal
     **/
    void setSettings(const std::shared_ptr<DecorationSettings> &settings);
    /**
     * @returns The DecorationSettings used for this Decoration.
     **/
    std::shared_ptr<DecorationSettings> settings() const;

    /**
     * Implement this method in inheriting classes to provide the rendering.
     *
     * The @p painter is set up to paint on an internal QPaintDevice. The painting is
     * implicitly double buffered.
     *
     * @param painter The painter which needs to be used for rendering
     * @param repaintArea The region which needs to be repainted.
     **/
    virtual void paint(QPainter *painter, const QRect &repaintArea) = 0;

    bool event(QEvent *event) override;

public Q_SLOTS:
    void requestClose();
    void requestToggleMaximization(Qt::MouseButtons buttons);
    void requestMinimize();
    void requestContextHelp();
    void requestToggleOnAllDesktops();
    void requestToggleShade();
    void requestToggleKeepAbove();
    void requestToggleKeepBelow();

#if KDECORATIONS2_ENABLE_DEPRECATED_SINCE(5, 21)
    /**
     * @deprecated
     * @see requestShowWindowMenu(const QRect &rect)
     */
    KDECORATIONS2_DEPRECATED_VERSION(5, 21, "Use Decoration::requestShowWindowMenu(QRect)")
    void requestShowWindowMenu();
#endif

    /**
     * @param rect the location at which to show the window menu
     */
    void requestShowWindowMenu(const QRect &rect);
    void requestShowToolTip(const QString &text);
    void requestHideToolTip();

    void showApplicationMenu(int actionId);
    void requestShowApplicationMenu(const QRect &rect, int actionId);

    void update(const QRect &rect);
    void update();

    /**
     * This method gets invoked from the framework once the Decoration is created and
     * completely setup.
     *
     * An inheriting class should override this method and perform all initialization in
     * this method instead of the constructor.
     *
     * @return true if initialization has been successful,
     * false otherwise (for example, a QML component could not be loaded)
     **/
    virtual bool init() = 0;

Q_SIGNALS:
    void blurRegionChanged();
    void bordersChanged();
    void resizeOnlyBordersChanged();
    void sectionUnderMouseChanged(Qt::WindowFrameSection);
    void titleBarChanged();
    void opaqueChanged(bool);
    void shadowChanged(const std::shared_ptr<DecorationShadow> &shadow);
    void damaged(const QRegion &region);

protected:
    /**
     * Constructor for the Decoration.
     *
     * The @p args are used by the decoration framework to pass meta information
     * to the Decoration. An inheriting class is supposed to pass the args to the
     * parent class.
     *
     * @param parent The parent of the Decoration
     * @param args Additional arguments passed in from the framework
     **/
    explicit Decoration(QObject *parent, const QVariantList &args);
    void setBorders(const QMargins &borders);
    void setResizeOnlyBorders(const QMargins &borders);
    void setBlurRegion(const QRegion &region);
    /**
     * An implementation has to invoke this method whenever the area
     * containing the controls and caption changes.
     * @param rect The new geometry of the titleBar in Decoration coordinates
     **/
    void setTitleBar(const QRect &rect);
    void setOpaque(bool opaque);
    void setShadow(const std::shared_ptr<DecorationShadow> &shadow);

    virtual void hoverEnterEvent(QHoverEvent *event);
    virtual void hoverLeaveEvent(QHoverEvent *event);
    virtual void hoverMoveEvent(QHoverEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);

private:
    friend class DecorationButton;
    class Private;
    std::unique_ptr<Private> d;
};

} // namespace

Q_DECLARE_METATYPE(KDecoration2::Decoration *)
