// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontpreview.h"
#include "dfontwidget.h"

#include <QApplication>
#include <QGuiApplication>
#include <QFileInfo>
#include <QPainter>
#include <QDebug>

#include <fontconfig/fontconfig.h>
#include <fontconfig/fcfreetype.h>

static const QString lowerTextStock = "abcdefghijklmnopqrstuvwxyz";
static const QString upperTextStock = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const QString punctuationTextStock = "0123456789.:,;(*!?')";
static const int textWidth = 1204;
const int FIXED_WIDTH = 1280;
const int FIXED_HEIGHT = 720;

static QString sampleString = nullptr;
static QString styleName = nullptr;


/*************************************************************************
 <Function>      DFontPreview
 <Description>   构造函数-创建一个字体预览视图类对象
 <Author>
 <Input>
    <param1>     parent          Description:父对象
 <Return>        DFontPreview    Description:返回一个字体预览视图类对象
 <Note>          null
*************************************************************************/
DFontPreview::DFontPreview(QWidget *parent)
    : QWidget(parent)
    , m_library(nullptr)
    , m_face(nullptr)
{
    initContents();

    setFixedSize(FIXED_WIDTH, FIXED_HEIGHT);
    //    setFixedSize(static_cast<int>(qApp->primaryScreen()->geometry().width() / 1.5),
    //                 static_cast<int>(qApp->primaryScreen()->geometry().height() / 1.5));
}

/*************************************************************************
 <Function>      ~DFontPreview
 <Description>   析构函数-析构字体预览视图类对象
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DFontPreview::~DFontPreview()
{
    FT_Done_Face(m_face);
    FT_Done_FreeType(m_library);
}

/*************************************************************************
 <Function>      setFileUrl
 <Description>   传入当前预览字体的路径
 <Author>
 <Input>
    <param1>     url             Description:当前预览字体的路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreview::setFileUrl(const QString &url)
{
    // fontDatabase->removeAllApplicationFonts();
    // fontDatabase->addApplicationFont(url);

    FT_Init_FreeType(&m_library);
    m_error = FT_New_Face(m_library, url.toUtf8().constData(), 0, &m_face);

    if (m_error != 0 && QFileInfo(url).completeSuffix() != "pcf.gz")
        return;

    sampleString = getSampleString().simplified();
    styleName = QString(m_face->style_name);

    repaint();
}

/*************************************************************************
 <Function>      paintEvent
 <Description>   重写绘图事件-实现预览效果
 <Author>
 <Input>
    <param1>     e               Description:事件对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreview::paintEvent(QPaintEvent *e)
{
    currentMaxWidth = 1;
    if (m_error != 0)
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QFont font;
    QString familyName = fontDatabase.applicationFontFamilies(0).first();
    font.setFamily(familyName);

    font.setStyleName(styleName);
    painter.setPen(Qt::black);

    const int padding = 30;
    const int x = 35;
    int y = 10;
    int fontSize = 25;

    font.setPointSize(fontSize);
    painter.setFont(font);

    const QFontMetrics metrics(font);

    // if we don't have lowercase/uppercase/punctuation text in the face
    // we omit it directly, and render a random text below.

    /*根据获取的新point进行绘制 UT000539 fix bug 27030*/
    if (checkFontContainText(m_face, lowerTextStock)) {
        const int lowerWidth = metrics.width(lowerTextStock);
        isNeedScroll(lowerWidth);
        const int lowerHeight = metrics.height();
        QPoint baseLinePoint = adjustPreviewFontBaseLinePoint(QRect(x, y + padding, lowerWidth, lowerHeight), metrics);
        painter.drawText(baseLinePoint.x(), baseLinePoint.y(), lowerTextStock);
        y += lowerHeight;
    }

    if (checkFontContainText(m_face, upperTextStock)) {
        const int upperWidth = metrics.width(upperTextStock);
        isNeedScroll(upperWidth);
        const int upperHeight = metrics.height();
        QPoint baseLinePoint = adjustPreviewFontBaseLinePoint(QRect(x, y + padding, upperWidth, upperHeight), metrics);
        painter.drawText(baseLinePoint.x(), baseLinePoint.y(), upperTextStock);
        y += upperHeight;
    }

    if (checkFontContainText(m_face, punctuationTextStock)) {
        const int punWidth = metrics.width(punctuationTextStock);
        isNeedScroll(punWidth);
        int punHeight = metrics.height();
        QPoint baseLinePoint = adjustPreviewFontBaseLinePoint(QRect(x, y + padding, punWidth, punHeight), metrics);
        painter.drawText(baseLinePoint.x(), baseLinePoint.y(), punctuationTextStock);
        y += punHeight;
    }

    for (int i = 0; i < 20; ++i) {
        fontSize += 3;
        font.setPointSize(fontSize);
        painter.setFont(font);

        QFontMetrics met(font);
        int sampleWidth = met.width(sampleString);
        isNeedScroll(sampleWidth);
        int sampleHeight = met.height();

        if (y + sampleHeight >= rect().height() - padding * 2)
            break;

        QPoint baseLinePoint = adjustPreviewFontBaseLinePoint(QRect(x, y + padding * 2, sampleWidth, sampleHeight), met);
        painter.drawText(baseLinePoint.x(), baseLinePoint.y(), sampleString);
        y += sampleHeight + padding;
    }
    /*判断是否需要滚动条 UT000539*/
    if (currentMaxWidth > viewWidth) {
        setFixedWidth(currentMaxWidth);
        m_needScroll = true;
    } else {
//        setFixedWidth(static_cast<int>(qApp->primaryScreen()->geometry().width() / 1.5));
        setFixedWidth(FIXED_WIDTH);
        m_needScroll = false;
    }
    QWidget::paintEvent(e);
}

