// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VNOTEITEM_H
#define VNOTEITEM_H

#include "common/datatypedef.h"

#include <DWidget>

#include <QDateTime>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <QTextDocument>

DWIDGET_USE_NAMESPACE

struct VNoteBlock;
struct VNoteFolder;

struct VNoteItem {
public:
    VNoteItem();
    //是否可用
    bool isValid();
    //删除数据
    void delNoteData();
    //查找数据
    bool search(const QString &keyword);
    //源数据设置
    void setMetadata(const QVariant &meta);
    //绑定记事本项
    void setFolder(VNoteFolder *folder);
    //获取记事本项
    VNoteFolder *folder() const;

    enum {
        INVALID_ID = -1
    };

    enum State {
        Normal,
        Deleted,
    };

    enum VNOTE_TYPE {
        VNT_Text = 0,
        VNT_Voice,
        VNT_Mixed,
    };
    //记事项id
    qint32 noteId {INVALID_ID};
    //记事本id
    qint64 folderId {INVALID_ID};
    //数据类型
    qint32 noteType {VNOTE_TYPE::VNT_Text};
    //状态
    qint32 noteState {State::Normal};
    //是否置顶
    qint32 isTop {0};
    //是否加密
    qint32 encryption {0};
    //标题名称
    QString noteTitle {""};
    //富文本内容
    QString htmlCode {""};
    //创建时间
    QDateTime createTime;
    //修改时间
    QDateTime modifyTime;
    //删除时间
    QDateTime deleteTime;
    //获取元数据
    QVariant &metaDataRef();
    const QVariant &metaDataConstRef() const;
    //获取语音最大id值
    qint32 &maxVoiceIdRef();
    qint32 voiceMaxId() const;

    VNOTE_DATAS datas;
    //创建数据块
    VNoteBlock *newBlock(int type);
    //添加数据块
    void addBlock(VNoteBlock *block);
    //指定数据块后面插入数据块
    void addBlock(VNoteBlock *before, VNoteBlock *block);
    //删除数据块
    void delBlock(VNoteBlock *block);
    //是否有语音项
    bool haveVoice() const;
    //是否有文本项
    bool haveText() const;
    //语音数目
    qint32 voiceCount() const;
    //获取文本内所有语音json数据
    QStringList getVoiceJsons() const;
    //获取html
    QString getFullHtml() const;

protected:
    QVariant metaData;

    //Use to make default voice name
    //auto increment.
    qint32 maxVoiceId {0};

    //TODO:
    //    Don't used now ,Used for quick lookup.
    VNoteFolder *ownFolder {nullptr};

    friend QDebug &operator<<(QDebug &out, VNoteItem &noteItem);
};

struct VNTextBlock;
struct VNVoiceBlock;

struct VNoteBlock {
    enum {
        InValid,
        Text,
        Voice
    };

    virtual ~VNoteBlock();
    //释放资源，语音时用于删除文件
    virtual void releaseSpecificData() = 0;
    //获取数据类型
    qint32 getType();

    qint32 blockType {InValid};
    /*
     * Comment:
     *      For text block, store the content of the text,
     * for voice block, store the result of audio to text.
     * */
    QString blockText;

    union {
        VNoteBlock *ptrBlock;
        VNTextBlock *ptrText;
        VNVoiceBlock *ptrVoice;
    };

protected:
    explicit VNoteBlock(qint32 type = InValid);
    explicit VNoteBlock(const VNoteBlock &);
    const VNoteBlock &operator=(const VNoteBlock &);
};

//文本内容
struct VNTextBlock : public VNoteBlock {
    VNTextBlock();
    virtual ~VNTextBlock() override;
    virtual void releaseSpecificData() override;
};

//语音内容
struct VNVoiceBlock : public VNoteBlock {
    VNVoiceBlock();
    virtual ~VNVoiceBlock() override;
    virtual void releaseSpecificData() override;

    QString voicePath {""};
    qint64 voiceSize {0};
    QString voiceTitle {""};
    bool state {false};
    QDateTime createTime;
};
#endif // VNOTEITEM_H
