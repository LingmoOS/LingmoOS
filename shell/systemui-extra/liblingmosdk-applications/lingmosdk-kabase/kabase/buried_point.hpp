#ifndef BURIED_POINT_H_
#define BURIED_POINT_H_

#include <QDBusInterface>
#include <QString>
#include <QJsonObject>

#include "currency.hpp"

namespace kdk
{
namespace kabase
{

class BuriedPoint
{
public:
    BuriedPoint();
    ~BuriedPoint();

    /* 埋点类型 */
    enum BuriedPointType {
        FunctionType = 0, /* 功能性打点 */
        PerformanceType,  /* 性能型打点 */
        StabilityType,    /* 稳定型打点 */
        TestType          /* 测试预留 */
    };

    /* 点位 */
    /* 规则说明 */
    /* 0x 00       0       000
     *    |        |        |
     * 应用标识   类型标识   点位
     */
    enum PT {
        LingmoIpmsgNicknameModify = 0x010001,        /* 昵称修改 */
        LingmoIpmsgOpenSaveDir = 0x010002,           /* 打开文件保存目录(主界面入口) */
        LingmoIpmsgMainSearch = 0x010003,            /* 主界面搜索 */
        LingmoIpmsgHistorySearch = 0x010004,         /* 历史记录搜索 */
        LingmoIpmsgSetTop = 0x010005,                /* 设置置顶 */
        LingmoIpmsgModifyFriendNotes = 0x010006,     /* 修改好友备注 */
        LingmoIpmsgViewInformation = 0x010007,       /* 查看资料 */
        LingmoIpmsgChangeDir = 0x010008,             /* 更改目录 */
        LingmoIpmsgCleanCache = 0x010009,            /* 清空缓存 */
        LingmoIpmsgClearChatRecord = 0x010010,       /* 清空聊天记录 */
        LingmoIpmsgClearSingleChatRecord = 0x010011, /* 清空单人聊天记录 */
        LingmoIpmsgDeleteRecord = 0x010012,          /* 删除记录 */
        LingmoIpmsgBatchDelete = 0x010013,           /* 批量删除 */
        LingmoIpmsgSendMessage = 0x010014,           /* 发送消息 */
        LingmoIpmsgSendFiles = 0x010015,             /* 发送文件 */
        LingmoIpmsgSendDir = 0x010016,               /* 发送目录 */
        LingmoIpmsgSendScreenshot = 0x010017,        /* 发送截屏 */
        LingmoIpmsgResend = 0x010018,                /* 重新发送 */
        LingmoIpmsgCopy = 0x010019,                  /* 复制 */
        LingmoIpmsgOpen = 0x010020,                  /* 打开 */
        LingmoIpmsgOpenDir = 0x010021,               /* 打开目录 */
        LingmoIpmsgSaveAs = 0x010022,                /* 另存为 */
        LingmoIpmsgTray = 0x010023,                  /* 托盘 */

        LingmoFontViewInstallFont = 0x020001,          /* 安装字体 */
        LingmoFontViewUninstallFont = 0x020002,        /* 卸载字体 */
        LingmoFontViewSearchFont = 0x020003,           /* 搜索字体 */
        LingmoFontViewApplyFont = 0x020004,            /* 应用字体 */
        LingmoFontViewExportFont = 0x020005,           /* 导出字体 */
        LingmoFontViewCollectionFont = 0x020006,       /* 收藏字体 */
        LingmoFontViewCancelCollectionFont = 0x020007, /* 取消收藏字体 */
        LingmoFontViewFontInformation = 0x020008,      /* 字体信息 */
        LingmoFontViewCopywritingChange = 0x020009,    /* 文案更改 */
        LingmoFontViewFontSizeChange = 0x020010,       /* 预览字号调整 */

        LingmoCalaulatorStandardMode = 0x030001,   /* 标准模式 */
        LingmoCalaulatorScientificMode = 0x030002, /* 科学模式 */
        LingmoCalaulatorRateMode = 0x030003,       /* 汇率模式 */
        LingmoCalaulatorProgrammerMode = 0x030004, /* 程序员模式 */

