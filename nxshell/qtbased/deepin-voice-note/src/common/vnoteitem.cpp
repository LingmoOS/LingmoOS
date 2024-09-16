// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vnoteitem.h"
#include "common/utils.h"

#include <DLog>
#include <DGuiApplicationHelper>

#include <QFile>
#include <QFileInfo>
#include <QRegExp>

//导出为html文件时的头部部分
static const QString htmlHead =
    "<!DOCTYPE html>"
    "<html lang=\"en\">"
    "<head>"
    "<meta charset=\"UTF-8\">"
    "<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "<title>Document</title>"
    "<style>"
    "body { font-family: 'Helvetica'; font-size: 14px; padding: 0px; line-height: 1.72; max-width: 780px; margin: 0 auto; word-wrap: break-word; }"
    ".demo { padding: 12px 0; border-radius: 10px; box-sizing: border-box; background-color: transparent !important; }"
    ".demo>div { display: inline-block; vertical-align: bottom; }"
    ".lf { margin: 2px 0 0 2px; position: absolute; }"
    ".icon { height: 21px; }"
    ".lr {float: right; margin: 0px 20px 0 0;}"
    ".li { background: rgba(0, 0, 0, 0.05); min-height: 30px; border-radius: 8px; margin-bottom: 10px; margin-top: 10px; font-weight: normal!important; font-style: normal!important; text-decoration: none!important; font-size: 14px;}"
    ".voicebtn { border-radius: 50%; width: 40px; height: 40px; background-color: #00a48a; margin: 0 20px 0 20px; overflow: hidden; overflow: hidden; box-shadow: 0px 4px 6px 0px #00a48a80; }"
    ".voicebtn.play {"
    "background-image: url(\"data:image/svg+xml;base64,PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGluZz0iVVRGLTgiPz4KPHN2ZyB3aWR0aD0iNDBweCIgaGVpZ2h0PSI0MHB4IiB2aWV3Qm94PSIwIDAgNDAgNDAiIHZlcnNpb249IjEuMSIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIiB4bWxuczp4bGluaz0iaHR0cDovL3d3dy53My5vcmcvMTk5OS94bGluayI+CiAgICA8IS0tIEdlbmVyYXRvcjogU2tldGNoIDU4ICg4NDY2MykgLSBodHRwczovL3NrZXRjaC5jb20gLS0+CiAgICA8dGl0bGU+cGxheTwvdGl0bGU+CiAgICA8ZGVzYz5DcmVhdGVkIHdpdGggU2tldGNoLjwvZGVzYz4KICAgIDxnIGlkPSLmtYXoibIiIHN0cm9rZT0ibm9uZSIgc3Ryb2tlLXdpZHRoPSIxIiBmaWxsPSJub25lIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiPgogICAgICAgIDxnIGlkPSLmjInpkq7nirbmgIEiIHRyYW5zZm9ybT0idHJhbnNsYXRlKC0zMTYuMDAwMDAwLCAtMTM4LjAwMDAwMCkiIGZpbGw9IiNGRkZGRkYiPgogICAgICAgICAgICA8ZyBpZD0icGxheV9ub3JtYWwiIHRyYW5zZm9ybT0idHJhbnNsYXRlKDMwNi4wMDAwMDAsIDEyOC4wMDAwMDApIj4KICAgICAgICAgICAgICAgIDxnIGlkPSJUcmlhbmdsZS0yIiB0cmFuc2Zvcm09InRyYW5zbGF0ZSgxMC4wMDAwMDAsIDEwLjAwMDAwMCkiPgogICAgICAgICAgICAgICAgICAgIDxwYXRoIGQ9Ik0yNC41NDUyMDIyLDE3Ljk4NDcwMTMgQzI2LjQ1MzIzNDMsMTkuMDk3NzIgMjYuNDU1Nzc2MywyMC45MDA3OTcyIDI0LjU0NTIwMjIsMjIuMDE1Mjk4NyBMMTkuNDU0Nzk3OCwyNC45ODQ3MDEzIEMxNy41NDY3NjU3LDI2LjA5NzcyIDE2LDI1LjIxMjY3MDEgMTYsMjMuMDA2ODk3NyBMMTYsMTYuOTkzMTAyMyBDMTYsMTQuNzg3NzcyOCAxNy41NDQyMjM3LDEzLjkwMDc5NzIgMTkuNDU0Nzk3OCwxNS4wMTUyOTg3IEwyNC41NDUyMDIyLDE3Ljk4NDcwMTMgWiIgaWQ9IlRyaWFuZ2xlIj48L3BhdGg+CiAgICAgICAgICAgICAgICA8L2c+CiAgICAgICAgICAgIDwvZz4KICAgICAgICA8L2c+CiAgICA8L2c+Cjwvc3ZnPg==\");"
    "background-size: contain;"
    "}"
    ".title { color: rgba(0, 26, 46, 1); font-size: 14px !important; line-height: 16px; }"
    ".minute { color: rgba(138, 161, 180, 1); font-size: 12px !important; line-height: 12px; margin-top: 5px; }"
    ".time { color: rgba(65, 77, 104, 1); font-size: 12px !important; line-height: 12px; }"
    ".title span { font-size: 14px !important; }"
    ".minute span { font-size: 12px !important; }"
    ".time span { font-size: 12px !important; }"
    ".padtop { padding-top: 6px; }"
    ".translate { line-height: 20px; font-size: 12px; }"
    ".translatePadding { padding: 5px 20px 10px 20px; }"
    ".translate>div { text-align: center; }"
    ".note-editable { padding: 20px 30px 0 30px; max-width: 720px; }"
    ".note-editable img { padding: 4px !important; max-width: 100% !important; }"
    "ol,"
    "ul { margin-bottom: 0px;  padding-left: 20px; }"
    "p { margin: 0 !important; word-wrap: break-word; }"
    "</style>"
    "</head>";

