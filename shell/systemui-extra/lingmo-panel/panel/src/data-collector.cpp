#include "data-collector.h"

QString DataCollector::positionToStr(const int &pos)
{
    switch (pos)
    {
    case 1:
        return QString("Top");
    case 2:
        return QString("Left");
    case 3:
        return QString("Right");
    case 0:
        return QString("Bottom");
    }

    return QString();
}

void DataCollector::setPanelPositionEvent(const int &pos)
{
    KCustomProperty *property = new KCustomProperty;
    property->key = strdup(QStringLiteral("panelPosition").toLocal8Bit().data());
    property->value = strdup(positionToStr(pos).toLocal8Bit().data());
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_append_custom_property(node, property, 1);
    kdk_dia_upload_default(node, strdup(QStringLiteral("adjust_panel_position_event").toLocal8Bit().data()),strdup(QStringLiteral("lingmo-panel").toLocal8Bit().data()));
    kdk_dia_data_free(node);
    free(property->key);
    free(property->value);
    delete property;
}

void DataCollector::setPanelSizeEvent(const int &panelSize)
{
    KCustomProperty *property = new KCustomProperty;
    property->key = strdup(QStringLiteral("panelSize").toLocal8Bit().data());
    property->value = strdup(QString::number(panelSize).toLocal8Bit().data());
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_append_custom_property(node, property, 1);
    kdk_dia_upload_default(node, strdup(QStringLiteral("adjust_panel_size_event").toLocal8Bit().data()),strdup(QStringLiteral("lingmo-panel").toLocal8Bit().data()));
    kdk_dia_data_free(node);
    free(property->key);
    free(property->value);
    delete property;
}
