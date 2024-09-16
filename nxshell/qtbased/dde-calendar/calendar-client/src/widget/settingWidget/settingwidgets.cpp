// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "settingwidgets.h"
#include "accountmanager.h"
#include "calendarmanage.h"
#include "units.h"

#include <DSettingsWidgetFactory>
#include <DSettingsOption>
#include <DHiDPIHelper>

#include <qglobal.h>

DWIDGET_USE_NAMESPACE
using namespace SettingWidget;

SettingWidgets::SettingWidgets(QWidget *parent) : QObject(parent)
{
}


SyncTagRadioButton::SyncTagRadioButton(QWidget *parent)
    : QWidget(parent)
{
}

bool SyncTagRadioButton::isChecked()
{
    return m_checked;
}

void SyncTagRadioButton::setChecked(bool checked)
{
    if (m_checked == checked)
        return;

    m_checked = checked;
    update();
}

void SyncTagRadioButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    QIcon icon = DStyle::standardIcon(this->style(), m_checked ? DStyle::SP_IndicatorChecked : DStyle::SP_IndicatorUnchecked);
    int y = (this->height() - 16) / 2;
    int x = (this->width() - 16) / 2;
    icon.paint(&painter, QRect(x, y, 16, 16), Qt::AlignCenter, isEnabled() ? QIcon::Normal : QIcon::Disabled);
}

void SyncTagRadioButton::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
    setChecked(!m_checked);
    emit clicked(isChecked());
}

void CalendarSettingSettings::removeGroup(const QString &groupName, const QString &groupName2)
{
    int index = this->indexOf(*this, groupName);
    if (index < 0)
        return;
    CalendarSettingGroups &groups = this->operator[](index)._groups;
    {
        int index = indexOf(groups, groupName2);
        if (index < 0)
            return;
        groups.removeAt(index);
    }
    if (groups.isEmpty()) {
        this->removeAt(index);
    }
}

void CalendarSettingSettings::removeGroup(const QString &groupName)
{
    int index = this->indexOf(*this, groupName);
    if (index < 0)
        return;
    this->removeAt(index);
}

int CalendarSettingSettings::indexOf(const CalendarSettingGroups &groups, const QString groupName)
{
    for (int k = 0; k < groups.count(); k++) {
        if (groups[k]._key == groupName)
            return k;
    }
    return -1;
}

int CalendarSettingSettings::indexOf(const CalendarSettingSettings &groups, const QString groupName)
{
    for (int k = 0; k < groups.count(); k++) {
        if (groups[k]._key == groupName)
            return k;
    }
    return -1;
}
