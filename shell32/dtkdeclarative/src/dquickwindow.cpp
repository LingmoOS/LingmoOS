// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dquickwindow.h"
#include "dapploader.h"
#include "private/dquickwindow_p.h"
#include "private/dquickbehindwindowblur_p.h"
#include "private/dquickbehindwindowblur_p_p.h"

#include <private/dquickapploaderitem_p.h>
#include <private/qquickpath_p.h>
#include <private/qquickpath_p_p.h>
#include <private/qquicktransition_p.h>
#include <private/qquickwindow_p.h>

#include <QPlatformSurfaceEvent>

DQUICK_BEGIN_NAMESPACE

DQuickWindowPrivate::DQuickWindowPrivate(DQuickWindow *qq)
    : DTK_CORE_NAMESPACE::DObjectPrivate(qq)
    , attached(new DQuickWindowAttached(qq))
{
}

DQuickWindow::DQuickWindow(QWindow *parent)
    : QQuickWindow(parent)
    , DObject(*new DQuickWindowPrivate(this))
{

}

DQuickWindow::~DQuickWindow()
{
}

/*!
 * \~chinese \brief DQuickWindow::windowAttached　用于获取窗口的附加属性对象，
 * \~chinese 可以设置通过这个对象设置窗口的圆角、边框等属性值。
 */
DQuickWindowAttached *DQuickWindow::attached() const
{
    D_DC(DQuickWindow);

    return d->attached;
}

/*!
 * \~chinese \brief DQuickWindow::qmlAttachedProperties 用于创建窗口的附加属性对象，
 * \~chinese 在 QML 中使用附加属性时，会自动调用此函数。
 */
DQuickWindowAttached *DQuickWindow::qmlAttachedProperties(QObject *object)
{
    QQuickWindow *window = qobject_cast<QQuickWindow *>(object);
    if (window) {
        return new DQuickWindowAttached(window);
    }

    return nullptr;
}

DQuickWindowAttachedPrivate::DQuickWindowAttachedPrivate(QWindow *window, DQuickWindowAttached *qq)
    : DObjectPrivate(qq)
    , window(window)
    , wmWindowTypes(DWindowManagerHelper::UnknowWindowType)
    , appLoaderItem(nullptr)
{
}

DQuickWindowAttachedPrivate::~DQuickWindowAttachedPrivate()
{
    if (handle) {
        delete handle;
    }
}

template<class Func>
inline static void updateValue(DQuickWindowAttachedPrivate::BoolOptional &value, Func call)
{
    if (value != DQuickWindowAttachedPrivate::Invalid) {
        call(value == DQuickWindowAttachedPrivate::True);
        value = DQuickWindowAttachedPrivate::Invalid;
    }
}
template<class Func>
inline static void updateValue(qint8 &value, Func call)
{
    if (value >= 0) {
        call(value);
        value = -1;
    }
}
template<class Func>
inline static void updateValue(QColor &value, Func call)
{
    if (value.isValid()) {
        call(value);
        value = QColor();
    }
}
template<class Func>
inline static void updateValue(QPoint &value, Func call)
{
    if (!value.isNull()) {
        call(value);
        value = QPoint();
    }
}

template<class Func>
inline static void updateValue(DTK_GUI_NAMESPACE::DPlatformHandle::EffectScenes &value, Func call)
{
    if (value != DTK_GUI_NAMESPACE::DPlatformHandle::EffectScene(0)) {
        call(value);
        value = DTK_GUI_NAMESPACE::DPlatformHandle::EffectScene(0);
    }
}

template<class Func>
inline static void updateValue(DTK_GUI_NAMESPACE::DPlatformHandle::EffectTypes &value, Func call)
{
    if (value != DTK_GUI_NAMESPACE::DPlatformHandle::EffectType(0)) {
        call(value);
        value = DTK_GUI_NAMESPACE::DPlatformHandle::EffectType(0);
    }
}

