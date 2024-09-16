// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "albumControl.h"
#include "dbmanager/dbmanager.h"
#include "fileMonitor/fileinotifygroup.h"
#include "imageengine/imageenginethread.h"
#include "utils/devicehelper.h"
#include <DDialog>
#include <DMessageBox>
#include <DGuiApplicationHelper>

#include <QStandardPaths>
#include <QFileInfo>
#include <QUrl>
#include <QFileDialog>
#include <QProcess>
#include <QRegularExpression>
#include <QDirIterator>
#include <QCoreApplication>
#include <QFuture>
#include <QtConcurrent>
#include <QApplication>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

namespace {
static QMap<QString, const char *> i18nMap {
    {"data", "Data Disk"}
};
const QString ddeI18nSym = QStringLiteral("_dde_");

static std::initializer_list<std::pair<QString, QString>> opticalmediakeys {
    {"optical",                "Optical"},
    {"optical_cd",             "CD-ROM"},
    {"optical_cd_r",           "CD-R"},
    {"optical_cd_rw",          "CD-RW"},
    {"optical_dvd",            "DVD-ROM"},
    {"optical_dvd_r",          "DVD-R"},
    {"optical_dvd_rw",         "DVD-RW"},
    {"optical_dvd_ram",        "DVD-RAM"},
    {"optical_dvd_plus_r",     "DVD+R"},
    {"optical_dvd_plus_rw",    "DVD+RW"},
    {"optical_dvd_plus_r_dl",  "DVD+R/DL"},
    {"optical_dvd_plus_rw_dl", "DVD+RW/DL"},
    {"optical_bd",             "BD-ROM"},
    {"optical_bd_r",           "BD-R"},
    {"optical_bd_re",          "BD-RE"},
    {"optical_hddvd",          "HD DVD-ROM"},
    {"optical_hddvd_r",        "HD DVD-R"},
    {"optical_hddvd_rw",       "HD DVD-RW"},
    {"optical_mo",             "MO"}
};
static QVector<std::pair<QString, QString>> opticalmediakv(opticalmediakeys);
static QMap<QString, QString> opticalmediamap(opticalmediakeys);

} //namespace

AlbumControl *AlbumControl::m_instance = nullptr;

QString sizeString(const QString &str)
{
    int begin_pos = str.indexOf('.');

    if (begin_pos < 0)
        return str;

    QString size = str;

    while (size.count() - 1 > begin_pos) {
        if (!size.endsWith('0'))
            return size;

        size = size.left(size.count() - 1);
    }

    return size.left(size.count() - 1);
}

QString formatSize(qint64 num, bool withUnitVisible = true, int precision = 1, int forceUnit = -1, QStringList unitList = QStringList())
{
    if (num < 0) {
        qWarning() << "Negative number passed to formatSize():" << num;
        num = 0;
    }

    bool isForceUnit = (forceUnit >= 0);
    QStringList list;
    qreal fileSize(num);

    if (unitList.size() == 0) {
        list << " B" << " KB" << " MB" << " GB" << " TB"; // should we use KiB since we use 1024 here?
    } else {
        list = unitList;
    }

    QStringListIterator i(list);
    QString unit = i.hasNext() ? i.next() : QStringLiteral(" B");

    int index = 0;
    while (i.hasNext()) {
        if (fileSize < 1024 && !isForceUnit) {
            break;
        }

        if (isForceUnit && index == forceUnit) {
            break;
        }

        unit = i.next();
        fileSize /= 1024;
        index++;
    }
    QString unitString = withUnitVisible ? unit : QString();
    return QString("%1%2").arg(sizeString(QString::number(fileSize, 'f', precision)), unitString);
}

AlbumControl::AlbumControl(QObject *parent)
    : QObject(parent)
{
    initMonitor();
    initDeviceMonitor();

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::newProcessInstance, this, &AlbumControl::onNewAPPOpen);
}

AlbumControl::~AlbumControl()
{

}

AlbumControl *AlbumControl::instance()
{
    if (!m_instance) {
        m_instance = new AlbumControl();
        m_instance->startMonitor();
    }

    return m_instance;
}

DBImgInfo AlbumControl::getDBInfo(const QString &srcpath, bool isVideo)
{
    using namespace LibUnionImage_NameSpace;
    QFileInfo srcfi(srcpath);
    DBImgInfo dbi;
    dbi.filePath = srcpath;
    dbi.importTime = QDateTime::currentDateTime();
    if (isVideo) {
        dbi.itemType = ItemTypeVideo;
        //获取视频信息
        MovieInfo movieInfo = MovieService::instance()->getMovieInfo(QUrl::fromLocalFile(srcpath));
        //对视频信息缓存
        m_movieInfos[srcpath] = movieInfo;

        dbi.changeTime = srcfi.lastModified();

        if (movieInfo.creation.isValid()) {
            dbi.time = movieInfo.creation;
        } else if (srcfi.birthTime().isValid()) {
            dbi.time = srcfi.birthTime();
        } else if (srcfi.metadataChangeTime().isValid()) {
            dbi.time = srcfi.metadataChangeTime();
        } else {
            dbi.time = dbi.changeTime;
        }
    } else {
        auto mds = getAllMetaData(srcpath);
        QString value = mds.value("DateTimeOriginal");
        dbi.itemType = ItemTypePic;
        dbi.changeTime = QDateTime::fromString(mds.value("DateTimeDigitized"), "yyyy/MM/dd hh:mm");
        if (!value.isEmpty()) {
            dbi.time = QDateTime::fromString(value, "yyyy/MM/dd hh:mm");
        } else if (srcfi.birthTime().isValid()) {
            dbi.time = srcfi.birthTime();
        } else if (srcfi.metadataChangeTime().isValid()) {
            dbi.time = srcfi.metadataChangeTime();
        } else {
            dbi.time = dbi.changeTime;
        }
    }
    return dbi;
}

void AlbumControl::initDeviceMonitor()
{
    m_deviceManager = DDeviceManager::instance();
    m_deviceManager->startMonitorWatch();

    DeviceHelper::instance()->loadAllDeviceInfos();
    getAllBlockDeviceName();
    QStringList deviceIds = DeviceHelper::instance()->getAllDeviceIds();
    for (auto id : deviceIds) {
        QVariantMap deveiceInfo = DeviceHelper::instance()->loadDeviceInfo(id);
        onMounted(id, deveiceInfo.value("MountPoint").toString(), static_cast<DeviceType>(deveiceInfo.value("DeviceType").toInt()));
    }
    connect(m_deviceManager, &DDeviceManager::deviceRemoved, this, &AlbumControl::onDeviceRemoved);
    connect(m_deviceManager, &DDeviceManager::mounted, this, &AlbumControl::onMounted);
    connect(m_deviceManager, &DDeviceManager::unmounted, this, &AlbumControl::onUnMounted);
}

bool AlbumControl::findPicturePathByPhone(QString &path)
{
    QDir dir(path);
    if (!dir.exists()) return false;
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    QFileInfo tempFileInfo;
    foreach (tempFileInfo, fileInfoList) {
        //针对ptp模式
        if (tempFileInfo.fileName().compare(ALBUM_PATHNAME_BY_PHONE) == 0) {
            path = tempFileInfo.absoluteFilePath();
            return true;
        } else {        //针对MTP模式
            //  return true;
            QDir subDir;
            subDir.setPath(tempFileInfo.absoluteFilePath());
            QFileInfoList subFileInfoList = subDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (QFileInfo subTempFileInfo : subFileInfoList) {
                if (subTempFileInfo.fileName().compare(ALBUM_PATHNAME_BY_PHONE) == 0) {
                    path = subTempFileInfo.absoluteFilePath();
                    return true;
                }
            }
        }
    }
    return false;
}

void AlbumControl::getAllInfos()
{
    m_infoList = DBManager::instance()->getAllInfos();
}

DBImgInfoList AlbumControl::getAllInfosByUID(QString uid)
{
    return DBManager::instance()->getAllInfosByUID(uid);
}

QString AlbumControl::getAllFilters()
{
    QStringList sList;
    for (const QString &i : LibUnionImage_NameSpace::unionImageSupportFormat())
        sList << ("*." + i);
    //添加视频过滤
    for (const QString &i : LibUnionImage_NameSpace::videoFiletypes())
        sList << "*." + i;
    QString filter = tr("All photos and videos");
    filter.append('(');
    filter.append(sList.join(" "));
    filter.append(')');
    return filter;
}

void AlbumControl::unMountDevice(const QString &devicePath)
{
    QString deviceId = DeviceHelper::instance()->getDeviceIdByMountPoint(devicePath);
    if (!deviceId.isEmpty() && DeviceHelper::instance()->detachDevice(deviceId)) {
        // 等待最多200ms超时
        QEventLoop loop;
        static const int overTime = 200;
        QTimer::singleShot(overTime, &loop, &QEventLoop::quit);
        loop.exec();

        DeviceHelper::instance()->loadAllDeviceInfos();

        // 设备不存在，则卸载成功，否则提示卸载失败
        if (!DeviceHelper::instance()->isExist(deviceId)) {
            m_durlAndNameMap.remove(devicePath);
            m_PhonePicFileMap.remove(devicePath);
        } else {
            DDialog msgbox;
            msgbox.setFixedWidth(400);
            msgbox.setIcon(DMessageBox::standardIcon(DMessageBox::Critical));
            msgbox.setTextFormat(Qt::AutoText);
            msgbox.setMessage(tr("Disk is busy, cannot eject now"));
            msgbox.insertButton(1, tr("OK"), false, DDialog::ButtonNormal);
            auto ret = msgbox.exec();
            Q_UNUSED(ret);
        }
    }

    emit sigMountsChange();
}

QStringList AlbumControl::getAllUrlPaths(const int &filterType)
{
    QStringList pathList;
    ItemType type = ItemType::ItemTypePic;
    switch (filterType) {
    case 0:
        type = ItemType::ItemTypeNull;
        break;
    case 1:
        type = ItemType::ItemTypePic;
        break;
    case 2:
        type = ItemType::ItemTypeVideo;
        break;
    }
    QStringList list = DBManager::instance()->getAllPaths(type);
    for (QString path : list) {
        pathList << "file://" + path;
    }
    return pathList;
}

QStringList AlbumControl::getAllPaths(const int &filterType)
{
    QStringList pathList;
    ItemType type = ItemType::ItemTypePic;
    switch (filterType) {
    case 0:
        type = ItemType::ItemTypeNull;
        break;
    case 1:
        type = ItemType::ItemTypePic;
        break;
    case 2:
        type = ItemType::ItemTypeVideo;
        break;
    }

    QStringList list = DBManager::instance()->getAllPaths(type);

    return list;
}

