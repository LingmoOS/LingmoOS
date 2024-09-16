// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFONTPREVIEWITEMDEF_H
#define DFONTPREVIEWITEMDEF_H

#include "dfontinfomanager.h"

#include <QMetaType>
#include <QString>
#include <QDebug>

#define FTM_DEFAULT_PREVIEW_TEXT QString(DTK_WIDGET_NAMESPACE::DApplication::translate("Font", "Don't let your dreams be dreams"))
#define FTM_DEFAULT_PREVIEW_CN_TEXT QString("因理想而出生，为责任而成长")
#define FTM_DEFAULT_PREVIEW_EN_TEXT QString("Don't let your dreams be dreams")
#define FTM_DEFAULT_PREVIEW_LENGTH (15)
#define FTM_DEFAULT_PREVIEW_EN_LENGTH (FTM_DEFAULT_PREVIEW_EN_TEXT.size())
#define FTM_DEFAULT_PREVIEW_DIGIT_TEXT QString("0123456789")
#define FTM_DEFAULT_PREVIEW_FONTSIZE 30
#define FTM_PREVIEW_ITEM_HEIGHT (72+2+10)
#define FTM_PREVIEW_ITEM_COMPACT_HEIGHT (50+2+10)

#define FONT_TTF QString("TrueType")
#define FONT_OTF QString("OpenType")
#define FONT_UNKNOWN QString("Unknown");

/*************************************************************************
 <Enum>          IconStatus
 <Description>   收藏图标状态
 <Author>
 <Value>
    <Value1>     IconNormal=0       Description:常规状态
    <Value2>     IconPress          Description:press状态
    <Value2>     IconHover          Description:hover状态
 <Note>          null
*************************************************************************/
typedef enum IconStatus {
    IconNormal = 0,
    IconPress,
    IconHover,
} IconStatus;

typedef enum FontType {
    TTF = 0,
    OTF,
    TTC,
    UNKNOWN,
} FontType;

/*************************************************************************
 <Enum>          枚举
 <Description>   字体状态枚举
 <Author>
 <Value>
    <Value1>     CLEAR               Description:无状态
    <Value2>     ENABLED             Description:是否启用
    <Value3>     COLLECTED           Description:是否收藏
    <Value4>     CHINESED            Description:是否中文字体
    <Value5>     MONOSPACE           Description:是否等宽字体

    <Value6>     NORMAL              Description:常规
    <Value7>     PRESS               Description:press状态
    <Value8>     HOVEER              Description:hover状态
    <Value9>     HOVERSTATE          Description:press&hover状态
 <Note>          null
*************************************************************************/
enum {
    CLEAR = 0,
    ENABLED = 0x01,
    COLLECTED = 0x02,
    CHINESED = 0x04,
    MONOSPACE = 0x08,
    HOVERSTATE = 0x30,
    FONTTYPE = 0x0F00,
};

/*************************************************************************
 <Struct>        FontData
 <Description>   字体信息结构体
 <Attribution>
    <Attr1>     strFontName           Description:字体名称 familyName-StyleName
    <Attr2>     fontState             Description:字体状态
 <Note>          null
*************************************************************************/
struct FontData {
    QString strFontName;
    short fontState;
    bool isSystemFont;
    FontData()
        : strFontName("")
        , fontState(0)
        , isSystemFont(false)
    {
    }

    //拷贝构造
    FontData(const QString &_strFontName, bool isEnabled, bool isCollected, bool isChinesed, bool isMono, FontType type, bool _isSystemFont)
        : strFontName(_strFontName)
        , fontState(0)
        , isSystemFont(_isSystemFont)
    {
        setEnabled(isEnabled);
        setCollected(isCollected);
        setChinese(isChinesed);
        setMonoSpace(isMono);
        setFontType(type);
    }

    //重载拷贝构造
    FontData(const QString &_strFontName, bool isEnabled, bool isCollected, bool isChinesed, bool isMono, const QString &type, bool _isSystemFont)
        : strFontName(_strFontName)
        , fontState(0)
        , isSystemFont(_isSystemFont)
    {

        setEnabled(isEnabled);
        setCollected(isCollected);
        setChinese(isChinesed);
        setMonoSpace(isMono);
        setFontType(type);
    }


    //更新启用状态
    void setEnabled(bool isEnabled)
    {
        fontState = isEnabled ? (fontState | ENABLED) : (fontState & ~ENABLED);
    }

    //返回启用状态
    bool isEnabled() const
    {
        return (fontState & ENABLED);
    }
    //更新收藏状态
    void setCollected(bool collected)
    {
        fontState = (collected ? (fontState | COLLECTED) : (fontState & ~COLLECTED));
    }

    //返回收藏状态
    bool isCollected() const
    {
        return (fontState & COLLECTED);
    }
    //更新是否为中文字体状态
    void setChinese(bool chinese)
    {
        fontState = (chinese ? (fontState | CHINESED) : (fontState & ~CHINESED));
    }

    //返回是否为中文字体状态
    bool isChinese() const
    {
        return (fontState & CHINESED);
    }
    //更新是否为等宽字体状态
    void setMonoSpace(bool isMonospace)
    {
        fontState = (isMonospace ? (fontState | MONOSPACE) : (fontState & ~MONOSPACE));
    }