/**
 * @brief VNoteItem::VNoteItem
 */
VNoteItem::VNoteItem()
{
}

/**
 * @brief VNoteItem::isValid
 * @return true 可用
 */
bool VNoteItem::isValid()
{
    return (noteId > INVALID_ID
            && folderId > INVALID_ID)
               ? true
               : false;
}

/**
 * @brief VNoteItem::delNoteData
 */
void VNoteItem::delNoteData()
{
    //Clear note attachments
    for (auto it : datas.datas) {
        it->releaseSpecificData();
    }
}

/**
 * @brief VNoteItem::search
 * @param keyword 搜索关键字
 * @return true 记事项内容包含关键字
 */
bool VNoteItem::search(const QString &keyword)
{
    bool fContainKeyword = false;

    //If title contain keyword,don't
    //need search data anymore.
    if (noteTitle.contains(keyword, Qt::CaseInsensitive)) {
        fContainKeyword = true;
    } else {
        if (!htmlCode.isEmpty()) { //富文本内容查找
            QTextDocument doc;
            doc.setHtml(htmlCode);
            fContainKeyword = doc.toPlainText().contains(keyword, Qt::CaseInsensitive);
        } else {
            //Need search data blocks in note
            for (auto it : datas.datas) {
                if (it->blockText.contains(keyword, Qt::CaseInsensitive)) {
                    fContainKeyword = true;
                    break;
                }
            }
        }
    }

    return fContainKeyword;
}

//bool VNoteItem::makeMetaData()
//{
//    bool isMetaDataOk = false;

//    return isMetaDataOk;
//}

/**
 * @brief VNoteItem::setMetadata
 * @param meta 源数据
 */
void VNoteItem::setMetadata(const QVariant &meta)
{
    metaData = meta;
}

/**
 * @brief VNoteItem::setFolder
 * @param folder
 */
void VNoteItem::setFolder(VNoteFolder *folder)
{
    ownFolder = folder;
}

/**
 * @brief VNoteItem::folder
 * @return 记事本数据
 */
VNoteFolder *VNoteItem::folder() const
{
    return ownFolder;
}

/**
 * @brief VNoteItem::metaDataRef
 * @return 源数据
 */
