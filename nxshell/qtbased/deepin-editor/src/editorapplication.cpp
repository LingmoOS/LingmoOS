// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "editorapplication.h"
#include "common/settings.h"
#include <QDebug>

EditorApplication::EditorApplication(int &argc, char *argv[]) : DApplication(argc, argv)
{
    const char *descriptionText = QT_TRANSLATE_NOOP(
        "MainWindow", "Text Editor is a powerful tool for viewing and editing text files.");
    const QString acknowledgementLink = "https://www.deepin.org/original/deepin-editor/";

    setAttribute(Qt::AA_UseHighDpiPixmaps);
    loadTranslator();
    setOrganizationName("deepin");
    setApplicationName("deepin-editor");
    setApplicationDisplayName(QObject::tr("Text Editor"));
    setApplicationVersion(VERSION);
    setProductIcon(QIcon::fromTheme("deepin-editor"));
    setProductName(DApplication::translate("MainWindow", "Text Editor"));
    setApplicationDescription(DApplication::translate("MainWindow", descriptionText) + "\n");
    setApplicationAcknowledgementPage(acknowledgementLink);
    setQuitOnLastWindowClosed(false);
}

EditorApplication::~EditorApplication()
{
    // app结束时，释放
    if (nullptr != StartManager::instance()) {
        delete StartManager::instance();
    }
}

void EditorApplication::handleQuitAction()
{
    activeWindow()->close();
}

bool EditorApplication::notify(QObject *object, QEvent *event)
{
    // ALT+M = 右键
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyevent = static_cast<QKeyEvent *>(event);
        /***add begin by ut001121 zhangmeng 20200801 截获DPushButton控件回车按键事件并模拟空格键点击事件,用以解决回车键不响应的问题***/
        // 回车键 恢复默认 添健按钮
        if ((object->metaObject()->className() == QStringLiteral("QPushButton")
                // 远程和自定义列表的返回按钮，编辑按钮
                || object->metaObject()->className() == QStringLiteral("IconButton")
                // 搜索框的上下搜索
                || object->metaObject()->className() == QStringLiteral("Dtk::Widget::DIconButton")
                // 设置里面的单选框
                || object->metaObject()->className() == QStringLiteral("QCheckBox")
                // 设置字体组合框
                || object->metaObject()->className() == QStringLiteral("QComboBox")
                // 设置窗口组合框
                || object->metaObject()->className() == QStringLiteral("ComboBox"))
                && (keyevent->key() == Qt::Key_Return || keyevent->key() == Qt::Key_Enter)) {
            DPushButton *pushButton = static_cast<DPushButton *>(object);
            // 模拟空格键按下事件
            pressSpace(pushButton);
            return true;
        }
        /***add end by ut001121***/
        // 左键
        // 远程和自定义列表的返回按钮 Key_Left
        if ((object->objectName() == QStringLiteral("CustomRebackButton")
                || object->objectName() == QStringLiteral("RemoteSearchRebackButton")
                || object->objectName() == QStringLiteral("RemoteGroupRebackButton"))
                && keyevent->key() == Qt::Key_Left) {
            DPushButton *pushButton = static_cast<DPushButton *>(object);
            // 模拟空格键按下事件
            pressSpace(pushButton);
            return true;
        }

        return QApplication::notify(object, event);
    }

    return QApplication::notify(object, event);
}

void EditorApplication::pressSpace(DPushButton *pushButton)
{
    // 模拟空格键按下事件
    QKeyEvent pressSpace(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier, " ");
    QApplication::sendEvent(pushButton, &pressSpace);
    // 设置定时
    QTimer::singleShot(80, this, [pushButton]() {
        // 模拟空格键松开事件
        QKeyEvent releaseSpace(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier, " ");
        QApplication::sendEvent(pushButton, &releaseSpace);
    });
}
