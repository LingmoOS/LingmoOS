// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmxmlwrapper.h"
#include "globaldef.h"

const QString FontConfigFileDir = QDir::homePath() + "/.config/fontconfig/conf.d/";
QString DFMXmlWrapper::m_fontConfigFilePath = FontConfigFileDir + FTM_REJECT_FONT_CONF_FILENAME;

DFMXmlWrapper::DFMXmlWrapper()
{
}


///*************************************************************************
// <Function>      createXmlFile
// <Description>   新建xml文件
// <Author>        null
// <Input>
//    <param1>     fileName        Description: 文件名
//    <param2>     rootName        Description:根节点名
//    <param3>     version         Description:xml版本
//    <param4>     encoding        Description:xml编码（字符集）
// <Return>        bool            Description:表示新建结果
// <Note>          null
//*************************************************************************/
//bool DFMXmlWrapper::createXmlFile(const QString &fileName,
//                                  const QString &rootName,
//                                  const QString &version,
//                                  const QString &encoding,
//                                  const QString &standalone)
//{
//    QFile file(fileName);

//    // 只写方式打开，并清空以前的信息
//    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
//        return false;
//    }

//    QDomDocument doc;
//    //添加处理指令（声明）
//    QDomProcessingInstruction instruction;
//    QString data;
//    data = "version=\"" + version + "\" encoding=\"" + encoding + "\" standalone=\"" + standalone + "\"";
//    instruction = doc.createProcessingInstruction("xml", data);
//    doc.appendChild(instruction);
//    QDomElement root = doc.createElement(rootName);
//    doc.appendChild(root); //添加根元素

//    QTextStream out(&file);
//    // 将文档保存到文件，4为子元素缩进字符数
//    doc.save(out, 4);
//    file.close();
//    return true;
//}

/*************************************************************************
 <Function>      createFontConfigFile
 <Description>   新建fontconfig配置文件
 <Author>        null
 <Input>
    <param1>     xmlFilePath         Description:文件路径
 <Return>        bool                Description:新建文件的结果
 <Note>          null
*************************************************************************/
bool DFMXmlWrapper::createFontConfigFile(const QString &xmlFilePath)
{
    QFile file(xmlFilePath);
    if (file.exists()) {
        qDebug() << "file is already exist!";
        return true;
    }

    if (!QFile::exists(FontConfigFileDir)) {
        QDir dir(FontConfigFileDir);
        dir.mkpath(FontConfigFileDir);
    }

    if (!file.open(QFile::WriteOnly | QFile::Text)) { // 只写模式打开文件
        qDebug() << QString("Cannot write file %1(%2).").arg(xmlFilePath).arg(file.errorString());
        return false;
    }

    QXmlStreamWriter writer(&file);
    // 自动格式化
    writer.setAutoFormatting(true);
    // 开始文档（XML 声明）
    writer.writeStartDocument("1.0");
    // DTD
    writer.writeDTD(QString::fromLocal8Bit("<!DOCTYPE fontconfig SYSTEM \"fonts.dtd\">"));

    writer.writeStartElement("fontconfig");  // 开始根元素 <fontconfig>
    writer.writeStartElement("selectfont");  // 开始子元素 <selectfont>
    writer.writeStartElement("rejectfont");  // 开始子元素 <rejectfont>
    writer.writeStartElement("pattern");
    writer.writeStartElement("patelt");
    writer.writeAttribute("name", "file");
    writer.writeTextElement("string", "[place your file path here!]");
    writer.writeEndElement();  // 结束子元素 </string>
    writer.writeEndElement();  // 结束子元素 </patelt>
    writer.writeEndElement();  // 结束子元素 </pattern>
    writer.writeEndElement();  // 结束子元素 </rejectfont>
    writer.writeEndElement();  // 结束子元素 </selectfont>
    writer.writeEndElement();  // 结束根元素 </fontconfig>
    writer.writeEndDocument();

    file.close();

    return true;
}

///*************************************************************************
// <Function>      deleteXmlFile
// <Description>   删除文件
// <Author>        null
// <Input>
//    <param1>     fileName            Description:文件名
// <Return>        bool                Description: 是否成功删除文件
// <Note>          null
//*************************************************************************/
//bool DFMXmlWrapper::deleteXmlFile(const QString &fileName)
//{
//    if (fileName.isEmpty()) {
//        return false;
//    }