/*************************************************************************
 <Function>      isNeedScroll
 <Description>   判断是否需要出现底部水平滚动条
 <Author>        UT000539
 <Input>
    <param1>     width           Description:当前预览文本最大宽度
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreview::isNeedScroll(const int width)
{
    if (m_needScroll == false) {
        if (width > textWidth) {
            m_needScroll = true;
        }
    }
    if (width > currentMaxWidth) {
        currentMaxWidth = width;
    }
}

/*************************************************************************
 <Function>      initContents
 <Description>   初始化预览文本内容
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreview::initContents()
{
    // libfont-manager.so中已经有CONTENTS.txt，需要重命名为contents.txt才可以安装。
    QFile file("/usr/share/deepin-font-manager/contents.txt");

    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray content = file.readAll();
    QTextStream stream(&content, QIODevice::ReadOnly);

    file.close();

    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        const QStringList items = line.split(QChar(':'));

        m_contents.insert(items.at(0), items.at(1));
    }
}

/*************************************************************************
 <Function>      getSampleString
 <Description>   获取预览文本-获取小写英文文本
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
QString DFontPreview::getSampleString()
{
    //[CPPCHECK]
    //局部变量sampleString与全局变量名称重复，更名sampString
    QString sampString = nullptr;
    bool isAvailable = false;

    // check the current system language sample string.
    sampString = getLanguageSampleString(QLocale::system().name());
    if (checkFontContainText(m_face, sampString) && !sampString.isEmpty()) {
        isAvailable = true;
    }

    // check english sample string.
    if (!isAvailable) {
        sampString = getLanguageSampleString("en");
        if (checkFontContainText(m_face, sampString)) {
            isAvailable = true;
        }
    }

    // random string from available chars.
    if (!isAvailable) {
        sampString = buildCharlistForFace(m_face, 36);
    }

    return sampString;
}

/*************************************************************************
 <Function>      getLanguageSampleString
 <Description>   获得当前语言下可预览字符串
 <Author>
 <Input>
    <param1>     language        Description:当前语言
 <Return>        QString         Description:返回当前语言下可预览字符串
 <Note>          null
*************************************************************************/
QString DFontPreview::getLanguageSampleString(const QString &language)
{
    QString result = nullptr;
    QString key = nullptr;
    if (m_contents.contains(language)) {
        key = language;
    } else {
        const QStringList parseList = language.split("_", QString::SkipEmptyParts);
        if (parseList.length() > 0 &&
                m_contents.contains(parseList.first())) {
            key = parseList.first();
        }
    }

    if (m_contents.contains(key)) {
        auto findResult = m_contents.find(key);
        result.append(findResult.value());
    }

    return result;
}