QVariantList AlbumControl::getAlbumAllInfos(const int &filterType)
{
    QVariantList reinfoList;
    ItemType type = ItemType::ItemTypePic;
    switch (filterType) {
    case 0:
        type = ItemType::ItemTypeNull;
        break;
    case 1:
        type = ItemType::ItemTypePic;
        break;
    case 2:
        type = ItemType::ItemTypeVideo;
        break;
    }
    DBImgInfoList infoList = DBManager::instance()->getAllInfosSort(type);
    for (DBImgInfo info : infoList) {
        QVariantMap reMap;
        reMap.insert("url", "file://" + info.filePath);
        reMap.insert("filePath", info.filePath);
        reMap.insert("pathHash", info.pathHash);
        reMap.insert("remainDays", info.remainDays);

        if (info.itemType == ItemTypePic) {
            reMap.insert("itemType", "pciture");
        } else if (info.itemType == ItemTypeVideo) {
            reMap.insert("itemType", "video");
        } else {
            reMap.insert("itemType", "other");
        }
        reinfoList << reMap;
        //}
    }
    return reinfoList;
}

bool AlbumControl::importAllImagesAndVideos(const QStringList &paths, const int UID, const bool notifyUI)
{
    //发送导入开始信号
    if (notifyUI)
        emit sigImportStart();

    ImportImagesThread *imagesthread = new ImportImagesThread;
    imagesthread->setData(paths, UID);
    imagesthread->setNotifyUI(notifyUI);
    QThreadPool::globalInstance()->start(imagesthread);

    return true;
}

bool AlbumControl::importAllImagesAndVideosUrl(const QList<QUrl> &paths, const int UID, bool checkRepeat/* = true*/)
{
    //发送导入开始信号
    emit sigImportStart();

    ImportImagesThread *imagesthread = new ImportImagesThread;
    imagesthread->setData(paths, UID, checkRepeat);
    QThreadPool::globalInstance()->start(imagesthread);

    return true;
}

QStringList AlbumControl::getAllTimelinesTitle(const int &filterType)
{
    return getTimelinesTitle(TimeLineEnum::All, filterType);
}

QStringList AlbumControl::getTimelinesTitlePaths(const QString &titleName, const int &filterType)
{
    QStringList pathsList;
    DBImgInfoList dblist;
    if (m_yearDateMap.keys().contains(titleName)) {
        dblist = m_yearDateMap.value(titleName);
    } else if (m_monthDateMap.keys().contains(titleName)) {
        dblist = m_monthDateMap.value(titleName);
    } else if (m_dayDateMap.keys().contains(titleName)) {
        dblist = m_dayDateMap.value(titleName);
    } else {
        dblist = m_importTimeLinePathsMap.value(titleName);
    }
    for (DBImgInfo info : dblist) {
        if (filterType == 2 && info.itemType == ItemTypePic) {
            continue ;
        } else if (filterType == 1 && info.itemType == ItemTypeVideo) {
            continue ;
        }
        pathsList << "file://" + info.filePath;
    }
    return pathsList;
}

QStringList AlbumControl::getAllImportTimelinesTitle(const int &filterType)
{
    return getTimelinesTitle(TimeLineEnum::Import, filterType);
}

QVariantMap AlbumControl::getTimelinesTitleInfos(const int &filterType)
{
    QVariantMap reMap;
    QStringList alltitles =  getTimelinesTitle(TimeLineEnum::All, filterType);
    for (QString titleName : alltitles) {
        QVariantList list;
        DBImgInfoList dbInfoList = m_timeLinePathsMap.value(titleName);
        for (DBImgInfo info : dbInfoList) {
            QVariantMap tmpMap;
            if (info.itemType == ItemTypePic) {
                if (filterType == 2) {
                    continue ;
                }
                tmpMap.insert("itemType", "pciture");
            } else if (info.itemType == ItemTypeVideo) {
                if (filterType == 1) {
                    continue ;
                }
                tmpMap.insert("itemType", "video");
            } else {
                tmpMap.insert("itemType", "other");
            }
            tmpMap.insert("url", "file://" + info.filePath);
            tmpMap.insert("filePath", info.filePath);
            tmpMap.insert("pathHash", info.pathHash);
            tmpMap.insert("remainDays", info.remainDays);
            list << tmpMap;
        }
        if (list.count() > 0) {
            reMap.insert(titleName, list);
        }
    }
    return reMap;
}


QStringList AlbumControl::getYearTimelinesTitle(const int &filterType)
{
    return getTimelinesTitle(TimeLineEnum::Year, filterType);
}

QVariantMap AlbumControl::getYearTimelinesInfos(const int &filterType)
{
    QVariantMap reMap;
    QStringList alltitles =  getTimelinesTitle(TimeLineEnum::Year, filterType);
    for (QString titleName : alltitles) {
        QVariantList list;
        DBImgInfoList dbInfoList = m_yearDateMap.value(titleName);
        for (DBImgInfo info : dbInfoList) {
            QVariantMap tmpMap;
            if (info.itemType == ItemTypePic) {
                if (filterType == 2) {
                    continue ;
                }
                tmpMap.insert("itemType", "pciture");
            } else if (info.itemType == ItemTypeVideo) {
                if (filterType == 1) {
                    continue ;
                }
                tmpMap.insert("itemType", "video");
            } else {
                tmpMap.insert("itemType", "other");
            }
            tmpMap.insert("url", "file://" + info.filePath);
            tmpMap.insert("filePath", info.filePath);
            tmpMap.insert("pathHash", info.pathHash);
            tmpMap.insert("remainDays", info.remainDays);
            list << tmpMap;
        }
        if (list.count() > 0) {
            reMap.insert(titleName, list);
        }
    }
    return reMap;
}

QStringList AlbumControl::getMonthTimelinesTitle(const int &filterType)
{
    return getTimelinesTitle(TimeLineEnum::Month, filterType);
}

QVariantMap AlbumControl::getMonthTimelinesInfos(const int &filterType)
{
    QVariantMap reMap;
    QStringList alltitles =  getTimelinesTitle(TimeLineEnum::Month, filterType);
    for (QString titleName : alltitles) {
        QVariantList list;
        DBImgInfoList dbInfoList = m_monthDateMap.value(titleName);
        for (DBImgInfo info : dbInfoList) {
            QVariantMap tmpMap;
            if (info.itemType == ItemTypePic) {
                if (filterType == 2) {
                    continue ;
                }
                tmpMap.insert("itemType", "pciture");
            } else if (info.itemType == ItemTypeVideo) {
                if (filterType == 1) {
                    continue ;
                }
                tmpMap.insert("itemType", "video");
            } else {
                tmpMap.insert("itemType", "other");
            }
            tmpMap.insert("url", "file://" + info.filePath);
            tmpMap.insert("filePath", info.filePath);
            tmpMap.insert("pathHash", info.pathHash);
            tmpMap.insert("remainDays", info.remainDays);
            list << tmpMap;
        }
        if (list.count() > 0) {
            reMap.insert(titleName, list);
        }
    }
    return reMap;
}

QStringList AlbumControl::getDayTimelinesTitle(const int &filterType)
{
    return getTimelinesTitle(TimeLineEnum::Day, filterType);
}

QVariantMap AlbumControl::getDayTimelinesInfos(const int &filterType)
{
    QVariantMap reMap;
    QStringList alltitles =  getTimelinesTitle(TimeLineEnum::Day, filterType);
    for (QString titleName : alltitles) {
        QVariantList list;
        DBImgInfoList dbInfoList = m_dayDateMap.value(titleName);
        for (DBImgInfo info : dbInfoList) {
            QVariantMap tmpMap;
            if (info.itemType == ItemTypePic) {
                if (filterType == 2) {
                    continue ;
                }
                tmpMap.insert("itemType", "pciture");
            } else if (info.itemType == ItemTypeVideo) {
                if (filterType == 1) {
                    continue ;
                }
                tmpMap.insert("itemType", "video");
            } else {
                tmpMap.insert("itemType", "other");
            }
            tmpMap.insert("url", "file://" + info.filePath);
            tmpMap.insert("filePath", info.filePath);
            tmpMap.insert("pathHash", info.pathHash);
            tmpMap.insert("remainDays", info.remainDays);
            list << tmpMap;
        }
        if (list.count() > 0) {
            reMap.insert(titleName, list);
        }
    }
    return reMap;
}

QStringList AlbumControl::getTimelinesTitle(TimeLineEnum timeEnum, const int &filterType)
{
    //设置需要查询的图片视频或者是全部
    ItemType typeItem = ItemType::ItemTypeNull;
    if (filterType == 1) {
        typeItem = ItemType::ItemTypePic;
    } else if (filterType == 2) {
        typeItem = ItemType::ItemTypeVideo;
    }
    //已导入
    if (timeEnum == Import) {
        QStringList list;
        m_importTimelines = DBManager::instance()->getImportTimelines();
        m_importTimeLinePathsMap.clear();
        QList<QDateTime> tmpDateList = m_importTimelines ;

        for (QDateTime time : tmpDateList) {
            //获取当前时间照片
            DBImgInfoList ImgInfoList = DBManager::instance()->getInfosByImportTimeline(time, typeItem);
            QStringList datelist = time.toString("yyyy.MM.dd.hh.mm").split(".");
            //加时间线标题
            QString date;
            if (datelist.count() > 4) {
                if (ImgInfoList.size() > 0) {
                    date = QString(QObject::tr("%1/%2/%3 %4:%5")).arg(datelist[0]).arg(datelist[1]).arg(datelist[2]).arg(datelist[3]).arg(datelist[4]);
                    m_importTimeLinePathsMap.insert(date, ImgInfoList);
                }
            }
        }
        //倒序
        QStringList relist;
        for (int i = m_importTimeLinePathsMap.keys().count() - 1 ; i >= 0 ; i--) {
            relist << m_importTimeLinePathsMap.keys().at(i);
        }

        return relist;
    }

    //时间线
    m_timelines = DBManager::instance()->getAllTimelines();
    QMap < QString, DBImgInfoList > tmpInfoMap;
    QList<QDateTime> tmpDateList = m_timelines ;

    for (QDateTime time : tmpDateList) {
        //获取当前时间照片
        DBImgInfoList ImgInfoList = DBManager::instance()->getInfosByTimeline(time, typeItem);
        QStringList datelist = time.toString("yyyy.MM.dd.hh.mm").split(".");
        //加时间线标题
        //QString date;
        if (datelist.count() > 4) {
            //date = QString(QObject::tr("%1-%2-%3-%4:%5")).arg(datelist[0]).arg(datelist[1]).arg(datelist[2]).arg(datelist[3]).arg(datelist[4]);

            switch (timeEnum) {
            case TimeLineEnum::Year :
                if (ImgInfoList.size() > 0) {
                    tmpInfoMap.insert(QString(QObject::tr("%1").arg(datelist[0])), ImgInfoList);
                }
                m_yearDateMap = tmpInfoMap;
                break;
            case TimeLineEnum::Month :
                if (ImgInfoList.size() > 0) {
                    tmpInfoMap.insert(QString(QObject::tr("%1/%2").arg(datelist[0]).arg(datelist[1])), ImgInfoList);
                }
                m_monthDateMap = tmpInfoMap;
                break;
            case TimeLineEnum::Day :
                if (ImgInfoList.size() > 0) {
                    tmpInfoMap.insert(QString(QObject::tr("%1/%2/%3").arg(datelist[0]).arg(datelist[1]).arg(datelist[2])), ImgInfoList);
                }
                m_dayDateMap = tmpInfoMap;
                break;
            case TimeLineEnum::All :
                if (ImgInfoList.size() > 0) {
                    tmpInfoMap.insert(QString(QObject::tr("%1/%2/%3 %4:%5")).arg(datelist[0]).arg(datelist[1]).arg(datelist[2]).arg(datelist[3]).arg(datelist[4]), ImgInfoList);
                }
                m_timeLinePathsMap = tmpInfoMap;
                break;
            default:
                break;
            }
        }
    }
    //倒序
    QStringList relist;
    for (int i = tmpInfoMap.keys().count() - 1 ; i >= 0 ; i--) {
        relist << tmpInfoMap.keys().at(i);
    }

    return relist;
}

