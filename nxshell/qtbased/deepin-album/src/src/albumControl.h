// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AlbumControl_H
#define AlbumControl_H

#include <QObject>
#include <QUrl>
#include "unionimage/unionimage.h"
#include "dbmanager/dbmanager.h"
#include "imageengine/movieservice.h"

#include <dfm-mount/ddevicemanager.h>
using namespace dfmmount;

class FileInotifyGroup;

class AlbumControl : public QObject
{
    Q_OBJECT
public:

    enum TimeLineEnum {
        All  = 0,
        Year = 1,
        Month  = 2,
        Day = 3,
        Import = 4
    };

    explicit AlbumControl(QObject *parent = nullptr);
    ~  AlbumControl();
    static AlbumControl *instance();

    //获得全部导入的DBImgInfoList
    Q_INVOKABLE void getAllInfos();

    //获得全部某个相册导入的DBImgInfoList
    Q_INVOKABLE DBImgInfoList getAllInfosByUID(QString uid);

    //获得支持导入的格式
    Q_INVOKABLE QString getAllFilters();

    //卸载设备
    Q_INVOKABLE void unMountDevice(const QString &devicePath);

    //获得全部导入的url路径
    Q_INVOKABLE QStringList getAllUrlPaths(const int &filterType = 0);

    //获得全部导入的路径
    Q_INVOKABLE QStringList getAllPaths(const int &filterType = 0);

    //获得全部导入的路径
    Q_INVOKABLE QVariantList getAlbumAllInfos(const int &filterType = 0);

    //导入图片，导入图片接口
    Q_INVOKABLE bool importAllImagesAndVideos(const QStringList &paths, const int UID = -1, const bool notifyUI = true);

    //导入图片，导入图片接口urls
    Q_INVOKABLE bool importAllImagesAndVideosUrl(const QList <QUrl> &paths, const int UID, bool checkRepeat = true);

    //获得全部创建时间线
    Q_INVOKABLE QStringList getAllTimelinesTitle(const int &filterType = 0);

    //获得年创建时间线
    Q_INVOKABLE QStringList getYearTimelinesTitle(const int &filterType = 0);

    //获得月创建时间线
    Q_INVOKABLE QStringList getMonthTimelinesTitle(const int &filterType = 0);

    //获得日创建时间线
    Q_INVOKABLE QStringList getDayTimelinesTitle(const int &filterType = 0);

    //获得某一创建时间线的全部路径
    Q_INVOKABLE QStringList getTimelinesTitlePaths(const QString &titleName, const int &filterType);

    //获得全部已经导入
    Q_INVOKABLE QStringList getAllImportTimelinesTitle(const int &filterType = 0);

    //获得某一导入时间的全部路径
    Q_INVOKABLE QStringList getImportTimelinesTitlePaths(const QString &titleName, const int &filterType = 0);

    //QVariantMap接口
    //获得默认时间线的全部info  , 0:全部 1:图片 2:视频
    Q_INVOKABLE QVariantMap getTimelinesTitleInfos(const int &filterType = 0);

    //获得年创建时间线 0:全部 1:图片 2:视频
    Q_INVOKABLE QVariantMap getYearTimelinesInfos(const int &filterType = 0);

    //获得月创建时间线  0:全部 1:图片 2:视频
    Q_INVOKABLE QVariantMap getMonthTimelinesInfos(const int &filterType = 0);

    //获得月创建时间线  0:全部 1:图片 2:视频
    Q_INVOKABLE QVariantMap getDayTimelinesInfos(const int &filterType = 0);

    //获得某一导入时间线的全部info  , 0:全部 1:图片 2:视频
    Q_INVOKABLE QVariantMap getImportTimelinesTitleInfos(const int &filterType = 0);

    //获得某一导入时间线的全部info  ,倒序， 0:全部 1:图片 2:视频
    Q_INVOKABLE QVariantList getImportTimelinesTitleInfosReverse(const int &filterType = 0);

    //获得图片和视频总数
    Q_INVOKABLE int getAllCount(const int &filterType = 0);

    //将文件放进最近删除(添加)
    Q_INVOKABLE void insertTrash(const QList< QUrl > &paths);

    //删除最近删除里面的文件
    Q_INVOKABLE void removeTrashImgInfos(const QList< QUrl > &paths);

    //恢复文件 //返回无法恢复的文件
    Q_INVOKABLE QStringList recoveryImgFromTrash(const QStringList &paths);

    //从最近删除里面删除图片
    Q_INVOKABLE void deleteImgFromTrash(const QStringList &paths);

    //将文件放进收藏中(添加)
    Q_INVOKABLE void insertCollection(const QList< QUrl > &paths);

