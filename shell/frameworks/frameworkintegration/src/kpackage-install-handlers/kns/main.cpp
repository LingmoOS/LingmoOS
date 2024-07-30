/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2016 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

#include <KLocalizedString>

#include <KNotification>

#include <KNSCore/EngineBase>
#include <KNSCore/Question>
#include <KNSCore/QuestionManager>
#include <KNSCore/ResultsStream>
#include <KNSCore/Transaction>

#include "knshandlerversion.h"

/**
 * Unfortunately there are two knsrc files for the window decorations, but only one is used in the KCM.
 * But both are used by third parties, consequently we can not remove one. To solve this we create a symlink
 * which links the old cache file to the new cache file, which is exposed on the GUI.
 * This way users can again remove window decorations that are installed as a dependency of a global theme.
 * BUG: 414570
 */
void createSymlinkForWindowDecorations()
{
    QFileInfo info(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/knewstuff3/aurorae.knsregistry"));
    // If we have created the symbolic link already we can exit the function here
    if (info.isSymbolicLink()) {
        return;
    }
    // Delete this file, it the KNS entries are not exposed in any GUI
    if (info.exists()) {
        QFile::remove(info.absoluteFilePath());
    }
    QFileInfo newFileInfo(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/knewstuff3/window-decorations.knsregistry"));
    QFile file(newFileInfo.absoluteFilePath());
    // Make sure that the file exists
    if (!newFileInfo.exists()) {
        file.open(QFile::WriteOnly);
        file.close();
    }
    file.link(info.absoluteFilePath());
}

int main(int argc, char **argv)
{
    createSymlinkForWindowDecorations();
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("kpackage-knshandler"));
    app.setApplicationVersion(knshandlerversion);
    app.setQuitLockEnabled(false);
    Q_ASSERT(app.arguments().count() == 2);

#ifdef TEST
    QStandardPaths::setTestModeEnabled(true);
#endif

    const QUrl url(app.arguments().last());
    Q_ASSERT(url.isValid());
    Q_ASSERT(url.scheme() == QLatin1String("kns"));

    QString knsname;
    const QStringList availableConfigFiles = KNSCore::EngineBase::availableConfigFiles();
    auto knsNameIt = std::find_if(availableConfigFiles.begin(), availableConfigFiles.end(), [&url](const QString &availableFile) {
        return availableFile.endsWith(QLatin1String("/") + url.host());
    });

    if (knsNameIt == availableConfigFiles.end()) {
        qWarning() << "couldn't find knsrc file for" << url.host();
        return 1;
    } else {
        knsname = *knsNameIt;
    }

    const auto pathParts = url.path().split(QLatin1Char('/'), Qt::SkipEmptyParts);
    if (pathParts.size() != 2) {
        qWarning() << "wrong format in the url path" << url << pathParts;
        return 1;
    }
    const auto providerid = pathParts.at(0);
    const auto entryid = pathParts.at(1);
    int linkid = 1;
    if (url.hasQuery()) {
        QUrlQuery query(url);
        if (query.hasQueryItem(QStringLiteral("linkid"))) {
            bool ok;
            linkid = query.queryItemValue(QStringLiteral("linkid")).toInt(&ok);
            if (!ok) {
                qWarning() << "linkid is not an integer" << url << pathParts;
                return 1;
            }
        }
    }

    KNSCore::EngineBase engine;
    int installedCount = 0;
    QObject::connect(KNSCore::QuestionManager::instance(), &KNSCore::QuestionManager::askQuestion, &engine, [](KNSCore::Question *question) {
        auto discardQuestion = [question]() {
            question->setResponse(KNSCore::Question::InvalidResponse);
        };
        switch (question->questionType()) {
        case KNSCore::Question::YesNoQuestion: {
            auto f = KNotification::event(KNotification::StandardEvent::Notification, question->title(), question->question());

            auto *yes = f->addAction(i18n("Yes"));
            QObject::connect(yes, &KNotificationAction::activated, question, [question] {
                question->setResponse(KNSCore::Question::YesResponse);
            });

            auto *no = f->addAction(i18n("No"));
            QObject::connect(no, &KNotificationAction::activated, question, [question] {
                question->setResponse(KNSCore::Question::NoResponse);
            });

            QObject::connect(f, &KNotification::closed, question, discardQuestion);
        } break;
        case KNSCore::Question::ContinueCancelQuestion: {
            auto f = KNotification::event(KNotification::StandardEvent::Notification, question->title(), question->question());

            auto *continueAction = f->addAction(i18n("Continue"));
            QObject::connect(continueAction, &KNotificationAction::activated, question, [question]() {
                question->setResponse(KNSCore::Question::ContinueResponse);
            });

            auto *cancelAction = f->addAction(i18n("Cancel"));
            QObject::connect(cancelAction, &KNotificationAction::activated, question, [question]() {
                question->setResponse(KNSCore::Question::CancelResponse);
            });

            QObject::connect(f, &KNotification::closed, question, discardQuestion);
        } break;
        case KNSCore::Question::InputTextQuestion:
        case KNSCore::Question::SelectFromListQuestion:
        case KNSCore::Question::PasswordQuestion:
            discardQuestion();
            break;
        }
    });

    const auto onError = [](KNSCore::ErrorCode::ErrorCode errorCode, const QString &message, const QVariant &metadata) {
        qWarning() << "kns error:" << errorCode << message << metadata;
        QCoreApplication::exit(1);
    };

    bool entryWasFound = false;
    const auto onEntriesLoded = [providerid, linkid, &installedCount, &engine, &entryWasFound, onError](const KNSCore::Entry::List list) {
        Q_ASSERT(list.size() == 1);
        entryWasFound = true;
        const auto entry = list.first();
        if (providerid != entry.providerId()) {
            qWarning() << "Wrong provider" << providerid << "instead of" << entry.providerId();
            QCoreApplication::exit(1);
        } else if (entry.status() == KNSCore::Entry::Downloadable) {
            qDebug() << "installing...";
            installedCount++;
            auto transaction = KNSCore::Transaction::install(&engine, entry, linkid);
            QObject::connect(transaction, &KNSCore::Transaction::signalErrorCode, onError);
            QObject::connect(transaction, &KNSCore::Transaction::signalEntryEvent, [&installedCount](auto entry, auto event) {
                if (event == KNSCore::Entry::StatusChangedEvent) {
                    if (entry.status() == KNSCore::Entry::Installed) {
                        installedCount--;
                    }
                    if (installedCount == 0) {
                        QCoreApplication::exit(0);
                    }
                }
            });
        } else if (installedCount == 0) {
            qDebug() << "already installed.";
            QCoreApplication::exit(0);
        }
    };
    QObject::connect(&engine, &KNSCore::EngineBase::signalProvidersLoaded, &engine, [&engine, &entryid, onEntriesLoded, &entryWasFound]() {
        qWarning() << "providers are loaded";
        KNSCore::Provider::SearchRequest request(KNSCore::Provider::Newest, KNSCore::Provider::ExactEntryId, entryid, QStringList{}, 0);
        KNSCore::ResultsStream *results = engine.search(request);
        QObject::connect(results, &KNSCore::ResultsStream::entriesFound, &engine, onEntriesLoded);
        QObject::connect(results, &KNSCore::ResultsStream::finished, &engine, [&entryWasFound, &entryid]() {
            if (!entryWasFound) {
                qWarning() << "Entry with id" << entryid << "could not be found";
                QCoreApplication::exit(1);
            }
        });
        results->fetch();
    });

    QObject::connect(&engine, &KNSCore::EngineBase::signalErrorCode, &engine, onError);
    if (!engine.init(knsname)) {
        qWarning() << "couldn't initialize" << knsname;
        return 1;
    }
    return app.exec();
}
