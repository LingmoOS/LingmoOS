// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DATETIMEWIDGET_H
#define DATETIMEWIDGET_H

#include <QWidget>

class QLabel;
class RegionFormat;
class DatetimeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DatetimeWidget(RegionFormat *regionFormat, QWidget *parent = nullptr) ;

    inline bool is24HourFormat() const { return m_24HourFormat; }
    inline QString getDateTime() { return m_dateTime; }
    void setDockPanelSize(const QSize &dockSize);
    void dockPositionChanged();

    void setRegionFormat(RegionFormat *newRegionFormat);

protected:
    void resizeEvent(QResizeEvent *event);

signals:
    void requestUpdateGeometry() const;

public slots:
    void set24HourFormat(const bool value);
    void updateDateTimeString();

private Q_SLOTS:
    void setShortDateFormat(int type);
    void setWeekdayFormat(int type);

private:
    void initUI();
    void adjustUI();
    void adjustFontSize();
    void updateDateTime();
    void updateWeekdayFormat();

private:
    QLabel *m_timeLabel;
    QLabel *m_dateLabel;
    QLabel *m_apLabel;
    QWidget *m_spacerItem;

private:
    bool m_24HourFormat;
    int m_weekdayFormatType;
    QString m_shortDateFormat;
    QString m_dateTime;
    QString m_weekFormat;
    QSize m_dockSize;

    RegionFormat *m_regionFormat;
};

#endif // DATETIMEWIDGET_H