    //新建相册
    Q_INVOKABLE void createAlbum(const QString &newName);

    //获得所有的普通自动导入相册id
    Q_INVOKABLE QList < int > getAllNormlAutoImportAlbumId();

    //获得所有的系统自动导入相册id
    Q_INVOKABLE QList < int > getAllSystemAutoImportAlbumId();

    //获得所有的自动导入相册id
    Q_INVOKABLE QList < int > getAllAutoImportAlbumId();

    //获得所有的自定义相册id
    Q_INVOKABLE QList < int > getAllCustomAlbumId();

    //获得所有的自定义相册名称
    Q_INVOKABLE QList < QString > getAllCustomAlbumName();

    //获得自定义的相册的全部info  albumId 0:我的收藏  1:截图录屏  2:相机 3:画板 4-~:其他自定义,filterType 0:全部 1:图片 2:视频
    Q_INVOKABLE QStringList getAlbumPaths(const int &albumId, const int &filterType = 0);

    //获得自定义的相册的全部info  albumId 0:我的收藏  1:截图录屏  2:相机 3:画板 4-~:其他自定义,filterType 0:全部 1:图片 2:视频
    Q_INVOKABLE QVariantMap getAlbumInfos(const int &albumId, const int &filterType = 0);

    //获得最近删除的相册的全部info  , 0:全部 1:图片 2:视频
    Q_INVOKABLE QVariantMap getTrashAlbumInfos(const int &filterType = 0);

    //添加到自定义相册
    Q_INVOKABLE bool addCustomAlbumInfos(int albumId, const QList <QUrl> &urls);

    //根据自定义相册id获取相册名称
    Q_INVOKABLE QString getCustomAlbumByUid(const int &index);

    //判断当前图片是否已收藏
    Q_INVOKABLE bool photoHaveFavorited(const QString &path);

    // 判断当前图片是否已添加到指定自定义相册
    Q_INVOKABLE bool photoHaveAddedToCustomAlbum(int albumId, const QString &path);

    //判断所传图片列表是否可收藏
    Q_INVOKABLE bool canFavorite(const QStringList &pathList);

    //判断所传图片列表是否可添加到自定义相册
    Q_INVOKABLE bool canAddToCustomAlbum(const int &albumId, const QStringList &pathList);

    //获得自定义相册图片count 0:我的收藏  1:截图录屏  2:相机 3:画板 4-~:其他自定义,filterType 0:全部 1:图片 2:视频
    Q_INVOKABLE int getCustomAlbumInfoConut(const int &albumId, const int &filterType = 0);

    //全部导入的info的count filterType 0:全部 1:图片 2:视频
    Q_INVOKABLE int getAllInfoConut(const int &filterType = 0);

    //全部最近删除info的count filterType 0:全部 1:图片 2:视频
    Q_INVOKABLE int getTrashInfoConut(const int &filterType = 0);

    //删除相册 0:我的收藏  1:截图录屏  2:相机 3:画板 4-~:其他自定义
    Q_INVOKABLE void removeAlbum(int UID);

    //删除某相册中的图片 0:我的收藏  1:截图录屏  2:相机 3:画板 4-~:其他自定义   paths:需要删除的图片地址
    Q_INVOKABLE void removeFromAlbum(int UID, const QStringList &paths);

    //添加某相册中的图片 0:我的收藏  1:截图录屏  2:相机 3:画板 4-~:其他自定义   paths:需要添加的图片地址
    Q_INVOKABLE bool insertIntoAlbum(int UID, const QStringList &paths);

    //添加某相册中的图片 0:我的收藏  1:截图录屏  2:相机 3:画板 4-~:其他自定义   paths:需要删除的图片地址
    Q_INVOKABLE bool insertImportIntoAlbum(int UID, const QStringList &paths);

    //修改相册名称
    Q_INVOKABLE bool renameAlbum(int UID, const QString &newName);

    //使用关键字在指定位置执行搜索 UID:相册的标识符，-1表示进行全数据库搜索，-2表示搜索最近删除；keywords:搜索依据
    //useAI为保留参数，false:不使用AI，只根据文件路径搜索；true:使用AI进行分析，根据关键字含义和图片内容进行搜索
    Q_INVOKABLE QVariant searchPicFromAlbum(int UID, const QString &keywords, bool useAI);

    Q_INVOKABLE DBImgInfoList searchPicFromAlbum2(int UID, const QString &keywords, bool useAI);

    //输入一张图片，获得可以导出的格式
    Q_INVOKABLE QStringList imageCanExportFormat(const QString &path);

