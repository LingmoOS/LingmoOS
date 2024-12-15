#include "linkbutton.h"

#include <QFontMetrics>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

QSize LinkButton::sizeHint() const {
    QSize size;
    QFontMetrics fontMetrics(this->font());
    size.setWidth(fontMetrics.width(this->text()));
    size.setHeight(fontMetrics.height());
    return size;
}
