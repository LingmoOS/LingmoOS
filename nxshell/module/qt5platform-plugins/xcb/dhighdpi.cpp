// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dhighdpi.h"
#include "vtablehook.h"
#include "dxcbxsettings.h"
#include "dplatformintegration.h"

#include "qxcbscreen.h"
#include "qxcbwindow.h"
#include "qxcbbackingstore.h"

#include <private/qhighdpiscaling_p.h>
#include <private/qguiapplication_p.h>
#include <QGuiApplication>
#include <QDebug>

DPP_BEGIN_NAMESPACE

bool DHighDpi::active = false;
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
QHash<QPlatformScreen*, qreal> DHighDpi::screenFactorMap;
#endif
QPointF DHighDpi::fromNativePixels(const QPointF &pixelPoint, const QWindow *window)
{
    return QHighDpi::fromNativePixels(pixelPoint, window);
}

inline static void init()
{
    // 禁用platform theme中的缩放机制
    // 当DHighDpi存在时不应该再使用这个过时的机制
    qputenv("D_DISABLE_RT_SCREEN_SCALE", "1");

    DHighDpi::init();
}
// 在插件被加载时先做一次初始化
Q_CONSTRUCTOR_FUNCTION(init)
void DHighDpi::init()
{
    if (QGuiApplication::testAttribute(Qt::AA_DisableHighDpiScaling)
            // 可以禁用此行为
            || qEnvironmentVariableIsSet("D_DXCB_DISABLE_OVERRIDE_HIDPI")
            // 无有效的xsettings时禁用
            || !DXcbXSettings::getOwner()) {
        // init函数可能会被重复调用, 此处应该清理VtableHook
        if (active) {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
            VtableHook::resetVfptrFun(&QXcbScreen::pixelDensity);
#endif
            VtableHook::resetVfptrFun(&QXcbScreen::logicalDpi);
            active = false;
        }
        return;
    }

    // 设置为完全控制缩放比例，避免被Qt“4舍5入”了缩放比
    qputenv("QT_SCALE_FACTOR_ROUNDING_POLICY", "PassThrough");

    // 强制开启使用DXCB的缩放机制，此时移除会影响此功能的所有Qt环境变量
    if (qEnvironmentVariableIsSet("D_DXCB_FORCE_OVERRIDE_HIDPI")) {
        qunsetenv("QT_AUTO_SCREEN_SCALE_FACTOR");
        qunsetenv("QT_SCALE_FACTOR");
        qunsetenv("QT_SCREEN_SCALE_FACTORS");
        qunsetenv("QT_ENABLE_HIGHDPI_SCALING");
        qunsetenv("QT_USE_PHYSICAL_DPI");
    }

    if (!QGuiApplication::testAttribute(Qt::AA_EnableHighDpiScaling)) {
        QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
        QHighDpiScaling::initHighDpiScaling();
    }

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    active = VtableHook::overrideVfptrFun(&QXcbScreen::pixelDensity, pixelDensity);

    if (active) {
        VtableHook::overrideVfptrFun(&QXcbScreen::logicalDpi, logicalDpi);
    }
#else
    active = VtableHook::overrideVfptrFun(&QXcbScreen::logicalDpi, logicalDpi);
#endif
}

bool DHighDpi::isActive()
{
    return active;
}

bool DHighDpi::overrideBackingStore()
{
    // 默认不开启，会降低绘图效率
    static bool enabled = qEnvironmentVariableIsSet("D_DXCB_HIDPI_BACKINGSTORE");
    return enabled;
}

QDpi DHighDpi::logicalDpi(QXcbScreen *s)
{
    static bool dpi_env_set = qEnvironmentVariableIsSet("QT_FONT_DPI");
    // 遵循环境变量的设置
    if (dpi_env_set) {
        return s->QXcbScreen::logicalDpi();
    }

    int dpi = 0;
    const QString screenName(QString("Qt/DPI/%1").arg(s->name()));
    QVariant value = DPlatformIntegration::xSettings(s->connection())->setting(screenName.toLocal8Bit());
    bool ok = false;

    dpi = value.toInt(&ok);

    // fallback
    if (!ok) {
        value = DPlatformIntegration::xSettings(s->connection())->setting("Xft/DPI");
        dpi = value.toInt(&ok);
    }

    // fallback
    if (!ok || dpi == 0) {
        qWarning() << "dpi is invalid got from xsettings(Qt/DPI/ and Xft/DPI), "
                      "fallback to get dpi from QXcbScreen::logicalDpi()";
        return s->QXcbScreen::logicalDpi();
    }

    qreal d = dpi / 1024.0;

    return QDpi(d, d);
}

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
qreal DHighDpi::pixelDensity(QXcbScreen *s)
{
    qreal scale = screenFactorMap.value(s, 0);

    if (qIsNull(scale)) {
        scale = s->logicalDpi().first / 96.0;
        screenFactorMap[s] = scale;
    }

    return scale;
}
#endif

qreal DHighDpi::devicePixelRatio(QPlatformWindow *w)
{
    qreal base_factor = QHighDpiScaling::factor(w->screen());
    return qCeil(base_factor) / base_factor;
}

void DHighDpi::onDPIChanged(xcb_connection_t *connection, const QByteArray &name, const QVariant &property, void *handle)
{
    static bool dynamic_dpi = qEnvironmentVariableIsSet("D_DXCB_RT_HIDPI");

    if (!dynamic_dpi)
        return;

    Q_UNUSED(connection)
    Q_UNUSED(name)

    // 判断值是否有效
    if (!property.isValid())
        return;

    qInfo() << Q_FUNC_INFO << name << property;

    // 清理过期的屏幕缩放值
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    if (QScreen *screen = reinterpret_cast<QScreen*>(handle)) {
        screenFactorMap.remove(screen->handle());
    } else {
        screenFactorMap.clear();
#else
    {
#endif
        // 刷新所有窗口
        for (QWindow *window : qGuiApp->allWindows()) {
            if (window->type() == Qt::Desktop)
                continue;

            // 更新窗口大小
            if (window->handle()) {
                QWindowSystemInterfacePrivate::GeometryChangeEvent gce(window, QHighDpi::fromNativePixels(window->handle()->geometry(), window));
                QGuiApplicationPrivate::processGeometryChangeEvent(&gce);
            }
        }
    }
}

DPP_END_NAMESPACE
