#include "timezonemodel.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "../PluginService/ksystemsetting_unit.h"
using namespace KServer;

QString getZoneList()
{
    QString tmp = KServer::ReadSettingIni("config", "language");
    QString lang = "";
     if(tmp.contains("zh_CN", Qt::CaseInsensitive)) {
        lang = ":/res/file/zone_ch";
     } else if(tmp.contains("en_US", Qt::CaseInsensitive) ||
               tmp.contains("de_DE", Qt::CaseInsensitive) ||
               tmp.contains("es_ES", Qt::CaseInsensitive) ||
               tmp.contains("fr_FR", Qt::CaseInsensitive)){
        lang = ":/res/file/zone_en";
     } else if(tmp.contains("bo_CN", Qt::CaseInsensitive)){
         lang = ":/res/file/zone_bo";
     } else if(tmp.contains("zh_HK", Qt::CaseInsensitive)){
         lang = ":/res/file/zone_hk";
     } else if(tmp.contains("mn_MN", Qt::CaseInsensitive)){
         lang = ":/res/file/zone_mn";
     } else if(tmp.contains("kk_KZ", Qt::CaseInsensitive)){
         lang = ":/res/file/zone_kk";
     } else if(tmp.contains("ky_KG", Qt::CaseInsensitive)){
         lang = ":/res/file/zone_ky";
     } else if(tmp.contains("ug_CN", Qt::CaseInsensitive)){
         lang = ":/res/file/zone_ug";
     } else {
         lang = ":/res/file/zone_ch";
     }
    return (QString)lang;
}

TimeZoneStructList loadZoneFile()
{
    TimeZoneStructList list;
    QFile file(getZoneList());
    if(file.open(QFile::ReadOnly)) {
        QTextStream filetext(&file);
        QStringList flist = filetext.readAll().split('\n');
        flist.removeLast();
        for(int j = 0; j < flist.length() - 2; j++) {
            QStringList str = flist.at(j).split(':');
            TimeZoneStruct item;
            item.encity = flist.at(j).split(';').at(1);
            item.country = str.at(0);
            QStringList tmp = str.at(2).split('/');
            item.zone = tmp.at(0);
//            if(str.at(2).split('/').length() >= 3) {
//                item.TZcity = str.at(2).split('/').at(2);
//            } else
//                item.TZcity = str.at(2).split('/').at(1);
            item.TZcity = tmp.at(tmp.length() - 1) + ":" + str.at(str.length() - 1).split(';').at(0);
            item.latitude = str.at(1).split('*').at(0);
            item.longitude = str.at(1).split('*').at(1);
            list.append(item);
        }
    } else {
        qCritical() << "fail to open the file";
    }
    return (TimeZoneStructList)list;
}

TimeZoneModel::TimeZoneModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_TZLists = loadZoneFile();
}

void TimeZoneModel::flushZone()
{
    m_TZLists.clear();
    m_TZLists = loadZoneFile();
}

void TimeZoneModel::setCurZone(int index)
{
    if(index < 0 || index >=m_TZLists.count()) {
        return;
    }
    m_curZone = m_TZLists.at(index);
    emit curZoneChanged();
}

TimeZoneStruct TimeZoneModel::getcurTimeZoneStruct()
{
    return m_curZone;
}

int TimeZoneModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (!parent.isValid())
        return m_TZLists.length();
    // FIXME: Implement me!
}

QVariant TimeZoneModel::data(const QModelIndex &index, int role) const
{
    if(role != Qt::DisplayRole) {
        return QVariant();
    }
    if (!index.isValid()) {
        return QVariant();
    } else {
        return m_TZLists.at(index.row()).TZcity;
    }
}

QModelIndex TimeZoneModel::getIndexByTZCity(QString tzcity)
{
    for(int i = 0; i < m_TZLists.length(); i++) {
        if(m_TZLists.at(i).TZcity == tzcity) {
            setCurZone(i);
            return index(i);
        }
    }
    return QModelIndex();
}
//QModelIndex TimeZoneModel::getIndexByLatLong(QStringList latlon)
//{
//    for(int i = 0; i < m_TZLists.length(); i++) {
//        if(m_TZLists.at(i).TZcity == tzcity) {
//            return index(i);
//        }
//    }
//    return QModelIndex();
//}

QModelIndex TimeZoneModel::getIndexByCountryCode(QString code)
{
    for(int i = 0; i < m_TZLists.length(); i++) {
        if(m_TZLists.at(i).country == code) {
            setCurZone(i);
            return index(i);
        }
    }
    return QModelIndex();
}

TimeZoneStruct TimeZoneModel::getLatLongByTZCity(QModelIndex index)
{
    if(index.isValid()) {
        return m_TZLists.at(index.row());
    } else {
        return TimeZoneStruct();
    }
}

//QStringList TimeZoneModel::getTZCityByLatLong(QString lat, QString lon)
//{

//}



