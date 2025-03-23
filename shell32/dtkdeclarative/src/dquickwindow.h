// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DWINDOW_H
#define DWINDOW_H

#include <QQuickWindow>
#include <DObject>

#include <dtkdeclarative_global.h>
#include <DWindowManagerHelper>
#include <DPlatformHandle>
#include <DGuiApplicationHelper>

QT_BEGIN_NAMESPACE
class QQmlComponent;
class QQuickPath;
class QQuickTransition;
QT_END_NAMESPACE

DQUICK_BEGIN_NAMESPACE

class DQuickWindowPrivate;
class DQuickWindowAttached;
class DQuickWindowAttachedPrivate;

class DQuickWindow : public QQuickWindow, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_UNCREATABLE("DQuickWindow Attached.")
    QML_NAMED_ELEMENT(DWindow)
    QML_ATTACHED(DQuickWindowAttached)
#endif
public:
    explicit DQuickWindow(QWindow *parent = nullptr);
    ~DQuickWindow() override;

    DQuickWindowAttached *attached() const;
    static DQuickWindowAttached *qmlAttachedProperties(QObject *object);

private:
    D_DECLARE_PRIVATE(DQuickWindow)
};

class DQuickAppLoaderItem;
class DQuickWindowAttachedPrivate;
class DQuickWindowAttached : public QObject, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT
    Q_PROPERTY(QQuickWindow *window READ window CONSTANT)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(int windowRadius READ windowRadius WRITE setWindowRadius NOTIFY windowRadiusChanged)
    Q_PROPERTY(int borderWidth READ borderWidth WRITE setBorderWidth NOTIFY borderWidthChanged)
    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(int shadowRadius READ shadowRadius WRITE setShadowRadius NOTIFY shadowRadiusChanged)
    Q_PROPERTY(QPoint shadowOffset READ shadowOffset WRITE setShadowOffset NOTIFY shadowOffsetChanged)
    Q_PROPERTY(QColor shadowColor READ shadowColor WRITE setShadowColor NOTIFY shadowColorChanged)
    Q_PROPERTY(bool translucentBackground READ translucentBackground WRITE setTranslucentBackground NOTIFY translucentBackgroundChanged)
    Q_PROPERTY(bool enableSystemResize READ enableSystemResize WRITE setEnableSystemResize NOTIFY enableSystemResizeChanged)
    Q_PROPERTY(bool enableSystemMove READ enableSystemMove WRITE setEnableSystemMove NOTIFY enableSystemMoveChanged)
    Q_PROPERTY(bool enableBlurWindow READ enableBlurWindow WRITE setEnableBlurWindow NOTIFY enableBlurWindowChanged)
    Q_PROPERTY(int alphaBufferSize READ alphaBufferSize WRITE setAlphaBufferSize NOTIFY alphaBufferSizeChanged)
    Q_PROPERTY(QQuickPath *clipPath READ clipPath WRITE setClipPath NOTIFY clipPathChanged)
    Q_PROPERTY(DTK_GUI_NAMESPACE::DWindowManagerHelper::WmWindowTypes wmWindowTypes READ wmWindowTypes WRITE setWmWindowTypes NOTIFY wmWindowTypesChanged)
    Q_PROPERTY(DTK_GUI_NAMESPACE::DWindowManagerHelper::MotifFunctions motifFunctions READ motifFunctions WRITE setMotifFunctions NOTIFY motifFunctionsChanged)
    Q_PROPERTY(DTK_GUI_NAMESPACE::DWindowManagerHelper::MotifDecorations motifDecorations READ motifDecorations WRITE setMotifDecorations NOTIFY motifDecorationsChanged)
    Q_PROPERTY(QQuickTransition *overlayExited READ overlayExited WRITE setOverlayExited NOTIFY overlayExitedChanged FINAL)
    Q_PROPERTY(QQmlComponent *loadingOverlay READ loadingOverlay WRITE setLoadingOverlay NOTIFY loadingOverlayChanged FINAL)
    Q_PROPERTY(DTK_QUICK_NAMESPACE::DQuickAppLoaderItem *appLoader READ appLoader NOTIFY appLoaderChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    Q_PROPERTY(DTK_GUI_NAMESPACE::DGuiApplicationHelper::ColorType themeType READ themeType WRITE setThemeType RESET resetThemeType NOTIFY themeTypeChanged)
#endif
    Q_PROPERTY(DTK_GUI_NAMESPACE::DPlatformHandle::EffectScene windowEffect READ windowEffect WRITE setWindowEffect NOTIFY windowEffectChanged)
    Q_PROPERTY(DTK_GUI_NAMESPACE::DPlatformHandle::EffectType windowStartUpEffect READ windowStartUpEffect WRITE setWindowStartUpEffect NOTIFY windowStartUpEffectChanged)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_ANONYMOUS
#endif

public:
    explicit DQuickWindowAttached(QWindow *window);

    QQuickWindow *window() const;
    bool isEnabled() const;

    int windowRadius() const;

    int borderWidth() const;
    QColor borderColor() const;

    int shadowRadius() const;
    QPoint shadowOffset() const;
    QColor shadowColor() const;

    QRegion frameMask() const;

    bool translucentBackground() const;
    bool enableSystemResize() const;
    bool enableSystemMove() const;
    bool enableBlurWindow() const;
    int alphaBufferSize() const;

    QQuickPath *clipPath() const;

    QQuickTransition *overlayExited() const;
    QQmlComponent *loadingOverlay() const;

    DQuickAppLoaderItem *appLoader() const;
    void setAppLoader(DQuickAppLoaderItem *item);

    DTK_GUI_NAMESPACE::DWindowManagerHelper::WmWindowTypes wmWindowTypes() const;
    DTK_GUI_NAMESPACE::DWindowManagerHelper::MotifFunctions motifFunctions() const;
    DTK_GUI_NAMESPACE::DWindowManagerHelper::MotifDecorations motifDecorations() const;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    DTK_GUI_NAMESPACE::DGuiApplicationHelper::ColorType themeType() const;
    void setThemeType(const DTK_GUI_NAMESPACE::DGuiApplicationHelper::ColorType &newThemeType);
    void resetThemeType();
#endif

    DTK_GUI_NAMESPACE::DPlatformHandle::EffectScene windowEffect() const;
    DTK_GUI_NAMESPACE::DPlatformHandle::EffectType windowStartUpEffect() const;

public Q_SLOTS:
    void setEnabled(bool e);

    void setWindowRadius(int windowRadius);

    void setBorderWidth(int borderWidth);
    void setBorderColor(const QColor &borderColor);

    void setShadowRadius(int shadowRadius);
    void setShadowOffset(const QPoint &shadowOffset);
    void setShadowColor(const QColor &shadowColor);

    void setTranslucentBackground(bool translucentBackground);
    void setEnableSystemResize(bool enableSystemResize);
    void setEnableSystemMove(bool enableSystemMove);
    void setEnableBlurWindow(bool enableBlurWindow);
    void setAlphaBufferSize(int size);

    void setWmWindowTypes(DTK_GUI_NAMESPACE::DWindowManagerHelper::WmWindowTypes wmWindowTypes);
    void setMotifFunctions(DTK_GUI_NAMESPACE::DWindowManagerHelper::MotifFunctions motifFunctions);
    void setMotifDecorations(DTK_GUI_NAMESPACE::DWindowManagerHelper::MotifDecorations motifDecorations);

    void popupSystemWindowMenu();

    bool setWindowBlurAreaByWM(const QVector<DPlatformHandle::WMBlurArea> &area);
    bool setWindowBlurAreaByWM(const QList<QPainterPath> &area);

    void setClipPathByWM(const QPainterPath &clipPath);
    void setClipPath(QQuickPath *path);
    void setOverlayExited(QQuickTransition *exit);
    void setLoadingOverlay(QQmlComponent *component);

    void setWindowEffect(DTK_GUI_NAMESPACE::DPlatformHandle::EffectScenes effect);
    void setWindowStartUpEffect(DTK_GUI_NAMESPACE::DPlatformHandle::EffectTypes type);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

Q_SIGNALS:
    void enabledChanged();
    void windowRadiusChanged();
    void borderWidthChanged();
    void borderColorChanged();
    void shadowRadiusChanged();
    void shadowOffsetChanged();
    void shadowColorChanged();
    void translucentBackgroundChanged();
    void enableSystemResizeChanged();
    void enableSystemMoveChanged();
    void enableBlurWindowChanged();
    void wmWindowTypesChanged();
    void motifFunctionsChanged();
    void motifDecorationsChanged();
    void alphaBufferSizeChanged();
    void clipPathChanged();
    void overlayExitedChanged();
    void loadingOverlayChanged();
    void appLoaderChanged();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void themeTypeChanged();
#endif
    void windowEffectChanged();
    void windowStartUpEffectChanged();

private:
    D_DECLARE_PRIVATE(DQuickWindowAttached)
    D_PRIVATE_SLOT(void _q_onWindowMotifHintsChanged(quint32))
    D_PRIVATE_SLOT(void _q_updateBlurAreaForWindow())
    D_PRIVATE_SLOT(void _q_updateClipPath())
    D_PRIVATE_SLOT(void _q_ensurePlatformHandle())
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    D_PRIVATE_SLOT(void _q_onPaletteChanged())
    D_PRIVATE_SLOT(void _q_updateWindowPalette())
#endif

    friend class DQuickBehindWindowBlur;
    friend class DQuickBehindWindowBlurPrivate;
};

DQUICK_END_NAMESPACE

QML_DECLARE_TYPEINFO(DTK_QUICK_NAMESPACE::DQuickWindow, QML_HAS_ATTACHED_PROPERTIES)

#endif // DWINDOW_H