    //输入一张图片，获得可以导出的格式
    Q_INVOKABLE bool saveAsImage(const QString &path, const QString &saveName, int index, const QString &fileFormat, int pictureQuality = 100, const QString &saveFolder = nullptr);

    //获得选择路径
    Q_INVOKABLE QString getFolder();

    //选择路径导出视频
    Q_INVOKABLE bool getFolders(const QStringList &paths);

    //选择路径导出文件及目录
    Q_INVOKABLE bool exportFolders(const QStringList &paths, const QString &dir);

    //用影院打开视频
    Q_INVOKABLE void openDeepinMovie(const QString &path);

    //根据输入路径生成时间范围
    Q_INVOKABLE QString getFileTime(const QString &path1, const QString &path2);

    //获得视频信息
    Q_INVOKABLE QString getMovieInfo(const QString key, const QString &path);

    //获取指定年份的总数
    Q_INVOKABLE int getYearCount(const QString &year);

    //获取年份
    Q_INVOKABLE QStringList getYears();

    //获取指定月份的总数
    Q_INVOKABLE int getMonthCount(const QString &year, const QString &month);

    //获取指定月份图片路径，最多6个
    Q_INVOKABLE QStringList getMonthPaths(const QString &year, const QString &month);

    //获取月份
    Q_INVOKABLE QStringList getMonths();

    //获取设备名称
    Q_INVOKABLE QStringList getDeviceNames();

    //获取设备路径
    Q_INVOKABLE QStringList getDevicePaths();

    //通过设备路径获得设备名称
    Q_INVOKABLE QString getDeviceName(const QString &devicePath);

    //获取设备的图片
    Q_INVOKABLE QStringList getDevicePicPaths(const QString &path);

    //获得device路径
    Q_INVOKABLE QVariantMap getDeviceAlbumInfos(const QString &devicePath, const int &filterType = 0);

    Q_INVOKABLE DBImgInfoList getDeviceAlbumInfos2(const QString &devicePath, const int &filterType = 0);

    //获得设备相册的图片和视频数量
    Q_INVOKABLE int getDeviceAlbumInfoConut(const QString &devicePath, const int &filterType);

    //手机照片导入 0为已导入，1-n为自定义相册
    Q_INVOKABLE void importFromMountDevice(const QStringList &paths, const int &index = 0);

    //获取年封面图片路径
    Q_INVOKABLE QString getYearCoverPath(const QString &year);

    //获取指定日期的照片路径
    Q_INVOKABLE QStringList getDayPaths(const QString &day);

    //获取指定日期的图片和视频数量
    Q_INVOKABLE int getDayInfoCount(const QString &day, const int &filterType);

    //获取日期
    Q_INVOKABLE QStringList getDays();

    //获得导入相册的数量
    Q_INVOKABLE int getImportAlubumCount();

    //获得所有id
    Q_INVOKABLE QList < int > getImportAlubumAllId();

    //获得所有路径
    Q_INVOKABLE QStringList getImportAlubumAllPaths();

    //获得所有名称
    Q_INVOKABLE QStringList getImportAlubumAllNames();

    //删除自定义导入相册
    Q_INVOKABLE void removeCustomAutoImportPath(int uid);

    //创建自定义导入相册
    Q_INVOKABLE void createNewCustomAutoImportAlbum(const QString &path = "");

    //获得视频时长
    Q_INVOKABLE QString getVideoTime(const QString &path = "");

    //判断是否是系统自动导入相册(截图录屏、相机、画板)
    Q_INVOKABLE bool isSystemAutoImportAlbum(int uid);

    //判断是否是普通自动导入相册
    Q_INVOKABLE bool isNormalAutoImportAlbum(int uid);

    //判断是否是自动导入相册
    Q_INVOKABLE bool isAutoImportAlbum(int uid);

    //判断是否是自定义相册
    Q_INVOKABLE bool isCustomAlbum(int uid);

    //判断默认路径是否存在
    Q_INVOKABLE bool isDefaultPathExists(int uid);

    // 显示快捷键提示界面
    Q_INVOKABLE void ctrlShiftSlashShortcut(int x, int y, int w, int h);

    // 根据两点获取矩形区域
    Q_INVOKABLE QRect rect(QPoint p1, QPoint p2);

    // 获取两个矩形的相交矩形区域
    Q_INVOKABLE QRect intersected(QRect r1, QRect r2);

    // 获取两个点的距离
    Q_INVOKABLE int manhattanLength(QPoint p1, QPoint p2);

    // QUrl路径转为本地路径
    Q_INVOKABLE QString url2localPath(QUrl url);

    // 检查是否有重复路径
    Q_INVOKABLE bool checkRepeatUrls(QStringList imported, QStringList urls, bool bNotify = true);

