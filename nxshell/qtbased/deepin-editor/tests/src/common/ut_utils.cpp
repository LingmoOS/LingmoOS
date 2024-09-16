// SPDX-FileCopyrightText: 2019 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_utils.h"

#include "../../src/common/settings.h"
#include "../../src/controls/tabbar.h"
#include "../../src/editor/editwrapper.h"
#include "../../src/widgets/window.h"
#include "../../src/startmanager.h"
#include "../../src/editor/dtextedit.h"
#include "../../src/common/utils.h"
extern "C" {
#include "../../src/basepub/load_libs.h"
}

#include "stub.h"
#include <QTextCodec>
#include <QByteArray>
#include <QMimeDatabase>
#include "qchar.h"
#include "QTextDecoder"
#include "qlocale.h"


namespace utilsstub {
QChar::Script scriptvalue = QChar::Script_Hiragana;
QChar::Script scriptstub()
{
    return scriptvalue;
}

QLocale::Script scriptvalue2 =  QLocale::ArabicScript;
QLocale::Script scriptstub2()
{
    return scriptvalue2;
}

bool rettruestub()
{
    return true;
}

bool retfalsestub()
{
    return false;
}

ushort unicodestub()
{
    return 0x0000;
}

QString namevalue;
QString namestub()
{
    return namevalue;
}

int retintstub()
{
    return 2;
}

}

using namespace utilsstub;

UT_Utils::UT_Utils()
{
}

void UT_Utils::SetUp()
{
    utils = new Utils;
    EXPECT_NE(utils, nullptr);
}

void UT_Utils::TearDown()
{
    delete utils;
    utils = nullptr;
}

//static QString getQrcPath(const QString &imageName);
TEST(UT_Utils_getQrcPath, UT_Utils_getQrcPath)
{
    EXPECT_NE(Utils::getQrcPath("aa").contains("aa"), false);

}

//static QString getQssPath(const QString &qssName);
TEST(UT_Utils_getQssPath, UT_Utils_getQssPath)
{
    EXPECT_NE(Utils::getQssPath("aa").contains("aa"), false);

}

//static QSize getRenderSize(int fontSize, const QString &string);
TEST(UT_Utils_getRenderSize, UT_Utils_getRenderSize)
{
    EXPECT_NE(Utils::getRenderSize(1, "aa").width(), 0);

}

//static void setFontSize(QPainter &painter, int textSize);
TEST(UT_Utils_setFontSize, UT_Utils_setFontSize)
{
    QImage image(10, 10, QImage::Format_RGB888);
    QPainter painter(&image);
    Utils::setFontSize(painter, 1);
    EXPECT_NE(painter.font().pixelSize(), 0);

}

//static void applyQss(QWidget *widget, const QString &qssName);
TEST(UT_Utils_applyQss, UT_Utils_applyQss)
{
    QWidget *widget = new QWidget;
    Utils::applyQss(widget, "1");

    EXPECT_NE(widget, nullptr);
    widget->deleteLater();
}

//static QString getKeyshortcut(QKeyEvent *keyEvent);
TEST(UT_Utils_getKeyshortcut, UT_Utils_getKeyshortcut)
{
    //QKeyEvent::Type
    QKeyEvent *keyEvent = new QKeyEvent(QEvent::KeyPress, 1, Qt::NoModifier);
    EXPECT_EQ(Utils::getKeyshortcut(keyEvent).contains("+"), false);

}

//static QString getKeyshortcutFromKeymap(Settings* settings, const QString &keyCategory, const QString &keyName);
TEST(UT_Utils_getKeyshortcutFromKeymap, UT_Utils_getKeyshortcutFromKeymap)
{
    EXPECT_NE(Utils::getKeyshortcutFromKeymap(Settings::instance(), "editor", "selectall"), " ");

}

//static bool fileExists(const QString &path);
TEST(UT_Utils_fileExists, UT_Utils_fileExists)
{
    EXPECT_NE(Utils::fileExists("aa"), true);
}