void AlbumControl::initMonitor()
{
    m_fileInotifygroup = new FileInotifyGroup(this) ;
    connect(m_fileInotifygroup, &FileInotifyGroup::sigMonitorChanged, this, &AlbumControl::slotMonitorChanged);
    connect(m_fileInotifygroup, &FileInotifyGroup::sigMonitorDestroyed, this, &AlbumControl::slotMonitorDestroyed);
}

void AlbumControl::startMonitor()
{
    //启动路径监控
    auto monitorPathsTuple = DBManager::getDefaultNotifyPaths_group();
    const QList<QStringList> &paths = std::get<0>(monitorPathsTuple);
    const QStringList &albumNames = std::get<1>(monitorPathsTuple);
    const QList<int> &UIDs = std::get<2>(monitorPathsTuple);
    for (int i = 0; i != UIDs.size(); ++i) {
        m_fileInotifygroup->startWatch(paths.at(i), albumNames.at(i), UIDs.at(i));
    }

    QMap <int, QString> customAutoImportUIDAndPaths = DBManager::instance()->getAllCustomAutoImportUIDAndPath();
    for (QString &eachItem : customAutoImportUIDAndPaths) {
        //0.先检查路径是否存在，不存在直接移除
        QFileInfo info(eachItem);
        int uid = customAutoImportUIDAndPaths.key(eachItem);
        if (!info.exists() || !info.isDir()) {
            DBManager::instance()->removeCustomAutoImportPath(customAutoImportUIDAndPaths.key(eachItem));
            continue;
        }

        //1.获取原有的路径
        auto originPaths = DBManager::instance()->getPathsByAlbum(uid);

        //2.获取现在的路径
        QFileInfoList infos = LibUnionImage_NameSpace::getImagesAndVideoInfo(eachItem, false);
        QStringList currentPaths;
        std::transform(infos.begin(), infos.end(), std::back_inserter(currentPaths), [](const QFileInfo & info) {
            return info.isSymLink() ? info.readSymLink() : info.absoluteFilePath();
        });

        //3.1获取已不存在的路径
        QFuture<QString> watcher = QtConcurrent::filtered(originPaths, [currentPaths](const QString & path) {
            return !currentPaths.contains(path);
        });
        watcher.waitForFinished();
        QStringList deleteFiles(watcher.results());

        //3.2移除已导入的路径
        auto watcher2 = QtConcurrent::filter(currentPaths, [originPaths](const QString & path) {
            return !originPaths.contains(path);
        });
        watcher2.waitForFinished();


        //4.删除不存在的路径
        if (!deleteFiles.isEmpty()) {
            DBManager::instance()->removeImgInfos(deleteFiles);
        }

        //5.执行导入
        if (!currentPaths.isEmpty()) {
            QStringList urls;
            for (QString path : currentPaths) {
                urls << QUrl::fromLocalFile(path).toString();
            }
            importAllImagesAndVideos(urls, -1, false);
            insertImportIntoAlbum(uid, urls);
        }
    }
    QStringList pathlist = DBManager::instance()->getAllPaths();
    QStringList needDeletes ;
    for (QString path : pathlist) {
        if (!QFileInfo(path).exists()) {
            needDeletes << path;
        }
    }
    DBManager::instance()->removeImgInfos(needDeletes);
}

bool AlbumControl::checkIfNotified(const QString &dirPath)
{
    return DBManager::instance()->checkCustomAutoImportPathIsNotified(dirPath);
}

void AlbumControl::slotMonitorChanged(QStringList fileAdd, QStringList fileDelete, QString album, int UID)
{
    //直接删除图片
    DBManager::instance()->removeImgInfos(fileDelete);
    AlbumDBType atype = AlbumDBType::AutoImport;
    DBManager::instance()->insertIntoAlbum(UID, fileAdd, atype);

    DBImgInfoList dbInfos;
    for (QString path : fileAdd) {

        bool bIsVideo = LibUnionImage_NameSpace::isVideo(path);
        DBImgInfo info =  getDBInfo(path, bIsVideo);
        info.albumUID = QString::number(UID);
        dbInfos << info;
    }
    //导入图片数据库ImageTable3
    DBManager::instance()->insertImgInfos(dbInfos);

    emit sigRefreshCustomAlbum(UID);
    emit sigRefreshImportAlbum();
    emit sigRefreshAllCollection();

}

void AlbumControl::slotMonitorDestroyed(int UID)
{
    //文件夹删除
    emit sigDeleteCustomAlbum(UID);
}

void AlbumControl::sltLoadMountFileList(const QString &path)
{
    QElapsedTimer time;
    time.start();
    QString strPath = path;
    if (!m_PhonePicFileMap.contains(strPath)) {
        //获取所选文件类型过滤器
        QStringList filters;
        for (QString i : LibUnionImage_NameSpace::unionImageSupportFormat()) {
            filters << "*." + i;
        }

        for (QString i : LibUnionImage_NameSpace::videoFiletypes()) {
            filters << "*." + i;
        }
        //定义迭代器并设置过滤器，包括子目录：QDirIterator::Subdirectories
        QDirIterator dir_iterator(strPath,
                                  filters,
                                  QDir::Files/* | QDir::NoSymLinks*/,
                                  QDirIterator::Subdirectories);
        QStringList allfiles;
        while (dir_iterator.hasNext()) {
            dir_iterator.next();
            allfiles << dir_iterator.filePath();
        }
        //重置标志位，可以执行线程
        m_PhonePicFileMap[strPath] = allfiles;
        //发送信号
    } else {
        //已加载过的设备，直接发送缓存的路径
    }

    qDebug() << __FUNCTION__ << QString(" load device path:%1 cost [%2]ms").arg(path).arg(time.elapsed());
}

void AlbumControl::onDeviceRemoved(const QString &deviceKey, DeviceType type)
{
    qDebug() << QString("deviceKey:%1 DeviceType:%2").arg(deviceKey).arg(static_cast<int>(type));
    onUnMountedExecute(deviceKey, type);
}

void AlbumControl::onMounted(const QString &deviceKey, const QString &mountPoint, DeviceType type)
{
    qDebug() << QString("deviceKey:%1 mountPoint:%2 DeviceType:%3").arg(deviceKey).arg(mountPoint).arg(static_cast<int>(type));

    QString uri = deviceKey;
    QString scheme = QUrl(uri).scheme();

    if (DeviceHelper::isSamba(uri)) {
        qWarning() << "uri is a smb path";
        return;
    }

    if ((scheme == "file") ||  //usb device
        (scheme == "gphoto2") ||                //phone photo
        //(scheme == "afc") ||                  //iPhone document
        (scheme == "mtp") ||                    //android file
        deviceKey.startsWith("/org")) {         //deviceId为/org前缀的外接设备路径

        const QVariantMap deviceInfo = DeviceHelper::instance()->loadDeviceInfo(uri, true);
        if (deviceInfo.isEmpty() || deviceInfo.value("MountPoint").toString().isEmpty()) {
            qWarning() << QString("deviceKey:%1 empty deviceInfo.").arg(uri);
            return;
        }

        QString label;
        if (static_cast<DeviceType>(deviceInfo.value("DeviceType").toInt()) == DeviceType::kBlockDevice)
            label = deviceInfo.value("IdLabel").toString();
        else if (static_cast<DeviceType>(deviceInfo.value("DeviceType").toInt()) == DeviceType::kProtocolDevice)
            label = deviceInfo.value("DisplayName").toString();
        qDebug() << "device.name" << label << " scheme type:" << scheme;
        if (m_durlAndNameMap.find(mountPoint) != m_durlAndNameMap.end()) {
            qDebug() << "Already has this device in device list. mountPoint:" << mountPoint;
            return;
        }
        QString strPath = mountPoint;
        if (strPath.isEmpty()) {
            qDebug() << "strPath.isEmpty()";
        }
        QString rename = "";
        qDebug() << mountPoint;
        rename = m_blkPath2DeviceNameMap[mountPoint];
        if ("" == rename) {
            rename = label;
        }
        //判断路径是否存在
        bool bFind = false;
        QDir dir(strPath);
        if (!dir.exists()) {
            qDebug() << "onLoadMountImagesStart() !dir.exists()";
            bFind = false;
        } else {
            bFind = true;
        }
        //U盘和硬盘挂载都是/media下的，此处判断若path不包含/media/,再调用findPicturePathByPhone函数搜索DCIM文件目录
        if (!strPath.contains("/media/")) {
            bFind = findPicturePathByPhone(strPath);
        }

        DeviceHelper::instance()->loadAllDeviceInfos();
        //路径存在
        if (bFind) {
            m_durlAndNameMap[mountPoint] = rename;
            emit sigMountsChange();
            //发送新增
            emit sigAddDevice(strPath);
        }
    }
}

void AlbumControl::onUnMounted(const QString &deviceKey, DeviceType type)
{
    qDebug() << QString("deviceKey:%1 DeviceType:%2").arg(deviceKey).arg(static_cast<int>(type));
    onUnMountedExecute(deviceKey, type);
}

void AlbumControl::onUnMountedExecute(const QString &deviceKey, DeviceType type)
{
    QVariantMap deviceInfo = DeviceHelper::instance()->loadDeviceInfo(deviceKey);
    if (deviceInfo.isEmpty())
        return;

    QString mountPoint = DeviceHelper::instance()->getMountPointByDeviceId(deviceKey);;
    QString strPath = mountPoint;
    if (!strPath.contains("/media/")) {
        findPicturePathByPhone(strPath);
    }
    if (m_durlAndNameMap.find(strPath) != m_durlAndNameMap.end())
        m_durlAndNameMap.erase(m_durlAndNameMap.find(strPath));

    DeviceHelper::instance()->loadAllDeviceInfos();

    if (m_PhonePicFileMap.contains(strPath))
        m_PhonePicFileMap.remove(strPath);

    emit sigMountsChange();
}

