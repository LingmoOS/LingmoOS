// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DNativeSettings>

#include <QGuiApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <QColor>
#include <QProcess>

DGUI_USE_NAMESPACE

int main(int argc, char *argv[])
{
    // 禁用输入法，防止输入法插件乱输出内容
    qunsetenv("QT_IM_MODULE");
    qputenv("QT_QPA_PLATFORM", "dxcb");
    QGuiApplication app(argc, argv);
    qunsetenv("QT_QPA_PLATFORM");

    QCommandLineParser parser;
    QCommandLineOption option_window({"w", "window"}, "resource id of window to examine");
    QCommandLineOption option_window_leader("window-leader", "use leader window of the window");
    QCommandLineOption option_select_window("select", "auto select a window on screen");
    QCommandLineOption option_domain({"p", "domain"}, "domain of settings property");
    QCommandLineOption option_set("set", "set a settings item to a given value. Can only and must specify a value(eg: [-s|-i|-c])");
    QCommandLineOption option_string({"s", "string"}, "set a string value of a settings item");
    QCommandLineOption option_int({"i", "int"}, "set a int value of a settings item");
    QCommandLineOption option_color({"c", "color"}, "set a color value of a settings item");
    QCommandLineOption option_remove({"r", "remove"}, "remove a settings item");

    option_window.setValueName("id");
    option_window.setDefaultValue("0");
    option_domain.setValueName("domain");
    option_domain.setDefaultValue(QString());
    option_set.setValueName("key");
    option_string.setValueName("value");
    option_int.setValueName("value");
    option_color.setValueName("value");
    option_remove.setValueName("key");

    parser.addOption(option_window);
    parser.addOption(option_window_leader);
    parser.addOption(option_select_window);
    parser.addOption(option_domain);
    parser.addOption(option_set);
    parser.addOption(option_string);
    parser.addOption(option_int);
    parser.addOption(option_color);
    parser.addOption(option_remove);
    parser.addPositionalArgument("keys", "key of get settings value", "[keys...]");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    quint32 window_id = 0;

    if (parser.isSet(option_select_window)) {
        QProcess xdotool;

        xdotool.start("xdotool", {"selectwindow"}, QIODevice::ReadOnly);

        if (!xdotool.waitForFinished()) {
            parser.showHelp(1);
        }

        bool ok = false;
        window_id = xdotool.readAllStandardOutput().trimmed().toInt(&ok);

        if (!ok) {
            parser.showHelp(1);
        }
    } else {
        const QString &window_id_value = parser.value(option_window);
        bool ok = false;

        if (window_id_value.startsWith("0x")) {
            window_id = window_id_value.toInt(&ok, 16);
        } else {
            window_id = window_id_value.toInt(&ok);
        }

        if (!ok) {
            parser.showHelp(-1);
        }
    }

    if (parser.isSet(option_window_leader)) {
        QProcess xprop;
        xprop.start("xprop", {"-id", QString::number(window_id), "WM_CLIENT_LEADER"}, QIODevice::ReadOnly);

        if (!xprop.waitForFinished()) {
            qFatal("%s\n", xprop.errorString().toLocal8Bit().constData());
            return -1;
        }

        const QByteArrayList &list = xprop.readAllStandardOutput().split(' ');
        bool ok = false;

        if (!list.isEmpty()) {
            quint32 id = list.last().trimmed().toInt(&ok, 16);

            if (ok) {
                window_id = id;
            }
        }

        if (!ok) {
            qFatal("%s\n", "not found WM_CLIENT_LEADER");
            return -1;
        }
    }

    DNativeSettings settings(window_id, parser.value(option_domain).toLatin1());

    if (!settings.isValid()) {
        qWarning() << "Settings is invalid, platform plugin is:" << qApp->platformName();
        return -1;
    }

    if (parser.isSet(option_set)) {
        QVariant value;

        if (parser.isSet(option_string)) { // string
            if (parser.isSet(option_int) || parser.isSet(option_color)) {
                parser.showHelp(-1);
            }

            value = parser.value(option_string).toLocal8Bit();
        } else if (parser.isSet(option_int)) { // int
            if (parser.isSet(option_string) || parser.isSet(option_color)) {
                parser.showHelp(-1);
            }

            bool ok = false;
            int integer = parser.value(option_int).toInt(&ok);

            if (!ok) {
                parser.showHelp(-1);
            }

            value = QVariant(integer);
        } else if (parser.isSet(option_color)) { // color
            if (parser.isSet(option_string) || parser.isSet(option_int)) {
                parser.showHelp(-1);
            }

            QColor color(parser.value(option_color));

            if (!color.isValid()) {
                parser.showHelp(-1);
            }

            value = QVariant(color);
        } else {
            parser.showHelp(-1);
        }

        // 设置一个属性的值
        const QByteArray &name = parser.value(option_set).toLatin1();

        if (name.isEmpty()) {
            parser.showHelp(-1);
        }

        settings.setSetting(name, value);
        qDebug() << name << settings.getSetting(name);
    } else if (parser.isSet(option_remove)) {
        const QByteArray &name = parser.value(option_remove).toLatin1();

        if (name.isEmpty()) {
            parser.showHelp(-1);
        }

        settings.setSetting(name, QVariant());
        qDebug() << name << settings.getSetting(name);
    } else {
        const QStringList &keys = parser.positionalArguments();

        if (keys.isEmpty()) {
            // 打印所有设置项
            qDebug() << settings;
        } else {
            for (const QString &key : keys) {
                qDebug() << key << settings.getSetting(key.toLatin1());
            }
        }
    }

    return 0;
}