//static bool fileIsWritable(const QString &path);
TEST(UT_Utils_fileIsWritable, UT_Utils_fileIsWritable)
{
    EXPECT_NE(Utils::fileIsWritable("aa"), true);
}

//static bool fileIsHome(const QString &path);
TEST(UT_Utils_fileIsHome, UT_Utils_fileIsHome)
{
    EXPECT_NE(Utils::fileIsHome("aa"), true);

}

//static void passInputEvent(int wid);

//static QPixmap dropShadow(const QPixmap &source, qreal radius, const QColor &color, const QPoint &offset);
//static QImage dropShadow(const QPixmap &px, qreal radius, const QColor &color);
TEST(UT_Utils_dropShadow, UT_Utils_dropShadow)
{
    QImage image(10, 10, QImage::Format_RGB888);
    EXPECT_NE(Utils::dropShadow(QPixmap::fromImage(image), 1.5, QColor("#000000"), QPoint(1, 1)).isNull(), true);
    Utils::dropShadow(QPixmap::fromImage(image), 1.5, QColor("#000000"));

}

//static QByteArray detectEncode(const QByteArray &data, const QString &fileName = QString());
TEST(UT_Utils_detectEncode, UT_Utils_detectEncode_001)
{
    EXPECT_NE(Utils::detectEncode("aa").size(), 0);
}

TEST(UT_Utils_detectEncode, UT_Utils_detectEncode_002)
{
    Utils *utils = new Utils;
    QByteArray array;
    array.clear();
    EXPECT_NE(utils->detectEncode(array).size(), 0);

    delete utils;
    utils = nullptr;

}

QTextCodec *stub_codecForUtfText(const QByteArray &ba, QTextCodec *defaultCodec)
{
    return nullptr;
}

TEST(UT_Utils_detectEncode, UT_Utils_detectEncode_003)
{
//    Utils* utils = new Utils;

//    typedef QTextCodec *(*fptr)(const QByteArray &, QTextCodec *);
//    fptr A_foo = (fptr)(&QTextCodec::codecForUtfText);

//    Stub st;
//    st.set(A_foo, stub_codecForUtfText);
//    QByteArray array("-");
//    EXPECT_NE(utils->detectEncode(array).size(),0);

//    delete utils;
//    utils = nullptr;
}

QMimeType stub_mimeTypeForData(const QByteArray &data)
{
    QMimeType type;
    QMimeDatabase base;
    type = base.mimeTypeForName("application/xml");
    return type;
}
TEST(UT_Utils_detectEncode, UT_Utils_detectEncode_004)
{
//    Utils* utils = new Utils;
//    Stub st;
//    st.set((QMimeType(QMimeDatabase::*)(const QByteArray &) const)ADDR(QMimeDatabase, mimeTypeForData), stub_mimeTypeForData);
//    EXPECT_NE(utils->detectEncode("aa", nullptr).size(),0);

//    delete utils;
//    utils = nullptr;
}

TEST(UT_Utils_detectEncode, UT_Utils_detectEncode_005)
{
//    Utils* utils = new Utils;

//    typedef QTextCodec *(*fptr)(const QByteArray &, QTextCodec *);
//    fptr A_foo = (fptr)(&QTextCodec::codecForUtfText);

//    Stub st;
//    st.set(A_foo, stub_codecForUtfText);

//    Stub s1;
//    s1.set(ADDR(QMimeType,name),namestub);
//    namevalue = "text/x-python";

//    QByteArray array("-");
//    EXPECT_NE(utils->detectEncode(array).size(),0);

//    delete utils;
//    utils = nullptr;
}