        LingmoWeatherCollection = 0x040001, /* 收藏 */
        LingmoWeatherChangeCity = 0x040002, /* 切换城市 */
        LingmoWeatherCitySearch = 0x040003, /* 城市搜索 */
        LingmoWeatherFullScreen = 0x040004, /* 全屏 */

        LingmoPhotoViewerOpenPicture = 0x050001,           /* 打开图片 */
        LingmoPhotoViewerSwitchPicture = 0x050002,         /* 切换图片 */
        LingmoPhotoViewerFlip = 0x050003,                  /* 翻转 */
        LingmoPhotoViewerRotate = 0x050004,                /* 旋转 */
        LingmoPhotoViewerAddPicture = 0x050005,            /* 添加图片 */
        LingmoPhotoViewerZoomInandOut = 0x050006,          /* 放大缩小 */
        LingmoPhotoViewerPictureDetails = 0x050007,        /* 图片详情 */
        LingmoPhotoViewerCutting = 0x050008,               /* 裁剪 */
        LingmoPhotoViewerCoyp = 0x050009,                  /* 复制 */
        LingmoPhotoViewerPrint = 0x050010,                 /* 打印 */
        LingmoPhotoViewerDelete = 0x050011,                /* 删除 */
        LingmoPhotoViewerSaveAs = 0x050012,                /* 另存为 */
        LingmoPhotoViewerRename = 0x050013,                /* 重命名 */
        LingmoPhotoViewerSetAsDesktopWallpaper = 0x050014, /* 设置为桌面壁纸 */
        LingmoPhotoViewerShowInFolder = 0x050015,          /* 在文件夹中显示 */
        LingmoPhotoViewerPicturePreview = 0x050016,        /* 图片预览 */
        LingmoPhotoViewerCurrentPointZoom = 0x050017,      /* 当前点缩放 */

        LingmoServiceSupportFileUpload = 0x060001,           /* 文件上传 */
        LingmoServiceSupportVideoape = 0x060002,             /* 录像 */
        LingmoServiceSupportFileCopy = 0x060003,             /* 文件拷贝 */
        LingmoServiceSupportJumpOfficicalWebsite = 0x060004, /* 跳转官网主页 */
        LingmoServiceSupportJumpOnlineService = 0x060005,    /* 跳转在线客服 */

        LingmoPrinterManuallyInstallPrinter = 0x070001, /* 手动安装打印机 */
        LingmoPrinterSetResolution = 0x070002,          /* 设置分辨率 */
        LingmoPrinterSetPaperSize = 0x070003,           /* 设置纸张大小 */
        LingmoPrinterSetPaperType = 0x070004,           /* 设置纸张类型 */
        LingmoPrinterSetCationSource = 0x070005,        /* 设置纸盒来源 */
        LingmoPrinterSetDuplexPrint = 0x070006,         /* 设置双面打印 */
        LingmoPrinterSetInkType = 0x070007,             /* 设置墨水类型 */
        LingmoPrinterADDRemovePrinter = 0x070008,       /* 添加删除打印机 */
        LingmoPrinterSetShareStartup = 0x070009,        /* 设置共享启动 */
        LingmoPrintTestPage = 0x070010,                 /* 打印测试页 */
        LingmoPrinterCancelPrintJob = 0x070011,         /* 取消打印任务 */
        LingmoPrinterDeletePrintJob = 0x070012,         /* 删除打印任务 */
        LingmoPrinterRePrint = 0x070013,                /* 重新打印 */
        LingmoPrinterManualyModifyDrive = 0x070014,     /* 手动修改驱动 */
        LingmoPrinterRename = 0x070015,                 /* 重命名 */

        LingmoRecorderRecording = 0x080001,        /* 录音 */
        LingmoRecorderPlayPause = 0x080002,        /* 播放暂停 */
        LingmoRecorderClip = 0x080003,             /* 剪辑 */
        LingmoRecorderSign = 0x080004,             /* 标记 */
        LingmoRecorderDelete = 0x080005,           /* 删除 */
        LingmoRecorderBluetoothRecord = 0x080006,  /* 蓝牙录音 */
        LingmoRecorderSaveAs = 0x080007,           /* 另存为 */
        LingmoRecorderOpenFileLocation = 0x080008, /* 打开文件位置 */
        LingmoRecorderTopicSwitch = 0x080009,      /* 主题切换 */

