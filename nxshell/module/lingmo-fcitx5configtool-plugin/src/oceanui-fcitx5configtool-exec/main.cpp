// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>

#include <configwidget.h>
#include <dbusprovider.h>

using namespace fcitx::kcm;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("oceanui-fcitx5configtool-exec");
    app.setApplicationVersion("1.0");

    // 创建命令行解析器
    QCommandLineParser parser;
    parser.setApplicationDescription("Fcitx5 Configuration Tool");
    parser.addHelpOption();
    parser.addVersionOption();

    const QString &transPath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    QStringList qtTrans = { QDir(transPath).filePath("qt_"), QDir(transPath).filePath("qtbase_") };
    for (const QString &trans : qtTrans) {
        QTranslator *translator = new QTranslator;
        const QString filename = trans + QLocale::system().name();
        bool ret = translator->load(filename);
        qInfo() << "Loading translator" << filename;

        if (!ret) {
            delete translator;
            continue;
        }

        app.installTranslator(translator);
    }

    // 添加命令行选项
    QCommandLineOption uriOption(QStringList() << "u"
                                               << "uri",
                                 "Specify uri",
                                 "uri");
    parser.addOption(uriOption);

    QCommandLineOption titleOption(QStringList() << "t"
                                                 << "title",
                                   "Specify dialog title",
                                   "title");
    parser.addOption(titleOption);

    // 解析命令行参数
    parser.process(app);

    // 获取参数值
    QString uri = parser.value(uriOption);
    QString title = parser.value(titleOption);
    qInfo() << "Received uri:" << uri << "title:" << title;
    if (uri.isEmpty()) {
        qWarning() << "Uri is required!";
        parser.showHelp(1);
    }

    DBusProvider *dbusProvider = new DBusProvider(&app);

    QPointer<QDialog> dialog = ConfigWidget::configDialog(nullptr,
                                                          dbusProvider,
                                                          uri,
                                                          title);
    if (!dialog) {
        qWarning() << "Failed to create config dialog!";
        return 1;
    }

    return dialog->exec();
}
