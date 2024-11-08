#ifndef FONT_TWIN_BUTTON_GROUP_H
#define FONT_TWIN_BUTTON_GROUP_H

#include "custom_push_button_group.h"
#include "font_size_push_button.h"
#include "font_color_push_button.h"

class FontTwinButtonGroup : public CustomPushButtonGroup
{
    Q_OBJECT
public:
    FontTwinButtonGroup(QWidget *parent = nullptr);
    ~FontTwinButtonGroup();

    FontSizePushButton *getFontSizeBtn();
    FontColorPushButton *getFontColorBtn();
    void widgetThemeUpdate(bool isClassical);
private:
    void initUi();
    void initConnect();
    FontSizePushButton *fontSizeBtn = nullptr;
    FontColorPushButton *fontColorBtn = nullptr;
};

#endif // FONT_TWIN_BUTTON_GROUP_H
