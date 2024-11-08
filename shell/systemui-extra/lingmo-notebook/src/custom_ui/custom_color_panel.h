#ifndef CUSTOM_COLOR_PANEL_H
#define CUSTOM_COLOR_PANEL_H

#include "ktranslucentfloor.h"
#include "kcolorbutton.h"
#include "lingmo_settings_monitor.h"

#include <QVBoxLayout>
#include <QButtonGroup>



class CustomColorPanel : public kdk::KTranslucentFloor
{
    Q_OBJECT

public:
    CustomColorPanel(QWidget *parent = nullptr);
    void init();
    void addColor(const QColor& color = QColor(Qt::black), bool isClicked = false);
    void setSelectColor(const QColor& color);
    static QString BLACK;
    static QString WHITE;

private:
    QVBoxLayout *m_layout{nullptr};
    QButtonGroup *m_group{nullptr};
    QList<kdk::KColorButton *> m_buttons;
    kdk::KColorButton *m_blackOrWhiteButton{nullptr};
    kdk::KColorButton *m_selectButton{nullptr};

    void handleStyleStatus(LingmoUISettingsMonitor::StyleStatus status);
    void handleTabletMode(LingmoUISettingsMonitor::TabletStatus status);
    bool isBlackOrWhite(const QColor& color);
    void setSelectColorButton(kdk::KColorButton *button);

protected:
    virtual void handleDarkMode();
    virtual void handleLightMode();
    virtual void handlePCMode();
    virtual void handlePADMode();
    void showEvent(QShowEvent *event) override;

signals:
    void currentColorChanged(const QColor& color);
};

#endif // CUSTOM_COLOR_PANEL_H