//    QFile file(fileName);
//    if (!file.setPermissions(QFile::WriteOwner)) // 修改文件属性
//        return false;
//    if (!QFile::remove(fileName)) // 删除文件
//        return false;

//    return true;
//}

/*************************************************************************
 <Function>      getNodeByName
 <Description>   根据节点名获取节点元素
 <Author>        null
 <Input>
    <param1>     rootEle         Description:根元素
    <param2>     nodeName        Description:节点名
    <param3>     node            Description:节点
 <Return>        bool            Description:查询的结果
 <Note>          null
*************************************************************************/
bool DFMXmlWrapper::getNodeByName(QDomElement &rootEle,
                                  const QString &nodeName,
                                  QDomElement &node)
{
    if (nodeName == rootEle.tagName()) { // 若为根节点，则返回
        node = rootEle;
        return true;
    }

    QDomElement ele = rootEle;

    for (ele = rootEle.firstChildElement(); ele.isElement(); ele = ele.nextSiblingElement()) {
        //递归处理子节点，获取节点
        if (getNodeByName(ele, nodeName, node)) {
            return true;
        }
    }
    return false;
}

/*************************************************************************
 <Function>      addNodesWithText
 <Description>   增加节点
 <Author>        null
 <Input>
    <param1>     fileName             Description:文件名
    <param2>     parentNodeName       Description:父节点名
    <param3>     nodeNameList         Description:null
    <param4>     nodeAttributeList    Description:节点的属性/值map列表
    <param5>     lastNodeText         Description:用于填充在最后一个节点的文本
 <Return>        bool                 Description:节点是否成功新建
 <Note>          比如父节点名叫"parentNode", nodeMapList包含"nodeA", "nodeB", "nodeC"三个节点)，
 　　　           则节点全部加入后变成：
                  <parentNode>
                     <nodeA>
                       <nodeB property="value">
                          <nodeC>
                             nodeText
                          </nodeC>
                       </nodeB>
                     </nodeA>
                  </parentNode>
*************************************************************************/
bool DFMXmlWrapper::addNodesWithText(const QString &fileName,
                                     const QString &parentNodeName,
                                     const QStringList &nodeNameList,
                                     const QList<QSTRING_MAP> &nodeAttributeList,
                                     const QString &lastNodeText)
{
    if (fileName.isEmpty()) { // 文件名为空
        return false;
    }

    // 新建QDomDocument类对象，它代表一个XML文档
    QDomDocument doc;
    // 建立指向“fileName”文件的QFile对象
    QFile file(fileName);
    // 以只读方式打开
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    // 将文件内容读到doc中
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }

    file.close();

    QDomElement rootEle = doc.documentElement();
    QDomElement parentNode;
    getNodeByName(rootEle, parentNodeName, parentNode);

    QDomElement currParentNode = parentNode;
    for (int i = 0; i < nodeNameList.size(); i++) {

        QString nodeName = nodeNameList.at(i);
        QSTRING_MAP attributeMap = nodeAttributeList.at(i);

        // 添加元素
        QDomElement childEle = doc.createElement(nodeName);
        currParentNode.appendChild(childEle);
        currParentNode = childEle;

        if (!attributeMap.empty()) {
            QString attribute = attributeMap.keys().first();
            QString value = attributeMap.value(attribute);

            childEle.setAttribute(attribute, value);
        }

        if (nodeNameList.size() - 1 == i) {
            QDomText text;
            text = doc.createTextNode(lastNodeText);
            childEle.appendChild(text);
        }
    }

    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        return false;
    }

    //输出到文件
    QTextStream out(&file);
    // 将文档保存到文件，4为子元素缩进字符数
    doc.save(out, 4);
    file.close();

    return true;
}

