/*
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: baijunjie <baijunjie@kylinos.cn>
 *
 */

#include "uof-extractor.h"
#include "quazip5/quazip.h"
#include "quazip5/quazipfile.h"
#include <QDomDocument>
#include <QXmlStreamReader>
#include <QFileInfo>
#include <QString>
#include <QQueue>
#include <QDebug>

#define MAX_CONTENT_LENGTH 20480000

using namespace LingmoUIFileMetadata;

const QStringList supportedMimeTypes = {
        QStringLiteral("application/wps-office.uof"),
        QStringLiteral("application/wps-office.uop"),
        QStringLiteral("application/wps-office.uos"),
        QStringLiteral("application/wps-office.uot"),
        QStringLiteral("application/xml")
};

UofExtractor::UofExtractor(QObject *parent) : ExtractorPlugin(parent) {

}

void UofExtractor::extract(ExtractionResult *result) {
    QFileInfo info(result->inputUrl());
    if (!info.exists() || info.isDir()) {
        return;
    }

    result->addType(Type::Document);

    QString suffix = info.suffix();
    if (suffix == "uof") {
        this->parseUofFile(result);
    } else {
        //参考标准 GJB/Z 165-2012 https://www.doc88.com/p-9089133923912.html
        //解析UOF2.0文件的元数据
        QuaZip file(result->inputUrl());
        if (!file.open(QuaZip::mdUnzip)) {
            return;
        }
        QuaZipFile fileR;
        QXmlStreamReader reader;

        //parse meta info
        if ((result->inputFlags() & ExtractionResult::Flag::ExtractMetaData) && file.setCurrentFile("_meta/meta.xml")) {
            fileR.setZip(&file);
            if (fileR.open(QIODevice::ReadOnly)) {
                reader.setDevice(&fileR);
                while (!reader.atEnd()) {
                    if (reader.readNextStartElement()) {
                        if (reader.name().toString() == "作者_5204") {
                            result->add(Property::Author, reader.readElementText());
                            continue;
                        }
                        if (reader.name().toString() == "标题_5201") {
                            result->add(Property::Title, reader.readElementText());
                            continue;
                        }
                        if (reader.name().toString() == "主题_5202") {
                            result->add(Property::Subject, reader.readElementText());
                            continue;
                        }
                        if (reader.name().toString() == "摘要_5206") {
                            result->add(Property::Description, reader.readElementText());
                            continue;
                        }
                        if (reader.name().toString() == "创建日期_5207") {
                            result->add(Property::CreationDate, reader.readElementText());
                            continue;
                        }
                        if (reader.name().toString() == "创建应用程序_520A") {
                            result->add(Property::Generator, reader.readElementText());
                            continue;
                        }
                        if (reader.name().toString() == "页数_5215") {
                            result->add(Property::PageCount, reader.readElementText());
                            continue;
                        }
                        if (reader.name().toString() == "字数_5216") {
                            result->add(Property::WordCount, reader.readElementText());
                        }
                    }
                }
                fileR.close();
            }
        }

        //parse the content of the UOF 2.0 file
        if (!(result->inputFlags() & ExtractionResult::Flag::ExtractPlainText)) {
            return;
        }

        if (suffix == "uot" ||suffix == "uos") {
            if (file.setCurrentFile("content.xml")) {
                fileR.setZip(&file);
                if (!fileR.open(QIODevice::ReadOnly)) {
                    file.close();
                    return;
                }
                reader.setDevice(&fileR);

                QString textContent;
                while (!reader.atEnd()) {
                    if (reader.readNextStartElement() && reader.name().toString() == "文本串_415B") {
                        textContent.append(reader.readElementText());
                        if (textContent.length() >= MAX_CONTENT_LENGTH / 3) {
                            break;
                        }
                    }
                }
                fileR.close();
                file.close();
                result->append(textContent);
            }
        } else if (suffix == "uop") {
            parsePptOfUof2(result);
        }
    }

}

QStringList UofExtractor::mimetypes() const {
    return supportedMimeTypes;
}

/**
 * @brief 查找elem的子节点
 * @param elem 起始节点
 * @param names 名称链
 * @param nodes 查找到的全部结果
 */