QJsonObject AlbumControl::createShorcutJson()
{
    //Translations
    QJsonObject shortcut1;
    shortcut1.insert("name", "Window sizing");
    shortcut1.insert("value", "Ctrl+Alt+F");
    QJsonObject shortcut2;
    shortcut2.insert("name", tr("Fullscreen"));
    shortcut2.insert("value", "F11");
    QJsonObject shortcut3;
    shortcut3.insert("name", tr("Exit fullscreen/slideshow"));
    shortcut3.insert("value", "Esc");
    QJsonObject shortcut4;
    shortcut4.insert("name", "Close application");
    shortcut4.insert("value", "Alt+F4");
    QJsonObject shortcut5;
    shortcut5.insert("name", tr("Help"));
    shortcut5.insert("value", "F1");
    QJsonObject shortcut6;
    shortcut6.insert("name", tr("Display shortcuts"));
    shortcut6.insert("value", "Ctrl+Shift+?");
    QJsonObject shortcut7;
    shortcut7.insert("name", tr("Display in file manager"));
    shortcut7.insert("value", "Alt+D");
    QJsonObject shortcut8;
    shortcut8.insert("name", tr("Slide show"));
    shortcut8.insert("value", "F5");
    QJsonObject shortcut9;
    shortcut9.insert("name", tr("View"));
    shortcut9.insert("value", "Enter");
    QJsonObject shortcut10;
    shortcut10.insert("name", tr("Export photos"));
    shortcut10.insert("value", "Ctrl+E");
    QJsonObject shortcut11;
    shortcut11.insert("name", tr("Import photos/videos"));
    shortcut11.insert("value", "Ctrl+O");
    QJsonObject shortcut12;
    shortcut12.insert("name", tr("Select all"));
    shortcut12.insert("value", "Ctrl+A");
    QJsonObject shortcut13;
    shortcut13.insert("name", tr("Copy"));
    shortcut13.insert("value", "Ctrl+C");
    QJsonObject shortcut14;
    shortcut14.insert("name", tr("Delete"));
    shortcut14.insert("value", "Delete");
    QJsonObject shortcut15;
    shortcut15.insert("name", tr("Photo/Video info"));
    shortcut15.insert("value", "Ctrl+I");
    QJsonObject shortcut16;
    shortcut16.insert("name", tr("Set as wallpaper"));
    shortcut16.insert("value", "Ctrl+F9");
    QJsonObject shortcut17;
    shortcut17.insert("name", tr("Rotate clockwise"));
    shortcut17.insert("value", "Ctrl+R");
    QJsonObject shortcut18;
    shortcut18.insert("name", tr("Rotate counterclockwise"));
    shortcut18.insert("value", "Ctrl+Shift+R");
    QJsonObject shortcut19;
    shortcut19.insert("name", " ");
    shortcut19.insert("value", "  ");
    QJsonObject shortcut20;
    shortcut20.insert("name", tr("Zoom in"));
    shortcut20.insert("value", "Ctrl+'+'");
    QJsonObject shortcut21;
    shortcut21.insert("name", tr("Zoom out"));
    shortcut21.insert("value", "Ctrl+'-'");
    QJsonObject shortcut22;
    shortcut22.insert("name", tr("Previous"));
    shortcut22.insert("value", "Left");
    QJsonObject shortcut23;
    shortcut23.insert("name", tr("Next"));
    shortcut23.insert("value", "Right");
    QJsonObject shortcut24;
    shortcut24.insert("name", tr("Favorite"));
    shortcut24.insert("value", ".");
    QJsonObject shortcut25;
    shortcut25.insert("name", tr("Unfavorite"));
    shortcut25.insert("value", ".");
    QJsonObject shortcut26;
    shortcut26.insert("name", tr("New album"));
    shortcut26.insert("value", "Ctrl+Shift+N");
    QJsonObject shortcut27;
    shortcut27.insert("name", tr("Rename album"));
    shortcut27.insert("value", "F2");
    QJsonObject shortcut28;
    shortcut28.insert("name", tr("Page up"));
    shortcut28.insert("value", "PageUp");
    QJsonObject shortcut29;
    shortcut29.insert("name", tr("Page down"));
    shortcut29.insert("value", "PageDown");



    QJsonArray shortcutArray1;
    shortcutArray1.append(shortcut2);
    shortcutArray1.append(shortcut8);
    shortcutArray1.append(shortcut3);
    shortcutArray1.append(shortcut9);
    shortcutArray1.append(shortcut10);
    shortcutArray1.append(shortcut11);
    shortcutArray1.append(shortcut12);
    shortcutArray1.append(shortcut13);
    shortcutArray1.append(shortcut14);
    shortcutArray1.append(shortcut15);
    shortcutArray1.append(shortcut16);
    shortcutArray1.append(shortcut17);
    shortcutArray1.append(shortcut18);
    shortcutArray1.append(shortcut7);
//    shortcutArray1.append(shortcut19);
    shortcutArray1.append(shortcut20);
    shortcutArray1.append(shortcut21);
    shortcutArray1.append(shortcut28);
    shortcutArray1.append(shortcut29);
    shortcutArray1.append(shortcut22);
    shortcutArray1.append(shortcut23);
    shortcutArray1.append(shortcut24);
    shortcutArray1.append(shortcut25);
    QJsonArray shortcutArray2;
    shortcutArray2.append(shortcut26);
    shortcutArray2.append(shortcut27);
    QJsonArray shortcutArray3;
    shortcutArray3.append(shortcut5);
    shortcutArray3.append(shortcut6);

//    shortcutArray.append(shortcut1);
//    shortcutArray.append(shortcut4);

    QJsonObject shortcut_group1;
//    shortcut_group.insert("groupName", tr("热键"));
//    shortcut_group.insert("groupName", tr("Hotkey"));
    shortcut_group1.insert("groupName", tr("Photos"));
    shortcut_group1.insert("groupItems", shortcutArray1);
    QJsonObject shortcut_group2;
    shortcut_group2.insert("groupName", tr("Albums"));
    shortcut_group2.insert("groupItems", shortcutArray2);
    QJsonObject shortcut_group3;
    shortcut_group3.insert("groupName", tr("Settings"));
    shortcut_group3.insert("groupItems", shortcutArray3);

    QJsonArray shortcutArrayall;
    shortcutArrayall.append(shortcut_group1);
    shortcutArrayall.append(shortcut_group2);
    shortcutArrayall.append(shortcut_group3);

    QJsonObject main_shortcut;
    main_shortcut.insert("shortcut", shortcutArrayall);

    return main_shortcut;
}

void AlbumControl::getAllBlockDeviceName()
{
    m_blkPath2DeviceNameMap.clear();
    QStringList blDevList = DeviceHelper::instance()->getBlockDeviceIds();
    for (const QString &blks : blDevList) {
        updateBlockDeviceName(blks);
    }
}

void AlbumControl::updateBlockDeviceName(const QString &blks)
{
    const QVariantMap deviceInfo = DeviceHelper::instance()->loadDeviceInfo(blks);
    if (deviceInfo.isEmpty())
        return;

    QStringList mps = deviceInfo.value("MountPoints").toStringList();
    qulonglong size = deviceInfo.value("SizeTotal").toULongLong();
    QString label = deviceInfo.value("IdLabel").toString();
    QString fs = deviceInfo.value("IdType").toString();
    QString udispname = "";
    if (label.startsWith(ddeI18nSym)) {
        QString i18nKey = label.mid(ddeI18nSym.size(), label.size() - ddeI18nSym.size());
        udispname = qApp->translate("DeepinStorage", i18nMap.value(i18nKey, i18nKey.toUtf8().constData()));
        goto runend;
    }

    if (mps.contains(QByteArray("/\0", 2))) {
        udispname = QCoreApplication::translate("PathManager", "System Disk");
        goto runend;
    }
    if (label.length() == 0) {
        bool bMediaAvailable = deviceInfo.value("MediaAvailable").toBool();
        bool bOpticalDrive = deviceInfo.value("OpticalDrive").toBool();
        bool bOpticalBlank = deviceInfo.value("OpticalBlank").toBool();
        bool bIsEncrypted = deviceInfo.value("IsEncrypted").toBool();
        QString media = deviceInfo.value("Media").toString();
        QStringList mediaCompatibility = deviceInfo.value("MediaCompatibility").toStringList();
        if (!bMediaAvailable && bOpticalDrive) {
            QString maxmediacompat;
            for (auto i = opticalmediakv.rbegin(); i != opticalmediakv.rend(); ++i) {
                if (mediaCompatibility.contains(i->first)) {
                    maxmediacompat = i->second;
                    break;
                }
            }
            udispname = QCoreApplication::translate("DeepinStorage", "%1 Drive").arg(maxmediacompat);
            goto runend;
        }
        if (bOpticalBlank) {
            udispname = QCoreApplication::translate("DeepinStorage", "Blank %1 Disc").arg(opticalmediamap[media]);
            goto runend;
        }
        if (bIsEncrypted) {
            udispname = QCoreApplication::translate("DeepinStorage", "%1 Encrypted").arg(formatSize(qint64(size)));
            goto runend;
        }
        udispname = QCoreApplication::translate("DeepinStorage", "%1 Volume").arg(formatSize(qint64(size)));
        goto runend;
    }
    udispname = label;

runend:
    //blk->mount({});
    QString strPath = "";
    QStringList qbl = deviceInfo.value("MountPoints").toStringList();
    QString mountPoint = "";
    QList<QString>::iterator qb = qbl.begin();
    while (qb != qbl.end()) {
        mountPoint += (*qb);
        ++qb;
    }
    if (!mountPoint.isEmpty()) {
        m_blkPath2DeviceNameMap[mountPoint] = udispname;
        qDebug() << QString("blks:%1 mountPoint:%2 udispname:%3").arg(blks).arg(mountPoint).arg(udispname);
    }
    return;
}

bool AlbumControl::isSystemAutoImportAlbum(int uid)
{
    return getAllSystemAutoImportAlbumId().contains(uid);
}

bool AlbumControl::isNormalAutoImportAlbum(int uid)
{
    return getAllNormlAutoImportAlbumId().contains(uid);
}

bool AlbumControl::isAutoImportAlbum(int uid)
{
    return getAllAutoImportAlbumId().contains(uid);
}

bool AlbumControl::isCustomAlbum(int uid)
{
    bool bCustom = getAllCustomAlbumId().contains(uid);
    return bCustom;
}

bool AlbumControl::isDefaultPathExists(int uid)
{
    return DBManager::defaultNotifyPathExists(uid);
}

void AlbumControl::ctrlShiftSlashShortcut(int x, int y, int w, int h)
{
    QRect rect = QRect(x, y, w, h);
    QPoint pos(rect.x() + rect.width() / 2, rect.y() + rect.height() / 2);
    QStringList shortcutString;
    QJsonObject json = createShorcutJson();

    QString param1 = "-j=" + QString(QJsonDocument(json).toJson());
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    shortcutString << param1 << param2;

    QProcess *shortcutViewProcess = new QProcess(this);
    shortcutViewProcess->startDetached("deepin-shortcut-viewer", shortcutString);

    connect(shortcutViewProcess, SIGNAL(finished(int)), shortcutViewProcess, SLOT(deleteLater()));
}

QRect AlbumControl::rect(QPoint p1, QPoint p2)
{
    QRect rt = QRect(p1, p2);
    return rt;
}

