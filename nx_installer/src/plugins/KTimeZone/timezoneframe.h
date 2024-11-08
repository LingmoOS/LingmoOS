#ifndef TIMEZONEFRAME_H
#define TIMEZONEFRAME_H

#include <QObject>
#include <QWidget>
#include <QImage>
#include <QPainter>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include "mapwidget.h"
#include "../PluginService/ui_unit/middleframemanager.h"
namespace KInstaller {
class TimeZoneFrame : public MiddleFrameManager
{
    Q_OBJECT
public:
    explicit TimeZoneFrame(QWidget *parent = nullptr);
    ~TimeZoneFrame();
    void addStyleSheet();
    void initUI();
    void initAllConnect();
    void translateStr();

    void initComboxZone();

    QString readSettingIni();                   //读写systemsetting.ini文件
    void writeSettingIni();                     //读写systemsetting.ini文件
    void setBoardTabOrder();

private:
    void changeEvent(QEvent *event);
    void keyPressEvent(QKeyEvent *event);
public:
    QComboBox* m_comboxZone;
    QLabel* m_mainTitle;
    MapWidget* m_mapWid;
    TimeZoneModel* m_zoneModel;
    TimeZoneStruct m_curzoneItem;

signals:
    void signalLanguageSetZone(TimeZoneStruct zoneItem);

public slots:
    void showCurrentZone(TimeZoneStruct zoneItem);
    void clickNextButton();
    void showCurrentZoneIndex(int index);

};
}

#endif // TIMEZONEFRAME_H