/*************************************************************************
 <Function>      checkFontContainText
 <Description>   检查字体库中是否有当前字符信息
 <Author>
 <Input>
    <param1>     face            Description:FT_Face操作对象
    <param2>     text            Description:判断文本
 <Return>        bool            Description:返回判断结果
 <Note>          null
*************************************************************************/
bool DFontPreview::checkFontContainText(FT_Face face, const QString &text)
{
    if (face == nullptr || face->num_charmaps == 0)
        return false;

    bool retval = true;

    int err = 0;
    if (face->charmap == nullptr)
        err = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
    if (err != 0) {
        for (int i = 0; i < face->num_charmaps; ++i) {
            err = FT_Select_Charmap(face, face->charmaps[i]->encoding);
            if (err == 0) {
                break;
            }
        }
    }

    for (auto ch : text) {
        if (!FT_Get_Char_Index(face, ch.unicode()) && ch != "，") {
            retval = false;
            break;
        }
    }

    return retval;
}

/*************************************************************************
 <Function>      isSpecialSymbol
 <Description>   判断是否为特殊字符
 <Author>
 <Input>
    <param1>     face            Description:FT_Face操作对象
    <param2>     ucs4            Description:uint操作对象
 <Return>        bool            Description:返回判断结果,true：特殊字符；false：正常字符
 <Note>          null
*************************************************************************/
bool isSpecialSymbol(FT_Face face, uint ucs4)
{
    unsigned int glyph = FT_Get_Char_Index(face, ucs4);
    int err;

    err = FT_Load_Glyph(face, glyph, FT_LOAD_NO_SCALE);

    if (err == 0 && face->glyph->metrics.width == 0 && face->glyph->metrics.height == 0) {
        return true;
    }

    if ((ucs4 >= 0x610 && ucs4 <= 0x615) || (ucs4 >= 0x64B && ucs4 <= 0x65E)  //Kufi
            || (ucs4 >= 0x21 && ucs4 <= 0x30) || (ucs4 >= 0x7E && ucs4 <= 0xBB) || (ucs4 >= 0x300 && ucs4 <= 0x36F)
            || (ucs4 >= 0x64B && ucs4 <= 0x655) || (ucs4 >= 0x670 && ucs4 <= 0x700) //Syriac
            || (ucs4 == 0x2D) || (ucs4 >= 0x591 && ucs4 <= 0x5C7) //hebrew
            || (ucs4 == 0x25CC) || (ucs4 >= 0xA980 && ucs4 <= 0xA983)
            || (ucs4 == 0x374 || ucs4 == 0x375)
            || (ucs4 >= 0x1801 && ucs4 <= 0x1805) || (ucs4 >= 0x2025 && ucs4 <= 0x2026)
            || (ucs4 >= 0x3001 && ucs4 <= 0x301B)
            || (ucs4 >= 0x1B00 && ucs4 <= 0x1B04) //balinese
            || (ucs4 >= 0x1B80 && ucs4 <= 0x1B82) || (ucs4 >= 0x1BA1 && ucs4 <= 0x1BAD) //sundanese
            || (ucs4 >= 0x11180 && ucs4 <= 0x11182) //sharada
            || (ucs4 >= 0xFE20 && ucs4 <= 0xFE2F) // caucasian
            || (ucs4 >= 0x11080 && ucs4 <= 0x11082) //kaithi
            || (ucs4 >= 0x610 && ucs4 <= 0x61A) //arabic
            || ((ucs4 >= 0x0EB4 && ucs4 <= 0x0EBC) || (ucs4 == 0x0EB1)) //lao
            || (ucs4 >= 0x11C92 && ucs4 <= 0x11CB6) //marchen
            || (ucs4 >= 0x1171D && ucs4 <= 0x1172B) //ahom
            || ((ucs4 >= 0x11300 && ucs4 <= 0x11303) || (ucs4 >= 0x1CD0 && ucs4 <= 0x1cF9) || (ucs4 == 0x20F0) || (ucs4 >= 0xD800 && ucs4 <= 0xDFFF)) //grantha
            || (ucs4  >= 0x10A01 && ucs4 <= 0x10A0F) || (ucs4 >= 0x10A38 && ucs4 <= 0x10A3F)) // kharoshthi
        return true;
    return false;
}