QRect AlbumControl::intersected(QRect r1, QRect r2)
{
    return r1.intersected(r2);
}

int AlbumControl::manhattanLength(QPoint p1, QPoint p2)
{
    QPoint point(p1 - p2);
    return point.manhattanLength();
}

QString AlbumControl::url2localPath(QUrl url)
{
    return LibUnionImage_NameSpace::localPath(url);
}

bool AlbumControl::checkRepeatUrls(QStringList imported, QStringList urls, bool bNotify)
{
    bool bRet = false;
    int noReadCount = 0; //记录已存在于相册中的数量，若全部存在，则不进行导入操作
    for (QString url : urls) {
        QFileInfo srcfi(url2localPath(url));
        if (!srcfi.exists()) {  //当前文件不存在
            noReadCount++;
            continue;
        }
        if (imported.contains(url))
            noReadCount++;
    }

    // 已全部存在
    if (noReadCount == urls.size()) {
        if (bNotify)
            emit sigRepeatUrls(urls);
        bRet = true;
    }

    return bRet;
}

QStringList AlbumControl::getImportTimelinesTitlePaths(const QString &titleName, const int &filterType)
{
    QStringList pathsList;
    DBImgInfoList dbInfoList = m_importTimeLinePathsMap.value(titleName);
    for (DBImgInfo info : dbInfoList) {
        if (filterType == 2 && info.itemType == ItemTypePic) {
            continue ;
        } else if (filterType == 1 && info.itemType == ItemTypeVideo) {
            continue ;
        }
        pathsList << "file://" + info.filePath;
    }
    return pathsList;
}

QVariantMap AlbumControl::getImportTimelinesTitleInfos(const int &filterType)
{
    QVariantMap reMap;
    QStringList alltitles = getAllImportTimelinesTitle(filterType);
    for (QString titleName : alltitles) {
        QVariantList list;
        DBImgInfoList dbInfoList = m_importTimeLinePathsMap.value(titleName);
        for (DBImgInfo info : dbInfoList) {
            QVariantMap tmpMap;
            if (info.itemType == ItemTypePic) {
                if (filterType == 2) {
                    continue ;
                }
                tmpMap.insert("itemType", "pciture");
            } else if (info.itemType == ItemTypeVideo) {
                if (filterType == 1) {
                    continue ;
                }
                tmpMap.insert("itemType", "video");
            } else {
                tmpMap.insert("itemType", "other");
            }
            tmpMap.insert("url", "file://" + info.filePath);
            tmpMap.insert("filePath", info.filePath);
            tmpMap.insert("pathHash", info.pathHash);
            tmpMap.insert("remainDays", info.remainDays);
            list << tmpMap;
        }
        if (list.count() > 0) {
            reMap.insert(titleName, list);
        }
    }

    return reMap;
}

QVariantList AlbumControl::getImportTimelinesTitleInfosReverse(const int &filterType)
{
    QVariantMap reMap = getImportTimelinesTitleInfos(filterType);

    QVariantList reList;
    if (reMap.size()) {
        for (auto it = --reMap.end(); it != --reMap.begin(); it--) {
            QVariantMap tmpMap;
            tmpMap.insert(it.key(), it.value());
            reList.push_back(tmpMap);
        }
    }

    return reList;
}

QVariantMap AlbumControl::getAlbumInfos(const int &albumId, const int &filterType)
{
    QVariantMap reMap;

    QVariantList list;
    DBImgInfoList dbInfoList = DBManager::instance()->getInfosByAlbum(albumId, false);
    QString title = DBManager::instance()->getAlbumNameFromUID(albumId);
    for (DBImgInfo info : dbInfoList) {
        QVariantMap tmpMap;
        if (info.itemType == ItemTypePic) {
            if (filterType == 2) {
                continue ;
            }
            tmpMap.insert("itemType", "pciture");
        } else if (info.itemType == ItemTypeVideo) {
            if (filterType == 1) {
                continue ;
            }
            tmpMap.insert("itemType", "video");
        } else {
            tmpMap.insert("itemType", "other");
        }
        tmpMap.insert("url", "file://" + info.filePath);
        tmpMap.insert("filePath", info.filePath);
        tmpMap.insert("pathHash", info.pathHash);
        tmpMap.insert("remainDays", info.remainDays);
        list << tmpMap;
    }
    if (list.count() > 0) {
        reMap.insert(title, list);
    }
    return reMap;
}

QVariantMap AlbumControl::getTrashAlbumInfos(const int &filterType)
{
    QVariantMap reMap;

    QVariantList list;
    DBImgInfoList dbInfoList = getTrashInfos(filterType);
    QString title = QObject::tr("Trash");
    for (DBImgInfo info : dbInfoList) {
        QVariantMap tmpMap;
        if (info.itemType == ItemTypePic) {
            if (filterType == 2) {
                continue ;
            }
            tmpMap.insert("itemType", "pciture");
        } else if (info.itemType == ItemTypeVideo) {
            if (filterType == 1) {
                continue ;
            }
            tmpMap.insert("itemType", "video");
        } else {
            tmpMap.insert("itemType", "other");
        }

        //设置url为删除的路径
        QString realPath = getDeleteFullPath(LibUnionImage_NameSpace::hashByString(info.filePath), DBImgInfo::getFileNameFromFilePath(info.filePath));
        tmpMap.insert("url", "file://" + realPath);
        tmpMap.insert("filePath", "file://" + info.filePath);
        tmpMap.insert("pathHash", info.pathHash);
        tmpMap.insert("remainDays", info.remainDays);
        list << tmpMap;
    }
    if (list.count() > 0) {
        reMap.insert(title, list);
    }
    return reMap;
}

bool AlbumControl::addCustomAlbumInfos(int albumId, const QList<QUrl> &urls)
{
    QStringList localpaths;
    for (QUrl path : urls) {
        localpaths << url2localPath(path);
    }
    QStringList curAlbumImgPathList = getAllUrlPaths();
    for (QString imagePath : localpaths) {
        if (QDir(imagePath).exists()) {
            //获取所选文件类型过滤器
            QStringList filters;
            for (QString i : LibUnionImage_NameSpace::unionImageSupportFormat()) {
                filters << "*." + i;
            }

            for (QString i : LibUnionImage_NameSpace::videoFiletypes()) {
                filters << "*." + i;
            }
            //定义迭代器并设置过滤器，包括子目录：QDirIterator::Subdirectories
            QDirIterator dir_iterator(imagePath,
                                      filters,
                                      QDir::Files/* | QDir::NoSymLinks*/,
                                      QDirIterator::Subdirectories);
            QList<QUrl> allfiles;
            while (dir_iterator.hasNext()) {
                dir_iterator.next();
                QFileInfo fileInfo = dir_iterator.fileInfo();
                allfiles << "file://" + fileInfo.filePath();
            }
            if (!allfiles.isEmpty()) {
                addCustomAlbumInfos(albumId, allfiles);
            }
        }
    }
    bool bRet = false;
    QStringList paths;
    for (QUrl url : urls) {
        paths << url2localPath(url);
    }
    AlbumDBType atype;
    if (albumId == 0) {
        atype = Favourite;
    } else if (albumId < 4) {
        atype = AutoImport;
    } else {
        atype = Custom;
    }
    bRet = DBManager::instance()->insertIntoAlbum(albumId, paths, atype);
    emit sigRefreshCustomAlbum(albumId);
    return bRet;
}

int AlbumControl::getAllCount(const int &filterType)
{
    ItemType typeItem = ItemType::ItemTypeNull;
    if (filterType == 1) {
        typeItem = ItemType::ItemTypePic;
    } else if (filterType == 2) {
        typeItem = ItemType::ItemTypeVideo;
    }
    int nCount = DBManager::instance()->getImgsCount(typeItem);
    return nCount;
}

void AlbumControl::insertTrash(const QList< QUrl > &paths)
{
    QStringList tmpList;
    for (QUrl url : paths) {
        QString imagePath = url2localPath(url);
        QFileInfo info(imagePath);
        //判断文件是否可写
        if (info.isWritable()) {
            tmpList << imagePath;
        }
    }

    DBImgInfoList infos;
    for (QString path : tmpList) {
        //infos << DBManager::instance()->getInfoByPath(path);
        DBImgInfoList tempInfos = DBManager::instance()->getInfosByPath(path);
        if (tempInfos.size()) {
            DBImgInfo insertInfo = tempInfos.first();
            QStringList uids;
            for (DBImgInfo info : tempInfos) {
                uids.push_back(info.albumUID);
            }
            insertInfo.albumUID = uids.join(",");
            infos << insertInfo;
        }
    }
    DBManager::instance()->insertTrashImgInfos(infos, false);
    //新增删除主相册数据库
    DBManager::instance()->removeImgInfos(tmpList);
    // 通知前端刷新相关界面，包括自定义相册/我的收藏/合集-所有项目/已导入
    sigRefreshCustomAlbum(-1);
    sigRefreshAllCollection();
    sigRefreshImportAlbum();
    sigRefreshSearchView();
}

void AlbumControl::removeTrashImgInfos(const QList< QUrl > &paths)
{
    QStringList localPaths ;
    for (QUrl path : paths) {
        localPaths << url2localPath(path);
    }
    DBManager::instance()->removeTrashImgInfos(localPaths);
}

QStringList AlbumControl::recoveryImgFromTrash(const QStringList &paths)
{
    QStringList localPaths;
    for (QUrl path : paths) {
        if (path.isLocalFile())
            localPaths << url2localPath(path);
        else
            localPaths << path.toString();
    }
    return DBManager::instance()->recoveryImgFromTrash(localPaths);
}

void AlbumControl::deleteImgFromTrash(const QStringList &paths)
{
    QStringList localPaths ;
    for (QUrl path : paths) {
        if (path.isLocalFile())
            localPaths << url2localPath(path);
        else
            localPaths << path.toString();
    }
    DBManager::instance()->removeTrashImgInfos(localPaths);
}

void AlbumControl::insertCollection(const QList< QUrl > &paths)
{
    QStringList tmpList;
    for (QUrl url : paths) {
        tmpList << url2localPath(url);
    }
    DBImgInfoList infos;
    for (QString path : tmpList) {
        infos << DBManager::instance()->getInfoByPath(path);
    }
}

void AlbumControl::createAlbum(const QString &newName)
{
    QString createAlbumName = getNewAlbumName(newName);
    int createUID = DBManager::instance()->createAlbum(createAlbumName, QStringList(" "));
    DBManager::instance()->insertIntoAlbum(createUID, QStringList(" "));
}

QList<int> AlbumControl::getAllNormlAutoImportAlbumId()
{
    QMap < int, QString > autoImportAlbum;
    QList<std::pair<int, QString>>  tmpList = DBManager::instance()->getAllAlbumNames(AutoImport);
    for (std::pair<int, QString> tmpPair : tmpList) {
        if (tmpPair.first > 3)
            autoImportAlbum.insert(tmpPair.first, tmpPair.second);
    }
    return autoImportAlbum.keys();
}

