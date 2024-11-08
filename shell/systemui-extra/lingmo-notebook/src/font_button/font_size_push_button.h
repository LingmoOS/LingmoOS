#ifndef FONT_SIZE_PUSH_BUTTON_H
#define FONT_SIZE_PUSH_BUTTON_H

#include "resizable_push_button.h"
#include <QString>

class FontSizePushButton : public ResizablePushButton
{
    Q_OBJECT
private:
    /* data */
public:
    explicit FontSizePushButton(QWidget *parent = 0,
                                const QString &size = QString());
    ~FontSizePushButton();
    void setButtonSize(const QString &size);

    void getButtonSize();
private:
    QString m_buttonSize{QString()};

};

#endif // FONT_SIZE_PUSH_BUTTON_H