void findNodes(const QDomElement &elem, QQueue<QString> &names, QList<QDomElement> &nodes)
{
    QString targetName = names.dequeue();
    QDomNode node = elem.firstChild();
    while (!node.isNull()) {
        QDomElement e = node.toElement();
        if (!e.isNull() && e.tagName() == targetName) {
            if (names.empty()) {
                nodes.append(e);

            } else {
                findNodes(e, names, nodes);
                break;
            }
        }
        node = node.nextSibling();
    }
}

void findNodesByAttr(const QDomElement &elem, QQueue <QString> &names, QList <QDomElement> &nodes, const QString &attr, const QStringList &values)
{
    findNodes(elem, names, nodes);

    QList<QDomElement>::iterator it = nodes.begin();
    while (it != nodes.end()) {
        if ((*it).hasAttribute(attr) && values.contains((*it).attribute(attr))) {
            it++;
        } else {
            it = nodes.erase(it);
        }
    }
}

bool findNodeText(const QDomElement &elem, QQueue<QString> &names, QString &content)
{
    QList<QDomElement> nodes;
    findNodes(elem, names, nodes);

    for (const auto &node : nodes) {
        content.append(node.text());
        if (content.length() >= MAX_CONTENT_LENGTH / 3) {
            return true;
        }
    }
    return false;
}

void findNodeAttr(const QDomElement &elem, QQueue<QString> &names, const QString &attr, QStringList &attrs)
{
    QList<QDomElement> nodes;
    findNodes(elem, names, nodes);

    for (const auto &node : nodes) {
        if (node.hasAttribute(attr)) {
            attrs.append(node.attribute(attr));
        }
    }
}

/**
 * 参考标准 GB/T 20916-2007
 * 参考文档 https://www.doc88.com/p-7458439230111.html
 */
void UofExtractor::parseUofFile(ExtractionResult *result) {
    QFile file(result->inputUrl());
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        return;
    }
    file.close();

    bool isPPT = false;
    QDomElement rootElem = doc.documentElement();
    QDomNode childNode = rootElem.firstChild();
    while (!childNode.isNull()) {
        QDomElement elem = childNode.toElement();
        if (!elem.isNull() && elem.tagName() == "uof:演示文稿") {
            isPPT = true;
        }
        if ((result->inputFlags() & ExtractionResult::Flag::ExtractMetaData) && elem.tagName() == "uof:元数据") {
            QDomNodeList list = elem.childNodes();
            for (int i = 0; i < list.size(); i++) {
                QDomNode childMetaNode = list.at(i);
                QString nodeText = childMetaNode.toElement().text();
                if (childMetaNode.nodeName() == "uof:作者") {
                    result->add(Property::Author, nodeText);
                    continue;
                }
                if (childMetaNode.nodeName() == "uof:创建应用程序") {
                    result->add(Property::Generator, nodeText);
                    continue;
                }
                if (childMetaNode.nodeName() == "uof::标题") {
                    result->add(Property::Title, nodeText);
                    continue;
                }
                if (childMetaNode.nodeName() == "uof:页数") {
                    result->add(Property::PageCount, nodeText);
                    continue;
                }
                if (childMetaNode.nodeName() == "uof:字数") {
                    result->add(Property::WordCount, nodeText);
                    continue;
                }
                if (childMetaNode.nodeName() == "uof:行数") {
                    result->add(Property::LineCount, nodeText);
                    continue;
                }
                if (childMetaNode.nodeName() == "uof:创建日期") {
                    result->add(Property::CreationDate, nodeText);
                    continue;
                }
                if (childMetaNode.nodeName() == "uof:关键字集") {
                    QDomNodeList keywordNodes = childNode.childNodes();
                    QStringList keywords;
                    for (int j = 0; j < keywordNodes.size(); j++) {
                        keywords.append(keywordNodes.at(i).toElement().text());
                    }
                    result->add(Property::Keywords, keywords);
                    continue;
                }
                if (childMetaNode.nodeName() == "uof:摘要") {
                    result->add(Property::Description, nodeText);
                }
            }
        }
        childNode = childNode.nextSibling();
    }

    if (!(result->inputFlags() & ExtractionResult::Flag::ExtractPlainText)) {
        return;
    }
    //单独处理ppt文档
    if (isPPT) {
        QList<QDomElement> nodes;
        QQueue<QString> names; //每个节点的名称
        names << "uof:演示文稿" << "演:主体" << "演:幻灯片集" << "演:幻灯片";

        findNodes(rootElem, names, nodes);

        if (nodes.empty()) {
            //TODO 在uof-ppt不存在锚点节点时，直接查找文本节点？
            return;
        }

        QStringList objs;
        QString content;
        //每一个 演:幻灯片 -> 锚点
        for (const auto &node : nodes) {
            names.clear();
            names << "uof:锚点";
            findNodeAttr(node, names, "uof:图形引用", objs);
        }

        nodes.clear();
        names.clear();
        names << "uof:对象集" << "图:图形";
        findNodesByAttr(rootElem, names, nodes, "图:标识符", objs);

        if (nodes.empty()) {
            return;
        }

        QList<QDomElement> paraNodes; //全部段落节点
        for (const auto &node : nodes) {
            names.clear();
            names << "图:文本内容" << "字:段落";
            findNodes(node, names, paraNodes);
        }

        nodes.clear();
        for (const auto &node : paraNodes) {
            names.clear();
            names << "字:句";
            findNodes(node, names, nodes); //全部段落下的全部句节点
        }

        for (const auto &node : nodes) {
            names.clear();
            names << "字:文本串";
            if (findNodeText(node, names, content)) {
                break;
            }
        }
        result->append(content);
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    QXmlStreamReader reader(& file);
    QString textContent;
    while (!reader.atEnd()) {
        //适用于文字处理与电子表格
        if (reader.readNextStartElement() && reader.name().toString() == "文本串") {
            textContent.append(reader.readElementText());
            if (textContent.length() >= MAX_CONTENT_LENGTH / 3) {
                break;
            }
        }
    }
    result->append(textContent);

    file.close();
}

