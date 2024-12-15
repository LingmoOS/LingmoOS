// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "theme.h"

#include "modules/api/utils.h"

#include <QDir>
#include <QDebug>

Theme::Theme(QString fileName)
    : deletable(false)
    , m_hasDark(false)
{
    QStringList nodes = fileName.split("/");
    if(nodes.size()>=2)
    {
        this->id= nodes[nodes.size()-2];
    }

    this->filePath = fileName.left(fileName.lastIndexOf("/"));

    QString home = utils::GetUserHomeDir();
    if(fileName.indexOf(home)!=-1){
        this->deletable=true;
    }else {
        this->deletable=false;
    }
}



bool Theme::Delete()
{
    if (!this->deletable) {
        qInfo() << "permission denied";
    }

    QDir dir(filePath.left(filePath.lastIndexOf("/")));
    if (!dir.exists()) {
        qInfo() << "not dir";
        return false;
    }

    return  dir.removeRecursively();
}

Theme::~Theme()
{

}

QString Theme::getId(){
    return id;
}

QString Theme::getPath()
{
    return filePath;
}

bool Theme::getDeleteable()
{
    return deletable;
}

QString Theme::name() const
{
    return m_name;
}

void Theme::setName(const QString &name)
{
    m_name = name;
}

QString Theme::comment() const
{
    return m_comment;
}

void Theme::setComment(const QString &comment)
{
    m_comment = comment;
}

QString Theme::example() const
{
    return m_example;
}

void Theme::setExample(const QString &example)
{
    m_example = example;
}

bool Theme::hasDark() const
{
    return m_hasDark;
}

void Theme::setHasDark(bool hasDark)
{
    m_hasDark = hasDark;
}
