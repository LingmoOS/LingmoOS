//
// Created by zpf on 2024/6/17.
//

#ifndef LINGMO_PANEL_COMMON_H
#define LINGMO_PANEL_COMMON_H

#include <QString>

namespace LingmoUIPanel {
/**
 * 屏幕不可用区域
 * 一般存在于屏幕四周
 */
class UnavailableArea {
public:
    int leftWidth = 0;
    int leftStart = 0;
    int leftEnd = 0;
    int rightWidth = 0;
    int rightStart = 0;
    int rightEnd = 0;
    int topWidth = 0;
    int topStart = 0;
    int topEnd = 0;
    int bottomWidth = 0;
    int bottomStart = 0;
    int bottomEnd = 0;
};

class NumberSetting {
public:
    NumberSetting(QString k, int d, int l, int r) : key(std::move(k)), defaultValue(d), min(l), max(r)
    {}

    QString key;
    int defaultValue{0};
    int min{0};
    int max{0};
};
#define PANEL_CONFIG_VERSION "1.0"
}
#endif //LINGMO_PANEL_COMMON_H
