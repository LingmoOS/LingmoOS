/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#include "custom_sound.h"

CustomSound::CustomSound()
{
}

CustomSound::~CustomSound()
{
    delete(doc);
}

bool CustomSound::createAudioFile()
{
    //打开或创建文件
    QString audioPath = QDir::homePath() + "/.config/customAudio.xml";
    if(!QFile::exists(audioPath)){
        QFile file(audioPath);
        if(!file.open(QFile::WriteOnly))
            return false;
        //写入xml头部
        QDomDocument doc;
        QDomProcessingInstruction instruction; //添加处理命令

        instruction = doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
        doc.appendChild(instruction);
        //添加根节点
        QDomElement root = doc.createElement("root");
        doc.appendChild(root);
        //添加子节点
        QDomElement node = doc.createElement("firstRun");
        QDomElement init = doc.createElement("init"); //创建子元素
        QDomText text = doc.createTextNode("true");
        init.appendChild(text);
        node.appendChild(init);
        root.appendChild(node);

        //输出到文件
        QTextStream out_stream(&file);
        doc.save(out_stream,4); //缩进4格
        file.close();
        return true;
    }
    return true;
}

//判断某个节点是否存在
bool CustomSound::isExist(QString nodeName)
{
    if (nodeName == "")
        return false;
    //打开文件
    QString errorStr;
    int errorLine;
    int errorCol;

    QString audioPath = QDir::homePath() + "/.config/customAudio.xml";
    QFile file(audioPath);

    if(!file.exists())
        createAudioFile();

    if(file.open(QFile::ReadOnly)){
        QDomDocument doc;
        if(doc.setContent(&file,true,&errorStr,&errorLine,&errorCol)){
            file.close();
            QDomElement root = doc.documentElement();
            QDomElement ele = root.firstChildElement();
            nodeName.remove(" ");
            nodeName.remove("/");
            nodeName.remove("(");
            nodeName.remove(")");
            nodeName.remove("[");
            nodeName.remove("]");
            nodeName.remove(";");
            if(nodeName != "" && nodeName.at(0)>='0' && nodeName.at(0)<='9'){
                nodeName = "Audio_"+nodeName;
            }
            while(!ele.isNull()) {
                if(ele.nodeName() == nodeName)
                    return true;
                ele = ele.nextSiblingElement();
            }
        }
        else
        {
            QFile::remove(audioPath);
            qDebug() << errorStr << "line: " << errorLine << "col: " << errorCol;
        }

        file.close();
    }

    return false;
}

//添加第一个子节点及其子元素
int CustomSound::addXmlNode(QString nodeName, bool initState)
{
    //打开文件
    QString audioPath = QDir::homePath() + "/.config/customAudio.xml";
    QFile file(audioPath);
    QDomDocument doc;//增加一个一级子节点以及元素
    if(file.open(QFile::ReadOnly)){
        if(doc.setContent(&file)){
            file.close();
        }
        else {
            file.close();
            return -1;
        }
    }
    else
        return -1;

    //添加新节点
    nodeName.remove(" ");
    nodeName.remove("/");
    nodeName.remove("(");
    nodeName.remove(")");
    nodeName.remove("[");
    nodeName.remove("]");
    nodeName.remove(";");
    if(nodeName != "" && nodeName.at(0)>='0' && nodeName.at(0)<='9'){
        nodeName = "Audio_"+nodeName;
    }
    QDomElement root=doc.documentElement();
    QDomElement node=doc.createElement(nodeName);

    QDomElement init=doc.createElement("init");
    QDomText text;
    if(initState)
        text = doc.createTextNode("true");
    else
        text = doc.createTextNode("false");
    init.appendChild(text);
    node.appendChild(init);
    root.appendChild(node);
    qDebug() << "addXmlNode" << nodeName ;
    //修改first-run状态
    QDomElement ele = root.firstChildElement();
    while(!ele.isNull()) {
        if(ele.nodeName() == "firstRun"){
            QString value = ele.firstChildElement().firstChild().nodeValue();
            if(value == "true")
                ele.firstChildElement().firstChild().setNodeValue("false");
        }
        ele = ele.nextSiblingElement();
    }

    if(file.open(QFile::WriteOnly|QFile::Truncate)) {
        //输出到文件
        QTextStream out_stream(&file);
        doc.save(out_stream,4); //缩进4格
        file.close();
    }

    return 0;
}

bool CustomSound::isFirstRun()
{

    QString audioPath = QDir::homePath() + "/.config/customAudio.xml";
    QFile file(audioPath);
    if(!file.exists()){
        createAudioFile();
    }

    if(file.open(QFile::ReadOnly)){
        QDomDocument doc;
        if(doc.setContent(&file)){
            file.close();
            QDomElement root = doc.documentElement();
            QDomElement ele = root.firstChildElement();

            qDebug()<<"===================ele.nodeName() :"<<ele.nodeName() ;
            if(ele.nodeName() == "first-run"){
                QString value = ele.firstChildElement().firstChild().nodeValue();
                if(value == "true") {
                    file.close();
                    return true;
                }
                else {
                    file.close();
                    return false;
                }
            }
        }
        file.close();
    }
    return false;
}