bool DFMXmlWrapper::addNodesWithTextList(const QString &fileName, const QString &parentNodeName, const QStringList &nodeNameList,
                                         const QList<QSTRING_MAP> &nodeAttributeList, const QStringList &lastNodeTextList)
{
    if (fileName.isEmpty()) { // 文件名为空
        return false;
    }

    // 新建QDomDocument类对象，它代表一个XML文档
    QDomDocument doc;
    // 建立指向“fileName”文件的QFile对象
    QFile file(fileName);
    // 以只读方式打开
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    // 将文件内容读到doc中
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }

    file.close();

    QDomElement rootEle = doc.documentElement();
    QDomElement parentNode;
    if (!getNodeByName(rootEle, parentNodeName, parentNode)) {
        return false;
    }

    for (const QString &lastNodeText : lastNodeTextList) {
        QDomElement currParentNode = parentNode;
        for (int i = 0; i < nodeNameList.size(); i++) {

            QString nodeName = nodeNameList.at(i);
            QSTRING_MAP attributeMap = nodeAttributeList.at(i);

            // 添加元素
            QDomElement childEle = doc.createElement(nodeName);
            currParentNode.appendChild(childEle);
            currParentNode = childEle;

            if (!attributeMap.empty()) {
                QString attribute = attributeMap.keys().first();
                QString value = attributeMap.value(attribute);

                childEle.setAttribute(attribute, value);
            }

            if (nodeNameList.size() - 1 == i) {
                QDomText text;
                text = doc.createTextNode(lastNodeText);
                childEle.appendChild(text);
            }
        }
    }

    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        return false;
    }


    //输出到文件
    QTextStream out(&file);
    // 将文档保存到文件，4为子元素缩进字符数
    doc.save(out, 4);
    file.close();

    return true;
}

//bool DFMXmlWrapper::addPatternNodesWithText(const QString &fileName,
//                                            const QString &parentNodeName,
//                                            const QString &lastNodeText)
//{
//    QStringList nodeNameList;
//    nodeNameList << "pattern" << "patelt" << "string";
//    QList<QMap<QString, QString>> attributeList;
//    QMap<QString, QString> map1;
//    QMap<QString, QString> map2;
//    map2.insert("name", "file");
//    QMap<QString, QString> map3;
//    attributeList.push_back(map1);
//    attributeList.push_back(map2);
//    attributeList.push_back(map3);

//    bool ret = DFMXmlWrapper::addNodesWithText(fileName, parentNodeName, nodeNameList, attributeList, lastNodeText);
//    return ret;
//}

bool DFMXmlWrapper::addPatternNodesWithTextList(const QString &fileName, const QString &parentNodeName, const QStringList &lastNodeTextList)
{
    QStringList nodeNameList;
    nodeNameList << "pattern" << "patelt" << "string";
    QList<QMap<QString, QString>> attributeList;
    QMap<QString, QString> map1;
    QMap<QString, QString> map2;
    map2.insert("name", "file");
    QMap<QString, QString> map3;
    attributeList.push_back(map1);
    attributeList.push_back(map2);
    attributeList.push_back(map3);

    bool ret = DFMXmlWrapper::addNodesWithTextList(fileName, parentNodeName, nodeNameList, attributeList, lastNodeTextList);
    return ret;
}

///*************************************************************************
// <Function>      deleteNodeWithText
// <Description>   删除节点
// <Author>        null
// <Input>
//    <param1>     fileName            Description:文件名
//    <param2>     nodeName            Description:节点名
//    <param3>     nodeText            Description:节点包含的文本
// <Return>        bool                Description:是否成功删除
// <Note>          null
//*************************************************************************/
//bool DFMXmlWrapper::deleteNodeWithText(const QString &fileName,
//                                       const QString &nodeName,
//                                       const QString &nodeText)
//{
//    if (fileName.isEmpty()) {
//        return false;
//    }

//    QFile file(fileName);
//    //打开文件
//    if (!file.open(QFile::ReadOnly))
//        return false;

//    QDomDocument doc;
//    if (!doc.setContent(&file)) {
//        file.close();
//        return false;
//    }
//    file.close();

//    QDomElement rootEle = doc.documentElement();
//    QDomElement nodeEle;
//    getNodeByName(rootEle, nodeName, nodeEle);

//    // 假如是根节点
//    if (rootEle == nodeEle) {
//        return false;
//    }

//    //根据节点包含的文本匹配到的节点，删除节点及其元素
//    QDomNode removeNode;
//    QDomNodeList list = nodeEle.parentNode().childNodes();
//    for (int i = 0; i < list.count(); i++) {
//        QDomNode node = list.at(i);
//        if (nodeEle.isElement()) {
//            if (node.toElement().text() == nodeText) {
//                removeNode = node;
//                break;
//            }
//        }
//    }

