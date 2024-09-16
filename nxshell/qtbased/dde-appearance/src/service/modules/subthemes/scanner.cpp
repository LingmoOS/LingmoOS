// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scanner.h"

#include "../api/utils.h"
#include "../common/commondefine.h"

#include <QDebug>

Scanner::Scanner(QObject *parent)
    :QObject (parent)
{

}

Scanner::~Scanner()
{

}

bool Scanner::isHidden(QString file, QString ty)
{
    KeyFile keyFile;
    keyFile.loadFile(file);

    bool hidden = false;

    if(ty == TYPEGTK) {
        hidden = keyFile.getBool("Desktop Entry","Hidden");
    } else if (ty == TYPEICON || ty == TYPECURSOR) {
        hidden = keyFile.getBool("Icon Theme","Hidden");
    } else if (ty == TYPEGLOBALTHEME) {
        hidden = keyFile.getBool("Deepin Theme","Hidden");
    }

    return hidden;
}

QString Scanner::query(QString uri)
{
    QString path = utils::deCodeURI(uri);
    QString mime = queryThemeMime(path);
    if (!mime.isEmpty()) {
        return mime;
    }

    return doQueryFile(path);
}

QString Scanner::queryThemeMime(QString file)
{
    if (gtkTheme(file)) {
        return MIMETYPEGTK;
    } else if (iconTheme(file)) {
        return MIMETYPEICON;
    } else if (cursorTheme(file)) {
        return MIMETYPECURSOR;
    } else if (globalTheme(file)) {
        return MIMETYPEGLOBAL;
    }
    return "";
}

QString Scanner::doQueryFile(QString file)
{
    GError **err = nullptr;
    if (!utils::isFileExists(file)) {
        return "";
    }

    GFile *g_file = g_file_new_for_path(file.toLatin1().data());
    const QString attributes = "standard::content-type";

    GFileInfo *fileinfo = g_file_query_info(g_file, attributes.toLatin1().data(), GFileQueryInfoFlags(G_FILE_QUERY_INFO_NONE), nullptr ,err);
    if(err != nullptr){
        return "";
    }

    const char *attribute = g_file_info_get_attribute_string(fileinfo, attributes.toLatin1().data());
    QString attributeString(attribute);

    return attributeString;
}

bool Scanner::globalTheme(QString file)
{
    KeyFile keyfile;
    keyfile.loadFile(file);
    if(!keyfile.loadFile(file)) {
        return false;
    }

    if(keyfile.getStr("Deepin Theme", "DefaultTheme").isEmpty()) {
        return false;
    }

    return true;
}

bool Scanner::gtkTheme(QString file)
{
    return utils::isFilesInDir({"gtk-2.0","gtk-3.0","metacity-1"}, file.left(file.lastIndexOf("/")));
}

bool Scanner::iconTheme(QString file)
{
    KeyFile keyfile;
    keyfile.loadFile(file);
    if(!keyfile.loadFile(file)) {
        return false;
    }

    if(keyfile.getStr("Icon Theme","Directories").isEmpty()) {
        return false;
    }

    return true;
}

bool Scanner::cursorTheme(QString file)
{
    QString dir = file.left(file.lastIndexOf("/"));
    QString tmp = dir + "/cursors" + "/left_ptr";
    QString mime = doQueryFile(tmp);

    if(mime.isEmpty() || mime != MIMETYPEXCURSOR) {
        return false;
    }

    return true;
}

bool Scanner::isGlobalTheme(QString uri)
{
    if(uri.size() == 0)
        return false;

    QString ty = query(uri);

    return ty == MIMETYPEGLOBAL;
}

bool Scanner::isGtkTheme(QString uri)
{
    if(uri.size() == 0)
        return false;

    QString ty = query(uri);

    return ty == MIMETYPEGTK;
}

bool Scanner::isIconTheme(QString uri)
{
    if(uri.size() == 0)
        return false;

    QString ty = query(uri);

    return ty == MIMETYPEICON;
}

bool Scanner::isCursorTheme(QString uri)
{
    if(uri.size() == 0)
        return false;

    QString ty = query(uri);

    return ty == MIMETYPECURSOR;
}

QVector<QString> Scanner::listSubDir(QString path)
{
    QVector<QString> subDirs;

    if(!utils::isDir(path)) {
        return subDirs;
    }

    QDir dir(path);
    QFileInfoList filenames = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);

    for(auto filename : filenames) {
        subDirs.push_back(filename.fileName());
    }

    return subDirs;
}

QVector<QString> Scanner::doListTheme(QString uri, QString ty, Fn fn)
{
    QString path = utils::deCodeURI(uri);

    QVector<QString> subDirs = listSubDir(path);
    if(!subDirs.size()) {
        qInfo() << "Path:" << path << "subDirs is empty";
    }

    QVector<QString> themes;
    for(auto subDir : subDirs) {
        QString tmp;
        if(ty == TYPECURSOR) {
            tmp = path + "/" +subDir + "/cursor.theme";
        } else {
            tmp = path + "/" + subDir + "/index.theme";
        }

        if(!fn(tmp) || isHidden(tmp, ty))
            continue;

        themes.push_back(tmp);
    }

    return themes;
}

QVector<QString> Scanner::listGlobalTheme(QString uri)
{
    Fn fn = std::bind(&Scanner::isGlobalTheme, this,std::placeholders::_1);
    return doListTheme(uri, TYPEGLOBALTHEME, fn);
}

QVector<QString> Scanner::listGtkTheme(QString uri)
{
    Fn fn = std::bind(&Scanner::isGtkTheme, this,std::placeholders::_1);
    return doListTheme(uri, TYPEGTK, fn);
}

QVector<QString> Scanner::listIconTheme(QString uri)
{
    Fn fn = std::bind(&Scanner::isIconTheme, this,std::placeholders::_1);
    return doListTheme(uri, TYPEICON, fn);
}

QVector<QString> Scanner::listCursorTheme(QString uri)
{
    Fn fn = std::bind(&Scanner::isCursorTheme, this,std::placeholders::_1);
    return doListTheme(uri, TYPECURSOR, fn);
}
