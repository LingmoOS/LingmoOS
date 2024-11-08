/*
 * Copyright (C) 2023 Lingmo OS Team
 *
 * Author:     Lingmo OS Team <lingmoos@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QGuiApplication>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QCommandLineParser>
#include <QMetaObject>
#include <QTranslator>
#include <QLocale>
#include <QIcon>
#include <QFile>

#include "documenthandler.h"
#include "highlightmodel.h"
#include "texteditor.h"

QStringList formatUriList(const QStringList &list)
{
    QStringList val;

    for (const QString &i : list) {
        QFileInfo path(i);
        if (path.exists()) {
            QString absPath = path.absoluteFilePath();
            if (!val.contains(absPath, Qt::CaseSensitive))
                val.append(absPath);
        }
        else
            qDebug() << "lingmo-texteditor: " << i << "doesn't exist";
    }

    return val;
}

void openFile(QObject *qmlObj, QString &fileUrl)
{
    QVariant val_return;
    QVariant val_arg(fileUrl);
    QMetaObject::invokeMethod(qmlObj,
                            "addPath",
                            Q_RETURN_ARG(QVariant,val_return),
                            Q_ARG(QVariant,val_arg));
}

void newTab(QObject *qmlObj)
{
    QVariant val_return;
    QVariant val_arg;
    QMetaObject::invokeMethod(qmlObj,
                            "addTab",
                            Q_RETURN_ARG(QVariant,val_return));
}

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);
    app.setOrganizationName("Lingmo");

    qmlRegisterType<DocumentHandler>("Lingmo.TextEditor", 1, 0, "DocumentHandler");
    qmlRegisterType<FileHelper>("Lingmo.TextEditor", 1, 0, "FileHelper");

    QCommandLineParser parser;
    parser.setApplicationDescription("A text editor specifically designed for LingmoOS.");
    parser.addHelpOption();
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsCompactedShortOptions);
    parser.addPositionalArgument("files", "Files", "[FILE1, FILE2,...]");

    parser.process(app);

    HighlightModel m;

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(url);

    QObject *root = engine.rootObjects().first();

    QStringList fileList = formatUriList(parser.positionalArguments());
    if (!fileList.isEmpty()) {
        for (QString &i : fileList)
            openFile(root, i);
    }
    else
        newTab(root);

    /** 加载翻译 */
    QLocale locale;
    QString qmFilePath = QString("%1/%2.qm").arg("/usr/share/lingmo-texteditor/translations/").arg(locale.name());
    if (QFile::exists(qmFilePath)) {
        QTranslator *translator = new QTranslator(QApplication::instance());
        if (translator->load(qmFilePath)) {
            QApplication::installTranslator(translator);
        } else {
            translator->deleteLater();
        }
    }

    return app.exec();
}