        LingmoCameraPreviewMonitor = 0x090001,         /* 预览画面 */
        LingmoCameraSingleShot = 0x090002,             /* 单拍 */
        LingmoCameraContinuousShot = 0x090003,         /* 连拍 */
        LingmoCameraDelay = 0x090004,                  /* 延时拍照 */
        LingmoCameraVideotape = 0x090005,              /* 录像 */
        LingmoCameraCameraSelection = 0x090006,        /* 摄像头选用 */
        LingmoCameraResolutionSelection = 0x090007,    /* 分辨率选用 */
        LingmoCameraVideoFormatSelection = 0x090008,   /* 视频格式选用 */
        LingmoCameraPictureFormatSelection = 0x090009, /* 图片格式选用 */
        LingmoCameraChangeSavePath = 0x090010,         /* 更改存储路径 */
        LingmoCameraThumbnail = 0x090011,              /* 缩略图 */
        LingmoCameraGridLine = 0x090012,               /* 网格线 */
        LingmoCameraMirrorFun = 0x090013,              /* 镜像功能 */

        LingmoNotebookOrderList = 0x100001,      /* 有序列表 */
        LingmoNotebookUnorderList = 0x100002,    /* 无序列表 */
        LingmoNotebookBold = 0x100003,           /* 加粗 */
        LingmoNotebookItalics = 0x100004,        /* 斜体 */
        LingmoNotebookUnderline = 0x100005,      /* 下划线 */
        LingmoNotebookDeleteline = 0x100006,     /* 删除线 */
        LingmoNotebookFontSize = 0x100007,       /* 字号 */
        LingmoNotebookFontColor = 0x100008,      /* 字体颜色 */
        LingmoNotebookInsertPicture = 0x100009,  /* 插入图片 */
        LingmoNotebookInterfaceColor = 0x100010, /* 界面配色 */
        LingmoNotebookDeleteCurrent = 0x100011,  /* 删除当前 */
        LingmoNotebookUiTop = 0x100012,          /* UI 置顶 */
        LingmoNotebookListMode = 0x100013,       /* 列表模式 */
        LingmoNotebookIconMode = 0x100014,       /* 图标模式 */
        LingmoNotebookNewNote = 0x100015,        /* 新建便签 */
        LingmoNotebookSearch = 0x100016,         /* 搜索 */
        LingmoNotebookDelete = 0x100017,         /* 删除 */
        LingmoNotebookModeChange = 0x100018,     /* 模式切换 */

        LingmoOsManagerGarbageClean = 0x110001,    /* 垃圾清理 */
        LingmoOsManagerFileShredding = 0x110002,   /* 文件粉碎 */

        LingmoGpuControllerBaseInfo = 0x120001,     /* 基本信息 */
        LingmoGpuControllerRunState = 0x120002,     /* 运行状态 */
        LingmoGpuControllerDriveInfo = 0x120003,    /* 驱动信息 */
        LingmoGpuControllerSwitch = 0x120003,       /* 显卡切换 */

        LingmoNetworkCheckStartCheck = 0x130001,    /* 开始检测 */

        LingmoGallerySwitchFolder = 0x140001,    /* 切换目录 */
        LingmoGalleryOpenViewer = 0x140002,      /* 打开灵墨看图 */

