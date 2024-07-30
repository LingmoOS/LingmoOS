/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QFileDialog>
#include <QMetaEnum>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("QFileDialogTest"));
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList(QStringLiteral("staticFunction")),
                                        QStringLiteral("Test one of the static convenience function: 'getOpenFileUrl', 'getExistingDirectory'"),
                                        QStringLiteral("function name")));
    parser.addOption(QCommandLineOption(QStringList(QStringLiteral("acceptMode")),
                                        QStringLiteral("File dialog acceptMode: 'open' or 'save'"),
                                        QStringLiteral("type"),
                                        QStringLiteral("open")));
    parser.addOption(QCommandLineOption(QStringList(QStringLiteral("confirmOverwrite")),
                                        QStringLiteral("Test overwrite option: 'on' or 'off'"),
                                        QStringLiteral("option"),
                                        QStringLiteral("on")));
    parser.addOption(QCommandLineOption(QStringList(QStringLiteral("nativeDialog")),
                                        QStringLiteral("Use the platform native dialog: 'on' or 'off'"),
                                        QStringLiteral("option"),
                                        QStringLiteral("on")));
    parser.addOption(QCommandLineOption(QStringList(QStringLiteral("fileMode")),
                                        QStringLiteral("File dialog fileMode: 'AnyFile' or 'ExistingFile' or 'Directory' or 'ExistingFiles'"),
                                        QStringLiteral("type")));
    parser.addOption(QCommandLineOption(QStringList(QStringLiteral("nameFilter")),
                                        QStringLiteral("Dialog nameFilter, e. g. 'cppfiles (*.cpp *.h *.hpp)', can be specified multiple times"),
                                        QStringLiteral("nameFilter"),
                                        QStringLiteral("Everything (*)")));
    parser.addOption(QCommandLineOption(QStringList(QStringLiteral("mimeTypeFilter")),
                                        QStringLiteral("Dialog mimeTypeFilter, e. g. 'application/json', can be specified multiple times"),
                                        QStringLiteral("mimeTypeFilter")));
    parser.addOption(QCommandLineOption(QStringList(QStringLiteral("selectNameFilter")),
                                        QStringLiteral("Initially selected nameFilter"),
                                        QStringLiteral("selectNameFilter")));
    parser.addOption(QCommandLineOption(QStringList(QStringLiteral("selectMimeTypeFilter")),
                                        QStringLiteral("Initially selected mimeTypeFilter"),
                                        QStringLiteral("selectMimeTypeFilter")));
    parser.addOption(QCommandLineOption(QStringList(QStringLiteral("selectFile")), QStringLiteral("Initially selected file"), QStringLiteral("filename")));
    parser.addOption(
        QCommandLineOption(QStringList(QStringLiteral("selectDirectory")), QStringLiteral("Initially selected directory"), QStringLiteral("dirname")));
    parser.addOption(
        QCommandLineOption(QStringList(QStringLiteral("modal")), QStringLiteral("Test modal dialog"), QStringLiteral("modality"), QStringLiteral("on")));
    parser.addOption(QCommandLineOption(QStringList(QStringLiteral("options")), QStringLiteral("See QFileDialog::Options"), QStringLiteral("option")));
    parser.process(app);

    const QString staticFunction = parser.value(QStringLiteral("staticFunction"));
    if (staticFunction == QLatin1String("getExistingDirectory")) {
        QString dir = QFileDialog::getExistingDirectory(nullptr, QStringLiteral("getExistingDirectory test"), QStringLiteral("/tmp"));
        qDebug() << dir;
        return 0;
    } else if (staticFunction == QLatin1String("getOpenFileUrl")) {
        qDebug() << QFileDialog::getOpenFileUrl(nullptr, QStringLiteral("getOpenFileUrl test"), QUrl::fromLocalFile(QDir::homePath()));
        return 0;
    }

    QFileDialog dialog;
    dialog.setAcceptMode(parser.value(QStringLiteral("acceptMode")) == QStringLiteral("open") ? QFileDialog::AcceptOpen : QFileDialog::AcceptSave);

    QString fileModeValue = parser.value(QStringLiteral("fileMode"));
    if (fileModeValue == QLatin1String("AnyFile")) {
        dialog.setFileMode(QFileDialog::AnyFile);
    } else if (fileModeValue == QLatin1String("ExistingFile")) {
        dialog.setFileMode(QFileDialog::ExistingFile);
    } else if (fileModeValue == QLatin1String("ExistingFiles")) {
        dialog.setFileMode(QFileDialog::ExistingFiles);
    } else if (fileModeValue == QLatin1String("Directory")) {
        dialog.setFileMode(QFileDialog::Directory);
    } else if (!fileModeValue.isEmpty()) {
        qDebug() << "Not implemented or not valid:" << fileModeValue;
        exit(0);
    }

    QStringList nameFilterList = parser.values(QStringLiteral("nameFilter"));
    if (nameFilterList.size() == 1) {
        dialog.setNameFilter(nameFilterList.first());
    } else {
        dialog.setNameFilters(nameFilterList);
    }

    if (parser.isSet(QStringLiteral("options"))) {
        auto optStrings = parser.values(QStringLiteral("options"));
        QFileDialog::Options options = {};
        const auto mo = QFileDialog::staticMetaObject;
        const auto enumerator = mo.indexOfEnumerator("Options");
        for (const auto &optString : std::as_const(optStrings)) {
            options |= QFileDialog::Option(mo.enumerator(enumerator).keyToValue(optString.toLatin1().constData()));
        }
        dialog.setOptions(options);
    }

    const auto mimeFilterList = parser.values(QStringLiteral("mimeTypeFilter"));
    if (!mimeFilterList.isEmpty()) {
        dialog.setMimeTypeFilters(mimeFilterList);
    }

    QString selectNameFilter = parser.value(QStringLiteral("selectNameFilter"));
    QString selectMimeTypeFilter = parser.value(QStringLiteral("selectMimeTypeFilter"));
    if (!selectNameFilter.isEmpty()) {
        dialog.selectNameFilter(selectNameFilter);
    } else if (!selectMimeTypeFilter.isEmpty()) {
        dialog.selectMimeTypeFilter(selectMimeTypeFilter);
    }

    if (parser.value(QStringLiteral("confirmOverwrite")) == QStringLiteral("off")) {
        dialog.setOption(QFileDialog::DontConfirmOverwrite, true);
    }

    if (parser.value(QStringLiteral("nativeDialog")) == QStringLiteral("off")) {
        dialog.setOption(QFileDialog::DontUseNativeDialog, true);
    }

    dialog.setDirectoryUrl(QUrl::fromUserInput(parser.value(QStringLiteral("selectDirectory")), {}, QUrl::AssumeLocalFile));
    dialog.selectFile(parser.value(QStringLiteral("selectFile")));

    int ret;
    if (parser.value(QStringLiteral("modal")) == QStringLiteral("off")) {
        dialog.show();
        ret = app.exec();
    } else {
        ret = dialog.exec();
    }

    if (dialog.result() == QDialog::Accepted) {
        qDebug() << "selected files" << dialog.selectedFiles();
        qDebug() << "selected urls" << dialog.selectedUrls();
        qDebug() << "selected mime type filter" << dialog.selectedMimeTypeFilter();
    }

    qDebug() << "mime type filter(s):" << dialog.mimeTypeFilters();

    return ret;
}
