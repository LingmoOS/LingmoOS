// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "qmlplugin.h"
#include "applet.h"
#include "appletitem.h"
#include "containmentitem.h"
#include "panel.h"
#include "dstypes.h"
#include "private/dsqmlglobal_p.h"
#include "quick/dsquickdrag_p.h"
#include "layershell/dlayershellwindow.h"

#include <qqml.h>

QML_DECLARE_TYPEINFO(DS_NAMESPACE::DLayerShellWindow, QML_HAS_ATTACHED_PROPERTIES)

DS_BEGIN_NAMESPACE

static inline void dsRegisterType(const char *uri, int versionMajor, int versionMinor, const char *qmlName)
{
    static QString urlTemplate = QStringLiteral("qrc:/ddeshell/qml/%1.qml");

    const QUrl url(urlTemplate.arg(qmlName));
    qmlRegisterType(url, uri, versionMajor, versionMinor, qmlName);
}

void QmlpluginPlugin::registerTypes(const char *uri)
{
    // @uri org.deepin.ds
    qmlRegisterModule(uri, 1, 0);

    qmlRegisterUncreatableType<Types>(uri, 1, 0, "Types", "Types");

    qmlRegisterAnonymousType<DApplet>(uri, 1);
    qmlRegisterType<DAppletItem>(uri, 1, 0, "AppletItem");
    qmlRegisterUncreatableType<DAppletItem>(uri, 1, 0, "Applet", "Applet Attached");
    qmlRegisterType<DContainmentItem>(uri, 1, 0, "ContainmentItem");
    qmlRegisterUncreatableType<DContainmentItem>(uri, 1, 0, "Containment", "Containment Attached");
    qmlRegisterUncreatableType<DPanel>(uri, 1, 0, "Panel", "Panel Attached");
    qmlRegisterSingletonInstance(uri, 1, 0, "DS", DQmlGlobal::instance());
    qmlRegisterType<DLayerShellWindow>(uri, 1, 0, "DLayerShellWindow");
    qmlRegisterUncreatableType<DLayerShellWindow>(uri, 1, 0, "DLayerShellWindow","LayerShell Attached");
    qmlRegisterUncreatableType<DQuickDrag>(uri, 1, 0, "DQuickDrag","DQuickDrag Attached");

    dsRegisterType(uri, 1, 0, "PanelPopup");
    dsRegisterType(uri, 1, 0, "PanelToolTip");
    dsRegisterType(uri, 1, 0, "PanelMenu");
    dsRegisterType(uri, 1, 0, "PanelPopupWindow");
    dsRegisterType(uri, 1, 0, "PanelToolTipWindow");
    dsRegisterType(uri, 1, 0, "PanelMenuWindow");
    dsRegisterType(uri, 1, 0, "QuickDragWindow");
}

void QmlpluginPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    QQmlExtensionPlugin::initializeEngine(engine, uri);
}

DS_END_NAMESPACE