    //获得路径集合中视频/图片数量
    Q_INVOKABLE QList<int> getPicVideoCountFromPaths(const QStringList &paths);
public:
    QString getDeleteFullPath(const QString &hash, const QString &fileName);

    //获得最近删除的文件
    DBImgInfoList getTrashInfos(const int &filterType = 0);

    //获得最近删除的文件
    DBImgInfoList getTrashInfos2(const int &filterType = 0);

    //获得收藏文件
    DBImgInfoList getCollectionInfos();

    //获得画板文件
    DBImgInfoList getDrawInfos();

    //获得截图录屏文件
    DBImgInfoList getScreenCaptureInfos();

    //获得相机文件
    DBImgInfoList getCameraInfos();

    //新相册名称
    const QString getNewAlbumName(const QString &baseName);

    //获得日月年所有创建时间线  0所有 1年 2月 3日
    QStringList getTimelinesTitle(TimeLineEnum timeEnum, const int &filterType = 0);

    //初始化
    void initMonitor();

    //启动路径监控
    void startMonitor();

    //是否已经处于监控
    bool checkIfNotified(const QString &dirPath);

    //获得数据库信息
    DBImgInfo getDBInfo(const QString &srcpath, bool isVideo);

    //初始化设备监控
    void initDeviceMonitor();

    //寻找手机里面是否有图片
    bool findPicturePathByPhone(QString &path);

public slots:
    //监控到改变
    void slotMonitorChanged(QStringList fileAdd, QStringList fileDelete, QString album, int UID);

    //自动导入路径被删除
    void slotMonitorDestroyed(int UID);

    //加载设备路径的数据
    void sltLoadMountFileList(const QString &strPath);

    void onDeviceRemoved(const QString &deviceKey, DeviceType type);
    void onMounted(const QString &deviceKey, const QString &mountPoint, DeviceType type);
    void onUnMounted(const QString &deviceKey, DeviceType type);

    //外部使用相册打开图片
    void onNewAPPOpen(qint64 pid, const QStringList &arguments);

private:
    QJsonObject createShorcutJson();

    void getAllBlockDeviceName();
    void updateBlockDeviceName(const QString &blks);
    void onUnMountedExecute(const QString &deviceKey, DeviceType type);

signals:
    void sigRefreshAllCollection();
    void sigRefreshCustomAlbum(int UID = -1);
    void sigRefreshImportAlbum();
    void sigRefreshSearchView();

    //导入开始信号
    void sigImportStart();
    //导入进度信号
    void sigImportProgress(int value, int max = 100);
    //导入完成信号
    void sigImportFinished();

    //自定义相册删除
    void sigDeleteCustomAlbum(int UID);

    void sigMountsChange();

    void sigRefreshSlider();

    void sigAddCustomAlbum(const int &UID);

    void sigAddDevice(const QString &path);

    void sigRefreashVideoTime(const QString &url, const QString &videoTimeStr);

    void sigRepeatUrls(const QStringList &urls);

    //发送打开看图查看图片信号
    void sigOpenImageFromFiles(const QStringList &paths);

    // 激活应用主窗口
    void sigActiveApplicationWindow();

    // 通知打开了非图片/视频文件的格式
    void sigInvalidFormat();

private :
    static AlbumControl *m_instance;
    DBImgInfoList m_infoList;  //全部已导入

    //时间线数据和已导入（合集）数据
    QList < QDateTime > m_timelines; //所有创建时间线
    QList < QDateTime > m_importTimelines; //所有已导入时间线
    QMap < QString, DBImgInfoList > m_importTimeLinePathsMap;  //每个已导入时间线的路径
    QMap < QString, DBImgInfoList > m_timeLinePathsMap;  //每个创建时间线的路径
    QMap < QString, DBImgInfoList > m_yearDateMap; //年数据集
    QMap < QString, DBImgInfoList > m_monthDateMap; //月数据集
    QMap < QString, DBImgInfoList > m_dayDateMap; //日数据集
    QMap < int, QString > m_customAlbum; //自定义相册
    QMap < QString, MovieInfo> m_movieInfos; //movieInfo的合集

    FileInotifyGroup *m_fileInotifygroup {nullptr}; //固定文件夹监控

    DDeviceManager* m_deviceManager {nullptr};

    QMap<QString, QString> m_durlAndNameMap;        // 挂载点-设备名称map表
    QMap<QString, QString> m_blkPath2DeviceNameMap; // 块设备id-名称map表
    QMap<QString, QStringList> m_PhonePicFileMap;   // 外部设备及其全部图片路径
    std::atomic_bool m_couldRun;
    bool m_bneedstop = false;
    QMutex m_mutex;
};

#endif // AlbumControl_H