/*************************************************************************
 <Function>      buildCharlistForFace
 <Description>   构建预览字符串
 <Author>
 <Input>
    <param1>     face            Description:FT_Face操作对象
    <param2>     length          Description:长度
 <Return>        QString         Description:预览预览字符串
 <Note>          null
*************************************************************************/
QString DFontPreview::buildCharlistForFace(FT_Face face, int length)
{
    QString retval;
    if (face == nullptr)
        return retval;

    bool specialFont = (INT_MAX == length);
    if (specialFont)
        length = 30;
    FcCharSet *fcs = nullptr;
    FcChar32 count = 0;

    // FcCharSet * FcFreeTypeCharSet(FT_Face face, FcBlanks *blanks);
    // Scans a FreeType face and returns the set of encoded Unicode chars. FcBlanks is deprecated, blanks is ignored and accepted only for compatibility with older code.
    fcs = FcFreeTypeCharSet(face, nullptr);

    // FcChar32 FcCharSetCount(const FcCharSet *a);
    // Returns the total number of Unicode chars in a.
    count = FcCharSetCount(fcs);
//    qDebug() << __FUNCTION__ << " total count = " << count;

    QList<uint> ucs4List;
    if (count > 0) {
        FcChar32 ucs4, pos, map[FC_CHARSET_MAP_SIZE];
        int retCount = 0;
        uint unicode = 0;

        for (ucs4 = FcCharSetFirstPage(fcs, map, &pos);
                ucs4 != FC_CHARSET_DONE;
                ucs4 = FcCharSetNextPage(fcs, map, &pos)) {
            for (uint i = 0; i < FC_CHARSET_MAP_SIZE; i++) {
                if (map[i] == 0)
                    continue;
                //[CPPCHECK]
                //此处出现int左移32位未定义的情况，原因为IA-32处理器(从Intel 286处理器开始)都会将移位计数掩盖为5位，从而导致最高计数为31位。
                //超出31位则不受控，由编译器自主输出。
                for (uint j = 0; j < 31; j++) {
                    if ((map[i] & (1 << j)) == 0)
                        continue;

                    unicode = ucs4 + i * 32 + j;
                    ucs4List << unicode;
                    retCount++;
                }
            }
        }

        int len = (length > retCount) ? retCount : length;

        uint firstChar = ucs4List[len - 1];
        int index = 0;

        if (static_cast<int>(count) > len) {
            for (int i = len - 2; i >= 0; i--) {
                if (firstChar - ucs4List[i] > static_cast<uint>((len - 1 - i) + len - 1)) {
                    index = i + 1;
                    break;
                }
            }
        }

//        qDebug() << __FUNCTION__ << len << index;
        QString outStr;
        if (len + index > retCount)
            index = retCount - len;
        if (index < 0)
            index = 0;

        if (specialFont)
            len = 7;
        for (int i = index; len > 0 && i < retCount - index; i++) {
            uint w1 = ucs4List.at(i);
            if (isSpecialSymbol(face, w1))
                continue;

            retval += QString::fromUcs4(&w1, 1);
            len--;
        }
    }

    // Destroy a character set
    FcCharSetDestroy(fcs);

    return retval;
}

/*************************************************************************
 <Function>      adjustPreviewFontBaseLinePoint
 <Description>   获取预览绘制的起始点
 <Author>        UT000539
 <Input>
    <param1>     fontPreviewRect    Description:传入绘制区域参数
    <param2>     previewFontMetrics Description:预览字体信息参数
 <Return>        QPoint             Description:返回预览绘制的起始点
 <Note>          null
*************************************************************************/
QPoint DFontPreview::adjustPreviewFontBaseLinePoint(const QRect &fontPreviewRect, const QFontMetrics &previewFontMetrics) const
{
    Q_UNUSED(previewFontMetrics);
    int commonFontDescent = fontPreviewRect.height() / 4;
    int baseLineX = fontPreviewRect.x();
    int baseLineY = fontPreviewRect.bottom() - commonFontDescent;

    return QPoint(baseLineX, baseLineY);
}
