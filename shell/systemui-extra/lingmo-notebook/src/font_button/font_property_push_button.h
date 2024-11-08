#ifndef __FONT_PROPERTY_PUSH_BUTTON_H__
#define __FONT_PROPERTY_PUSH_BUTTON_H__

#include "resizable_push_button.h"


class FontPropertyPushButton : public ResizablePushButton
{
    Q_OBJECT
public:
    FontPropertyPushButton(QWidget *parent = nullptr);
    ~FontPropertyPushButton();

};
#endif // __FONT_PROPERTY_PUSH_BUTTON_H__