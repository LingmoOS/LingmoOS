// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "desktopappsearcher.h"
#include "desktopappsearcher_p.h"
#include "desktopappworker.h"
#include "global/builtinsearch.h"
#include "utils/chineseletterhelper.h"

#include <QFileInfo>
#include <QDebug>

using namespace GrandSearch;
DCORE_USE_NAMESPACE

DesktopAppSearcherPrivate::DesktopAppSearcherPrivate(DesktopAppSearcher *parent)
    : q(parent)
{
    m_appDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);

    qInfo() << "application dirs:" << m_appDirs;
    qInfo() << "XDG_DATA_DIRS:" << qgetenv("XDG_DATA_DIRS");

    m_fileWatcher = new QFileSystemWatcher(q);
    m_fileWatcher->addPaths(m_appDirs);
}

DesktopAppSearcherPrivate::~DesktopAppSearcherPrivate()
{
    if (m_creating) {
        m_creating = false;

        qDebug() << "wait finished.";
        m_creatingIndex.waitForFinished();
        m_updatingIndex.waitForFinished();
        qDebug() << "DesktopAppSearcher finished.";
    }
}

void DesktopAppSearcherPrivate::createIndex(DesktopAppSearcherPrivate *d)
{
    QTime time;
    time.start();

    QHash<QString, QList<DesktopAppPointer>> indexTable;

    //获取所有应用
    QMap<QString, DesktopEntryPointer> apps = DesktopAppSearcherPrivate::scanDesktopFile(d->m_appDirs, d->m_creating);
    if (!d->m_creating)
        return;

    const QString locale = QLocale::system().name().simplified();
    for (auto app = apps.begin(); app != apps.end(); ++app) {
        //强制中断
        if (!d->m_creating)
            return;

        DesktopEntryPointer value = app.value();

        //获取索引关键字
        auto keys = DesktopAppSearcherPrivate::desktopIndex(value, locale);
        if (Q_UNLIKELY(keys.isEmpty()))
            continue;

        DesktopAppPointer info(new MatchedItem());
        info->name = value->ddeDisplayName();
        info->item = app.key();
        info->icon = value->stringValue("Icon", "Desktop Entry", "application-x-desktop");//若没有图标，则使用默认"application-x-desktop"
        info->type = "application/x-desktop"; //此项写固定值，.desktop文件的mimetype为application/x-desktop
        info->searcher = d->q->name();

        //放入索引表
        for (const QString &key : keys) {
            auto iter = indexTable.find(key);
            if (iter != indexTable.end() ) {
                iter->append(info);
            } else {
                indexTable.insert(key, {info});
            }
        }
    }

    //初始化完成
    QWriteLocker lk(&d->m_lock);
    d->m_indexTable = indexTable;
    d->m_inited = true;
    d->m_creating = false;

    qInfo() << "create index completed, spend" << time.elapsed() << "cout" << indexTable.size();
}

void DesktopAppSearcherPrivate::updateIndex(DesktopAppSearcherPrivate *d)
{
    while (d->m_needUpdateIndex) {
        d->m_needUpdateIndex = false;
        d->m_creating = true;

        qInfo() << "update index...";
        createIndex(d);
    }
}

QMap<QString, DesktopEntryPointer> DesktopAppSearcherPrivate::scanDesktopFile(const QStringList &paths, volatile bool &runing)
{
    QMap<QString, DesktopEntryPointer> entrys;
    QSet<QString> duplicate;

    for (const QString &dirPath : paths) {
        //中断
        if (!runing)
            break;

        QDir dir(dirPath);
        if (!dir.isReadable())
            continue;

        QFileInfoList entryInfoList = dir.entryInfoList({"*.desktop"}, QDir::Files, QDir::Name);
        for (const QFileInfo &fileInfo : entryInfoList) {
            //中断
            if (!runing)
                break;

            const QString path = fileInfo.absoluteFilePath();
            const QString fileName = fileInfo.fileName();

            // 重复的
            if (duplicate.contains(fileName))
                continue;

            DesktopEntryPointer pointer(new DDesktopEntry(path));
            if (isHidden(pointer))
                continue;
            //正常解析时pointer->status()返回的不是NoError
            //if (pointer->status() == DDesktopEntry::NoError)
            entrys.insert(path, pointer);
            duplicate.insert(fileName);
        }
    }

    return entrys;
}

bool DesktopAppSearcherPrivate::isHidden(DesktopEntryPointer pointer)
{
    // NoDisplay
    {
        QString noDisplay = pointer->stringValue("NoDisplay");
        if (noDisplay.compare("true", Qt::CaseInsensitive) == 0)
            return true;
    }

    // Hidden
    {
        QString hidden = pointer->stringValue("Hidden");
        if (hidden.compare("true", Qt::CaseInsensitive) == 0)
            return true;
    }

    static QString desktop = qgetenv("XDG_CURRENT_DESKTOP");
    if (!desktop.isEmpty()) {
        // NotShowIn
        {
            QStringList notShows = pointer->stringValue("NotShowIn").split(';', QString::SkipEmptyParts);
            if (notShows.contains(desktop, Qt::CaseInsensitive))
                return true;
        }

        // OnlyShowIn
        {
            QString onlyShow = pointer->stringValue("OnlyShowIn");
            if (!onlyShow.isEmpty()) {
                QStringList onlyShows = pointer->stringValue("OnlyShowIn").split(';');
                if (!onlyShows.isEmpty() && !onlyShows.contains(desktop, Qt::CaseInsensitive))
                    return true;
            }
        }
    }

    return false;
}

