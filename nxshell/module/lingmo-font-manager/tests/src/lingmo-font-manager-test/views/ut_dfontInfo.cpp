// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfontinfomanager.h"

#include "../third-party/stub/stub.h"
#include <gtest/gtest.h>


#include <QProcess>
#include <QDebug>
#include <QDir>


namespace  {

class TestDFontInfo : public testing::Test
{
public:

    void SetUp() override
    {
        fontInfo.familyName = "familyname_one";
        fontInfo.styleName = "stylename_one";
        fontInfo.psname = "psname_one";
        fontInfo.trademark = "trademark_one";
        fontInfo.fullname = "fullname_one";

        fontinfo2.familyName = "familyname_two";
        fontinfo2.styleName = "stylename_two";
    }
    void TearDown() override
    {

    }

public:
    DFontInfo fontInfo;
    DFontInfo fontinfo2;
};

class TestDFontInfoGetFileNames : public testing::Test
{
public:
    virtual void SetUp()
    {
        dfm = DFontInfoManager::instance();
    }
    virtual void TearDown()
    {

    }
public:
    DFontInfoManager *dfm = nullptr;
    //任何一个存在字体的路径
    QString path = "/usr/share/fonts/truetype/lohit-devanagari";
    //任何不存在的一个路径，用以提供判断
    QString path2 = "/usr/share/abc";
    QString path3 = "/usr/share/fonts/X11/Type1";
    QStringList returnList;
};


//参数化测试
class TestcheckStyleNameFirst : public::testing::TestWithParam<QString>
{
public:
    DFontInfoManager *dfm = DFontInfoManager::instance();
};

//getInstFontPath
class TestDFontInfoManager : public testing::Test
{
public:

    void SetUp() override
    {
        dfm = DFontInfoManager::instance();
    }
    void TearDown() override
    {

    }
public:
    DFontInfoManager *dfm = nullptr;
    QString originPath;
    QString familyName;
    DFontInfo fontinfo;
    QString filepath = QString();
    QString sysDir = QDir::homePath() + "/.local/share/fonts";
    QString target;
};

DFontInfo stub_getFontinfo()
{
    DFontInfo info;
    info.psname = "first";

    return info;
}
}

QString stub_returnStr()
{
    return QString();
}

QStringList stub_getFilenames()
{
    QStringList list;
    list << "first";
    return list;
}


//refreshList
TEST_F(TestDFontInfoManager, checkRefreshList)
{
    Stub s;
    s.set(ADDR(DFontInfoManager, getFontInfo), stub_getFontinfo);

    QStringList list;
    list << "first";

    dfm->refreshList(list);

    DFontInfo info;
    info.psname = "first";

    EXPECT_TRUE(dfm->isFontInstalled(info));

    dfm->refreshList(QStringList());
}

//getFontPath
TEST_F(TestDFontInfoManager, checkGetFontPath)
{
    QString file;
    file = dfm->getFontPath();

    EXPECT_FALSE(file.isNull());

}



TEST_F(TestDFontInfo, equalsign_is_normal)
{
    EXPECT_EQ(false, fontInfo == fontinfo2);
    DFontInfo fontinfor3(fontInfo);
    EXPECT_EQ("familyname_one", fontinfor3.familyName);
    EXPECT_EQ("stylename_one", fontinfor3.styleName);
    EXPECT_EQ("psname_one", fontinfor3.psname);
    EXPECT_EQ("trademark_one", fontinfor3.trademark);
    EXPECT_EQ("fullname_one", fontinfor3.fullname);
}

TEST_F(TestDFontInfo, tostring_is_normal)
{
    QString str = "FontInfo : familyname_one, stylename_one, psname = psname_one, trademark = trademark_one, fullname = fullname_one";
    EXPECT_EQ(true, str == fontInfo.toString());
}

TEST_F(TestDFontInfoManager, checkGetFileNames)
{
    QString path = "null";
    QStringList list = dfm->getFileNames(path);
    EXPECT_TRUE(list.count() == 0);

    path = "/usr/share/fonts/truetype/";
    list = dfm->getFileNames(path);
    //通过这行看编译环境中可能存在的字体文件,用来调整测试用例
    qDebug() << list << "+++++++++++++++++++++++++++++++++++++++++++" << endl;
    EXPECT_TRUE(list.count() != 0);
}

TEST_F(TestDFontInfoManager, checkGetAllFontPath)
{
    Stub s;
    s.set(ADDR(QProcess, readAllStandardOutput), stub_returnStr);

    Stub s1;
    s1.set(ADDR(DFontInfoManager, getFileNames), stub_getFilenames);

    QStringList list = dfm->getAllFontPath(false);
    EXPECT_TRUE(list.count() == 1);
    EXPECT_TRUE(list.first() == "first");

    list = dfm->getAllFontPath(true);
    EXPECT_TRUE(list.count() == 1);
    EXPECT_TRUE(list.first() == "first");
}



