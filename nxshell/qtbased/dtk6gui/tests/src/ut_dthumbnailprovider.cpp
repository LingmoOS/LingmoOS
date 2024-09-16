// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test.h"
#include "dthumbnailprovider.h"

#include <DObjectPrivate>
#include <QMimeDatabase>
#include <QQueue>
#include <QReadWriteLock>
#include <QSet>
#include <QSignalSpy>
#include <QWaitCondition>
#include <QDebug>
#include <QImageReader>

DGUI_BEGIN_NAMESPACE
#ifndef UT_DThumbnailProviderPrivate
#define UT_DThumbnailProviderPrivate
class DThumbnailProviderPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate
{
public:
    DThumbnailProviderPrivate(DThumbnailProvider *qq);

    void init();

    QString sizeToFilePath(DThumbnailProvider::Size size) const;

    QString errorString;
    // MAX
    qint64 defaultSizeLimit = INT64_MAX;
    QHash<QMimeType, qint64> sizeLimitHash;
    QMimeDatabase mimeDatabase;

    static QSet<QString> hasThumbnailMimeHash;

    struct ProduceInfo
    {
        QFileInfo fileInfo;
        DThumbnailProvider::Size size;
        DThumbnailProvider::CallBack callback;
    };

    QQueue<ProduceInfo> produceQueue;
    QSet<QPair<QString, DThumbnailProvider::Size>> discardedProduceInfos;

    bool running = true;

    QWaitCondition waitCondition;
    QReadWriteLock dataReadWriteLock;

    D_DECLARE_PUBLIC(DThumbnailProvider)
};
#endif
DGUI_END_NAMESPACE

DGUI_USE_NAMESPACE

class TDThumbnailProvider : public DTest
{
protected:
    void SetUp();
    void TearDown();

    DThumbnailProvider *provider;
    DThumbnailProviderPrivate *provider_d;
};

void TDThumbnailProvider::SetUp()
{
    provider = DThumbnailProvider::instance();
    provider_d = provider->d_func();
}

void TDThumbnailProvider::TearDown()
{
    // gtest会释放静态部分数据 导致双重释放程序崩溃这里手动将数据清空防止崩溃事情发生
    provider_d->hasThumbnailMimeHash.clear();
}

#define TESTRES_PATH ":/images/logo_icon.svg"
#define TESTRES_PATH_1 "no_exist"

TEST_F(TDThumbnailProvider, TestCreate)
{
    if (qgetenv("QT_QPA_PLATFORM").contains("offscreen"))
        return;

    QFileInfo fi(TESTRES_PATH);
    QSignalSpy finishedSpy(provider, SIGNAL(createThumbnailFinished(const QString &, const QString &)));
    QString ret = provider->createThumbnail(fi, DThumbnailProvider::Normal);
    ASSERT_FALSE(ret.isEmpty());
    ASSERT_EQ(finishedSpy.count(), 1);
    ASSERT_FALSE(provider->thumbnailFilePath(fi, DThumbnailProvider::Normal).isEmpty());

    QFileInfo fi_notexisted(TESTRES_PATH_1);
    QSignalSpy failedSpy(provider, SIGNAL(createThumbnailFailed(const QString &)));
    ret = provider->createThumbnail(fi_notexisted, DThumbnailProvider::Normal);
    ASSERT_TRUE(ret.isEmpty());
    ASSERT_EQ(finishedSpy.count(), 1);
    ASSERT_FALSE(provider->errorString().isEmpty());
    ASSERT_TRUE(provider->thumbnailFilePath(fi_notexisted, DThumbnailProvider::Normal).isEmpty());
}

TEST_F(TDThumbnailProvider, testAttribute)
{
    if (qgetenv("QT_QPA_PLATFORM").contains("offscreen"))
        return;

    ASSERT_TRUE(provider->hasThumbnail(QFileInfo(TESTRES_PATH)));

    enum { TEST_DEFALUTSIZELIMIT = 25 };
    provider->setDefaultSizeLimit(TEST_DEFALUTSIZELIMIT);
    ASSERT_EQ(provider->defaultSizeLimit(), TEST_DEFALUTSIZELIMIT);

    QMimeDatabase base;
    auto type = base.mimeTypeForFile(TESTRES_PATH);
    provider->setSizeLimit(type, TEST_DEFALUTSIZELIMIT);
    ASSERT_EQ(provider->sizeLimit(type), TEST_DEFALUTSIZELIMIT);
}

void testCallBack(const QString &)
{
    return;
}

TEST_F(TDThumbnailProvider, TestProducrQueue)
{
    provider_d->running = true;
    provider->appendToProduceQueue(QFileInfo(TESTRES_PATH), DThumbnailProvider::Small);
    provider->appendToProduceQueue(QFileInfo(TESTRES_PATH), DThumbnailProvider::Large, &testCallBack);
    ASSERT_FALSE(provider_d->produceQueue.isEmpty());

    provider->removeInProduceQueue(QFileInfo(TESTRES_PATH), DThumbnailProvider::Small);
    ASSERT_FALSE(provider_d->discardedProduceInfos.isEmpty());
    provider_d->running = false;
}