QList<int> AlbumControl::getAllSystemAutoImportAlbumId()
{
    QMap < int, QString > systemAlbum;
    QList<std::pair<int, QString>>  tmpList = DBManager::instance()->getAllAlbumNames(AutoImport);
    for (std::pair<int, QString> tmpPair : tmpList) {
        if (tmpPair.first > 0 && tmpPair.first <= 3)
            systemAlbum.insert(tmpPair.first, tmpPair.second);
    }
    return systemAlbum.keys();
}

QList <int> AlbumControl::getAllAutoImportAlbumId()
{
    QMap < int, QString > systemAlbum;
    QList<std::pair<int, QString>>  tmpList = DBManager::instance()->getAllAlbumNames(AutoImport);
    for (std::pair<int, QString> tmpPair : tmpList) {
        systemAlbum.insert(tmpPair.first, tmpPair.second);
    }
    return systemAlbum.keys();
}

QList < int > AlbumControl::getAllCustomAlbumId()
{
    QMap < int, QString > customAlbum;
    QList<std::pair<int, QString>>  tmpList = DBManager::instance()->getAllAlbumNames(Custom);
    for (std::pair<int, QString> tmpPair : tmpList) {
        customAlbum.insert(tmpPair.first, tmpPair.second);
    }
    m_customAlbum = customAlbum;
    return customAlbum.keys();
}

QList < QString > AlbumControl::getAllCustomAlbumName()
{
    QMap < int, QString > customAlbum;
    QList<std::pair<int, QString>>  tmpList = DBManager::instance()->getAllAlbumNames(Custom);
    for (std::pair<int, QString> tmpPair : tmpList) {
        customAlbum.insert(tmpPair.first, tmpPair.second);
    }
    m_customAlbum = customAlbum;
    return customAlbum.values();
}

QStringList AlbumControl::getAlbumPaths(const int &albumId, const int &filterType)
{
    qDebug() << "1085" << albumId;
    QStringList relist;
    DBImgInfoList dbInfoList = DBManager::instance()->getInfosByAlbum(albumId, false);
    QString title = DBManager::instance()->getAlbumNameFromUID(albumId);
    for (DBImgInfo info : dbInfoList) {
        QVariantMap tmpMap;
        if (info.itemType == ItemTypePic) {
            if (filterType == 2) {
                continue ;
            }
        } else if (info.itemType == ItemTypeVideo) {
            if (filterType == 1) {
                continue ;
            }
        }
        relist << "file://" + info.filePath;
    }
    return relist;
}

QString AlbumControl::getCustomAlbumByUid(const int &index)
{
    // 我的收藏和系统相册名称为固定名称，可直接根据索引获取，以便做翻译处理
    if (0 == index)
        return tr("Favorites");
    else if (1 == index)
        return tr("Screen Capture");
    else if (2 == index)
        return tr("Camera");
    else if (3 == index)
        return tr("Draw");

    return DBManager::instance()->getAlbumNameFromUID(index);
}

DBImgInfoList AlbumControl::getTrashInfos(const int &filterType)
{
    DBImgInfoList allTrashInfos = DBManager::instance()->getAllTrashInfos_getRemainDays();
    QDateTime currentTime = QDateTime::currentDateTime();
    DBImgInfoList list;
    for (int i = allTrashInfos.size() - 1; i >= 0; i--) {
        DBImgInfo pinfo = allTrashInfos.at(i);
        if (!QFile::exists(pinfo.filePath) &&
                !QFile::exists(getDeleteFullPath(pinfo.pathHash, pinfo.getFileNameFromFilePath()))) {
            allTrashInfos.removeAt(i);
        } else if (pinfo.remainDays <= 0) {
            list << pinfo;
            allTrashInfos.removeAt(i);
        } else if (pinfo.itemType == ItemTypePic) {
            if (filterType == 2) {
                allTrashInfos.removeAt(i);
            }
        } else if (pinfo.itemType == ItemTypeVideo) {
            if (filterType == 1) {
                allTrashInfos.removeAt(i);
            }
        }
    }
    //清理删除时间过长图片
    if (!list.isEmpty()) {
        QStringList image_list;
        for (DBImgInfo info : list) {
            image_list << info.filePath;
        }
        DBManager::instance()->removeTrashImgInfosNoSignal(image_list);
    }
    return allTrashInfos;
}

DBImgInfoList AlbumControl::getTrashInfos2(const int &filterType)
{
    DBImgInfoList allTrashInfos = DBManager::instance()->getAllTrashInfos_getRemainDays();
    QDateTime currentTime = QDateTime::currentDateTime();
    DBImgInfoList list;
    for (int i = allTrashInfos.size() - 1; i >= 0; i--) {
        DBImgInfo pinfo = allTrashInfos.at(i);
        if (!QFile::exists(pinfo.filePath) &&
                !QFile::exists(getDeleteFullPath(pinfo.pathHash, pinfo.getFileNameFromFilePath()))) {
            allTrashInfos.removeAt(i);
        } else if (pinfo.remainDays <= 0) {
            list << pinfo;
            allTrashInfos.removeAt(i);
        } else if (pinfo.itemType != filterType && filterType != ItemTypeNull) {
            allTrashInfos.removeAt(i);
        }
    }
    //清理删除时间过长图片
    if (!list.isEmpty()) {
        QStringList image_list;
        for (DBImgInfo info : list) {
            image_list << info.filePath;
        }
        DBManager::instance()->removeTrashImgInfosNoSignal(image_list);
    }
    return allTrashInfos;
}

DBImgInfoList AlbumControl::getCollectionInfos()
{
    return DBManager::instance()->getInfosByAlbum(DBManager::SpUID::u_Favorite, false);
}

DBImgInfoList AlbumControl::getScreenCaptureInfos()
{
    return DBManager::instance()->getInfosByAlbum(DBManager::SpUID::u_ScreenCapture, false);
}

DBImgInfoList AlbumControl::getCameraInfos()
{
    return DBManager::instance()->getInfosByAlbum(DBManager::SpUID::u_Camera, false);
}

QString AlbumControl::getDeleteFullPath(const QString &hash, const QString &fileName)
{
    //防止文件过长,采用只用hash的名称;
    return albumGlobal::DELETE_PATH + "/" + hash + "." + QFileInfo(fileName).suffix();
}

//需求变更：允许相册重名，空字符串返回Unnamed，其余字符串返回本名
const QString AlbumControl::getNewAlbumName(const QString &baseName)
{
    QString albumName;
    if (baseName.isEmpty()) {
        albumName = tr("Unnamed");
    } else {
        albumName = baseName;
    }
    return static_cast<const QString>(albumName);
}

bool AlbumControl::canFavorite(const QStringList &pathList)
{
    bool bCanFavorite = false;
    for (int i = 0; i < pathList.size(); i++) {
        if (!pathList[i].isEmpty() && !photoHaveFavorited(pathList[i])) {
            bCanFavorite = true;
            break;
        }
    }

    return bCanFavorite;
}

bool AlbumControl::canAddToCustomAlbum(const int &albumId, const QStringList &pathList)
{
    bool bCanAddToCustom = false;
    for (int i = 0; i < pathList.size(); i++) {
        if (!pathList[i].isEmpty() && !photoHaveAddedToCustomAlbum(albumId, pathList[i])) {
            bCanAddToCustom = true;
            break;
        }
    }

    return bCanAddToCustom;
}

bool AlbumControl::photoHaveFavorited(const QString &path)
{
    bool bRet = DBManager::instance()->isImgExistInAlbum(DBManager::SpUID::u_Favorite, url2localPath(path));
    return bRet;
}

bool AlbumControl::photoHaveAddedToCustomAlbum(int albumId, const QString &path)
{
    return DBManager::instance()->isImgExistInAlbum(albumId, url2localPath(path));
}

int AlbumControl::getCustomAlbumInfoConut(const int &albumId, const int &filterType)
{
    int rePicVideoConut = 0;
    DBImgInfoList dbInfoList = DBManager::instance()->getInfosByAlbum(albumId, false);
    for (DBImgInfo info : dbInfoList) {
        QVariantMap tmpMap;
        if (info.itemType == ItemTypePic) {
            if (filterType == 2) {
                continue ;
            }
        } else if (info.itemType == ItemTypeVideo) {
            if (filterType == 1) {
                continue ;
            }
        }
        rePicVideoConut++;
    }
    return rePicVideoConut;
}

int AlbumControl::getAllInfoConut(const int &filterType)
{
    ItemType type = ItemTypeNull;
    if (filterType == 2) {
        type = ItemTypeVideo ;
    }
    if (filterType == 1) {
        type = ItemTypePic ;
    }
    return DBManager::instance()->getImgsCount(type);
}

int AlbumControl::getTrashInfoConut(const int &filterType)
{
    DBImgInfoList allTrashInfos;
    if (filterType == 0)
        allTrashInfos = getTrashInfos2(ItemTypeNull);
    else if (filterType == 1)
        allTrashInfos = getTrashInfos2(ItemTypePic);
    else if (filterType == 2)
        allTrashInfos = getTrashInfos2(ItemTypeVideo);

    return allTrashInfos.size();
}

void AlbumControl::removeAlbum(int UID)
{
    DBManager::instance()->removeAlbum(UID);
}

void AlbumControl::removeFromAlbum(int UID, const QStringList &paths)
{
    AlbumDBType atype = AlbumDBType::Custom;
    if (UID == 0) {
        atype = AlbumDBType::Favourite;
    }
    //判断是否是自动导入
    if (isAutoImportAlbum(UID)) {
        atype = AlbumDBType::AutoImport;
    }

    QStringList localPaths ;
    for (QString path : paths) {
        localPaths << url2localPath(path);
    }

    DBManager::instance()->removeCustomAlbumIdByPaths(UID, localPaths);

    DBManager::instance()->removeFromAlbum(UID, localPaths, atype);
}

bool AlbumControl::insertIntoAlbum(int UID, const QStringList &paths)
{
    AlbumDBType atype = AlbumDBType::Custom;
    if (UID == 0) {
        atype = AlbumDBType::Favourite;
    }
    QStringList localPaths ;
    for (QString path : paths) {
        localPaths << url2localPath(path);
    }

    DBManager::instance()->addCustomAlbumIdByPaths(UID, localPaths);

    return DBManager::instance()->insertIntoAlbum(UID, localPaths, atype);
}

bool AlbumControl::insertImportIntoAlbum(int UID, const QStringList &paths)
{
    AlbumDBType atype = AlbumDBType::AutoImport;
    if (UID == 0) {
        atype = AlbumDBType::Favourite;
    }
    QStringList localPaths ;
    for (QString path : paths) {
        localPaths << url2localPath(path);
    }
    return DBManager::instance()->insertIntoAlbum(UID, localPaths, atype);
}

bool AlbumControl::renameAlbum(int UID, const QString &newName)
{
    DBManager::instance()->renameAlbum(UID, newName);
    return true;
}