TEST(UT_Utils_detectEncode, UT_Utils_detectEncode_006)
{
//    Utils* utils = new Utils;

//    typedef QTextCodec *(*fptr)(const QByteArray &, QTextCodec *);
//    fptr A_foo = (fptr)(&QTextCodec::codecForUtfText);

//    Stub st;
//    st.set(A_foo, stub_codecForUtfText);

//    Stub s1;
//    s1.set(ADDR(QMimeType,name),namestub);
//    namevalue = "application/xml";

//    Stub s2;
//    s2.set(ADDR(QString,size),retintstub);

//    Stub s3;
//    s3.set((QLocale::Script(QLocale::*)() const )ADDR(QLocale,script), scriptstub2);
//    scriptvalue2 = QLocale::ArabicScript;


//    QByteArray array("-");
//    EXPECT_NE(utils->detectEncode(array).size(),0);

//    delete utils;
//    utils = nullptr;
}

TEST(UT_Utils_detectEncode, UT_Utils_detectEncode_007)
{
    Utils *utils = new Utils;

    typedef QTextCodec *(*fptr)(const QByteArray &, QTextCodec *);
    fptr A_foo = (fptr)(&QTextCodec::codecForUtfText);

    Stub st;
    st.set(A_foo, stub_codecForUtfText);

    Stub s1;
    s1.set(ADDR(QMimeType, name), namestub);
    namevalue = "application/xml";

    Stub s2;
    s2.set(ADDR(QString, size), retintstub);

    Stub s3;
    s3.set((QLocale::Script(QLocale::*)() const)ADDR(QLocale, script), scriptstub2);
    scriptvalue2 = QLocale::SimplifiedChineseScript;

    QByteArray array("-");
    EXPECT_NE(utils->detectEncode(array).size(), 0);

    scriptvalue2 = QLocale::TraditionalChineseScript;
    EXPECT_NE(utils->detectEncode(array).size(), 0);

    scriptvalue2 = QLocale::CyrillicScript;
    EXPECT_NE(utils->detectEncode(array).size(), 0);

    scriptvalue2 = QLocale::GreekScript;
    EXPECT_NE(utils->detectEncode(array).size(), 0);

    scriptvalue2 = QLocale::HebrewScript;
    EXPECT_NE(utils->detectEncode(array).size(), 0);

    scriptvalue2 = QLocale::JapaneseScript;
    EXPECT_NE(utils->detectEncode(array).size(), 0);

    scriptvalue2 = QLocale::KoreanScript;
    EXPECT_NE(utils->detectEncode(array).size(), 0);

    scriptvalue2 = QLocale::ThaiScript;
    EXPECT_NE(utils->detectEncode(array).size(), 0);

    scriptvalue2 = QLocale::AvestanScript;
    EXPECT_NE(utils->detectEncode(array).size(), 0);


    delete utils;
    utils = nullptr;
}

//static QByteArray getEncode(const QByteArray &data);
TEST(UT_Utils_getEncode, UT_Utils_getEncode)
{
    EXPECT_NE(Utils::getEncode("aa").isEmpty(), true);

}

//static qreal easeInOut(qreal x);
TEST(UT_Utils_easeInOut, UT_Utils_easeInOut)
{
    EXPECT_NE(Utils::easeInOut(0.1), 0);
}

//static qreal easeInQuad(qreal x);
TEST(UT_Utils_easeInQuad, UT_Utils_easeInQuad)
{
    EXPECT_NE(Utils::easeInQuad(0.1), 0);
}

//static qreal easeInQuint(qreal x);
TEST(UT_Utils_easeInQuint, UT_Utils_easeInQuint)
{
    EXPECT_NE(Utils::easeInQuint(0.1), 0);
}

//static qreal easeOutQuad(qreal x);
TEST(UT_Utils_easeOutQuad, UT_Utils_easeOutQuad)
{
    EXPECT_NE(Utils::easeOutQuad(0.1), 0);
}

//static qreal easeOutQuint(qreal x);
TEST(UT_Utils_easeOutQuint, UT_Utils_easeOutQuint)
{
    EXPECT_NE(Utils::easeOutQuint(0.1), 0);
}

//static QVariantMap getThemeMapFromPath(const QString &filepath);
TEST(UT_Utils_getThemeMapFromPath, UT_Utils_getThemeMapFromPath)
{
    EXPECT_NE(Utils::getThemeMapFromPath("aa").isEmpty(), false);
}