QVariant &VNoteItem::metaDataRef()
{
    return metaData;
}

/**
 * @brief VNoteItem::metaDataConstRef
 * @return 源数据
 */
const QVariant &VNoteItem::metaDataConstRef() const
{
    return metaData;
}

/**
 * @brief VNoteItem::maxVoiceIdRef
 * @return 语音项最大id
 */
qint32 &VNoteItem::maxVoiceIdRef()
{
    return maxVoiceId;
}

/**
 * @brief VNoteItem::voiceMaxId
 * @return 语音项最大的id
 */
qint32 VNoteItem::voiceMaxId() const
{
    return maxVoiceId;
}

/**
 * @brief VNoteItem::newBlock
 * @param type
 * @return 生成的数据块
 */
VNoteBlock *VNoteItem::newBlock(int type)
{
    return datas.newBlock(type);
}

/**
 * @brief VNoteItem::addBlock
 * @param block
 */
void VNoteItem::addBlock(VNoteBlock *block)
{
    if (VNoteBlock::Voice == block->getType()) {
        maxVoiceId++;
    }

    datas.addBlock(block);
}

/**
 * @brief VNoteItem::addBlock
 * @param before
 * @param block
 */
void VNoteItem::addBlock(VNoteBlock *before, VNoteBlock *block)
{
    if (VNoteBlock::Voice == block->getType()) {
        maxVoiceId++;
    }

    datas.addBlock(before, block);
}

/**
 * @brief VNoteItem::delBlock
 * @param block
 */
void VNoteItem::delBlock(VNoteBlock *block)
{
    datas.delBlock(block);
}

/**
 * @brief VNoteItem::haveVoice
 * 判断是否存在语言
 * @return true 有语音
 */
bool VNoteItem::haveVoice() const
{
    if (htmlCode.isEmpty()) {
        return datas.voiceBlocks.size() > 0;
    }
    //匹配语音块标签的正则表达式
    QRegExp rx("<div.+jsonkey.+>");
    rx.setMinimal(true); //最小匹配
    if (rx.indexIn(htmlCode) == -1) {
        return false;
    }
    return true;
}

/**
 * @brief VNoteItem::haveText
 * @return true 有文本
 */
bool VNoteItem::haveText() const
{
    if (!htmlCode.isEmpty()) { //富文本文本内容判断
        if (htmlCode == "<p><br></p>") {
            return false;
        }
        return true;
    }

    bool fHaveText = false;
    for (auto it : datas.textBlocks) {
        if (!it->blockText.isEmpty()) {
            fHaveText = true;
            break;
        }
    }

    return fHaveText;
}

/**
 * @brief VNoteItem::voiceCount
 * @return 语音数量
 */
qint32 VNoteItem::voiceCount() const
{
    //老版本
    if (htmlCode.isEmpty()) {
        return datas.voiceBlocks.size();
    }
    //新富文本版本
    return getVoiceJsons().size();
}

/**
 * @brief VNoteItem::getVoiceJsons
 * 获取文本中所有的语音json字符串
 * @return 语音json字符串列表
 */
QStringList VNoteItem::getVoiceJsons() const
{
    //匹配语音块标签的正则表达式
    QRegExp rx("<div.+jsonkey.+>");
    rx.setMinimal(true); //最小匹配
    //匹配语音json数据的正则表达式
    QRegExp rxJson("\\{.*\\}");
    rxJson.setMinimal(true); //最小匹配
    QStringList list;
    int pos = 0;
    //查找语音块
    while ((pos = rx.indexIn(htmlCode, pos)) != -1) {
        //获取语音json数据
        if (rxJson.indexIn(rx.cap(0)) != -1) {
            list << rxJson.cap(0).replace("&quot;", "\"");
        }
        pos += rx.matchedLength();
    }
    return list;
}

/**
 * @brief VNoteItem::getFullHtml
 * 通过补全css样式和将图片路径转换为base64编码得到完整html字符串
 * @return  完整html字符串
 */
