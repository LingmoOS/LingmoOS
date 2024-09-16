// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.

// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DAppLoader>
#include <QGuiApplication>
#include <QQmlApplicationEngine>

DQUICK_USE_NAMESPACE

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    // 避免窗口菜单背景显示灰色 https://github.com/linuxdeepin/dtk/issues/70
    qputenv("D_POPUP_MODE", "embed");
    DAppLoader appLoader(APP_NAME, LOCALLIBPATH);
    return appLoader.exec(argc, argv);
}
