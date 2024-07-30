/*
    This file is part of LibSyndication.
    SPDX-FileCopyrightText: 2005 Frank Osterfeld <osterfeld@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "testloader.h"

#include "atom/parser.h"
#include "dataretriever.h"
#include "feed.h"
#include "specificdocument.h"
#include "syndication_version.h"

#include <QCoreApplication>

#include <QUrl>

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>

#include <cstdlib>
#include <iostream>

using namespace Syndication;

class SimpleRetriever : public Syndication::DataRetriever
{
    Q_OBJECT
public:
    explicit SimpleRetriever()
        : mMgr(new QNetworkAccessManager(this))
        , mReply(nullptr)
    {
    }
    ~SimpleRetriever() override
    {
        delete mReply;
        delete mMgr;
    }

    void retrieveData(const QUrl &url) override
    {
        mReply = mMgr->get(QNetworkRequest(url));
        connect(mReply, &QNetworkReply::finished, this, [this]() {
            if (mReply->error() == QNetworkReply::NoError) {
                Q_EMIT dataRetrieved(mReply->readAll(), true);
            } else {
                Q_EMIT dataRetrieved({}, false);
            }
        });
    }
    void abort() override
    {
        if (mReply) {
            mReply->abort();
        }
    }
    int errorCode() const override
    {
        if (mReply) {
            return mReply->error();
        } else {
            return 0;
        }
    }

private:
    QNetworkAccessManager *const mMgr;
    QNetworkReply *mReply;
};

TestLibSyndication::TestLibSyndication(const QString &url)
{
    QUrl kurl = QUrl::fromUserInput(url);

    std::cerr << kurl.url().toLocal8Bit().data() << std::endl;
    Loader *loader = Loader::create(this, SLOT(slotLoadingComplete(Syndication::Loader *, Syndication::FeedPtr, Syndication::ErrorCode)));
    loader->loadFrom(kurl, new SimpleRetriever());
}

void TestLibSyndication::slotLoadingComplete(Syndication::Loader *loader, Syndication::FeedPtr feed, Syndication::ErrorCode error)
{
    Q_UNUSED(loader)
    Q_UNUSED(error)

    if (feed) {
        std::cout << feed->debugInfo().toUtf8().data() << std::endl;
        exit(0);
    } else {
        std::cerr << "error: " << error << std::endl;
        exit(1);
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        std::cerr << "filename expected" << std::endl;
        return 1;
    }

    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    app.setApplicationVersion(QStringLiteral(SYNDICATION_VERSION_STRING));
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("+url"), QStringLiteral("URL of feed")));
    parser.process(app);

    if (parser.positionalArguments().count() != 1) {
        parser.showHelp();
    }

    TestLibSyndication *tester = new TestLibSyndication(parser.positionalArguments().at(0));
    Q_UNUSED(tester)

    return app.exec();
}

#include "moc_testloader.cpp"
#include "testloader.moc"
