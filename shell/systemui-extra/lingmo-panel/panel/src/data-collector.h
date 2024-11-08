#ifndef DATACOLLECTOR_H
#define DATACOLLECTOR_H

#include <lingmosdk/diagnosetest/libkydatacollect.h>
#include <QString>

class DataCollector
{
private:
    /**
     * 将panel的位置转化为英文字符串上下左右  上: 1, 下: 0, 左: 2, 右: 3, 如果为其他值，则说明任务栏不存在
     * @param pos panel的位置
     */
    static QString positionToStr(const int &pos);

public:
    /**
     * 发送panel位置改变事件的数据锚点
     * @param pos panel的位置
     */
    static void setPanelPositionEvent(const int &pos);
    /**
     * 发送panel大小改变事件的数据锚点描点
     * @param panelSize panel的大小
     */
    static void setPanelSizeEvent(const int &panelSize);
};

#endif // DATACOLLECTOR_H
