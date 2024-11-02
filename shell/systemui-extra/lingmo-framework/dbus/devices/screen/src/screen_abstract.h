#ifndef __SCREEN_ABSTRACT_H__
#define __SCREEN_ABSTRACT_H__

#include <QObject>
#include <QProcess>
#include <QMetaEnum>
#include <QMetaObject>
#include <QtDBus/QtDBus>
#include <QDBusInterface>
#include <QtCore/QCoreApplication>

#include "screen_define.h"
#include "QGSettings/qgsettings.h"


class ScreenAbstract: public QObject, QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "lingmo.framework.devices.screen")
public:

    explicit ScreenAbstract(QObject *parent = nullptr);
    ~ScreenAbstract();


public:
    /**
     * @brief setScreenBrightness 设置显示器亮度，需要先获取所有显示器的亮度再使用。
     * @param name 显示器名称
     * @param screenBrightness 指定亮度
     * @return
     */
    virtual bool setScreenBrightness(QString name, uint screenBrightness) = 0;

    /**
     * @brief getScreenBrightness 获取显示器的亮度
     * @param name
     * @return 指定显示器的亮度
     */
    virtual uint getScreenBrightness(QString name) = 0;

    /**
     * @brief setAllScreenBrightness 分别设置显示器的亮度
     * @param screenMap 显示器亮度与其对应的显示器亮度
     * @return 成功标记
     */
    virtual bool setScreensBrightness(OutputGammaInfoList outputsInfo) = 0;

    /**
     * @brief setAllScreenSameBrightness 设置所有显示为同一个亮度
     * @param brighenss 全部亮度
     * @return 与参数一致
     */
    virtual bool setAllScreenSameBrightness(uint brighenss) = 0;

    /**
     * @brief getAllScreenBrightness
     * @return 与参数一致
     */
    virtual OutputGammaInfoList getAllScreenBrightness() = 0;

private:
    int brightControlByUPM();
    bool isControlBright();
};

#endif
