// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "statictextworker.h"
#include "global/builtinsearch.h"
#include "global/searchconfigdefine.h"
#include "configuration/configer.h"
#include "global/searchhelper.h"

#include <QByteArray>

using namespace GrandSearch;

StaticTextWorker::StaticTextWorker(const QString &name, QObject *parent) : ProxyWorker(name, parent)
{

}

void StaticTextWorker::setContext(const QString &context)
{
    if (context.isEmpty())
        qWarning() << "search key is empty.";
    m_context = context;
}

bool StaticTextWorker::isAsync() const
{
    //同步搜索，需分配线程资源
    return false;
}

bool StaticTextWorker::working(void *context)
{
    //准备状态切运行中，否则直接返回
    if (!m_status.testAndSetRelease(Ready, Runing))
        return false;

    Q_UNUSED(context);
    if (m_context.isEmpty()) {
        m_status.storeRelease(Completed);
        return true;
    }

    auto config = ConfigerIns->group(GRANDSEARCH_WEB_GROUP);
    auto searchEngine = config->value(GRANDSEARCH_WEB_SEARCHENGINE, QString(""));
    QString defaultUrl = createUrl(searchEngine);

    MatchedItem ret;

    QString encodeString(QUrl::toPercentEncoding(m_context));
    QString url = defaultUrl.arg(QString(encodeString));

    ret.item = url;
    ret.name = tr("Search for \"%1\"").arg(m_context);
    ret.type = "x-scheme-handler/http";
    ret.icon = ret.type; //给一个无效的图标名，由前端寻找默认浏览器图标
    ret.searcher = name();

    {
        QMutexLocker lk(&m_mtx);
        m_items.append(ret);
    }

    m_status.storeRelease(Completed);

    qDebug() << "echo text...";
    //不能使用QTimer延迟发送，会出现任务结束后再发信号，此时已经无效。
    QThread::msleep(100);
    emit unearthed(this);

    return true;
}

void StaticTextWorker::terminate()
{
    m_status.storeRelease(Terminated);
}

ProxyWorker::Status StaticTextWorker::status()
{
    return static_cast<ProxyWorker::Status>(m_status.loadAcquire());
}

bool StaticTextWorker::hasItem() const
{
    QMutexLocker lk(&m_mtx);
    return !m_items.isEmpty();
}

MatchedItemMap StaticTextWorker::takeAll()
{
    QMutexLocker lk(&m_mtx);
    MatchedItems items = std::move(m_items);

    Q_ASSERT(m_items.isEmpty());
    lk.unlock();

    //添加分组
    MatchedItemMap ret;
    ret.insert(group(), items);

    return ret;
}

QString StaticTextWorker::group() const
{
    return GRANDSEARCH_GROUP_WEB;
}

QString StaticTextWorker::createUrl(const QString &searchEngine) const
{
    static const QString baidu("https://www.baidu.com/s?&wd=%0");
    static const QString google("https://www.google.com/search?q=%0");

    static const QHash<QString, QString> searchEngineChinese{{GRANDSEARCH_WEB_SEARCHENGINE_GOOGLE, "https://www.google.com/search?q=%0"},
                                                {GRANDSEARCH_WEB_SEARCHENGINE_BING, "https://bing.com/search?q=%0"},
                                                {GRANDSEARCH_WEB_SEARCHENGINE_YAHOO, "https://search.yahoo.com/search?p=%0"},
                                                {GRANDSEARCH_WEB_SEARCHENGINE_360, "https://www.so.com/s?&q=%0"},
                                                {GRANDSEARCH_WEB_SEARCHENGINE_SOGOU, "https://www.sogou.com/web?query=%0"},
                                                {GRANDSEARCH_WEB_SEARCHENGINE_360AI, "https://www.sou.com/?q=%0&src=ec_tongxin"}};
    static const QHash<QString, QString> searchEngineEnglish{{GRANDSEARCH_WEB_SEARCHENGINE_BING, "https://bing.com/search?q=%0"},
                                                {GRANDSEARCH_WEB_SEARCHENGINE_YAHOO, "https://search.yahoo.com/search?p=%0"},
                                                {GRANDSEARCH_WEB_SEARCHENGINE_BAIDU, "https://www.baidu.com/s?&wd=%0"}};

    if (searchEngine == GRANDSEARCH_WEB_SEARCHENGINE_CUSTOM) {
        auto config = Configer::instance()->group(GRANDSEARCH_WEB_GROUP);
        QString searchEngineAddr = config->value(GRANDSEARCH_WEB_SEARCHENGINE_CUSTOM_ADDR, QString(""));
        searchEngineAddr = QString::fromStdString(QByteArray::fromBase64(searchEngineAddr.toUtf8()).toStdString());
        if (searchEngineAddr.isEmpty() || !searchEngineAddr.contains("%0")) {
            if (SearchHelper::instance()->isSimplifiedChinese()) {
                searchEngineAddr = baidu;
            } else {
                searchEngineAddr = google;
            }
        }
        return searchEngineAddr;
    }

    if (SearchHelper::instance()->isSimplifiedChinese()) {
        return searchEngineChinese.value(searchEngine, baidu);
    } else {
        return searchEngineEnglish.value(searchEngine, google);
    }
}
