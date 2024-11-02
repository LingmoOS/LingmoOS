#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <string>

namespace kabase
{

/* 应用名 */
enum AppName {
    LingmoIpmsg = 0,          /* 传书 */
    LingmoFontViewer,         /* 字体管理器 */
    LingmoCalculator,         /* 灵墨计算器 */
    LingmoGpuController,      /* 显卡控制器 */
    LingmoMusic,              /* 音乐 */
    LingmoWeather,            /* 天气 */
    LingmoPhotoViewer,        /* 看图 */
    LingmoServiceSupport,     /* 服务与支持 */
    LingmoPrinter,            /* 灵墨打印 */
    LingmoCalendar,           /* 日历 */
    LingmoRecorder,           /* 录音 */
    LingmoCamera,             /* 摄像头 */
    LingmoNotebook,           /* 便签 */
    LingmoOsManager,          /* 灵墨管家 */
    LingmoNetworkCheck,       /* 网络检测工具 */
    LingmoGallery,            /* 相册 */
    LingmoScanner,            /* 扫描 */
    LingmoMobileAssistant,    /* 多端协同 */
    LingmoTest                /* 测试预留 */
};

class Utils
{
public:
    Utils() = default;
    ~Utils() = default;

    static std::string getAppName(AppName appName)
    {
        switch (appName) {
        case AppName::LingmoCalculator:
            return "lingmo-calaulator";
        case AppName::LingmoCalendar:
            return "lingmo-calendar";
        case AppName::LingmoCamera:
            return "lingmo-camera";
        case AppName::LingmoFontViewer:
            return "lingmo-font-viewer";
        case AppName::LingmoGpuController:
            return "lingmo-gpu-controller";
        case AppName::LingmoIpmsg:
            return "lingmo-ipmsg";
        case AppName::LingmoMusic:
            return "lingmo-music";
        case AppName::LingmoPhotoViewer:
            return "lingmo-photo-viewer";
        case AppName::LingmoPrinter:
            return "lingmo-printer";
        case AppName::LingmoRecorder:
            return "lingmo-recorder";
        case AppName::LingmoServiceSupport:
            return "lingmo-service-support";
        case AppName::LingmoWeather:
            return "lingmo-weather";
        case AppName::LingmoNotebook:
            return "lingmo-notebook";
        case AppName::LingmoOsManager:
            return "lingmo-os-manager";
        case AppName::LingmoNetworkCheck:
            return "lingmo-network-check-tools";
        case AppName::LingmoGallery:
            return "lingmo-gallery";
        case AppName::LingmoScanner:
            return "lingmo-scanner";
        case AppName::LingmoMobileAssistant:
            return "lingmo-mobile-assistant";
        default:
            return "";
        }

        /* 不应该被执行 */
        return "";
    };
};

}

#endif