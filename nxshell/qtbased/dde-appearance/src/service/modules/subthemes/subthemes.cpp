// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "subthemes.h"
#include "impl/appearancemanager.h"
#include "../api/themethumb.h"
#include "../api/utils.h"
#include "../common/commondefine.h"
#include <QDir>
#include <QThread>
#include <QDebug>

DCORE_USE_NAMESPACE

const QString gsKeyExcludedIcon = "Excluded_Icon_Themes";
// 初始化用定时器空闲时延时处理
enum SubthemesInitStatus {
    initBegin,
    initGtkThemes = initBegin,
    initGlobalThemes,
    initCursorThemes,
    initCursorThumbnail,
    initIconThemes,
    initIconThumbnail,
    initFinished,
};

Subthemes::Subthemes(AppearanceManager *parent)
    : QObject(parent)
    , themeApi(new ThemesApi(parent))
    , initStatus(initBegin)
    , timer(new QTimer(this))
{
    connect(timer, &QTimer::timeout, this, &Subthemes::init);
    timer->start(500);

    gtkThumbnailMap["deepin"] ="light";
    gtkThumbnailMap["deepin-dark"] ="dark";
    gtkThumbnailMap["deepin-auto"] ="auto";
}

Subthemes::~Subthemes()
{
}

void Subthemes::refreshGtkThemes()
{
    gtkThemes = getThemes(themeApi->listGtkTheme());
}

void Subthemes::refreshIconThemes()
{
    iconThemes.clear();
    QVector<QSharedPointer<Theme>> infos = getThemes(themeApi->listIconTheme());

    QStringList blacklist;
    DConfig dConfig(APPEARANCESCHEMA);
    if(dConfig.isValid()){
        blacklist = dConfig.value(gsKeyExcludedIcon).toStringList();
    }

    for (auto info : infos) {
        if(blacklist.indexOf(info->getId()) != -1)
        {
            continue;
        }
        KeyFile keyFile(',');
        keyFile.loadFile(info->getPath()+"/index.theme");
        info->setName(keyFile.getLocaleStr("Icon Theme","Name"));
        info->setComment(keyFile.getLocaleStr("Icon Theme","Comment"));
        iconThemes.push_back(info);
    }
}

void Subthemes::refreshCursorThemes()
{
    cursorThemes = getThemes(themeApi->listCursorTheme());
    for(auto &&info:cursorThemes) {
        KeyFile keyFile(',');
        keyFile.loadFile(info->getPath()+"/cursor.theme");
        info->setName(keyFile.getLocaleStr("Icon Theme","Name"));
        info->setComment(keyFile.getLocaleStr("Icon Theme","Comment"));
    }
}

void Subthemes::refreshGlobalThemes()
{
    globalThemes.clear();
    QVector<QSharedPointer<Theme>> infos = getThemes(themeApi->listGlobalTheme());

    for (auto &&info : infos) {
        KeyFile keyFile(',');
        keyFile.loadFile(info->getPath()+"/index.theme");
        info->setName(keyFile.getLocaleStr("Deepin Theme","Name"));
        info->setComment(keyFile.getLocaleStr("Deepin Theme","Comment"));
        QStringList example =keyFile.getStrList("Deepin Theme","Example");

        for (QList<QString>::iterator i = example.begin(); i != example.end(); ++i) {
            QFileInfo file(*i);
            if (file.isRelative()) {
                QDir themefile(info->getPath());
                file.setFile(themefile, *i);
                *i = file.absoluteFilePath();
            }
        }
        info->setExample(example.join(','));
        info->setHasDark(keyFile.containKey("Deepin Theme","DarkTheme"));
        globalThemes.push_back(info);
    }
}

QVector<QSharedPointer<Theme>> Subthemes::listGtkThemes()
{
    if(gtkThemes.empty()) {
        refreshGtkThemes();
    }

    return gtkThemes;
}

QVector<QSharedPointer<Theme>> Subthemes::listIconThemes()
{
    if(iconThemes.empty()) {
        refreshIconThemes();
    }

    return iconThemes;
}

QVector<QSharedPointer<Theme>> Subthemes::listCursorThemes()
{
    if(cursorThemes.empty()) {
        refreshCursorThemes();
    }

    return cursorThemes;
}

QVector<QSharedPointer<Theme> > Subthemes::listGlobalThemes()
{
    if(globalThemes.empty()) {
        refreshGlobalThemes();
    }

    return globalThemes;
}

bool Subthemes::deleteGtkTheme(const QString& name)
{
    QVector<QSharedPointer<Theme>>::Iterator iter = gtkThemes.begin();
    while(iter != gtkThemes.end()){
        if((*iter)->getId() == name)
        {
            (*iter)->Delete();
            iter = gtkThemes.erase(iter);
            return true;
        }else {
            iter++;
        }
    }

    return false;
}

bool Subthemes::deleteIconTheme(const QString& name)
{
    QVector<QSharedPointer<Theme>>::Iterator iter = iconThemes.begin();
    while(iter != iconThemes.end()){
        if((*iter)->getId() == name)
        {
            (*iter)->Delete();
            iter = iconThemes.erase(iter);
            return true;
        }else {
            iter++;
        }
    }

    return false;
}