TEST_F(TestDFontInfoManager, getFontType_is_normal)
{
    EXPECT_EQ("TrueType", dfm->getFontType("/usr/share/fonts/truetype/liberation/LiberationMono-Italic.ttf"));

    EXPECT_EQ("TrueType", dfm->getFontType("/usr/share/fonts/fonts-cesi/CESI_XBS_GB18030.TTF"));

    EXPECT_EQ("TrueType", dfm->getFontType("/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc"));

    EXPECT_EQ("OpenType", dfm->getFontType(QDir::homePath() + "/Desktop/1048字体/Addictype-Regular.otf"));

    EXPECT_EQ("Unknown", dfm->getFontType("/usr/share/fonts/X11/Type1/c0649bt_.pfb"));

    //异常检查
    EXPECT_EQ("Unknown", dfm->getFontType(""));

}

TEST_P(TestcheckStyleNameFirst, checkStyleName_Is_Normal)
{
    QString n =  GetParam();
    DFontInfo f;
    f.styleName = "?";
    f.psname = QString(n);

    dfm->checkStyleName(f);
    qDebug() << f.psname << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << f.styleName;
    EXPECT_EQ(f.psname, f.styleName);
}

INSTANTIATE_TEST_CASE_P(HandleTrueReturn, TestcheckStyleNameFirst, testing::Values("Regular", "Bold", "Light", "Thin", "ExtraLight", "ExtraBold",
                                                                                   "Medium", "DemiBold", "AnyStretch", "UltraCondensed",
                                                                                   "ExtraCondensed", "Condensed", "SemiCondensed", "Unstretched",
                                                                                   "SemiExpanded", "Expanded", "ExtraExpanded", "UltraExpanded"));

TEST_F(TestDFontInfoManager, getErrorFontInfo_is_normal)
{
    //系统字体应该为已安装,执行函数得到的结果已安装为false,出现错误.
    fontinfo = dfm->getFontInfo(QDir::homePath() + "/Desktop/abc.ttf");
    EXPECT_EQ(true, fontinfo.isError);
}

TEST_F(TestDFontInfoManager, getDefaultPreview_is_normal)
{
    qint8 Long = 1;
    QString str = dfm->getDefaultPreview(QDir::homePath() + "/Desktop/abc.ttf", Long);
    EXPECT_EQ(true, str.isNull());
}

TEST_F(TestDFontInfoManager, checkGetFontFamilyStyle)
{
    QString filepath = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf";
    QStringList list = dfm->getFontFamilyStyle(filepath);
    EXPECT_TRUE(list.contains("DejaVu Sans"));
}


//TODO 第三方库代码暂时不知道怎么打桩,返回值无法确定,无法断言
TEST_F(TestDFontInfoManager, checkgetFonts)
{
    QStringList list = dfm->getFonts(DFontInfoManager::All);

    list = dfm->getFonts(DFontInfoManager::Chinese);

    list = dfm->getFonts(DFontInfoManager::MonoSpace);

    Q_UNUSED(list)
}

TEST_F(TestDFontInfoManager, getAllChineseFontCount_is_normal)
{
    int count = dfm->getAllChineseFontPath().count();

    QProcess process;
    process.start("fc-list :lang=zh");
    process.waitForFinished(-1);

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split(QChar('\n'));

    EXPECT_EQ(lines.count() - 1, count);
}


TEST_F(TestDFontInfoManager, getAllMonoFontCount_is_normal)
{
    int count = dfm->getAllMonoSpaceFontPath().count();

    QProcess process;
    process.start("fc-list :spacing=mono");
    process.waitForFinished(-1);

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split(QChar('\n'));

    EXPECT_EQ(lines.count() - 1, count);
}


//isFontInstalled检测已安装字体是否已安装
TEST_F(TestDFontInfoManager, fontIsInstalled_installedFont_isnormal)
{
//因为static成员 dataList没有其他办法访问,所以先调用这个函数
    QStringList list;
    list << "/usr/share/fonts/fonts-cesi/CESI_XBS_GB13000.TTF";
    dfm->refreshList(list);
    DFontInfo fontInfo = dfm->getFontInfo("/usr/share/fonts/fonts-cesi/CESI_XBS_GB13000.TTF");

    EXPECT_EQ(true, dfm->isFontInstalled(fontInfo));
}

//isFontInstalled检测未安装字体是否已安装
TEST_F(TestDFontInfoManager, fontIsInstalled_notInstalledFont_isnormal)
{
//因为static成员 dataList没有其他办法访问,所以先调用这个函数
    dfm->refreshList(QStringList());
    DFontInfo fontInfo = dfm->getFontInfo(QDir::homePath() + "/Desktop/1048字体/食物.ttf");

    EXPECT_EQ(false, dfm->isFontInstalled(fontInfo));
}

//isFontInstalled检测异常字体是否已安装
TEST_F(TestDFontInfoManager, fontIsInstalled_errorFont_isnormal)
{
//因为static成员 dataList没有其他办法访问,所以先调用这个函数
    dfm->refreshList(QStringList());
    DFontInfo fontInfo;
    EXPECT_EQ(false, dfm->isFontInstalled(fontInfo));
}
