        LingmoMobileAssistantAndroidConn = 0x150001,        /* 安卓链接 */
        LingmoMobileAssistantPcConn = 0x150002,             /* PC链接 */
        LingmoMobileAssistantUsbConn = 0x150003,            /* Usb链接 */
        LingmoMobileAssistantWifiConn = 0x150004,           /* Wifi链接 */
        LingmoMobileAssistantDeviceDiscovery = 0x150005,    /* 设备发现 */
        LingmoMobileAssistantDisconnect = 0x150006,         /* 断开链接 */
        LingmoMobileAssistantMobileScreen = 0x150007,       /* 手机投屏 */
        LingmoMobileAssistantPcScreen = 0x150008,           /* PC 投屏 */
        LingmoMobileAssistantPictureList = 0x150009,        /* 图片列表 */
        LingmoMobileAssistantVideoList = 0x150010,          /* 视频列表 */
        LingmoMobileAssistantAudioList = 0x150011,          /* 音频列表 */
        LingmoMobileAssistantDocList = 0x150012,            /* 文档列表 */
        LingmoMobileAssistantQQFileList = 0x150013,         /* QQ 文件列表 */
        LingmoMobileAssistantWechatFileList = 0x150014,     /* 微信文件列表 */
        LingmoMobileAssistantMobileStorage = 0x150015,      /* 手机存储 */
        LingmoMobileAssistantSwitchView = 0x150016,         /* 切换视图 */
        LingmoMobileAssistantRefreshList = 0x150017,        /* 刷新列表 */
        LingmoMobileAssistantUploadFile = 0x150018,         /* 上传文件 */
        LingmoMobileAssistantDownloadFile = 0x150019,       /* 下载文件 */
        LingmoMobileAssistantOpenFile = 0x150020,           /* 打开文件 */

        LingmoScannerOneClickBeautification = 0x160001,    /* 一键美化 */
        LingmoScannerRectification = 0x160002,             /* 自动纠偏 */
        LingmoScannerTextRecognition = 0x160003,           /* 文本识别 */
        LingmoScannerCutting = 0x160004,                   /* 裁剪 */
        LingmoScannerRotate = 0x160005,                    /* 旋转 */
        LingmoScannerImage = 0x160006,                     /* 镜像 */
        LingmoScannerAddWatermark = 0x160007,              /* 加水印 */
        LingmoScannerSendMail = 0x160008,                  /* 发送邮件 */
        LingmoScannerSaveAs = 0x160009,                    /* 另存为 */
        LingmoScannerSinglePageScan = 0x160010,            /* 单页扫描 */
        LingmoScannerMultiPageScan = 0x160011,             /* 多页扫描 */

        LingmoCalendarMonthDetails = 0x170001,    /* 查看月详情 */
        LingmoCalendarMonthSwitch = 0x170002,     /* 月切换 */
        LingmoCalendarDoday = 0x170003,           /* 定位到今天 */

        TestFunPoint = 0x999999 /* 测试预留 */
    };

    /**
     * @brief 功能性打点
     *
     * @param packageName 应用名
     * @param point 点位
     *
     * @retval 0 成功
     * @retval 非0 失败
     */
    int functionBuriedPoint(AppName packageName, PT point);

    /**
     * @brief 上传打点数据
     *
     * @param packageName 应用名
     * @param buriedPointType 埋点类型
     * @param buriedPointData 埋点数据
     *
     * @retval 0 成功
     * @retval 非0 失败
     */
    int uploadMessage(AppName packageName, BuriedPointType buriedPointType, QJsonObject buriedPointData);

private:
    /* dbus 返回状态 */
    enum returnState {
        OK = 0,                                     /* 存储成功 */
        InvalidArgumentFormat = 1,                  /* 参数格式错误 */
        InvalidTid = 2,                             /* tid异常 , 但消息存储成功 */
        InvalidUploadedMessageSha256 = 3,           /* shan256异常 */
        InvalidUploadedMessageSha256Decryption = 4, /* sha256解密异常 */
        InvalidCreateTimeStamp = 5                  /* 时间字段异常 */
    };

    void evpError(void);
    int checkDir(void);
    QByteArray encrypt(const QByteArray &md5, const QByteArray &keyBase64); /* 加密 */
    QString getBuriedPointType(BuriedPointType type);
    QString getBuriedPointData(QJsonObject data);
    QString decToHex(int dec);

    QString m_packageInfoIdConfigPath;
    QDBusInterface *m_dbusInterface = nullptr;
};

} /* namespace kabase */
} /* namespace kdk */

#endif