bool DQuickWindowAttachedPrivate::ensurePlatformHandle()
{
    if (handle)
        return true;

    if (!DPlatformHandle::setEnabledNoTitlebarForWindow(window, true)) {
        qWarning() << "Failed to enable NoTitlebar for the window:"  << window;
        return false;
    }

    Q_ASSERT(DPlatformHandle::isEnabledNoTitlebar(window));
    explicitEnable = Invalid;
    D_Q(DQuickWindowAttached);
    handle = new DPlatformHandle(window);

    updateValue(explicitWindowRadius, std::bind(&DPlatformHandle::setWindowRadius, handle, std::placeholders::_1));
    updateValue(explicitBorderWidth, std::bind(&DPlatformHandle::setBorderWidth, handle, std::placeholders::_1));
    updateValue(explicitBorderColor, std::bind(&DPlatformHandle::setBorderColor, handle, std::placeholders::_1));
    updateValue(explicitShadowRadius, std::bind(&DPlatformHandle::setShadowRadius, handle, std::placeholders::_1));
    updateValue(explicitShadowOffset, std::bind(&DPlatformHandle::setShadowOffset, handle, std::placeholders::_1));
    updateValue(explicitShadowColor, std::bind(&DPlatformHandle::setShadowColor, handle, std::placeholders::_1));
    updateValue(explicitTranslucentBackground, std::bind(&DPlatformHandle::setTranslucentBackground, handle, std::placeholders::_1));
    updateValue(explicitEnableSystemResize, std::bind(&DPlatformHandle::setEnableSystemResize, handle, std::placeholders::_1));
    updateValue(explicitEnableSystemMove, std::bind(&DPlatformHandle::setEnableSystemMove, handle, std::placeholders::_1));
    updateValue(explicitEnableBlurWindow, std::bind(&DPlatformHandle::setEnableBlurWindow, handle, std::placeholders::_1));
    updateValue(explicitEffectScene, std::bind(&DPlatformHandle::setWindowEffect, handle, std::placeholders::_1));
    updateValue(explicitEffectType, std::bind(&DPlatformHandle::setWindowStartUpEffect, handle, std::placeholders::_1));

    QObject::connect(handle, &DPlatformHandle::borderColorChanged, q, &DQuickWindowAttached::borderColorChanged);
    QObject::connect(handle, &DPlatformHandle::borderWidthChanged, q, &DQuickWindowAttached::borderWidthChanged);
    QObject::connect(handle, &DPlatformHandle::shadowColorChanged, q, &DQuickWindowAttached::shadowColorChanged);
    QObject::connect(handle, &DPlatformHandle::shadowOffsetChanged, q, &DQuickWindowAttached::shadowOffsetChanged);
    QObject::connect(handle, &DPlatformHandle::shadowRadiusChanged, q, &DQuickWindowAttached::shadowRadiusChanged);
    QObject::connect(handle, &DPlatformHandle::windowRadiusChanged, q, &DQuickWindowAttached::windowRadiusChanged);
    QObject::connect(handle, &DPlatformHandle::translucentBackgroundChanged, q, &DQuickWindowAttached::translucentBackgroundChanged);
    QObject::connect(handle, &DPlatformHandle::enableSystemMoveChanged, q, &DQuickWindowAttached::enableSystemMoveChanged);
    QObject::connect(handle, &DPlatformHandle::enableSystemResizeChanged, q, &DQuickWindowAttached::enableSystemResizeChanged);
    QObject::connect(handle, &DPlatformHandle::enableBlurWindowChanged, q, &DQuickWindowAttached::enableBlurWindowChanged);
    QObject::connect(handle, SIGNAL(enableBlurWindowChanged()), q, SLOT(_q_updateBlurAreaForWindow()));
    QObject::connect(handle, &DPlatformHandle::windowEffectChanged, q, &DQuickWindowAttached::windowEffectChanged);
    QObject::connect(handle, &DPlatformHandle::windowStartUpEffectChanged, q, &DQuickWindowAttached::windowStartUpEffectChanged);
    Q_EMIT q->enabledChanged();
    return true;
}

void DQuickWindowAttachedPrivate::destoryPlatformHandle()
{
    handle->setEnabledNoTitlebarForWindow(window, false);
    delete handle;
    handle = nullptr;
}

void DQuickWindowAttachedPrivate::_q_onWindowMotifHintsChanged(quint32 winId)
{
    D_Q(DQuickWindowAttached);

    if (q->window()->winId() != winId)
        return;

    auto functions_hints = DWindowManagerHelper::getMotifFunctions(q->window());
    if (functions_hints != motifFunctions) {
        motifFunctions = functions_hints;
        Q_EMIT q->motifFunctionsChanged();
    }

    auto decorations_hints = DWindowManagerHelper::getMotifDecorations(q->window());
    if (decorations_hints != motifDecorations) {
        motifDecorations = decorations_hints;
        Q_EMIT q->motifDecorationsChanged();
    }
}

