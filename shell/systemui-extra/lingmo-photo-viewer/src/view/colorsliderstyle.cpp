#include "colorsliderstyle.h"
#include <QDebug>
#include <QStyleOptionComplex>
#include <QPainter>

ColorSliderStyle::ColorSliderStyle()
{

}

ColorSliderStyle::~ColorSliderStyle()
{

}

int ColorSliderStyle::styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget, QStyleHintReturn *returnData) const
{
    if (hint == QStyle::SH_Slider_AbsoluteSetButtons) {
        return (Qt::LeftButton);
    }
    return QProxyStyle::styleHint(hint, option, widget, returnData);

}
