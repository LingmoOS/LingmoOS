// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#define protected public
#include "dpopupwindowhandle_p.h"

#include <QQuickItem>
#include <QQuickWindow>

#include <DVtableHook>

DCORE_USE_NAMESPACE
DQUICK_BEGIN_NAMESPACE

// className prepend string of QT_NAMESPACE if existed.
bool inheritsTheClassType(QObject *object, QString className) {
#if defined(QT_NAMESPACE)
#define D_GET_NAMESPACE_STR_IMPL(M) #M "::"
#define D_GET_NAMESPACE_STR(M) D_GET_NAMESPACE_STR_IMPL(M)
    className.prepend(D_GET_NAMESPACE_STR(QT_NAMESPACE));
#undef D_GET_NAMESPACE_STR
#undef D_GET_NAMESPACE_STR_IMPL
#endif
    return object && object->inherits(qPrintable(className));
}

static inline bool shouldCreatePopupWindowForMode(const DQMLGlobalObject::PopupMode mode)
{
    switch (mode) {
    case DQMLGlobalObject::WindowMode:
        return true;
    case DQMLGlobalObject::EmbedMode:
        return false;
    case DQMLGlobalObject::AutoMode:
        // TODO https://github.com/linuxdeepin/dtk/issues/70
        if (qEnvironmentVariableIsEmpty("D_POPUP_MODE"))
            return false;
        return qEnvironmentVariable("D_POPUP_MODE") != "embed";
    }
    return false;
}
DQMLGlobalObject::PopupMode DPopupWindowHandle::m_popupMode = DQMLGlobalObject::AutoMode;
DPopupWindowHandle::DPopupWindowHandle(QObject *parent)
    : QObject (parent)
{
    // after `autoWindowMode` property initialized to createHandle.
    connect(popup(), SIGNAL(windowChanged(QQuickWindow *)), this, SLOT(createHandle()));
}

DPopupWindowHandle::~DPopupWindowHandle()
{
}

DPopupWindowHandle *DPopupWindowHandle::qmlAttachedProperties(QObject *object)
{
    if (!inheritsTheClassType(object, "QQuickPopup"))
        return nullptr;

    return new DPopupWindowHandle(object);
}

void DPopupWindowHandle::setPopupMode(const DQMLGlobalObject::PopupMode mode)
{
    m_popupMode = mode;
}

QQuickWindow *DPopupWindowHandle::window() const
{
    return m_handle ? m_handle->window() : nullptr;
}

QQmlComponent *DPopupWindowHandle::delegate() const
{
    return m_delegate;
}

void DPopupWindowHandle::setDelegate(QQmlComponent *delegate)
{
    m_delegate = delegate;
}

bool DPopupWindowHandle::forceWindowMode() const
{
    return m_forceWindowMode;
}

void DPopupWindowHandle::setForceWindowMode(bool forceWindowMode)
{
    if (m_forceWindowMode == forceWindowMode)
        return;

    m_forceWindowMode = forceWindowMode;
    if (!m_forceWindowMode && m_handle) {
        m_handle.reset();
        Q_EMIT windowChanged();
    }
    if (m_forceWindowMode) {
        // try to create handle.
        createHandle();
    }
}

void DPopupWindowHandle::createHandle()
{
    if (!needCreateHandle())
        return;

    auto window = qobject_cast<QQuickWindow *>(m_delegate->create(m_delegate->creationContext()));
    Q_ASSERT(window);

    m_handle.reset(new DPopupWindowHandleImpl(window, popup()));
    Q_EMIT windowChanged();
}

bool DPopupWindowHandle::needCreateHandle() const
{
    // has created.
    if (m_handle)
        return false;

    // no delegate.
    if (!m_delegate) {
        if (m_forceWindowMode)
            qWarning() << "delegate don't set but forceWindowMode has been set.";

        return false;
    }
    // forceWindowMode > `D_POPUP_MODE` > popupMode
    return m_forceWindowMode || shouldCreatePopupWindowForMode(m_popupMode);
}

QObject *DPopupWindowHandle::popup() const
{
    return parent();
}