QString VNoteItem::getFullHtml() const
{
    //html字符串
    QString html = htmlHead;

    DPalette dp = DGuiApplicationHelper::instance()->applicationPalette();
    //获取系统高亮色
    QString activeHightColor = dp.color(DPalette::Active, DPalette::Highlight).name();
    //替换颜色
    html.replace("#00a48a", activeHightColor);

    html.append("<body> <div class=\"note-editable\" contenteditable=\"false\">");
    //匹配图片块标签的正则表达式
    QRegExp rx("<img.+src=.+>");
    rx.setMinimal(true); //最小匹配
    //匹配本地图片路径的正则表达式（图片位置限制在images文件夹，后缀限制为a-z长度为3到4位）
    QRegExp rxPath("(/\\S+)+/images/[\\w\\-]+\\.[a-z]{3,4}");
    rxPath.setMinimal(false); //最大匹配
    int pos = 0;
    int last = 0;
    //查找语音块
    while ((last = rx.indexIn(htmlCode, pos)) != -1) {
        html.append(htmlCode.mid(pos, last - pos));
        pos = last;
        //图片标签
        QString imgLabel = rx.cap(0);
        if ((last = rxPath.indexIn(imgLabel)) == -1) {
            //不存在路径
            html.append(imgLabel);
        } else {
            //转换图片
            QString base64 = "";
            if (!Utils::pictureToBase64(rxPath.cap(0), base64)) {
                //无效图片路径
                html.append(imgLabel);
            } else {
                //图片路径转换为base64编码
                html.append(imgLabel.mid(0, last))
                    .append(base64)
                    .append(imgLabel.mid(last + rxPath.matchedLength(), imgLabel.size() - last - rxPath.matchedLength()));
            }
        }
        pos += rx.matchedLength();
    }
    //html文件添加尾部
    html.append(htmlCode.mid(pos, htmlCode.size() - pos)).append("</div> </body> </html>");
    return html;
}

QDebug &operator<<(QDebug &out, VNoteItem &noteItem)
{
    out << "\n{ "
        << "noteId=" << noteItem.noteId << ","
        << "folderId=" << noteItem.folderId << ","
        << "noteType=" << noteItem.noteType << ","
        << "noteState=" << noteItem.noteState << ","
        << "noteTitle=" << noteItem.noteTitle << ","
        << "metaData=" << noteItem.metaData << ","
        << "createTime=" << noteItem.createTime << ","
        << "modifyTime=" << noteItem.modifyTime << ","
        << "deleteTime=" << noteItem.deleteTime << ","
        << "maxVoiceId=" << noteItem.maxVoiceId
        << " }\n";

    return out;
}

/**
 * @brief VNoteBlock::VNoteBlock
 * @param type
 */
VNoteBlock::VNoteBlock(qint32 type)
    : blockType(type)
{
    ptrBlock = this;
}

/**
 * @brief VNoteBlock::~VNoteBlock
 */
VNoteBlock::~VNoteBlock()
{
}

/**
 * @brief VNoteBlock::getType
 * @return 数据类型
 */
qint32 VNoteBlock::getType()
{
    return blockType;
}

/**
 * @brief VNTextBlock::VNTextBlock
 */
VNTextBlock::VNTextBlock()
    : VNoteBlock(Text)
{
}

VNTextBlock::~VNTextBlock()
{
}

/**
 * @brief VNTextBlock::releaseSpecificData
 */
void VNTextBlock::releaseSpecificData()
{
    //TODO:
    //    Add text specific operation code here.
    //
    //Do nothing for text now.
}

/**
 * @brief VNVoiceBlock::VNVoiceBlock
 */
VNVoiceBlock::VNVoiceBlock()
    : VNoteBlock(Voice)
{
    blockType = Voice;
}

VNVoiceBlock::~VNVoiceBlock()
{
}

/**
 * @brief VNVoiceBlock::releaseSpecificData
 */
void VNVoiceBlock::releaseSpecificData()
{
    //TODO:
    //    Add voice specific operation code here:
    qInfo() << "Remove file:" << voicePath;

    QFileInfo fileInfo(voicePath);

    if (fileInfo.exists()) {
        QFile::remove(voicePath);
    }
}