bool Subthemes::deleteCursorTheme(const QString& name){
    QVector<QSharedPointer<Theme>>::Iterator iter = cursorThemes.begin();
    while(iter != cursorThemes.end()){
        if((*iter)->getId() == name)
        {
            (*iter)->Delete();
            iter = cursorThemes.erase(iter);
            return true;
        }else {
            iter++;
        }
    }

    return false;
}

bool Subthemes::isGlobalTheme(QString id)
{
    for(auto iter : globalThemes)
    {
        if(iter->getId() == id)
        {
            return true;
        }
    }
    return false;
}

bool Subthemes::isGtkTheme(QString id)
{
    for(auto iter : gtkThemes)
    {
        if(iter->getId() == id)
        {
            return true;
        }
    }
    return false;
}

bool Subthemes::isIconTheme(QString id)
{
    for(auto iter : iconThemes)
    {
        if(iter->getId() == id)
        {
            return true;
        }
    }
    return false;
}

bool Subthemes::isCursorTheme(QString id)
{
    for(auto iter : cursorThemes)
    {
        if(iter->getId() == id)
        {
            return true;
        }
    }
    return false;
}

bool Subthemes::setGlobalTheme(QString id)
{
    return themeApi->setGlobalTheme(id);
}

bool Subthemes::setGtkTheme(QString id)
{
    return themeApi->setGtkTheme(id);
}

bool Subthemes::setIconTheme(QString id)
{
    return themeApi->setIconTheme(id);
}

bool Subthemes::setCursorTheme(QString id)
{
    return  themeApi->setCursorTheme(id);
}

QString Subthemes::getGlobalThumbnail(QString id)
{
    QSharedPointer<Theme> theme;
    for(auto iter: globalThemes)
    {
        if(iter->getId()==id)
        {
            theme = iter;
        }
    }

    if(!theme)
    {
        return "";
    }

    QString path = theme->getPath()+"/index.theme";
    return getGlobal(id,path);
}

QString Subthemes::getGtkThumbnail(QString id)
{
    QSharedPointer<Theme> theme;
    for(auto iter: gtkThemes)
    {
        if(iter->getId()==id)
        {
            theme = iter;
        }
    }

    if(!theme)
    {
        return "";
    }

    QString path = theme->getPath()+"/index.theme";
    return getGtk(id, path);
}

QString Subthemes::getIconThumbnail(QString id)
{
    QSharedPointer<Theme> theme;
    for(auto iter: iconThemes)
    {
        if(iter->getId()==id)
        {
            theme = iter;
        }
    }

    if(!theme)
    {
        return "";
    }

    QString path = theme->getPath()+"/index.theme";
    return getIcon(id,path);
}

QString Subthemes::getCursorThumbnail(QString id)
{
    QSharedPointer<Theme> theme;
    for(auto iter: cursorThemes)
    {
        if(iter->getId()==id)
        {
            theme = iter;
        }
    }

    if(!theme)
    {
        return "";
    }

    QString path = theme->getPath();
    return getCursor(id, path);
}

QVector<QSharedPointer<Theme>> Subthemes::getThemes(QVector<QString> files)
{
    QVector<QSharedPointer<Theme>> infos;
    for (auto file : files) {

        QSharedPointer<Theme> theme(new Theme(file));
        infos.push_back(theme);
    }

    return infos;
}

bool Subthemes::isDeletable(QString file)
{
    QString home = utils::GetUserHomeDir();

    if(file.indexOf(home)!=-1){
        return true;
    }

    return false;
}

bool Subthemes::isItemInList(QString item, QVector<QString> lists)
{
    for(auto list : lists) {
        if(item == list) {
            return true;
        }
    }

    return false;
}

QString Subthemes::getBasePath(QString filename)
{
    int index = filename.lastIndexOf("/");

    return filename.mid(0,index);
}

QMap<QString,QString>& Subthemes::getGtkThumbnailMap()
{
    return gtkThumbnailMap;
}

void Subthemes::init()
{
    switch (initStatus) {
    case initGtkThemes:
        listGtkThemes();
        break;
    case initGlobalThemes:
        listGlobalThemes();
        break;
    case initCursorThemes:
        listCursorThemes();
        for (auto &&theme : cursorThemes) {
            thumbnailThemes << theme->getId();
        }
        break;
    case initCursorThumbnail:
        if (!thumbnailThemes.isEmpty()) {
            getCursorThumbnail(thumbnailThemes.takeFirst());
            return;
        }
        break;
    case initIconThemes:
        listIconThemes();
        for (auto &&theme : iconThemes) {
            thumbnailThemes << theme->getId();
        }
        break;
    case initIconThumbnail:
        if (!thumbnailThemes.isEmpty()) {
            getIconThumbnail(thumbnailThemes.takeFirst());
            return;
        }
        break;
    case initFinished:
    default:
        timer->stop();
        timer->deleteLater();
        timer = nullptr;
        return;
        break;
    }
    initStatus++;
}