//static bool isMimeTypeSupport(const QString &filepath);
TEST(UT_Utils_isMimeTypeSupport, UT_Utils_isMimeTypeSupport)
{
    EXPECT_NE(Utils::isMimeTypeSupport("aa"), false);
}

//static bool isDraftFile(const QString &filepath);
TEST(UT_Utils_isDraftFile, UT_Utils_isDraftFile)
{
    EXPECT_NE(Utils::isDraftFile("aa"), true);
}

//static const QStringList getEncodeList();
TEST(UT_Utils_getEncodeList, UT_Utils_getEncodeList)
{
    Utils::getEncodeList();
    EXPECT_NE(Utils::renderSVG("", QSize(40, 40), false).isNull(), false);

}

TEST(UT_Utils_codecConfidenceForData, UT_Utils_codecConfidenceForData_001)
{
//    QByteArray data = "123";
//    QTextCodec *codec = QTextCodec::codecForName("KOI8-R");
//    QLocale::Country country= QLocale::China;

//    //Script script() const
//    Stub s1;
//    s1.set((QChar::Script(QChar::*)() const )ADDR(QChar,script), scriptstub);

//    scriptvalue = QChar::Script_Hiragana;
//    Utils::codecConfidenceForData(codec,data,country);

//    scriptvalue = QChar::Script_Han;
//    Utils::codecConfidenceForData(codec,data,country);

//    scriptvalue = QChar::Script_Hangul;
//    Utils::codecConfidenceForData(codec,data,country);

//    scriptvalue = QChar::Script_Cyrillic;
//    Utils::codecConfidenceForData(codec,data,country);

//    scriptvalue = QChar::Script_Devanagari;
//    EXPECT_NE(Utils::codecConfidenceForData(codec,data,country),2.2);
}

TEST(UT_Utils_codecConfidenceForData, UT_Utils_codecConfidenceForData_002)
{
//    QByteArray data = "123";
//    //QByteArray data(1,0xfffe);
//    QTextCodec *codec = QTextCodec::codecForName("KOI8-R");
//    QLocale::Country country= QLocale::China;

//    //Script script() const
//    Stub s1;
//    s1.set((QChar::Script(QChar::*)() const )ADDR(QChar,script), scriptstub);
//    Stub s2;
//    s2.set((bool (QChar::*)() const )ADDR(QChar,isSurrogate), rettruestub);
//    Stub s3;
//    s3.set((bool (QChar::*)() const )ADDR(QChar,isHighSurrogate), rettruestub);
//    Stub s4;
//    s4.set((ushort (QChar::*)() const )ADDR(QChar,unicode), unicodestub);
//    Stub s5;
//    s5.set((ushort& (QChar::*)() )ADDR(QChar,unicode), unicodestub);

//    scriptvalue = QChar:: Script_Buhid;
//    EXPECT_NE(Utils::codecConfidenceForData(codec,data,country),2.2);
}


TEST(UT_Utils_clearChildrenFoucusEx, clearChildrenFoucusEx)
{
    QWidget *wgt = new QWidget;
    QPushButton *btn = new QPushButton(wgt);

    Utils::clearChildrenFoucusEx(wgt);

    EXPECT_NE(wgt, nullptr);
    EXPECT_NE(btn, nullptr);

    wgt->deleteLater();
    btn->deleteLater();

}

TEST(UT_Utils_setChildrenFocus, setChildrenFocus)
{
    QWidget *wgt = new QWidget;
    QPushButton *btn = new QPushButton(wgt);

    Utils::setChildrenFocus(wgt, Qt::NoFocus);

    EXPECT_NE(wgt, nullptr);
    EXPECT_NE(btn, nullptr);

    wgt->deleteLater();
    btn->deleteLater();

}

TEST(UT_Utils_getProcessCountByName, getProcessCountByName)
{
    char a[10] = {"12345"};
    Utils::getProcessCountByName(a);

    EXPECT_NE(a[0], '2');
}