void DQuickWindowAttachedPrivate::addBlur(DQuickBehindWindowBlur *blur)
{
    Q_ASSERT(!blurList.contains(blur));
    blurList.append(blur);
    _q_updateBlurAreaForWindow();
}

void DQuickWindowAttachedPrivate::removeBlur(DQuickBehindWindowBlur *blur)
{
    if (blurList.removeOne(blur))
        _q_updateBlurAreaForWindow();
}

void DQuickWindowAttachedPrivate::updateBlurAreaFor(DQuickBehindWindowBlur *blur)
{
    Q_ASSERT(blurList.contains(blur));
    _q_updateBlurAreaForWindow();
}

void DQuickWindowAttachedPrivate::_q_updateBlurAreaForWindow()
{
    D_Q(DQuickWindowAttached);
    if (q->enableBlurWindow())
        return;

    QList<QPainterPath> blurPathList;
    QVector<DPlatformHandle::WMBlurArea> blurAreaList;

    for (const DQuickBehindWindowBlur *blur : qAsConst(blurList)) {
        if (!blur->d_func()->isValidBlur())
            continue;

        if (blur->d_func()->blurPath.isEmpty()) {
            blurAreaList.append(blur->d_func()->blurArea);
        } else {
            blurPathList.append(blur->d_func()->blurPath);
        }
    }

    bool blurSuc = false;

    if (blurPathList.isEmpty()) {
        blurSuc = q->setWindowBlurAreaByWM(blurPathList);
        blurSuc = q->setWindowBlurAreaByWM(blurAreaList);
    } else {
        // convert to QPainterPath
        for (const DPlatformHandle::WMBlurArea &area : qAsConst(blurAreaList)) {
            QPainterPath path;
            path.addRoundedRect(area.x, area.y, area.width, area.height, area.xRadius, area.yRaduis);
            blurPathList << path;
        }

        blurSuc = q->setWindowBlurAreaByWM(QVector<DPlatformHandle::WMBlurArea>{});
        blurSuc = q->setWindowBlurAreaByWM(blurPathList);
    }

    if (!blurSuc) {
        q->setEnableBlurWindow(true);
    }
}

void DQuickWindowAttachedPrivate::_q_updateClipPath()
{
    Q_Q(DQuickWindowAttached);

    Q_ASSERT(clipPath);
    q->setClipPathByWM(clipPath->path());
}