QSet<QString> DesktopAppSearcherPrivate::desktopIndex(const DesktopEntryPointer &app, const QString &locale)
{
    QSet<QString> idxs;
    if (app.isNull())
        return idxs;

    static const QString XDeepinVendor = "X-Deepin-Vendor";
    static const QString deepin = "deepin";

    //dde名称,X-Deepin-Vendor=deepin
    bool useGeneric = app->stringValue(XDeepinVendor) == deepin;

    //默认名称
    QString defaultName = DesktopAppSearcherPrivate::desktopName(app, "", useGeneric);
    if (!defaultName.isEmpty())
        idxs << defaultName;

    //当前语言的名字
    QString localName = DesktopAppSearcherPrivate::desktopName(app, locale, useGeneric);
    if (!localName.isEmpty() && defaultName != localName)
        idxs << localName;

    //简体中文
    static const QString zhCN = "zh_CN";
    QString zhCNName;
    if (locale == zhCN)
        zhCNName = localName;
    else
        zhCNName = DesktopAppSearcherPrivate::desktopName(app, zhCN, useGeneric);

    if (!zhCNName.isEmpty()) {
        idxs << zhCNName;

        //全拼以及拼音首字母
        QString firstPys;
        QString fullPys;
        if (Ch2PyIns->convertChinese2Pinyin(zhCNName, firstPys, fullPys)) {
            if (!fullPys.isEmpty())
                idxs << fullPys;

            if (!firstPys.isEmpty())
                idxs << firstPys;
        }
    }

    return idxs;
}

QString DesktopAppSearcherPrivate::desktopName(const DesktopEntryPointer &app, const QString &locale, bool generic)
{
    QString name;
    if (app.isNull())
        return name;

    static const QString keyName = "Name";
    static const QString keyGenericName = "GenericName";

    //本地语言为空时获取默认值
    if (locale.isEmpty()) {
        if (generic)
            name = app->stringValue(keyGenericName);

        if (name.isEmpty())
            name = app->stringValue(keyName);
        return name;
    }

    //去掉_的本地标识
    const QString localeSplited = DesktopAppSearcherPrivate::splitLocaleName(locale);

    //找本地名称，GenericName[]
    if (generic) {
        name = app->stringValue(QString("%1[%2]").arg(keyGenericName).arg(locale));

        //没有则找简写的
        if (name.isEmpty() && !localeSplited.isEmpty())
            name = app->stringValue(QString("%1[%2]").arg(keyGenericName).arg(localeSplited));
    }

    //继续找Name[]
    if (name.isEmpty()) {
        name = app->stringValue(QString("%1[%2]").arg(keyName).arg(locale));

        //没有则找简写的
        if (name.isEmpty() && !localeSplited.isEmpty())
            name = app->stringValue(QString("%1[%2]").arg(keyName).arg(localeSplited));
    }

    return name;
}

QString DesktopAppSearcherPrivate::splitLocaleName(const QString &locale)
{
    QString ret;

    //去掉_后的本地语言标识
    QStringList localeList = locale.split("_");
    if (localeList.size() == 2 && !localeList.first().isEmpty())
        ret = localeList.first();

    return ret;
}

DesktopAppSearcher::DesktopAppSearcher(QObject *parent)
    : Searcher(parent)
    , d(new DesktopAppSearcherPrivate(this))
{
    connect(d->m_fileWatcher, &QFileSystemWatcher::directoryChanged, this, &DesktopAppSearcher::onDirectoryChanged);
}

DesktopAppSearcher::~DesktopAppSearcher()
{
    delete d;
    d = nullptr;
}

void DesktopAppSearcher::asyncInit()
{
    if (d->m_inited || d->m_creating)
        return;

    //开始遍历目录创建索引
    d->m_creating = true;
    d->m_creatingIndex = QtConcurrent::run(DesktopAppSearcherPrivate::createIndex, d);
}

QString DesktopAppSearcher::name() const
{
    return GRANDSEARCH_CLASS_APP_DESKTOP;
}

bool DesktopAppSearcher::isActive() const
{
    return d->m_inited;
}

bool DesktopAppSearcher::activate()
{
    return false;
}

ProxyWorker *DesktopAppSearcher::createWorker() const
{
    auto worker = new DesktopAppWorker(name());
    {
        QWriteLocker lk(&d->m_lock);
        worker->setIndexTable(d->m_indexTable);
    }

    return worker;
}

bool DesktopAppSearcher::action(const QString &action, const QString &item)
{
    Q_UNUSED(item);
    qWarning() << "no such action:" << action << ".";
    return false;
}

void DesktopAppSearcher::onDirectoryChanged(const QString &path)
{
    Q_UNUSED(path);

    if (d->m_updatingIndex.isRunning()) {
        d->m_needUpdateIndex = true;
        d->m_creating = false;
        return;
    }

    d->m_needUpdateIndex = true;
    d->m_updatingIndex = QtConcurrent::run(DesktopAppSearcherPrivate::updateIndex, d);
}
