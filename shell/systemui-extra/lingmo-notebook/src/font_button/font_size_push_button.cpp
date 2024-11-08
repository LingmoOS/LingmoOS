#include <QObject>
#include <QVariant>

#include "font_size_push_button.h"

FontSizePushButton::FontSizePushButton(QWidget *parent,
                                       const QString &size)
    : ResizablePushButton(parent)
    , m_buttonSize(size)
{
    setButtonSize(size);
}

FontSizePushButton::~FontSizePushButton()
{
}

void FontSizePushButton::setButtonSize(const QString &size)
{
    setText(size);
}