void DQuickWindowAttachedPrivate::_q_ensurePlatformHandle()
{
    if (explicitEnable == True && DWindowManagerHelper::instance()->hasNoTitlebar())
        ensurePlatformHandle();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void DQuickWindowAttachedPrivate::ensurePalette()
{
    Q_Q(DQuickWindowAttached);

    if (paletteInit)
        return;
    paletteInit = true;

    quickPalette = new QQuickPalette(q);
    inactiveQuickPalette = new QQuickPalette(q);

    paletteConnections = std::array<QMetaObject::Connection, 2> {
        QObject::connect(DGuiApplicationHelper::instance(), SIGNAL(applicationPaletteChanged()),
                         q, SLOT(_q_onPaletteChanged())),
        QObject::connect(window, SIGNAL(activeChanged()), q, SLOT(_q_updateWindowPalette()))
    };
}

void DQuickWindowAttachedPrivate::_q_onPaletteChanged()
{
    DPalette palette = DGuiApplicationHelper::instance()->applicationPalette(themeType);
    DPalette inactivePalette;

    for (int i = 0; i < QPalette::NColorRoles; ++i) {
        QPalette::ColorRole role = static_cast<QPalette::ColorRole>(i);
        inactivePalette.setBrush(QPalette::All, role, palette.brush(QPalette::Inactive, role));
        // The QML control will set the opacity property to 0.4 on the disabled state
        // The palette don't need set color for the QPalette::Disabled group.
        palette.setBrush(QPalette::All, role, palette.brush(QPalette::Active, role));
    }

    quickPalette->fromQPalette(palette);
    inactiveQuickPalette->fromQPalette(inactivePalette);

    _q_updateWindowPalette();
}

void DQuickWindowAttachedPrivate::_q_updateWindowPalette()
{
    Q_Q(DQuickWindowAttached);
    auto pt = window->isActive() ? quickPalette : inactiveQuickPalette;
    // Write qml property to cancle any binding on palette.
    // NOTE: this is not recoverable, i.e. setting DWindow.themeType will invalidate binding on palette.
    QQmlProperty::write(q->window(), "palette", QVariant::fromValue(pt));
}
#endif

DQuickWindowAttached::DQuickWindowAttached(QWindow *window)
    : QObject(window)
    , DObject(*new DQuickWindowAttachedPrivate(window, this))
{
    window->installEventFilter(this);
    QObject::connect(DWindowManagerHelper::instance(), SIGNAL(windowMotifWMHintsChanged(quint32)),
                     this, SLOT(_q_onWindowMotifHintsChanged(quint32)));
}

QQuickWindow *DQuickWindowAttached::window() const
{
    return qobject_cast<QQuickWindow *>(parent());
}

/*!
 * \~chinese \property DQuickWindowAttached::isEnabled
 * \~chinese \brief 这个属性用于判定是否使用了 DTK 风格的窗口。
 */
bool DQuickWindowAttached::isEnabled() const
{
    D_DC(DQuickWindowAttached);
    return d->handle && (DPlatformHandle::isEnabledDXcb(window())
        || DGuiApplicationHelper::testAttribute(DGuiApplicationHelper::IsTreelandPlatform));
}

/*!
 * \~chinese \property DQuickWindowAttached::windowRadius
 * \~chinese \brief 这个属性保存着窗口圆角值。
 */
int DQuickWindowAttached::windowRadius() const
{
    D_DC(DQuickWindowAttached);

    if (!d->handle) {
        return 0;
    }

    return d->handle->windowRadius();
}

/*!
 * \~chinese \property DQuickWindowAttached::borderWidth
 * \~chinese \brief 这个属性保存着窗口边框的宽度。
 */
int DQuickWindowAttached::borderWidth() const
{
    D_DC(DQuickWindowAttached);

    if (!d->handle) {
        return 0;
    }

    return d->handle->borderWidth();
}

/*!
 * \~chinese \property DQuickWindowAttached::borderColor
 * \~chinese \brief 这个属性保存这窗口边框的颜色。
 */
QColor DQuickWindowAttached::borderColor() const
{
    D_DC(DQuickWindowAttached);

    if (!d->handle) {
        return QColor();
    }

    return d->handle->borderColor();
}

/*!
 * \~chinese \property DQuickWindowAttached::shadowRadius
 * \~chinese \brief 这个属性保存着窗口阴影半径。
 */
int DQuickWindowAttached::shadowRadius() const
{
    D_DC(DQuickWindowAttached);

    if (!d->handle) {
        return 0;
    }

    return d->handle->shadowRadius();
}

/*!
 * \~chinese \property DQuickWindowAttached::shadowOffset
 * \~chinese \brief 这个属性保存着窗口阴影偏移量。
 */
QPoint DQuickWindowAttached::shadowOffset() const
{
    D_DC(DQuickWindowAttached);

    if (!d->handle) {
        return QPoint();
    }

    return d->handle->shadowOffset();
}

/*!
 * \~chinese \property DQuickWindowAttached::shadowColor
 * \~chinese \brief 这个属性保存着窗口阴影颜色。
 */
QColor DQuickWindowAttached::shadowColor() const
{
    D_DC(DQuickWindowAttached);

    if (!d->handle) {
        return QColor();
    }

    return d->handle->shadowColor();
}

/*!
 * \~chinese \property DQuickWindowAttached::frameMask
 * \~chinese \brief 设置 Frame Window 的遮罩，和 \a clipPath 不同的是，它的裁剪包括阴影部分。
 * \~chinese \note 由于实现机制限制，使用此属性裁剪 Frame Window 时，无法去除边缘产生的锯齿。
 */
QRegion DQuickWindowAttached::frameMask() const
{
    D_DC(DQuickWindowAttached);

    if (!d->handle) {
        return QRegion();
    }

    return d->handle->frameMask();
}

int DQuickWindowAttached::alphaBufferSize() const
{
    return window()->format().alphaBufferSize();
}

QQuickPath *DQuickWindowAttached::clipPath() const
{
    D_DC(DQuickWindowAttached);

    return d->clipPath;
}

QQuickTransition *DQuickWindowAttached::overlayExited() const
{
    D_DC(DQuickWindowAttached);
    return d->overlayExitedTransition;
}

void DQuickWindowAttached::setOverlayExited(QQuickTransition *trans)
{
    D_D(DQuickWindowAttached);
    if (d->overlayExitedTransition == trans)
        return;
    d->overlayExitedTransition = trans;
    Q_EMIT overlayExitedChanged();
}

QQmlComponent *DQuickWindowAttached::loadingOverlay() const
{
    D_DC(DQuickWindowAttached);
    return d->loadingOverlay;
}

DQuickAppLoaderItem *DQuickWindowAttached::appLoader() const
{
    D_DC(DQuickWindowAttached);
    if (!d->appLoaderItem) {
        const_cast<DQuickWindowAttachedPrivate *>(d)->appLoaderItem =
                new DQuickAppLoaderItem(window()->contentItem());
    }
    return d->appLoaderItem;
}

void DQuickWindowAttached::setAppLoader(DQuickAppLoaderItem *item)
{
    D_D(DQuickWindowAttached);
    if (d->appLoaderItem == item)
        return;

    // AppLoaderItem 会在窗口加载完毕后进行创建，因此
    // 在窗口创建初期，AppLoaderItem 需要指定一个默认
    // 值，防止 Qt 在运行时出现警告和报错
    if (d->appLoaderItem) {
        d->appLoaderItem->deleteLater();
    }
    d->appLoaderItem = item;
    Q_EMIT appLoaderChanged();
}

void DQuickWindowAttached::setLoadingOverlay(QQmlComponent *component)
{
    D_D(DQuickWindowAttached);
    if (component == d->loadingOverlay)
        return;

    d->loadingOverlay = component;
    Q_EMIT loadingOverlayChanged();
}

void DQuickWindowAttached::setWindowEffect(DTK_GUI_NAMESPACE::DPlatformHandle::EffectScenes effect)
{
    D_D(DQuickWindowAttached);

    d->ensurePlatformHandle();
    if (d->handle)
        d->handle->setWindowEffect(effect);
    else
        d->explicitEffectScene = effect;
}

void DQuickWindowAttached::setWindowStartUpEffect(DTK_GUI_NAMESPACE::DPlatformHandle::EffectTypes type)
{
    D_D(DQuickWindowAttached);

    d->ensurePlatformHandle();
    if (d->handle)
        d->handle->setWindowStartUpEffect(type);
    else
        d->explicitEffectType = type;
}

/*!
 * \~chinese \property DQuickWindowAttached::translucentBackground
 * \~chinese \brief 如果此属性值为 true，则在更新窗口绘制内容之前会先清空要更新区域内的图像，否则不清空。
 */
bool DQuickWindowAttached::translucentBackground() const
{
    D_DC(DQuickWindowAttached);

    if (!d->handle) {
        return false;
    }

    return d->handle->translucentBackground();
}

/*!
 * \~chinese \brief DQuickWindowAttached::enableSystemResize
 * \~chinese \return 如果此属性值为 true，则允许外界改变窗口的大小（如使用鼠标拖拽窗口边框），否则不允许。
 * \~chinese \note 此属性仅仅控制 dxcb 中的行为，不会影响窗口管理器的行为。
 */
bool DQuickWindowAttached::enableSystemResize() const
{
    D_DC(DQuickWindowAttached);

    if (!d->handle) {
        return false;
    }

    return d->handle->enableSystemResize();
}

/*!
 * \~chinese \property DQuickWindowAttached::enableSystemMove
 * \~chinese \brief 如果此属性值为 ture，则允许外界移动窗口的位置（如使用鼠标拖拽移动窗口），否则不允许。
  * \~chinese \note 此属性仅仅控制 dxcb 中的行为，不会影响窗口管理器的行为。
 */
bool DQuickWindowAttached::enableSystemMove() const
{
    D_DC(DQuickWindowAttached);

    if (!d->handle) {
        return false;
    }

    return d->handle->enableSystemMove();
}

/*!
 * \~chinese \property DQuickWindowAttached::enableBlurWindow
 * \~chinese \brief 如果此属性为 true，则窗口有效区域内的背景将呈现出模糊效果，否则无特效。
 */
bool DQuickWindowAttached::enableBlurWindow() const
{
    D_DC(DQuickWindowAttached);

    if (!d->handle) {
        return false;
    }

    return d->handle->enableBlurWindow();
}

/*!
 * \~chinese \brief DQuickWindowAttached::wmWindowTypes 返回此窗口在窗口管理器级别的窗口类型
 * \~chinese 需要注意的是，此值只是内部状态的记录，只会在调用 \a setWmWindowTypes
 * \~chinese 时更新，默认值为 \a DWindowManagerHelper::UnknowWindowType
 */
DWindowManagerHelper::WmWindowTypes DQuickWindowAttached::wmWindowTypes() const
{
    D_DC(DQuickWindowAttached);

    return d->wmWindowTypes;
}

DWindowManagerHelper::MotifFunctions DQuickWindowAttached::motifFunctions() const
{
    D_DC(DQuickWindowAttached);

    return d->motifFunctions;
}

DWindowManagerHelper::MotifDecorations DQuickWindowAttached::motifDecorations() const
{
    D_DC(DQuickWindowAttached);

    return d->motifDecorations;
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
DTK_GUI_NAMESPACE::DGuiApplicationHelper::ColorType DQuickWindowAttached::themeType() const
{
    D_DC(DQuickWindowAttached);
    return d->themeType;
}

void DQuickWindowAttached::setThemeType(const DTK_GUI_NAMESPACE::DGuiApplicationHelper::ColorType &newThemeType)
{
    D_D(DQuickWindowAttached);

    if (d->themeType == newThemeType)
        return;
    d->themeType = newThemeType;

    d->ensurePalette();
    d->_q_onPaletteChanged();

    emit themeTypeChanged();
}

void DQuickWindowAttached::resetThemeType()
{
    D_D(DQuickWindowAttached);
    if (!d->paletteInit) {
        return;
    }

    d->paletteInit = false;

    for (const QMetaObject::Connection &connection : std::as_const(d->paletteConnections))
        disconnect(connection);
    d->paletteConnections = {};
    delete d->quickPalette;
    d->quickPalette = nullptr;
    delete d->inactiveQuickPalette;
    d->inactiveQuickPalette = nullptr;

    if (d->themeType != DGuiApplicationHelper::UnknownType) {
        d->themeType = DGuiApplicationHelper::UnknownType;
        emit themeTypeChanged();
    }
    QQuickWindowPrivate::get(window())->resetPalette();
}
#endif


DTK_GUI_NAMESPACE::DPlatformHandle::EffectScene DQuickWindowAttached::windowEffect() const
{
    D_DC(DQuickWindowAttached);

    if (!d->handle) {
        return DTK_GUI_NAMESPACE::DPlatformHandle::EffectScene(0);
    }

    return d->handle->windowEffect();
}

DTK_GUI_NAMESPACE::DPlatformHandle::EffectType DQuickWindowAttached::windowStartUpEffect() const
{
    D_DC(DQuickWindowAttached);

    if (!d->handle) {
        return DTK_GUI_NAMESPACE::DPlatformHandle::EffectType(0);
    }

    return d->handle->windowStartUpEffect();
}

/*!
 * \~chinese \brief DQuickWindowAttached::setEnabled　设置当前的窗口为 DTK 风格。
 * \~chinese \note 只能把默认风格设置为 DTK 风格，不能把 DTK 设置为默认风格。
 * \~chinese \param \a true 使用 DTK 风格， \a false 无效。
 */
void DQuickWindowAttached::setEnabled(bool e)
{
    D_D(DQuickWindowAttached);
    if (e == isEnabled())
        return;

    if (!e) {
        d->destoryPlatformHandle();
        Q_EMIT enabledChanged();
        return;
    }

    if (!d->ensurePlatformHandle()) {
        d->explicitEnable = static_cast<DQuickWindowAttachedPrivate::BoolOptional>(e);
        QObject::connect(DWindowManagerHelper::instance(), SIGNAL(hasNoTitlebarChanged()), this, SLOT(_q_ensurePlatformHandle())
                         , Qt::UniqueConnection);
    }
}

/*!
 * \~chinese \brief DQuickWindowAttached::setWindowRadius　设定窗口的圆角
 * \~chinese \param windowRadius　窗口的圆角值
 */
void DQuickWindowAttached::setWindowRadius(int windowRadius)
{
    D_D(DQuickWindowAttached);

    d->ensurePlatformHandle();
    if (d->handle)
        d->handle->setWindowRadius(windowRadius);
    else
        d->explicitWindowRadius = windowRadius;
}

/*!
 * \~chinese \brief DQuickWindowAttached::setBorderWidth 设定边框的宽度
 * \~chinese \param borderWidth　边框的宽度
 */
void DQuickWindowAttached::setBorderWidth(int borderWidth)
{
    D_D(DQuickWindowAttached);

    d->ensurePlatformHandle();
    if (d->handle)
        d->handle->setBorderWidth(borderWidth);
    else
        d->explicitBorderWidth = borderWidth;
}

/*!
 * \~chinese \brief DQuickWindowAttached::setBorderColor 设定边框的颜色
 * \~chinese \param borderColor　边框的颜色
 */
void DQuickWindowAttached::setBorderColor(const QColor &borderColor)
{
    D_D(DQuickWindowAttached);

    d->ensurePlatformHandle();
    if (d->handle)
        d->handle->setBorderColor(borderColor);
    else
        d->explicitBorderColor = borderColor;
}

/*!
 * \~chinese \brief DQuickWindowAttached::setShadowRadius 设定阴影区域的圆角
 * \~chinese \param shadowRadius　阴影区域圆角大小
 */
void DQuickWindowAttached::setShadowRadius(int shadowRadius)
{
    D_D(DQuickWindowAttached);

    d->ensurePlatformHandle();
    if (d->handle)
        d->handle->setShadowRadius(shadowRadius);
    else
        d->explicitShadowRadius = shadowRadius;
}

/*!
 * \~chinese \brief DQuickWindowAttached::setShadowOffset 设定阴影区域的偏移距离
 * \~chinese \param shadowOffset　阴影区域的偏移距离
 */
void DQuickWindowAttached::setShadowOffset(const QPoint &shadowOffset)
{
    D_D(DQuickWindowAttached);

    d->ensurePlatformHandle();
    if (d->handle)
        d->handle->setShadowOffset(shadowOffset);
    else
        d->explicitShadowOffset = shadowOffset;
}

/*!
 * \~chinese \brief DQuickWindowAttached::setShadowColor 设定阴影的颜色
 * \~chinese \param shadowColor　阴影的颜色
 */
void DQuickWindowAttached::setShadowColor(const QColor &shadowColor)
{
    D_D(DQuickWindowAttached);

    d->ensurePlatformHandle();
    if (d->handle)
        d->handle->setShadowColor(shadowColor);
    else
        d->explicitShadowColor = shadowColor;
}

/*!
 * \~chinese \brief DQuickWindowAttached::setTranslucentBackground 设定时候擦除背景
 * \~chinese \param translucentBackground true擦除背景　false不擦除背景
 */
void DQuickWindowAttached::setTranslucentBackground(bool translucentBackground)
{
    D_D(DQuickWindowAttached);

    d->ensurePlatformHandle();
    if (d->handle)
        d->handle->setTranslucentBackground(translucentBackground);
    else
        d->explicitTranslucentBackground = static_cast<DQuickWindowAttachedPrivate::BoolOptional>(translucentBackground);
}

/*!
 * \~chinese \brief DQuickWindowAttached::setEnableSystemResize　设定是否允许系统调整窗口大小
 * \~chinese \param enableSystemResize　true允许系统调整　false不允许系统调整
 */
void DQuickWindowAttached::setEnableSystemResize(bool enableSystemResize)
{
    D_D(DQuickWindowAttached);

    d->ensurePlatformHandle();
    if (d->handle)
        d->handle->setEnableSystemResize(enableSystemResize);
    else
        d->explicitEnableSystemResize = static_cast<DQuickWindowAttachedPrivate::BoolOptional>(enableSystemResize);
}

/*!
 * \~chinese \brief DQuickWindowAttached::setEnableSystemMove 设定时候允许系统移动窗口
 * \~chinese \param enableSystemMove　true允许移动　false不允许移动
 */
void DQuickWindowAttached::setEnableSystemMove(bool enableSystemMove)
{
    D_D(DQuickWindowAttached);

    d->ensurePlatformHandle();
    if (d->handle)
        d->handle->setEnableSystemMove(enableSystemMove);
    else
        d->explicitEnableSystemMove = static_cast<DQuickWindowAttachedPrivate::BoolOptional>(enableSystemMove);
}

/*!
 * \~chinese \brief DQuickWindowAttached::setEnableBlurWindow　设定窗口有效区域内的背景将呈现出模糊效果。
 * \~chinese \param enableBlurWindow true有特效　false无特效。
 */
void DQuickWindowAttached::setEnableBlurWindow(bool enableBlurWindow)
{
    D_D(DQuickWindowAttached);

    d->ensurePlatformHandle();
    if (d->handle)
        d->handle->setEnableBlurWindow(enableBlurWindow);
    else
        d->explicitEnableBlurWindow = static_cast<DQuickWindowAttachedPrivate::BoolOptional>(enableBlurWindow);
}

/*!
 * \~chinese \brief DQuickWindowAttached::setWmWindowTypes 为此窗口设置与本地窗口管理器
 * \~chinese 息息相关的窗口类型，这些类型不保证在所有平台下都能生效，因此可能会影响程序
 * \~chinese 的跨平台行为，请尽量使用 \a QWindow::setFlags 设置所需要的窗口类型。
 * \~chinese \param wmWindowTypes 新的窗口类型，此枚举值可组合使用
 * \~chinese \note 调用此接口设置的窗口类型会与 \a QWindow::flags 中控制窗口类型的
 * \~chinese 部分共同生效
 */
void DQuickWindowAttached::setWmWindowTypes(DWindowManagerHelper::WmWindowTypes wmWindowTypes)
{
    D_D(DQuickWindowAttached);

    if (d->wmWindowTypes == wmWindowTypes)
        return;

    d->wmWindowTypes = wmWindowTypes;

    DWindowManagerHelper::setWmWindowTypes(window(), wmWindowTypes);
    Q_EMIT wmWindowTypesChanged();
}

void DQuickWindowAttached::setMotifFunctions(Gui::DWindowManagerHelper::MotifFunctions motifFunctions)
{
    D_D(DQuickWindowAttached);

    if (d->motifFunctions == motifFunctions)
        return;

    d->motifFunctions = motifFunctions;

    DWindowManagerHelper::setMotifFunctions(window(), motifFunctions);
    Q_EMIT motifFunctionsChanged();
}

void DQuickWindowAttached::setMotifDecorations(DWindowManagerHelper::MotifDecorations motifDecorations)
{
    D_D(DQuickWindowAttached);

    if (d->motifDecorations == motifDecorations)
        return;

    d->motifDecorations = motifDecorations;
    DWindowManagerHelper::setMotifDecorations(window(), motifDecorations);
    Q_EMIT motifDecorationsChanged();
}

void DQuickWindowAttached::popupSystemWindowMenu()
{
    DWindowManagerHelper::popupSystemWindowMenu(window());
}

bool DQuickWindowAttached::setWindowBlurAreaByWM(const QVector<DPlatformHandle::WMBlurArea> &area)
{
    D_D(DQuickWindowAttached);

    d->ensurePlatformHandle();
    if (d->handle) {
        return d->handle->setWindowBlurAreaByWM(area);
    }

    return false;
}

bool DQuickWindowAttached::setWindowBlurAreaByWM(const QList<QPainterPath> &area)
{
    D_D(DQuickWindowAttached);

    d->ensurePlatformHandle();
    if (d->handle) {
        return d->handle->setWindowBlurAreaByWM(area);
    }

    return false;
}

void DQuickWindowAttached::setClipPathByWM(const QPainterPath &clipPath)
{
    D_D(DQuickWindowAttached);

    d->ensurePlatformHandle();
    if (d->handle)
        d->handle->setClipPath(clipPath);
}

void DQuickWindowAttached::setClipPath(QQuickPath *path)
{
    D_D(DQuickWindowAttached);

    if (path == d->clipPath)
        return;

    if (d->clipPath)
        disconnect(d->clipPath, nullptr, this, nullptr);

    d->clipPath = path;
    Q_EMIT clipPathChanged();

    if (d->clipPath) {
        QQuickPathPrivate *pathPrivate = QQuickPathPrivate::get(d->clipPath);
        Q_ASSERT(pathPrivate);

        if (pathPrivate->componentComplete)
            setClipPathByWM(d->clipPath->path());

        connect(d->clipPath, SIGNAL(changed()), this, SLOT(_q_updateClipPath()));
    } else {
        setClipPathByWM(QPainterPath());
    }
}

bool DQuickWindowAttached::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parent()) {
        if (event->type() == QEvent::PlatformSurface) {
            QPlatformSurfaceEvent *surface = static_cast<QPlatformSurfaceEvent *>(event);
            if (surface->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {
                D_D(DQuickWindowAttached);
                d->_q_onWindowMotifHintsChanged(static_cast<quint32>(d->window->winId()));
                watched->removeEventFilter(this);
            }
        }
    }

    return false;
}

void DQuickWindowAttached::setAlphaBufferSize(int size)
{
    if (alphaBufferSize() == size)
        return;

    QQuickWindow *w = window();
    QSurfaceFormat fmt = w->requestedFormat();
    fmt.setAlphaBufferSize(size);
    w->setFormat(fmt);
    Q_EMIT alphaBufferSizeChanged();
}

DQUICK_END_NAMESPACE

#include "moc_dquickwindow.cpp"
