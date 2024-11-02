#ifndef COLORSLIDERSTYLE_H
#define COLORSLIDERSTYLE_H
#include <QProxyStyle>

class ColorSliderStyle : public QProxyStyle
{
public:
    ColorSliderStyle();
    ~ColorSliderStyle();
    int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const; // 让handle滑块直接移到鼠标点击处
};


#endif // COLORSLIDERSTYLE_H
