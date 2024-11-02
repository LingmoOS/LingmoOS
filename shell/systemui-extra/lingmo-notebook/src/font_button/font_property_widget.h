#ifndef __FONT_PROPERTY_WIDGET_H__
#define __FONT_PROPERTY_WIDGET_H__

#include <QWidget>
#include "font_property_push_button.h"
QT_BEGIN_NAMESPACE
namespace Ui { class FontPropertyWidget; }
QT_END_NAMESPACE



class FontPropertyWidget : public QWidget
{
    Q_OBJECT
public:
    FontPropertyWidget(QWidget *parent = nullptr);
    ~FontPropertyWidget();

    FontPropertyPushButton *boldBtn();
    FontPropertyPushButton *italicBtn();
    FontPropertyPushButton *underlineBtn();
    FontPropertyPushButton *strikeOutBtn();
    FontPropertyPushButton *unorderedBtn();
    FontPropertyPushButton *orderedBtn();
    FontPropertyPushButton *insertBtn();
protected:
    virtual void handlePCMode();
    virtual void handlePADMode();
    void showEvent(QShowEvent *event) override;

private:
    void handleTabletMode(LingmoUISettingsMonitor::TabletStatus status);
private:
    Ui::FontPropertyWidget *ui;

};
#endif // __FONT_PROPERTY_WIDGET_H__