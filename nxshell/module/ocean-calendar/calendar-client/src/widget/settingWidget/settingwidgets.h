// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SETTINGWIDGETS_H
#define SETTINGWIDGETS_H

#include "jobtypelistview.h"

#include <DIconButton>
#include <DSettingsDialog>

#include <QPushButton>
#include <QLabel>
#include <QWidget>
#include <QComboBox>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

DWIDGET_USE_NAMESPACE


namespace  SettingWidget{
    struct CalendarSettingOption{
        CalendarSettingOption(){}
        CalendarSettingOption(const std::initializer_list<QString> &list){
            int index = 0;
            for(auto d : list) {
                if(0 == index) _key     = d;
                if(1 == index) _name    = d;
                if(2 == index) _type    = d;
                if(3 == index) _default = d;
                if(4 == index) _text    = d;
                index ++;
            }
        }

        QJsonObject toJson(){
            QJsonObject obj;
            obj.insert("key",      _key);
            obj.insert("type",     _type);
            obj.insert("name",     _name);
            obj.insert("default",  _default);
            obj.insert("text",     _text);
            return obj;
        }

        QString _key;
        QString _type;
        QString _name;
        QString _default;
        QString _text;
    };
    struct CalendarSettingOptions : public QList<CalendarSettingOption>{
        CalendarSettingOptions(){}
        CalendarSettingOptions(const std::initializer_list<CalendarSettingOption> &list){
            for(auto d : list)
                append(d);
        }
        QJsonArray tojson()
        {
            QJsonArray arr;
            for(auto option: *this) {
                arr.append(option.toJson());
            }
            return arr;
        }
    };

    struct CalendarSettingGroup{
        QString _key;
        QString _name;
        CalendarSettingOptions _options;
        QJsonObject toJson(){
            QJsonObject obj;
            obj.insert("key",      _key);
            obj.insert("name",     _name);
            obj.insert("options",  _options.tojson());
            return obj;
        }
    };

    struct CalendarSettingGroups : public QList<CalendarSettingGroup>
    {
        CalendarSettingGroups(){}
        CalendarSettingGroups(const std::initializer_list<CalendarSettingGroup> &list){
            for(auto d : list)
                append(d);
        }
        QJsonArray toJson()
        {
            QJsonArray arr;
            for(auto group: *this) {
                arr.append(group.toJson());
            }
            return arr;
        }
    };

    struct CalendarSettingSetting {
        QString _key;
        QString _name;
        CalendarSettingGroups _groups;
        QJsonObject toJson() {

            QJsonObject obj;
            obj.insert("key",      _key);
            obj.insert("name",     _name);
            obj.insert("groups",   _groups.toJson());
            return obj;
        }
    };

    struct CalendarSettingSettings : public QList<CalendarSettingSetting>
    {
        CalendarSettingSettings(){}
        CalendarSettingSettings(const std::initializer_list<CalendarSettingSetting> &list){
            for(auto d : list)
                append(d);
        }
        QJsonArray toJson()
        {
            QJsonArray arr;
            for(auto setting: *this) {
                arr.append(setting.toJson());
            }
            return arr;
        }

        void removeGroup(const QString &groupName, const QString &groupName2);
        void removeGroup(const QString &groupName);
        int indexOf(const CalendarSettingGroups &groups, const QString groupName);
        int indexOf(const CalendarSettingSettings &groups, const QString groupName);
    };


    /**
 * @brief The SyncTagRadioButton class 实现checkstate效果的类
 */
    class SyncTagRadioButton : public QWidget
    {
        Q_OBJECT
    public:
        SyncTagRadioButton(QWidget *parent = nullptr);

        bool isChecked();

        /**
     * @brief setChecked 是否选中
     */
        void setChecked(bool checked);

    signals:
        void clicked(bool checked);

    protected:
        void paintEvent(QPaintEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;

    private:
        bool m_checked = true;
    };
}//~CalendarSetting


class SettingWidgets : public QObject
{
    Q_OBJECT
public:

    explicit SettingWidgets(QWidget *parent = nullptr);

};

#endif // SETTINGWIDGETS_H