QVariant AlbumControl::searchPicFromAlbum(int UID, const QString &keywords, bool useAI)
{
    DBImgInfoList dbInfos;
    if (useAI) { //使用AI进行分析
        ;
    } else { //不使用AI分析，直接按文件路径搜索
        if (UID == -1) {
            dbInfos = DBManager::instance()->getInfosForKeyword(keywords);
        } else if (UID == -2) {
            dbInfos = DBManager::instance()->getTrashInfosForKeyword(keywords);
        } else {
            dbInfos = DBManager::instance()->getInfosForKeyword(UID, keywords);
        }
    }

    QStringList paths;
    std::transform(dbInfos.begin(), dbInfos.end(), std::back_inserter(paths), [](const DBImgInfo & info) {
        return "file://" + info.filePath;
    });

    return paths;
}

DBImgInfoList AlbumControl::searchPicFromAlbum2(int UID, const QString &keywords, bool useAI)
{
    DBImgInfoList dbInfos;
    if (useAI) { //使用AI进行分析
        ;
    } else { //不使用AI分析，直接按文件路径搜索
        if (UID == -1) {
            dbInfos = DBManager::instance()->getInfosForKeyword(keywords);
        } else if (UID == -2) {
            dbInfos = DBManager::instance()->getTrashInfosForKeyword(keywords);
        } else {
            dbInfos = DBManager::instance()->getInfosForKeyword(UID, keywords);
        }
    }

    return dbInfos;
}

QStringList AlbumControl::imageCanExportFormat(const QString &path)
{
    QString localPath = url2localPath(path);
    QStringList formats;
    formats << "jpg";
    formats << "jpeg";
    formats << "png";
    formats << "bmp";
    formats << "pgm";
    formats << "xbm";
    formats << "xpm";
    QFileInfo info(localPath);
    info.suffix();
    if (!formats.contains(info.suffix())) {
        if (!info.suffix().isEmpty())
            formats << info.suffix();
    }
    return formats;
}

bool AlbumControl::saveAsImage(const QString &path, const QString &saveName, int index, const QString &fileFormat, int pictureQuality, const QString &saveFolder)
{
    bool bRet = false;
    QString localPath = url2localPath(path);
    QString savePath;
    QString finalSaveFolder;
    switch (index) {
    case 0:
        finalSaveFolder = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        break;
    case 1:
        finalSaveFolder = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        break;
    case 2:
        finalSaveFolder = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
        break;
    case 3:
        finalSaveFolder = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        break;
    case 4:
        finalSaveFolder = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
        break;
    case 5:
        finalSaveFolder = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
        break;
    default :
        finalSaveFolder = saveFolder;
        break;
    }
    savePath = finalSaveFolder + "/" + saveName + "." + fileFormat;
    QStringList formats;
    formats << "jpg";
    formats << "jpeg";
    formats << "png";
    formats << "bmp";
    formats << "pgm";
    formats << "xbm";
    formats << "xpm";
    QFileInfo info(localPath);
    if (!formats.contains(info.suffix())) {

        QFileInfo fileinfo(savePath);
        if (fileinfo.exists() && !fileinfo.isDir()) {
            //目标位置与原图位置相同则直接返回
            if (localPath == savePath) {
                return true;
            }
            //目标位置与原图位置不同则先删除再复制
            if (QFile::remove(savePath)) {
                bRet = QFile::copy(localPath, savePath);
            }
        } else {
            bRet = QFile::copy(localPath, savePath);
        }
    } else {
        QImage m_saveImage;
        QString errMsg;
        LibUnionImage_NameSpace::loadStaticImageFromFile(localPath, m_saveImage, errMsg);
        bRet = m_saveImage.save(savePath, fileFormat.toUpper().toLocal8Bit().data(), pictureQuality);
    }


    return bRet;
}

QString AlbumControl::getFolder()
{
    QFileDialog dialog;
    QString fileDir("");
    dialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    if (dialog.exec()) {
        fileDir = dialog.selectedFiles().first();
    }
    return fileDir;
}

bool AlbumControl::getFolders(const QStringList &paths)
{
    bool bRet = true;
    QFileDialog dialog;
    QString fileDir;
    dialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    if (dialog.exec()) {
        fileDir = dialog.selectedFiles().first();
    }
    QStringList localPaths;
    for (QString path : paths) {
        localPaths << url2localPath(path);
    }
    if (!fileDir.isEmpty()) {

        for (QString path : localPaths) {

            QString savePath = fileDir + "/" + QFileInfo(path).completeBaseName() + "." + QFileInfo(path).completeSuffix();
            QFileInfo fileinfo(savePath);
            if (fileinfo.exists() && !fileinfo.isDir()) {
                //目标位置与原图位置相同则直接返回
                if (path == savePath) {
                    return true;
                }
                //目标位置与原图位置不同则先删除再复制
                if (QFile::remove(savePath)) {
                    bRet = QFile::copy(path, savePath);
                }
            } else {
                bRet = QFile::copy(path, savePath);
            }
        }

    }
    return bRet;
}

bool AlbumControl::exportFolders(const QStringList &paths, const QString &dir)
{
    bool bRet = true;
    QFileDialog dialog;
    QString fileDir;
    dialog.setDirectory(QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setOption(QFileDialog::ShowDirsOnly);
    if (dialog.exec()) {
        fileDir = dialog.selectedFiles().first();
    }
    QStringList localPaths;
    for (QString path : paths) {
        localPaths << url2localPath(path);
    }
    if (!fileDir.isEmpty()) {

        QString newDir = fileDir + "/" + dir;
        QDir a;
        a.mkdir(newDir);


        for (QString path : localPaths) {

            QString savePath = newDir + "/" + QFileInfo(path).completeBaseName() + "." + QFileInfo(path).completeSuffix();
            QFileInfo fileinfo(savePath);
            if (fileinfo.exists() && !fileinfo.isDir()) {
                //目标位置与原图位置相同则直接返回
                if (path == savePath) {
                    return true;
                }
                //目标位置与原图位置不同则先删除再复制
                if (QFile::remove(savePath)) {
                    bRet = QFile::copy(path, savePath);
                }
            } else {
                bRet = QFile::copy(path, savePath);
            }
        }
    }
    return bRet;
}

void AlbumControl::openDeepinMovie(const QString &path)
{
    QString localPath = url2localPath(path);
    if (LibUnionImage_NameSpace::isVideo(localPath)) {
        QProcess *process = new QProcess(this);
        QStringList arguments;
        arguments << path;
        bool isopen = process->startDetached("deepin-movie", arguments);
        if (!isopen) {
            arguments.clear();
            arguments << "-o" << path;
            process->startDetached("dde-file-manager", arguments);
        }

        connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
    }
}

QString AlbumControl::getFileTime(const QString &path1, const QString &path2)
{
    auto time1 = DBManager::instance()->getFileImportTime(url2localPath(path1));
    auto time2 = DBManager::instance()->getFileImportTime(url2localPath(path2));

    auto str1 = time1.toString("yyyy/MM/dd");
    auto str2 = time2.toString("yyyy/MM/dd");

    QString language = QLocale::system().name();
    if (language == "zh_CN") {
        str1 = QString(tr("%1Year%2Month%3Day"))
                   .arg(time1.date().year())
                   .arg(time1.date().month())
                   .arg(time1.date().day());
        str2 = QString(tr("%1Year%2Month%3Day"))
                   .arg(time2.date().year())
                   .arg(time2.date().month())
                   .arg(time2.date().day());
    }
    if (time1 < time2) {
        return str1 + "-" + str2;
    } else {
        return str2 + "-" + str1;
    }
}

QString AlbumControl::getMovieInfo(const QString key, const QString &path)
{
    QString value = "";
    if (!path.isEmpty()) {
        QString localPath = url2localPath(path);
        if (!m_movieInfos.keys().contains(localPath)) {
            MovieInfo movieInfo = MovieService::instance()->getMovieInfo(QUrl::fromLocalFile(localPath));
            //对视频信息缓存
            m_movieInfos[localPath] = movieInfo;
        }
        MovieInfo movieInfo = m_movieInfos.value(localPath);
        if (QString("Video CodecID").contains(key)) {
            value = movieInfo.vCodecID;
        } else if (QString("Video CodeRate").contains(key)) {
            if (movieInfo.vCodeRate == 0) {
                value = "-";
            } else {
                value = movieInfo.vCodeRate > 1000 ? QString::number(movieInfo.vCodeRate / 1000) + " kbps"
                        : QString::number(movieInfo.vCodeRate) + " bps";
            }
        } else if (QString("FPS").contains(key)) {
            value = movieInfo.fps == 0 ? "-" : QString::number(movieInfo.fps) + " fps";
        } else if (QString("Proportion").contains(key)) {
            value = movieInfo.proportion <= 0 ? "-" : QString::number(movieInfo.proportion);
        } else if (QString("Resolution").contains(key)) {
            value = movieInfo.resolution;
        } else if (QString("Audio CodecID").contains(key)) {
            value = movieInfo.aCodeID;
        } else if (QString("Audio CodeRate").contains(key)) {
            if (movieInfo.aCodeRate == 0) {
                value = "-";
            } else {
                value = movieInfo.aCodeRate > 1000 ? QString::number(movieInfo.aCodeRate / 1000) + " kbps"
                        : QString::number(movieInfo.aCodeRate) + " bps";
            }
        } else if (QString("Audio digit").contains(key)) {
            value = movieInfo.aDigit == 0 ? "-" : QString::number(movieInfo.aDigit);
        } else if (QString("Channels").contains(key)) {
            value = movieInfo.channels == 0 ? "-" : QString::number(movieInfo.channels) + tr("Channel");
        } else if (QString("Sampling").contains(key)) {
            value = movieInfo.sampling == 0 ? "-" : QString::number(movieInfo.sampling) + " hz";
        } else if (QString("DateTimeOriginal").contains(key)) {
            QFileInfo info(localPath);
            if (info.lastModified().isValid()) {
                value = info.lastModified().toString("yyyy/MM/dd HH:mm");
            } else if (info.birthTime().isValid()) {
                value = info.birthTime().toString("yyyy/MM/dd HH:mm");
            }
        } else if (QString("Type").contains(key)) {
            value = movieInfo.fileType.toLower();
        } else if (QString("Size").contains(key)) {
            value = movieInfo.sizeStr();
        } else if (QString("Duration").contains(key)) {
            value = movieInfo.duration;
        } else if (QString("Path").contains(key)) {
            value = movieInfo.filePath;
        }
    }
    return value;
}

int AlbumControl::getYearCount(const QString &year)
{
    return DBManager::instance()->getYearCount(year);
}

QStringList AlbumControl::getYears()
{
    return DBManager::instance()->getYears();
}

int AlbumControl::getMonthCount(const QString &year, const QString &month)
{
    return DBManager::instance()->getMonthCount(year, month);
}

QStringList AlbumControl::getMonthPaths(const QString &year, const QString &month)
{
    return DBManager::instance()->getMonthPaths(year, month, 6);
}

QStringList AlbumControl::getMonths()
{
    return DBManager::instance()->getMonths();
}

QStringList AlbumControl::getDeviceNames()
{
    return m_durlAndNameMap.values();
}

QStringList AlbumControl::getDevicePaths()
{
    return m_durlAndNameMap.keys();
}

QString AlbumControl::getDeviceName(const QString &devicePath)
{
    return m_durlAndNameMap.value(devicePath);
}

QStringList AlbumControl::getDevicePicPaths(const QString &strPath)
{
    // 若设备路径未被扫描，先扫描出所有图片/视频文件，设备容量越大，文件越大，扫描耗时越长
    if (m_PhonePicFileMap.find(strPath) == m_PhonePicFileMap.end()) {
        sltLoadMountFileList(strPath);
    }

    QStringList pathsList;
    if (m_PhonePicFileMap.find(strPath) != m_PhonePicFileMap.end()) {
        QStringList list = m_PhonePicFileMap.value(strPath);
        for (QString path : list) {
            pathsList << "file://" + path;
        }
    }
    return pathsList;
}

QVariantMap AlbumControl::getDeviceAlbumInfos(const QString &devicePath, const int &filterType)
{
    QVariantMap reMap;
    QVariantList listVar;
    QStringList list = getDevicePicPaths(devicePath);
    QString title = devicePath;
    for (QString path : list) {
        QVariantMap tmpMap;
        if (LibUnionImage_NameSpace::isImage(url2localPath(path))) {
            if (filterType == 2) {
                continue ;
            }
            tmpMap.insert("itemType", "pciture");
        } else if (LibUnionImage_NameSpace::isVideo(url2localPath(path))) {
            if (filterType == 1) {
                continue ;
            }
            tmpMap.insert("itemType", "video");
        } else {
            tmpMap.insert("itemType", "other");
        }
        tmpMap.insert("url", path);
        tmpMap.insert("filePath", url2localPath(path));
        tmpMap.insert("pathHash", "");
        tmpMap.insert("remainDays", "");
        listVar << tmpMap;
    }
    if (listVar.count() > 0) {
        reMap.insert(title, listVar);
    }
    return reMap;
}

DBImgInfoList AlbumControl::getDeviceAlbumInfos2(const QString &devicePath, const int &filterType)
{
    DBImgInfoList infoList;

    QStringList list = getDevicePicPaths(devicePath);
    for (QString path : list) {
        DBImgInfo info;
        if (LibUnionImage_NameSpace::isImage(url2localPath(path))) {
            if (filterType == ItemTypeVideo) {
                continue ;
            }
            info.itemType = ItemTypePic;
        } else if (LibUnionImage_NameSpace::isVideo(url2localPath(path))) {
            if (filterType == ItemTypePic) {
                continue ;
            }
            info.itemType = ItemTypeVideo;
        } else {
            continue;
        }
        info.filePath = url2localPath(path);
        info.pathHash = "";
        info.remainDays = 0;
        infoList << info;
    }

    return infoList;
}

int AlbumControl::getDeviceAlbumInfoConut(const QString &devicePath, const int &filterType)
{
    int rePicVideoConut = 0;
    QStringList list = getDevicePicPaths(devicePath);
    for (QString path : list) {
        QVariantMap tmpMap;
        if (LibUnionImage_NameSpace::isImage(url2localPath(path))) {
            if (filterType == ItemTypePic) {
                rePicVideoConut++;
            }
        } else if (LibUnionImage_NameSpace::isVideo(url2localPath(path))) {
            if (filterType == ItemTypeVideo) {
                rePicVideoConut++;
            }
        }
    }
    return rePicVideoConut;
}

QList<int> AlbumControl::getPicVideoCountFromPaths(const QStringList &paths)
{
    int countPic = 0;
    int countVideo = 0;
    QList<int> ret;

    for (QString path : paths) {
        if (LibUnionImage_NameSpace::isImage(url2localPath(path))) {
            countPic++;
        } else if (LibUnionImage_NameSpace::isVideo(url2localPath(path))) {
            countVideo++;
        }
    }

    ret.append(countPic);
    ret.append(countVideo);

    return ret;
}

void AlbumControl::importFromMountDevice(const QStringList &paths, const int &index)
{
    //采用线程执行导入
    QThread *thread = QThread::create([ = ] {
        QStringList localPaths;
        for (QString path : paths)
        {
            localPaths << url2localPath(path);
        }
        QStringList newPathList;
        DBImgInfoList dbInfos;
        QString strHomePath = QDir::homePath();
        //获取系统现在的时间
        QString strDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
        QString basePath = QString("%1%2%3/%4").arg(strHomePath, "/Pictures/", tr("Pictures"), strDate);
        QDir dir;
        if (!dir.exists(basePath))
        {
            dir.mkpath(basePath);
        }
        for (QString strPath : localPaths)
        {
            //取出文件名称
            QStringList pathList = strPath.split("/", Qt::SkipEmptyParts);
            QStringList nameList = pathList.last().split(".", Qt::SkipEmptyParts);
            QString strNewPath = QString("%1%2%3%4%5%6").arg(basePath, "/", nameList.first(),
                                                             QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()), ".", nameList.last());
            //判断新路径下是否存在目标文件，若不存在，继续循环
            if (!dir.exists(strPath)) {
                continue;
            }
            //判断新路径下是否存在目标文件，若存在，先删除掉
            if (dir.exists(strNewPath)) {
                dir.remove(strNewPath);
            }
            newPathList << strNewPath;
            QFileInfo fi(strNewPath);
            //复制失败的图片不算在成功导入
            if (QFile::copy(strPath, strNewPath)) {
                dbInfos << getDBInfo(strNewPath, LibUnionImage_NameSpace::isVideo(strNewPath));
            } else {
                newPathList.removeOne(strNewPath);
            }

        }
        if (!dbInfos.isEmpty())
        {
            QStringList pathslist;
            int idblen = dbInfos.length();
            for (int i = 0; i < idblen; i++) {
                if (m_bneedstop) {
                    return;
                }
                pathslist << dbInfos[i].filePath;
            }


            DBManager::instance()->insertImgInfos(dbInfos);
            if (index > 0) {
                DBManager::instance()->insertIntoAlbum(index, pathslist);
                emit sigRefreshCustomAlbum(index);
            }
            emit sigRefreshImportAlbum();
            emit sigRefreshAllCollection();
        }
    });
    thread->start();
    connect(thread, &QThread::destroyed, thread, &QObject::deleteLater);

}

