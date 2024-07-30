/*
    This file is part of the Purpose library.
    SPDX-FileCopyrightText: 2017 René J.V. Bertin <rjvbertin@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QString>

#include "../phabricatorjobs.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QString projectName;
    QString diffID;
    QString patchFile;
    QString updateComment;

    app.setApplicationName(QStringLiteral("testphabricator"));
    QCommandLineParser parser;
    const QCommandLineOption projectNameOption(QStringLiteral("project"),
                                               QStringLiteral("a directory holding the project"),
                                               QStringLiteral("project"),
                                               projectName);
    const QCommandLineOption diffIDOption(QStringLiteral("ID"),
                                          QStringLiteral("set the revision ID to update (when missing, create a new diff)"),
                                          QStringLiteral("ID"),
                                          diffID);
    const QCommandLineOption patchFileOption(QStringLiteral("patch"), QStringLiteral("the patch to upload"), QStringLiteral("patch"), patchFile);
    const QCommandLineOption updateCommentOption(QStringLiteral("message"),
                                                 QStringLiteral("comment describing the patch update"),
                                                 QStringLiteral("message"),
                                                 updateComment);
    const QCommandLineOption listOption(QStringLiteral("list"), QStringLiteral("list your open differential revisions"));
    parser.addOption(projectNameOption);
    parser.addOption(diffIDOption);
    parser.addOption(patchFileOption);
    parser.addOption(updateCommentOption);
    parser.addOption(listOption);
    parser.addHelpOption();
    parser.addVersionOption();

    parser.process(app);
    if (parser.isSet(projectNameOption)) {
        projectName = parser.value(projectNameOption);
    }
    if (parser.isSet(listOption)) {
        Phabricator::DiffRevList diffList(projectName);
        if (diffList.error()) {
            qCritical() << "Error creating diffList:" << diffList.errorString() << ";" << diffList.error();
        } else {
            diffList.exec();
            if (diffList.error()) {
                qCritical() << "Error getting diffList:" << diffList.errorString() << ";" << diffList.error();
            } else {
                qWarning() << "Open differential revisions:" << diffList.reviewMap();
                const auto reviews = diffList.reviews();
                for (const auto &rev : reviews) {
                    qWarning() << rev;
                }
            }
        }
    } else {
        if (parser.isSet(diffIDOption)) {
            diffID = parser.value(diffIDOption);
        }
        if (parser.isSet(patchFileOption)) {
            patchFile = parser.value(patchFileOption);
            if (diffID.isEmpty()) {
                Phabricator::NewDiffRev newDiffRev(QUrl::fromLocalFile(patchFile), projectName);
                newDiffRev.exec();
                if (newDiffRev.error()) {
                    qCritical() << "Error creating new diff diff:" << newDiffRev.errorString() << ";" << newDiffRev.error();
                } else {
                    qWarning() << "New differential diff to be completed online:" << newDiffRev.diffURI();
                }
            } else {
                if (parser.isSet(updateCommentOption)) {
                    updateComment = parser.value(updateCommentOption);
                }
                Phabricator::UpdateDiffRev submitDiffRev(QUrl::fromLocalFile(patchFile), projectName, diffID, updateComment);
                submitDiffRev.exec();
                if (submitDiffRev.error()) {
                    qCritical() << "Error creating new diff diff:" << submitDiffRev.errorString() << ";" << submitDiffRev.error();
                } else {
                    qWarning() << "Updated differential revision; please edit comment online:" << submitDiffRev.diffURI();
                }
            }
        } else {
            qCritical() << "need a patchfile";
        }
    }
    exit(0);
}