bool loadZipFileToDoc(QuaZip &zipFile, QDomDocument &doc, const QString &fileName)
{
    if (!zipFile.isOpen() && !zipFile.open(QuaZip::mdUnzip)) {
        return false;
    }

    if (!zipFile.setCurrentFile(fileName)) {
        return false;
    }

    QuaZipFile file(&zipFile);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    doc.clear();
    if (!doc.setContent(&file)) {
        file.close();
        return false;
    }
    file.close();

    return true;
}

//ppt文档的内容存放在graphics.xml中，需要先解析content中的引用再解析graphics内容
void UofExtractor::parsePptOfUof2(ExtractionResult *result) {
    QuaZip zipFile(result->inputUrl());
    QDomDocument doc;

    if (!loadZipFileToDoc(zipFile, doc, "content.xml")) {
        return;
    }

    QDomElement rootElem = doc.documentElement();
    QList<QDomElement> nodes;
    QQueue<QString> names; //每个节点的名称
    names << "演:幻灯片集_6C0E" << "演:幻灯片_6C0F";
    findNodes(rootElem, names, nodes);

    if (nodes.empty()) {
        return;
    }

    QStringList attrs;
    for (const auto &node : nodes) {
        names.clear();
        names << "uof:锚点_C644";
        findNodeAttr(node, names, "图形引用_C62E", attrs);
    }

    if (attrs.empty()) {
        return;
    }

    if (!loadZipFileToDoc(zipFile, doc, "graphics.xml")) {
        return;
    }

    nodes.clear();
    names.clear();
    names << "图:图形_8062";
    rootElem = doc.documentElement();
    findNodesByAttr(rootElem, names, nodes, "标识符_804B", attrs);

    QList<QDomElement> nodes416B; //字:段落_416B
    for (const auto &node : nodes) {
        names.clear();
        names << "图:文本_803C" << "图:内容_8043" << "字:段落_416B";
        findNodes(node, names, nodes416B);
    }

    nodes.clear();
    for (const auto &node : nodes416B) {
        names.clear();
        names << "字:句_419D";
        findNodes(node, names, nodes); //所有的 字:句_419D
    }

    QString textContent;
    for (const auto &node : nodes) {
        names.clear();
        names << "字:文本串_415B";
        if (findNodeText(node, names, textContent)) {
            break;
        }
    }
    result->append(textContent);
}