//    if (removeNode.isElement()) {
//        QDomNode parentNode = removeNode.parentNode();
//        if (parentNode.isElement()) {
//            parentNode.removeChild(removeNode);
//        } else {
//            qDebug() << "delete node failed!" << endl;
//            return false;
//        }
//    } else {
//        qDebug() << "delete node failed!" << endl;
//    }

//    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
//        return false;
//    }

//    //输出到文件
//    QTextStream out_stream(&file);
//    doc.save(out_stream, 4); //缩进4格
//    file.close();

//    return true;
//}

/*************************************************************************
 <Function>      deleteNodeWithTextList
 <Description>   删除节点v
 <Author>        null
 <Input>
    <param1>     fileName                Description:null
    <param2>     nodeName                Description:null
    <param3>     nodeTextList            Description:所有节点包含的文本
 <Return>        null                     Description:null
 <Note>          null
*************************************************************************/
bool DFMXmlWrapper::deleteNodeWithTextList(const QString &fileName, const QString &nodeName, const QStringList &nodeTextList)
{
    if (fileName.isEmpty()) {
        return false;
    }

    QFile file(fileName);
    //打开文件
    if (!file.open(QFile::ReadOnly))
        return false;

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();

    QDomElement rootEle = doc.documentElement();
    QDomElement nodeEle;
    if (!getNodeByName(rootEle, nodeName, nodeEle)) {
        return false;
    }

    // 假如是根节点
    if (rootEle == nodeEle) {
        return false;
    }

    //根据节点包含的文本匹配到的节点，删除节点及其元素
    QList<QDomNode> removeNodeList;
    QDomNodeList list = nodeEle.parentNode().childNodes();
    for (int i = 0; i < list.count(); i++) {
        QDomNode node = list.at(i);
        if (nodeEle.isElement()) {
            if (nodeTextList.contains(node.toElement().text())) {
                removeNodeList << node;
            }
        }
    }

    for (QDomNode &removeNode : removeNodeList) {
        if (removeNode.isElement()) {
            QDomNode parentNode = removeNode.parentNode();
            if (parentNode.isElement()) {
                parentNode.removeChild(removeNode);
            } else {
                qDebug() << "delete node failed!" << endl;
                return false;
            }
        } else {
            qDebug() << "delete node failed!" << endl;
        }
    }

    if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
        return false;
    }

    //输出到文件
    QTextStream out_stream(&file);
    doc.save(out_stream, 4); //缩进4格
    file.close();

    return true;
}

/*************************************************************************
 <Function>      queryAllChildNodes_Text
 <Description>   查询所有子节点文本
 <Author>        null
 <Input>
    <param1>     fileName            Description:文件名
    <param2>     nodeName            Description:节点名
    <param3>     textList            Description:子节点文本
 <Return>        bool                Description:查询结果
 <Note>          null
*************************************************************************/
bool DFMXmlWrapper::queryAllChildNodes_Text(const QString &fileName,
                                            const QString &nodeName,
                                            QStringList &textList)
{
    if (fileName.isEmpty()) {
        return false;
    }

    // 新建QDomDocument类对象，它代表一个XML文档
    QDomDocument doc;
    // 建立指向“fileName”文件的QFile对象
    QFile file(fileName);
    // 以只读方式打开
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    // 将文件内容读到doc中
    if (!doc.setContent(&file)) {
        file.close();
        file.remove();
        return false;
    }
    file.close();

    //根元素
    QDomElement rootEle = doc.documentElement();
    QDomElement node;
    if (!getNodeByName(rootEle, nodeName, node)) {
        return false;
    }

    QDomNodeList list = node.childNodes();
    for (int i = 0; i < list.count(); i++) {
        QDomNode n = list.at(i);
        if (node.isElement()) {
            QString path = n.toElement().text();
            if (QFile::exists(path))
                textList << path;
        }
    }

    return true;
}

/*************************************************************************
 <Function>      getFontConfigDisableFontPathList
 <Description>   查询所有禁用字体文件路径列表
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
QStringList DFMXmlWrapper::getFontConfigDisableFontPathList()
{
    QStringList strDisableFontPathList;
    QString fontConfigFilePath = DFMXmlWrapper::m_fontConfigFilePath;
    DFMXmlWrapper::queryAllChildNodes_Text(fontConfigFilePath, "rejectfont", strDisableFontPathList);

    return strDisableFontPathList;
}


