// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WEBVIEWCONTENT_H
#define WEBVIEWCONTENT_H

#include <QWidget>
#include <QtWebEngineWidgets/qwebengineview.h>

class QVBoxLayout;

namespace dss {
namespace module {

class WebviewContent : public QWidget
{
    Q_OBJECT

public:
    explicit WebviewContent(QWidget *parent = nullptr);

private:
    void initUI();

private:
    QVBoxLayout *m_mainLayout;
    QWebEngineView *m_webview;
};

} // namespace module
} // namespace dss
#endif // WEBVIEWCONTENT_H