QString AlbumControl::getYearCoverPath(const QString &year)
{
    auto paths = DBManager::instance()->getYearPaths(year, 1);
    if (paths.isEmpty())
        return "";
    return paths[0];
}

//获取指定日期的照片路径
QStringList AlbumControl::getDayPaths(const QString &day)
{
    return DBManager::instance()->getDayPaths(day);
}

int AlbumControl::getDayInfoCount(const QString &day, const int &filterType)
{
    int rePicVideoConut = 0;
    QStringList list = getDayPaths(day);
    for (QString path : list) {
        QVariantMap tmpMap;
        if (LibUnionImage_NameSpace::isImage(url2localPath(path))) {
            if (filterType == ItemTypePic) {
                rePicVideoConut++;
            }
        } else if (LibUnionImage_NameSpace::isVideo(url2localPath(path))) {
            if (filterType == ItemTypeVideo) {
                rePicVideoConut++;
            }
        }
    }
    return rePicVideoConut;
}

//获取日期
QStringList AlbumControl::getDays()
{
    return DBManager::instance()->getDays();
}

int AlbumControl::getImportAlubumCount()
{
    QMap <int, QString> customAutoImportUIDAndPaths = DBManager::instance()->getAllCustomAutoImportUIDAndPath();
    return customAutoImportUIDAndPaths.count();
}

QList<int> AlbumControl::getImportAlubumAllId()
{
    QMap <int, QString> customAutoImportUIDAndPaths = DBManager::instance()->getAllCustomAutoImportUIDAndPath();
    return customAutoImportUIDAndPaths.keys();
}

QStringList AlbumControl::getImportAlubumAllPaths()
{
    QMap <int, QString> customAutoImportUIDAndPaths = DBManager::instance()->getAllCustomAutoImportUIDAndPath();
    return customAutoImportUIDAndPaths.values();
}

QStringList AlbumControl::getImportAlubumAllNames()
{
    return DBManager::instance()->getAllCustomAutoImportNames();
}

void AlbumControl::removeCustomAutoImportPath(int uid)
{
    return DBManager::instance()->removeCustomAutoImportPath(uid);
}

void AlbumControl::createNewCustomAutoImportAlbum(const QString &path)
{
    QString folder = path;
    if (!QFileInfo(folder).isDir()) {
        folder = getFolder();
    }

    // 点击取消，不再执行自动导入相册流程
    if (folder.isEmpty())
        return;

    //自定义自动导入路径的相册名是文件夹最后一级的名字
    QString albumName = folder.split('/').last();
    int UID = DBManager::instance()->createNewCustomAutoImportPath(folder, albumName);

    QStringList urls;
    urls << QUrl::fromLocalFile(folder).toString();
    importAllImagesAndVideos(urls, UID);
}

QString AlbumControl::getVideoTime(const QString &path)
{
    if (!LibUnionImage_NameSpace::isVideo(url2localPath(path)))
        return "00:00";

    //采用线程执行导入
    QThread *thread = QThread::create([ = ] {
        m_mutex.lock();
        QString reString;
        reString = MovieService::instance()->getMovieInfo(QUrl(path)).duration;
        if (reString == "-")
        {
            reString = "00:00";
        }

        if (reString.left(2) == "00")
        {
            reString = reString.right(5);
        }

        emit sigRefreashVideoTime(path, reString);
        m_mutex.unlock();
    });
    thread->start();
    connect(thread, &QThread::destroyed, thread, &QObject::deleteLater);
    return "00:00";
}

//外部使用相册打开图片
void AlbumControl::onNewAPPOpen(qint64 pid, const QStringList &arguments)
{
    qDebug() << __FUNCTION__ << "---";
    Q_UNUSED(pid);
    QStringList paths;
    QStringList validPaths;
    if (arguments.length() > 1) {
        //arguments第1个参数是进程名，图片paths参数需要从下标1开始
        for (int i = 1; i < arguments.size(); ++i) {
            QString qpath = arguments.at(i);
            //BUG#79815，添加文件URL解析（BUG是平板上的，为防止UOS的其它个别版本也改成传URL，干脆直接全部支持）
            auto urlPath = QUrl(qpath);
            if (urlPath.scheme() == "file") {
                qpath = url2localPath(urlPath);
            }

            if (QUrl::fromUserInput(qpath).isLocalFile()) {
                qpath = "file://" + qpath;
            }

            if (LibUnionImage_NameSpace::isImage(url2localPath(qpath))
                    || LibUnionImage_NameSpace::isVideo(url2localPath(qpath))) {
                validPaths.append(qpath);
            }
            paths.append(qpath);
        }

        if (!paths.isEmpty() > 0) {
            if (validPaths.count() > 0) {
                emit sigOpenImageFromFiles(validPaths);
            } else {
                emit sigInvalidFormat();
            }
        }
    }

    emit sigActiveApplicationWindow();
}
