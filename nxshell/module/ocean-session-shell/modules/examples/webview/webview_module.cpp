// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "webview_module.h"

#include <QWidget>

namespace dss {
namespace module {

WebviewModule::WebviewModule(QObject *parent)
    : QObject(parent)
    , m_webviewContent(nullptr)
{
    setObjectName(QStringLiteral("WebviewModule"));
}

WebviewModule::~WebviewModule()
{
    if (m_webviewContent) {
        delete m_webviewContent;
    }
}

void WebviewModule::init()
{
    initUI();
}

void WebviewModule::initUI()
{
    if (m_webviewContent) {
        return;
    }
    m_webviewContent = new WebviewContent();
}

} // namespace module
} // namespace dss