    //返回是否为等宽字体状态
    bool isMonoSpace() const
    {
        return (fontState & MONOSPACE);
    }
    //更新hover或press状态
    void setHoverState(IconStatus state)
    {
        fontState &= ~HOVERSTATE;
        fontState |= ((state) << 4);
    }

    //返回hover或press状态
    IconStatus getHoverState() const
    {
        return static_cast<IconStatus>((fontState & HOVERSTATE) >> 4);
    }

    void setFontType(FontType type)
    {
        fontState &= ~FONTTYPE;
        fontState |= (type << 8);
    }

    void setFontType(const QString &suffix)
    {
        if (suffix == FONT_TTF) {
            setFontType(TTF);
        } else if (suffix == FONT_OTF) {
            setFontType(OTF);
        } else {
            setFontType(UNKNOWN);
        }
    }

    FontType getFontType() const
    {
        return static_cast<FontType>((fontState & FONTTYPE) >> 8);
    }

    bool operator==(const FontData &info) const
    {
        return ((strFontName == info.strFontName) && (getFontType() == info.getFontType()));
    }
};

Q_DECLARE_METATYPE(FontData)


struct FontDelegateData {
    QString familyName;
    QString styleName;
    QString ownPreview;

    FontDelegateData()
        : familyName("")
        , styleName("")
        , ownPreview("")
    {

    }

    FontDelegateData(const QString &_familyName, const QString &_styleName,
                     const QString &_ownPreview)
        : familyName(_familyName)
        , styleName(_styleName)
        , ownPreview(_ownPreview)

    {
    }

    FontDelegateData(const FontDelegateData &other)
        : familyName(other.familyName)
        , styleName(other.styleName)
        , ownPreview(other.ownPreview)
    {

    }
};

Q_DECLARE_METATYPE(FontDelegateData)

/*************************************************************************
 <Struct>        DFontPreviewItemData
 <Description>   ItemModel信息结构体
 <Attribution>
    <Attr1>      fontInfo         Description:字体信息
    <Attr2>      strFontName      Description:字体名称
    <Attr3>      isEnabled        Description:是否启用
    <Attr4>      isCollected      Description:是否收藏
    <Attr5>      isChineseFont    Description:是否中文字体
    <Attr6>      isMonoSpace      Description:是否等宽字体
    <Attr7>      appFontId        Description:字体唯一id,对应数据库表中的fontId

    <Attr8>      filePath         Description:字体路径
    <Attr9>      familyName       Description:字体组名
    <Attr10>      styleName       Description:字体风格
    <Attr11>      type            Description:字体类型
    <Attr12>      version         Description:版本信息
    <Attr13>      copyright       Description:版权信息
    <Attr14>      description     Description:字体描述内容
    <Attr15>      sysVersion      Description:系统版本
    <Attr16>      fullname        Description:字体全名
    <Attr17>      psname          Description:字体PS名称
    <Attr18>      trademark       Description:字体商标信息
    <Attr19>      isInstalled     Description:是否已安装
    <Attr20>      isSystemFont    Description:是否为系统字体
    <Attr21>      defaultPreview  Description:默认预览内容
    <Attr22>      previewLang     Description:预览语言
 <Note>          null
*************************************************************************/
struct DFontPreviewItemData {
    DFontInfo fontInfo;       //字体信息
    FontData fontData;

    QString strFontId;        //字体唯一id,对应数据库表中的fontId

    int appFontId;

    DFontPreviewItemData()
        : fontInfo(DFontInfo())
        , strFontId("")
        , appFontId(-1)
    {
    }

    DFontPreviewItemData(const QString &_filePath, const QString &_familyName, const QString &_styleName, const QString &_type,
                         const QString &_version, const QString &_copyright, const QString &_desc, const QString &_sysVer,
                         const QString &_fullname, const QString &_psname, const QString &_trademark, bool _isInstalled,
                         bool _isError, bool _isSystemFont, bool _isEnabled, bool _isCollected, bool _isChinese,
                         bool _isMono, const QString &_strFontName, const QString &_sp3FamilyName, const QString &_strFontId)
        : fontInfo(DFontInfo(_filePath, _familyName, _styleName, _type, _version, _copyright, _desc, _sysVer, _fullname,
                             _psname, _trademark, _isInstalled, _isError, _isSystemFont, _sp3FamilyName))
        , fontData(FontData(_strFontName, _isEnabled, _isCollected, _isChinese, _isMono, _type, fontInfo.isSystemFont))
        , strFontId(_strFontId)
        , appFontId(-1)
    {

    }

    DFontPreviewItemData(const DFontPreviewItemData &other)
        : fontInfo(other.fontInfo)
        , fontData(other.fontData)
        , strFontId(other.strFontId)
        , appFontId(other.appFontId)

    {
    }

    bool operator==(const DFontPreviewItemData &info) const
    {
        if (info.fontData.strFontName.isEmpty())
            return (fontInfo == info.fontInfo);

        return (fontData == info.fontData);
    }
};

Q_DECLARE_METATYPE(DFontPreviewItemData)
#endif  // DFONTPREVIEWITEMDEF_H