// it's not to call QQuickPopupItem's reposition when handle is positioning.
static constexpr char const *PopupWindowHandlePointer = "_d_popup_window_handle";
static inline void popupGeometryChanged(QQuickItem *obj, const QRectF &newGeometry, const QRectF &oldGeometry)
{
    DPopupWindowHandleImpl *handle = obj->property(PopupWindowHandlePointer).value<DPopupWindowHandleImpl *>();
    Q_ASSERT(handle);
    if (!handle->isPositioning()) {
        // only in `reposition` to override virtual function.
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        DVtableHook::callOriginalFun(obj, &QQuickItem::geometryChanged, newGeometry, oldGeometry);
#else
        DVtableHook::callOriginalFun(obj, &QQuickItem::geometryChange, newGeometry, oldGeometry);
#endif
    }
}
static inline void popupUpdatePolish(QQuickItem *obj)
{
    DPopupWindowHandleImpl *handle = obj->property(PopupWindowHandlePointer).value<DPopupWindowHandleImpl *>();
    Q_ASSERT(handle);
    if (handle->isPositioning()) {
        // avoid to call original function in `reposition`.
        handle->setPositioning(false);
    } else {
        // only sepcial scene to override virtual function.
        DVtableHook::callOriginalFun(obj, &QQuickItem::updatePolish);
    }
}

DPopupWindowHandleImpl::DPopupWindowHandleImpl(QQuickWindow *window, QObject *parent)
    : QObject(parent)
    , m_window(window)
    , m_popup(parent)
{
    Q_ASSERT(popupItem());

    connect(popup(), SIGNAL(opened()), this, SLOT(reposition()));
    popupItem()->setProperty(PopupWindowHandlePointer, QVariant::fromValue(this));
    // geometryChanged would call reposition of `PopupItem`.
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    DVtableHook::overrideVfptrFun(popupItem(), &QQuickItem::geometryChanged, &popupGeometryChanged);
#else
    DVtableHook::overrideVfptrFun(popupItem(), &QQuickItem::geometryChange, &popupGeometryChanged);
#endif
    // updatePolish would call reposition of `PopupItem`.
    DVtableHook::overrideVfptrFun(popupItem(), &QQuickItem::updatePolish, &popupUpdatePolish);

    // TODO QML Window with Qt::Popup flag not behaving correctly (QTBUG-69777)
    connect(m_window, &QWindow::activeChanged, this, &DPopupWindowHandleImpl::close);
    connect(popup(), SIGNAL(closed()), this, SLOT(close()));
}

DPopupWindowHandleImpl::~DPopupWindowHandleImpl()
{
    QQuickItem *item = popupItem();
    if (item) {
        // reset original virtual function.
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        DVtableHook::resetVfptrFun(item, &QQuickItem::geometryChanged);
#else
        DVtableHook::resetVfptrFun(item, &QQuickItem::geometryChange);
#endif
        DVtableHook::resetVfptrFun(item, &QQuickItem::updatePolish);
        disconnect(item, nullptr, this, nullptr);
    }
    disconnect(popup(), nullptr, this, nullptr);
    disconnect(m_window, nullptr, this, nullptr);

    m_window->deleteLater();
    m_window = nullptr;
}

QQuickWindow *DPopupWindowHandleImpl::window() const
{
    return m_window;
}

QObject *DPopupWindowHandleImpl::popup() const
{
    return m_popup;
}

QQuickItem *DPopupWindowHandleImpl::popupItem() const {
    for (auto item : popup()->children()) {
        if (inheritsTheClassType(item, "QQuickPopupItem"))
            return qobject_cast<QQuickItem *>(item);
    }
    return nullptr;
}

bool DPopupWindowHandleImpl::isPositioning() const
{
    return m_positioning;
}

void DPopupWindowHandleImpl::setPositioning(bool positioning)
{
    m_positioning = positioning;
}

void DPopupWindowHandleImpl::reposition()
{
    if (isPositioning())
        return;

    setPositioning(true);

    m_window->resize(popupItem()->size().toSize());
    // set window's position to origin popupItem' leftTop position.
    m_window->setPosition(popupItem()->mapToGlobal(QPoint(0, 0)).toPoint());
    // reset popupItem's position to window's contentItem leftTop position.
    popupItem()->setPosition(m_window->contentItem()->position());
    popupItem()->setParentItem(m_window->contentItem());

    m_window->show();
    m_window->requestActivate();
}

void DPopupWindowHandleImpl::close()
{
    if (!m_window->isActive() || !popup()->property("visible").toBool()) {
        m_window->hide();
        // window hide but popup's visible is true, and it effects popup next open.
        popup()->setProperty("visible", false);
    }
}
DQUICK_END_NAMESPACE

#include "moc_dpopupwindowhandle_p.cpp"

