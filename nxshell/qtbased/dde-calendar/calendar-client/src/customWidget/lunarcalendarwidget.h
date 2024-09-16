// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef LUNARCALENDARWIDGET_H
#define LUNARCALENDARWIDGET_H

#include <QCalendarWidget>
class QLabel;
class CalenderStyle;

class LunarCalendarWidget : public QCalendarWidget
{
    Q_OBJECT
public:
    explicit LunarCalendarWidget(QWidget *parent = nullptr);
    virtual ~LunarCalendarWidget();

    /**
     * @brief setLunarYearText 设置阴历年描述语
     * @param text
     */
    void setLunarYearText(const QString &text);
    /**
     * @brief lunarYearText 返回阴历年描述语
     * @return
     */
    QString lunarYearText();

protected:
    virtual QSize minimumSizeHint() const override;

private:
    QLabel *m_lunarLabel   = nullptr;//阴历年的label
    CalenderStyle *m_style = nullptr;//阴历相关样式
};

#endif // LUNARCALENDARWIDGET_H
