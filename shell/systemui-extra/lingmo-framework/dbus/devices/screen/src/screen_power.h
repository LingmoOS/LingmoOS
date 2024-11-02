#ifndef __SCREEN_POWER_H__
#define __SCREEN_POWER_H__

#include "screen_abstract.h"

class ScreenBrightNode: public  ScreenAbstract
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "lingmo.framework.devices.screen")
public:

    explicit ScreenBrightNode(QObject *parent = nullptr);
    ~ScreenBrightNode();

public:
    /**
     * @brief setScreenBrightness 设置显示器亮度，需要先获取所有显示器的亮度再使用。
     * @param name 显示器名称
     * @param screenBrightness 指定亮度
     * @return
     */
     bool setScreenBrightness(QString name, uint screenBrightness);

    /**
     * @brief getScreenBrightness 获取显示器的亮度
     * @param name
     * @return 指定显示器的亮度
     */
     uint getScreenBrightness(QString name);

    /**
     * @brief setAllScreenBrightness 分别设置显示器的亮度
     * @param screenMap 显示器亮度与其对应的显示器亮度
     * @return 成功标记
     */
     bool setScreensBrightness(OutputGammaInfoList outputsInfo);

    /**
     * @brief setAllScreenSameBrightness 设置所有显示为同一个亮度
     * @param brighenss 全部亮度
     * @return 与参数一致
     */
     bool setAllScreenSameBrightness(uint brighenss);

    /**
     * @brief getAllScreenBrightness
     * @return 与参数一致
     */
     OutputGammaInfoList getAllScreenBrightness();
};

#endif
