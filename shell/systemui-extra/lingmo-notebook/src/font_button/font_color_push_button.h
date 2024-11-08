#ifndef FONT_COLOR_PUSH_BUTTON_H
#define FONT_COLOR_PUSH_BUTTON_H

#include "resizable_push_button.h"
#include "lingmo_settings_monitor.h"
#include <QColor>

class FontColorPushButton : public ResizablePushButton
{
    Q_OBJECT
private:
    /* data */
public:
    enum BtnType {
        None = 0,
        Circle,
        Rect
    };
    explicit FontColorPushButton(QWidget *parent = 0,
                                 const QColor &color = QColor(),
                                 BtnType type = BtnType::None);
    ~FontColorPushButton();
    void setButtonColor(const QColor &color);
    void setIconType(BtnType type);

    QColor buttonColor() const;
private:
    QColor m_buttonColor{QColor()};
    BtnType m_IconType{Rect};
    void handleStyleStatus(LingmoUISettingsMonitor::StyleStatus status);
    QIcon getButtonIcon(const QColor &buttonColor, BtnType iconType);
protected:
    virtual void handleDarkMode();
    virtual void handleLightMode();
    void showEvent(QShowEvent *event) override;

protected:
    void handlePCMode() override;
    void handlePADMode() override;
};

#endif // FONT_COLOR_PUSH_BUTTON_H