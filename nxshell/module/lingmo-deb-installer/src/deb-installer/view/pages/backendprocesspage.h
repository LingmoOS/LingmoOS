// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKENDPROCESSPAGE_H
#define BACKENDPROCESSPAGE_H

#include <QWidget>

class ProcessWidget;
class NoProcessWidget;
class QStackedLayout;

class BackendProcessPage : public QWidget
{
    Q_OBJECT
public:
    enum DisplayMode {
        APT_INIT,    // apt初始化
        READ_PKG,    // 读取包数据
        PROCESS_FIN  // 处理结束
    };

    explicit BackendProcessPage(QWidget *parent = nullptr);

    void setDisplayPage(DisplayMode mode);
    void setPkgProcessRate(int currentRate, int pkgCount);

signals:

public slots:

private:
    ProcessWidget *processWidget;
    NoProcessWidget *noProcessWidget;
    QStackedLayout *allLayout;
};

#endif  // BACKENDPROCESSPAGE_H
