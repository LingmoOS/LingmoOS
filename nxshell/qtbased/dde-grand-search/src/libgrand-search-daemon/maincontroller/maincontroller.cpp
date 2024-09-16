// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "maincontroller.h"
#include "maincontroller_p.h"
#include "configuration/configer.h"
#include "global/searchhelper.h"

#include <QDebug>

using namespace GrandSearch;

//10分钟后进入休眠模式
#define DORMANT_INTERVAL 10 * 60 * 1000

MainControllerPrivate::MainControllerPrivate(MainController *parent)
    : QObject(parent)
    , q(parent)
{
    connect(&m_dormancy, &QTimer::timeout, this, &MainControllerPrivate::dormancy);
    m_dormancy.setSingleShot(true);

    m_dormancy.setInterval(DORMANT_INTERVAL);
}

MainControllerPrivate::~MainControllerPrivate()
{
    if (m_currentTask) {
        m_currentTask->stop();
        m_currentTask->deleteSelf();
        m_currentTask = nullptr;
    }
}

void MainControllerPrivate::buildWorker(TaskCommander *task, const QSet<QString> &blankList)
{
    Q_ASSERT(task);
    Q_ASSERT(m_searchers);

    auto searchers = m_searchers->searchers();

    //搜索项是否启用
    auto config = ConfigerIns->group(GRANDSEARCH_PREF_SEARCHERENABLED);
    Q_ASSERT(config);

    QStringList searcherData;
    {
        // 类目、后缀搜索的判读与准备
        QStringList groupList, suffixList, keywordList;
        if (searchHelper->parseKeyword(task->content(), groupList, suffixList, keywordList)) {
            if (!keywordList.isEmpty() || !suffixList.isEmpty() || !groupList.isEmpty()) {
                searcherData = checkSearcher(groupList, suffixList, keywordList);
                const auto &keyword = buildKeywordInJson(groupList, suffixList, keywordList);
                if (!keyword.isEmpty())
                    task->setContent(keyword);
            }
        }
    }

    for (auto searcher : searchers) {
        Q_ASSERT(searcher);
        const QString name = searcher->name();

        // 判断搜索项是否可用
        if (blankList.contains(name) || !config->value(name, true)
                || (!searcherData.isEmpty() && !searcherData.contains(name)))
            continue;

        // 判断是否激活，若未激活则先激活
        qDebug() << "searcher create worker" << name;
        if (searcher->isActive() || searcher->activate()) {
            if (auto worker = searcher->createWorker()) {
                task->join(worker);
            }
        } else {
            qWarning() << name << "is unenabled.";
        }
    }
}

void MainControllerPrivate::buildWorker(TaskCommander *task, const QList<QString> &searchers)
{
    Q_ASSERT(task);
    Q_ASSERT(m_searchers);

    for (const QString &name : searchers) {
        auto searcher = m_searchers->searcher(name);
        if (!searcher) {
            qWarning() << "no such shearch:" << name;
            continue;
        }

        qDebug() << "searcher create worker" << name;
        if (searcher->isActive() || searcher->activate()) {
            if (auto worker = searcher->createWorker()) {
                task->join(worker);
            }
        } else {
            qWarning() << name << "is unenabled.";
        }
    }
}

QStringList MainControllerPrivate::checkSearcher(const QStringList &groupList, const QStringList &suffixList, const QStringList &keywordList)
{
    QStringList data;
    for (const auto &group : groupList) {
        const auto &searchers = searchHelper->getSearcherByGroupName(group);
        if (searchers.isEmpty())
            continue;
        data.append(searchers);
    }

    // 后缀不为空，说明需要文件搜索项
    if (!suffixList.isEmpty()) {
        data.append(GRANDSEARCH_CLASS_FILE_DEEPIN);
        data.append(GRANDSEARCH_CLASS_FILE_FSEARCH);
    } else {
        // 后缀、类目为空，搜索关键字不为空
        // 说明分隔符前后的字段既不是后缀也不是类目
        if (groupList.isEmpty() && !keywordList.isEmpty()) {
            data.append(GRANDSEARCH_CLASS_FILE_DEEPIN);
            data.append(GRANDSEARCH_CLASS_FILE_FSEARCH);
            data.append(GRANDSEARCH_CLASS_APP_DESKTOP);
        }
    }

    return data;
}

QString MainControllerPrivate::buildKeywordInJson(const QStringList &groupList, const QStringList &suffixList, const QStringList &keywordList)
{
    QJsonDocument doc;
    QJsonObject obj;
    QJsonArray groupArr, suffixArr, keywordArr;
    for (const auto &group : groupList)
        groupArr.append(group);

    for (const auto &suffix : suffixList)
        suffixArr.append(suffix);

    for (const auto &keyword : keywordList) {
        if (keyword.isEmpty())
            continue;
        keywordArr.append(keyword);
    }

    obj[JSON_GROUP_ATTRIBUTE] = groupArr;
    obj[JSON_SUFFIX_ATTRIBUTE] = suffixArr;
    obj[JSON_KEYWORD_ATTRIBUTE] = keywordArr;
    doc.setObject(obj);
    return doc.toJson(QJsonDocument::Compact);
}

void MainControllerPrivate::dormancy()
{
    if (m_currentTask && !m_currentTask->isFinished()) {
        qInfo() << "task is not finished, restart dormancy.";
        m_dormancy.start();
        return;
    }

    qInfo() << "dormancy...";
    if (m_searchers)
        m_searchers->dormancy();
}

MainController::MainController(QObject *parent)
    : QObject(parent)
    , d(new MainControllerPrivate(this))
{
}

bool MainController::init()
{
    Q_ASSERT(d->m_searchers == nullptr);

    d->m_searchers = new SearcherGroup(this);
    if (!d->m_searchers->init())
        return false;

    //初始化配置模块
    return ConfigerIns->init();
}

bool MainController::newSearch(const QString &key)
{
    qInfo() << "new search, current task:" << d->m_currentTask << key.size();
    if (Q_UNLIKELY(key.isEmpty()))
        return false;

    auto func = [this](TaskCommander *task) {
        d->buildWorker(task);
    };

    return d->buildTask(key, func);
}

void MainController::terminate()
{
    //停止任务
    if (d->m_currentTask) {
        disconnect(d->m_currentTask, nullptr, this, nullptr);
        d->m_currentTask->stop();
        d->m_currentTask->deleteSelf();
        d->m_currentTask = nullptr;
    }
}

QByteArray MainController::getResults() const
{
    if (d->m_currentTask) {
        MatchedItemMap items = d->m_currentTask->getResults();

        //序列化
        QByteArray bytes;
        QDataStream stream(&bytes, QIODevice::WriteOnly);
        stream << items;

        return bytes;
    }

    return QByteArray();
}

QByteArray MainController::readBuffer() const
{
    if (d->m_currentTask) {
        MatchedItemMap items = d->m_currentTask->readBuffer();

        //序列化
        QByteArray bytes;
        QDataStream stream(&bytes, QIODevice::WriteOnly);
        stream << items;

        return bytes;
    }

    return QByteArray();
}

bool MainController::isEmptyBuffer() const
{
    if (d->m_currentTask) {
        return d->m_currentTask->isEmptyBuffer();
    }

    return true;
}

bool MainController::searcherAction(const QString &name, const QString &action, const QString &item)
{
    if (name.isEmpty() || action.isEmpty() || item.isEmpty()) {
        qWarning() << "input is empty.";
        return false;
    }

    if (auto searcher = d->m_searchers->searcher(name)) {
        return searcher->action(action, item);
    }

    qWarning() << "no such search:" << name;
    return false;
}