TEST(UT_Utils_killProcessByName, killProcessByName)
{
    char a[10] = {"12345"};
    Utils::killProcessByName(a);

    EXPECT_NE(a[0], '2');
}


TEST(UT_Utils_isShareDirAndReadOnly, isShareDirAndReadOnly)
{
//    Stub s1;
//    s1.set((bool(QDir::*)() const)ADDR(QDir,exists),rettruestub);

//    Stub s2;
//    s2.set((bool(QDir::*)(const QString&) const)ADDR(QDir,exists),rettruestub);


//    typedef bool (*fptr)(QFile*,QFile::OpenMode);
//    fptr A_foo = (fptr)((bool(QFile::*)(QFile::OpenMode))&QFile::open);
//    Stub s3;
//    s3.set(A_foo,rettruestub);

//    EXPECT_NE(Utils::isShareDirAndReadOnly("1/2/3"),true);
}

// static RegionIntersectType checkRegionIntersect(int x1, int y1, int x2, int y2);
TEST(UT_Utils_checkRegionIntersect, checkRegionIntersect)
{
    // 测试各边界判断
    Utils::RegionIntersectType type;
    type = Utils::checkRegionIntersect(10, 20, 0, 9);
    ASSERT_EQ(type, Utils::ELeft);
    type = Utils::checkRegionIntersect(10, 20, 0, 10);
    ASSERT_NE(type, Utils::ELeft);

    type = Utils::checkRegionIntersect(10, 20, 21, 30);
    ASSERT_EQ(type, Utils::ERight);
    type = Utils::checkRegionIntersect(10, 20, 20, 30);
    ASSERT_NE(type, Utils::ERight);

    type = Utils::checkRegionIntersect(10, 20, 9, 15);
    ASSERT_EQ(type, Utils::EIntersectLeft);
    type = Utils::checkRegionIntersect(10, 20, 15, 21);
    ASSERT_EQ(type, Utils::EIntersectRight);
    type = Utils::checkRegionIntersect(10, 20, 9, 21);
    ASSERT_EQ(type, Utils::EIntersectOutter);
    type = Utils::checkRegionIntersect(10, 20, 10, 20);
    ASSERT_EQ(type, Utils::EIntersectInner);
}

QByteArray supportEncoding_readAll_stub()
{
    return QByteArray();
}

TEST(UT_Utils_getSupportEncoding, getSupportEncoding)
{
    auto encoding = Utils::getSupportEncoding();
    ASSERT_FALSE(encoding.isEmpty());
}

TEST(UT_Utils_getSupportEncoding, getSupportEncodingWithError)
{
    Stub stub1;
    typedef QVector<QPair<QString, QStringList> > VecType;
    stub1.set(ADDR(VecType, isEmpty), supportEncoding_readAll_stub);
    Stub stub2;
    stub2.set(ADDR(QIODevice, readAll), supportEncoding_readAll_stub);

    auto encoding = Utils::getSupportEncoding();
    ASSERT_TRUE(encoding.isEmpty());
}

TEST(UT_Utils_getSupportEncodingList, getSupportEncodingList)
{
    QStringList encodingList = Utils::getSupportEncodingList();
    ASSERT_FALSE(encodingList.isEmpty());
    ASSERT_TRUE(encodingList.contains("UTF-8"));
}

void uos_document_clip_copy_false(const char *path, int *intercept)
{
    Q_UNUSED(path)
    if (intercept) {
        *intercept = 1;
    }
}
#if _ZPD_
TEST(UT_Utils_zpdLib, enableClipCopy_notLoad_True)
{
    EXPECT_TRUE(Utils::enableClipCopy(""));
    EXPECT_TRUE(Utils::enableClipCopy(QString::null));

    getLoadZPDLibsInstance()->m_document_clip_copy = &uos_document_clip_copy_false;
    EXPECT_FALSE(Utils::enableClipCopy(""));

    getLoadZPDLibsInstance()->m_document_clip_copy = nullptr;
}
#endif
