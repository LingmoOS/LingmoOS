// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QtTest/QtTest>
#include <gtest/gtest.h>

#include <QJsonDocument>

#include <DPrintPreviewDialog>

#include "service/permissionconfig.h"
#include "imageengine.h"

#ifdef DTKWIDGET_CLASS_DWaterMarkHelper

class UT_PermissionConfig : public ::testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;
};

void UT_PermissionConfig::SetUp()
{
    PermissionConfig::instance()->valid = true;
    PermissionConfig::instance()->useWaterMarkPlugin = false;
    PermissionConfig::instance()->targetImagePath.clear();
    PermissionConfig::instance()->currentImagePath.clear();
}

void UT_PermissionConfig::TearDown()
{
    PermissionConfig::instance()->valid = false;
    PermissionConfig::instance()->useWaterMarkPlugin = false;
}

static QString permission_config()
{
    QByteArray permission("{ \"permission\": { \"edit\": true, \"copy\": true, \"setWallpaper\": true, "
                          "\"pictureSwitch\": true, \"printCount\": 2, "
                          "\"ignoreDevicePixelRatio\": true, \"breakPrintSpacingLimit\": true } }");
    return permission.toBase64();
}

TEST_F(UT_PermissionConfig, parseConfigOption_NormalParam_Pass)
{
    QString configParam;
    QStringList imageList;
    QString tempPath = QApplication::applicationDirPath() + "/svg.svg";
    QStringList tmpList{tempPath};
    QStringList arguments{qApp->arguments().first(), "--config=test", tempPath};

    EXPECT_TRUE(PermissionConfig::instance()->parseConfigOption(arguments, configParam, imageList));
    EXPECT_EQ(configParam, QString("test"));
    EXPECT_EQ(imageList, tmpList);

    QStringList arguments2{qApp->arguments().first(), "--config", "test", tempPath};
    EXPECT_TRUE(PermissionConfig::instance()->parseConfigOption(arguments2, configParam, imageList));
    EXPECT_EQ(configParam, QString("test"));
}

TEST_F(UT_PermissionConfig, parseConfigOption_NormalParam_Fail)
{
    QString configParam;
    QStringList imageList;
    QString tempPath = QApplication::applicationDirPath() + "/svg.svg";
    QStringList arguments{qApp->arguments().first(), "config", "test", tempPath};

    EXPECT_FALSE(PermissionConfig::instance()->parseConfigOption({}, configParam, imageList));
    EXPECT_TRUE(configParam.isEmpty());
    EXPECT_TRUE(imageList.isEmpty());

    QStringList tmpList{"config", "test", tempPath};
    EXPECT_FALSE(PermissionConfig::instance()->parseConfigOption(arguments, configParam, imageList));
    EXPECT_TRUE(configParam.isEmpty());
    EXPECT_EQ(imageList, tmpList);
}

TEST_F(UT_PermissionConfig, initFromArguments_NormalParam_Pass)
{
    PermissionConfig::instance()->valid = false;
    PermissionConfig::instance()->authFlags = PermissionConfig::NoAuth;

    QString tempPath = QApplication::applicationDirPath() + "/svg.svg";
    QStringList args;
    args << qApp->arguments().first() << QString("--config=%1").arg(permission_config()) << tempPath;

    PermissionConfig::instance()->initFromArguments(args);
    EXPECT_TRUE(PermissionConfig::instance()->isValid());

    PermissionConfig::Authorises auth =
        PermissionConfig::Authorises(PermissionConfig::EnableCopy | PermissionConfig::EnableEdit |
                                     PermissionConfig::EnableSwitch | PermissionConfig::EnableWallpaper);
    EXPECT_EQ(PermissionConfig::instance()->authFlags, auth);
    EXPECT_EQ(PermissionConfig::instance()->targetImagePath, tempPath);

    EXPECT_EQ(PermissionConfig::instance()->ignoreDevicePixelRatio, true);
    EXPECT_EQ(PermissionConfig::instance()->breakPrintSpacingLimit, true);
}

TEST_F(UT_PermissionConfig, initFromArguments_NormalParam_Fail)
{
    PermissionConfig::instance()->valid = false;
    PermissionConfig::instance()->authFlags = PermissionConfig::NoAuth;

    PermissionConfig::instance()->initFromArguments({});
    EXPECT_FALSE(PermissionConfig::instance()->isValid());
    EXPECT_EQ(PermissionConfig::instance()->authFlags, PermissionConfig::NoAuth);
    EXPECT_TRUE(PermissionConfig::instance()->targetImagePath.isEmpty());
}

TEST_F(UT_PermissionConfig, checkAuthFlag_NotInit_Pass)
{
    PermissionConfig::instance()->authFlags = PermissionConfig::Authorises(0b11000000111111);
    EXPECT_TRUE(PermissionConfig::instance()->checkAuthFlag(PermissionConfig::EnableEdit));
    EXPECT_TRUE(PermissionConfig::instance()->checkAuthFlag(PermissionConfig::EnableCopy));
    EXPECT_TRUE(PermissionConfig::instance()->checkAuthFlag(PermissionConfig::EnableDelete));
    EXPECT_TRUE(PermissionConfig::instance()->checkAuthFlag(PermissionConfig::EnableRename));
    EXPECT_TRUE(PermissionConfig::instance()->checkAuthFlag(PermissionConfig::EnableSwitch));
    EXPECT_TRUE(PermissionConfig::instance()->checkAuthFlag(PermissionConfig::EnableWallpaper));
    EXPECT_TRUE(PermissionConfig::instance()->checkAuthFlag(PermissionConfig::EnableReadWaterMark));
    EXPECT_TRUE(PermissionConfig::instance()->checkAuthFlag(PermissionConfig::EnablePrintWaterMark));
    EXPECT_TRUE(PermissionConfig::instance()->hasReadWaterMark());
    EXPECT_TRUE(PermissionConfig::instance()->hasPrintWaterMark());

    PermissionConfig::instance()->currentImagePath = "1.png";
    PermissionConfig::instance()->targetImagePath = "1.png";
    EXPECT_FALSE(PermissionConfig::instance()->checkAuthInvalid());
    EXPECT_TRUE(PermissionConfig::instance()->checkAuthInvalid("2.png"));
}

TEST_F(UT_PermissionConfig, checkAuthFlag_NotInit_Fail)
{
    EXPECT_TRUE(PermissionConfig::instance()->valid);
    PermissionConfig::instance()->authFlags = PermissionConfig::NoAuth;
    EXPECT_FALSE(PermissionConfig::instance()->checkAuthFlag(PermissionConfig::EnableEdit));
    EXPECT_FALSE(PermissionConfig::instance()->checkAuthFlag(PermissionConfig::EnableCopy));
    EXPECT_FALSE(PermissionConfig::instance()->checkAuthFlag(PermissionConfig::EnableDelete));
    EXPECT_FALSE(PermissionConfig::instance()->checkAuthFlag(PermissionConfig::EnableRename));
    EXPECT_FALSE(PermissionConfig::instance()->checkAuthFlag(PermissionConfig::EnableSwitch));
    EXPECT_FALSE(PermissionConfig::instance()->checkAuthFlag(PermissionConfig::EnableWallpaper));
    EXPECT_FALSE(PermissionConfig::instance()->checkAuthFlag(PermissionConfig::EnableReadWaterMark));
    EXPECT_FALSE(PermissionConfig::instance()->checkAuthFlag(PermissionConfig::EnablePrintWaterMark));
    EXPECT_FALSE(PermissionConfig::instance()->hasReadWaterMark());
    EXPECT_FALSE(PermissionConfig::instance()->hasPrintWaterMark());
}

TEST_F(UT_PermissionConfig, print_Count_Pass)
{
    QString tmpPath("/tmp/tmp.png");
    PermissionConfig::instance()->targetImagePath = tmpPath;

    PermissionConfig::instance()->printLimitCount = 1;
    EXPECT_EQ(PermissionConfig::instance()->printLimitCount, 1);
    EXPECT_TRUE(PermissionConfig::instance()->isPrintable(tmpPath));

    PermissionConfig::instance()->triggerPrint(tmpPath);
    EXPECT_EQ(PermissionConfig::instance()->printLimitCount, 0);
    EXPECT_FALSE(PermissionConfig::instance()->isPrintable(tmpPath));

    PermissionConfig::instance()->printLimitCount = -1;
    EXPECT_TRUE(PermissionConfig::instance()->isPrintable(tmpPath));
    PermissionConfig::instance()->triggerPrint(tmpPath);
    EXPECT_EQ(PermissionConfig::instance()->printLimitCount, -1);
    EXPECT_TRUE(PermissionConfig::instance()->isUnlimitPrint());
}

// DTKWidget 主线和定制线的水印接口不同，通过版本进行区分
// 主线水印接口在 5.6.9 之后引入.
// 因此，判断定制线：存在水印接口，版本不低于 5.4.42.7 且低于 5.5
#if DTK_VERSION_CHECK(5, 4, 42, 7) <= DTK_VERSION && DTK_VERSION < DTK_VERSION_CHECK(5, 5, 0, 0)

TEST_F(UT_PermissionConfig, watarMark_JsonData_Pass_5_4_42)
{
    // Default value
    auto markData = PermissionConfig::instance()->readWaterMarkData();
    EXPECT_EQ(markData.type, WaterMarkType::None);
    EXPECT_EQ(markData.opacity, 1);
    EXPECT_EQ(markData.layout, WaterMarkLayout::Center);
    EXPECT_TRUE(markData.text.isEmpty());

    QJsonObject param{{"text", "test"}, {"opacity", 0}, {"layout", 1}};

    PermissionConfig::instance()->initReadWaterMark(param);
    markData = PermissionConfig::instance()->readWaterMarkData();
    EXPECT_EQ(markData.type, WaterMarkType::Text);
    EXPECT_EQ(markData.opacity, 0);
    EXPECT_EQ(markData.layout, WaterMarkLayout::Tiled);
    EXPECT_EQ(markData.text, QString("test"));

    PermissionConfig::instance()->initPrintWaterMark(param);
    markData = PermissionConfig::instance()->printWaterMarkData();
    EXPECT_EQ(markData.type, WaterMarkType::Text);
}

#else

TEST_F(UT_PermissionConfig, watarMark_JsonData_Pass)
{
    // Default value
    auto markData = PermissionConfig::instance()->readWaterMarkData();
    EXPECT_EQ(markData.type(), WaterMarkData::WaterMarkType::None);
    EXPECT_EQ(markData.opacity(), 1);
    EXPECT_EQ(markData.layout(), WaterMarkData::WaterMarkLayout::Center);
    EXPECT_TRUE(markData.text().isEmpty());

    QJsonObject param{{"text", "test"}, {"opacity", 0}, {"layout", 1}};

    PermissionConfig::instance()->initReadWaterMark(param);
    markData = PermissionConfig::instance()->readWaterMarkData();
    EXPECT_EQ(markData.type(), WaterMarkData::WaterMarkType::Text);
    EXPECT_EQ(markData.opacity(), 0);
    EXPECT_EQ(markData.layout(), WaterMarkData::WaterMarkLayout::Tiled);
    EXPECT_EQ(markData.text(), QString("test"));

    PermissionConfig::instance()->initPrintWaterMark(param);
    markData = PermissionConfig::instance()->printWaterMarkData();
    EXPECT_EQ(markData.type(), WaterMarkData::WaterMarkType::Text);
}
#endif  // VERSION CHECK

TEST_F(UT_PermissionConfig, triggerAction_SignalNotify_Pass)
{
    QJsonObject notifyData;
    QObject::connect(PermissionConfig::instance(), &PermissionConfig::authoriseNotify, [&](const QJsonObject &notify) {
        notifyData = notify;
    });

    auto TriggerFunction = [&](PermissionConfig::TidType tid, const QString &operate) {
        // PermissionConfig::targetImagePath is empty.
        PermissionConfig::instance()->triggerAction(tid, "");
        // ReportMode::ReportAndBroadcast = 0b11
        auto policyData = notifyData.value("policy").toObject();
        EXPECT_EQ(policyData.value("reportMode").toInt(), 3);

        QJsonObject data = notifyData.value("info").toObject();
        EXPECT_EQ(data.value("tid").toInt(), tid);
        EXPECT_EQ(data.value("operate").toString(), operate);
    };

    PermissionConfig::instance()->status = PermissionConfig::NotOpen;

    TriggerFunction(PermissionConfig::TidOpen, "open");
    TriggerFunction(PermissionConfig::TidEdit, "edit");
    TriggerFunction(PermissionConfig::TidCopy, "copy");
    // Close will set PermissionConfig::valid to false.
    TriggerFunction(PermissionConfig::TidClose, "close");
    EXPECT_FALSE(PermissionConfig::instance()->isValid());
    EXPECT_EQ(PermissionConfig::instance()->status, PermissionConfig::Close);
}

TEST_F(UT_PermissionConfig, hasPrintWaterMark_usingWaterMark_Pass)
{
    PermissionConfig::instance()->authFlags = PermissionConfig::EnablePrintWaterMark;
    EXPECT_TRUE(PermissionConfig::instance()->hasPrintWaterMark());

    PermissionConfig::instance()->useWaterMarkPlugin = true;
    EXPECT_FALSE(PermissionConfig::instance()->hasPrintWaterMark());
}

TEST_F(UT_PermissionConfig, detectWaterMarkPluginExists_Pass)
{
    PermissionConfig::instance()->detectWaterMarkPluginExists();

    QStringList plugins = DPrintPreviewDialog::availablePlugins();
    if (plugins.contains("WaterMarkFilter")) {
        EXPECT_TRUE(PermissionConfig::instance()->useWaterMarkPlugin);
    } else {
        EXPECT_FALSE(PermissionConfig::instance()->useWaterMarkPlugin);
    }
}

TEST_F(UT_PermissionConfig, initWaterMarkPluginEnvironment_CheckPlugin_Pass)
{
    DWIDGET_USE_NAMESPACE
    QStringList plugins = DPrintPreviewDialog::availablePlugins();
    if (plugins.contains("WaterMarkFilter")) {
        EXPECT_TRUE(PermissionConfig::instance()->initWaterMarkPluginEnvironment());
    } else {
        EXPECT_FALSE(PermissionConfig::instance()->initWaterMarkPluginEnvironment());
    }

    QByteArray envData = qgetenv("DEEPIN_WATERMARK");
    EXPECT_FALSE(envData.isEmpty());
}

TEST_F(UT_PermissionConfig, initWaterMarkPluginEnvironment_CheckValue)
{
    static const qreal sc_defaultFontSize = 65.0;
    // 验证打印水印设置值
#if DTK_VERSION_CHECK(5, 4, 42, 7) <= DTK_VERSION && DTK_VERSION < DTK_VERSION_CHECK(5, 5, 0, 0)
    WaterMarkData &watermark = PermissionConfig::instance()->printWaterMark;
    watermark.layout = WaterMarkLayout::Center;
#else
    PermissionConfig::AdapterWaterMarkData &watermark = PermissionConfig::instance()->printAdapterWaterMark;
    watermark.layout = PermissionConfig::AdapterWaterMarkData::Center;
#endif // VERSION CHECK
    watermark.rotation = 45;
    watermark.opacity = 0.3;
    watermark.font.setPointSize(30);
    watermark.text = "123";
    watermark.lineSpacing = 0;
    watermark.spacing = 0;
    watermark.color = "#FF00FF";

    // 强制设置环境变量
    PermissionConfig::instance()->initWaterMarkPluginEnvironment();

    QByteArray envData = qgetenv("DEEPIN_WATERMARK");
    QJsonDocument doc = QJsonDocument::fromJson(envData);
    QJsonObject root = doc.object();

    EXPECT_EQ(watermark.rotation, root.value("angle").toDouble());
    EXPECT_EQ(watermark.opacity, root.value("transparency").toDouble() / 100);
    EXPECT_EQ(watermark.text, root.value("customText").toString());
    EXPECT_EQ(watermark.color, root.value("textColor").toString());
    EXPECT_EQ(DPrintPreviewWatermarkInfo::TextWatermark, root.value("watermarkType").toInt());
    EXPECT_EQ(DPrintPreviewWatermarkInfo::Custom, root.value("textType").toInt());
    EXPECT_EQ(DPrintPreviewWatermarkInfo::Center, root.value("layout").toInt());

    qreal realFontSize = (root.value("size").toDouble() * sc_defaultFontSize) / 100.0;
    EXPECT_EQ(watermark.font.pointSize(), qFloor(realFontSize + 0.5));
    EXPECT_TRUE(qFuzzyCompare(0, root.value("rowSpacing").toDouble()));
    EXPECT_TRUE(qFuzzyCompare(0, root.value("columnSpacing").toDouble()));

    // 计算间距
    watermark.lineSpacing = 100;
    watermark.spacing = 60;
    PermissionConfig::instance()->initWaterMarkPluginEnvironment();
    envData = qgetenv("DEEPIN_WATERMARK");
    doc = QJsonDocument::fromJson(envData);
    root = doc.object();

    QFontMetrics fm(watermark.font);
    QSize textSize = fm.size(Qt::TextSingleLine, watermark.text);
    qreal realRowSpacing = (qreal(watermark.lineSpacing + textSize.height()) / textSize.height()) - 1.0;
    qreal realColumnSpacing = (qreal(watermark.spacing + textSize.width()) / textSize.width()) - 1.0;
    EXPECT_TRUE(qFuzzyCompare(realRowSpacing, root.value("rowSpacing").toDouble()));
    EXPECT_TRUE(qFuzzyCompare(realColumnSpacing, root.value("columnSpacing").toDouble()));
}

TEST_F(UT_PermissionConfig, installFilterPrintDialog_Filter_Disable)
{
    auto ins = PermissionConfig::instance();
    DPrintPreviewDialog dialog;

    ins->breakPrintSpacingLimit = false;
    ins->printRowSpacing = 10.0;
    ins->printColumnSpacing = 2.0;
    EXPECT_FALSE(ins->installFilterPrintDialog(&dialog));

    ins->breakPrintSpacingLimit = true;
    EXPECT_FALSE(ins->installFilterPrintDialog(&dialog));
}

TEST_F(UT_PermissionConfig, installFilterPrintDialog_Filter_Enable)
{
    auto ins = PermissionConfig::instance();
    DPrintPreviewDialog dialog;

    ins->breakPrintSpacingLimit = true;
    ins->printRowSpacing = 11.0;
    ins->printColumnSpacing = 2.0;
    EXPECT_TRUE(ins->installFilterPrintDialog(&dialog));

    ins->printRowSpacing = 0;
    ins->printColumnSpacing = 2.1;
    EXPECT_TRUE(ins->installFilterPrintDialog(&dialog));
}

TEST_F(UT_PermissionConfig, installFilterPrintDialog_FilterData_Pass)
{
    auto ins = PermissionConfig::instance();
    DPrintPreviewDialog dialog;

    ins->breakPrintSpacingLimit = true;
    ins->printRowSpacing = 11.0;
    ins->printColumnSpacing = 2.1;
    EXPECT_TRUE(ins->installFilterPrintDialog(&dialog));

    DPrintPreviewWidget *widget = dialog.findChild<DPrintPreviewWidget *>();
    ASSERT_NE(nullptr, widget);

    widget->setProperty("_d_print_waterMarkRowSpacing", 2.0);
    EXPECT_NE(widget->property("_d_print_waterMarkRowSpacing").toReal(), 2.0);
    EXPECT_TRUE(qFuzzyCompare(widget->property("_d_print_waterMarkRowSpacing").toReal(), 11.0));
    widget->setProperty("_d_print_waterMarkColumnSpacing", 1.0);
    EXPECT_NE(widget->property("_d_print_waterMarkColumnSpacing").toReal(), 2.0);
    EXPECT_TRUE(qFuzzyCompare(widget->property("_d_print_waterMarkColumnSpacing").toReal(), 2.1));

    ins->printRowSpacing = 15.0;
    ins->printColumnSpacing = 12.0;
    widget->setProperty("_d_print_waterMarkRowSpacing", 2.0);
    EXPECT_TRUE(qFuzzyCompare(widget->property("_d_print_waterMarkRowSpacing").toReal(), 15.0));
    widget->setProperty("_d_print_waterMarkColumnSpacing", 1.0);
    EXPECT_TRUE(qFuzzyCompare(widget->property("_d_print_waterMarkColumnSpacing").toReal(), 12.0));
}

TEST_F(UT_PermissionConfig, installFilterPrintDialog_FilterData_Ignore)
{
    auto ins = PermissionConfig::instance();
    DPrintPreviewDialog dialog;

    ins->breakPrintSpacingLimit = true;
    ins->printRowSpacing = 11.0;
    ins->printColumnSpacing = 2.1;
    EXPECT_TRUE(ins->installFilterPrintDialog(&dialog));

    DPrintPreviewWidget *widget = dialog.findChild<DPrintPreviewWidget *>();
    ASSERT_NE(nullptr, widget);

    ins->printRowSpacing = 5.0;
    ins->printColumnSpacing = 1.0;
    widget->setProperty("_d_print_waterMarkRowSpacing", 2.0);
    EXPECT_TRUE(qFuzzyCompare(widget->property("_d_print_waterMarkRowSpacing").toReal(), 2.0));
    widget->setProperty("_d_print_waterMarkColumnSpacing", 1.0);
    EXPECT_TRUE(qFuzzyCompare(widget->property("_d_print_waterMarkColumnSpacing").toReal(), 1.0));
}

#if DTK_VERSION_CHECK(5, 4, 42, 7) <= DTK_VERSION && DTK_VERSION < DTK_VERSION_CHECK(5, 5, 0, 0)

TEST_F(UT_PermissionConfig, convertAdapterWaterMarkData_Equal_Pass)
{
    WaterMarkData data;
    data.type = WaterMarkData::Text;
    data.layout = WaterMarkData::Tiled;
    data.text = "123Test";
    QFont f;
    f.setPointSize(15);
    data.font = f;
    QColor color(Qt::red);
    data.color = color;
    data.lineSpacing = 10;
    data.rotation = 10;

    PermissionConfig::AdapterWaterMarkData adptData;
    adptData.type = PermissionConfig::AdapterWaterMarkData::Text;
    adptData.layout = PermissionConfig::AdapterWaterMarkData::Tiled;
    adptData.text = "123Test";
    adptData.font = f;
    adptData.color = color;
    adptData.lineSpacing = 10;
    adptData.rotation = 10;

    auto cvtData = PermissionConfig::instance()->convertAdapterWaterMarkData(adptData);
    EXPECT_EQ(data.type, cvtData.type);
    EXPECT_EQ(data.layout, cvtData.layout);
    EXPECT_EQ(data.text, cvtData.text);
    EXPECT_EQ(data.font, cvtData.font);
    EXPECT_EQ(data.color, cvtData.color);
    EXPECT_EQ(data.lineSpacing, cvtData.lineSpacing);
    EXPECT_EQ(data.rotation, cvtData.rotation);
}

#else

TEST_F(UT_PermissionConfig, convertAdapterWaterMarkData_Equal_Pass)
{
    WaterMarkData data;
    data.setType(WaterMarkData::Text);
    data.setLayout(WaterMarkData::Tiled);
    data.setText("123Test");
    QFont f;
    f.setPointSize(15);
    data.setFont(f);
    QColor color(Qt::red);
    data.setColor(color);
    data.setLineSpacing(10);
    data.setRotation(10);

    PermissionConfig::AdapterWaterMarkData adptData;
    adptData.type = PermissionConfig::AdapterWaterMarkData::Text;
    adptData.layout = PermissionConfig::AdapterWaterMarkData::Tiled;
    adptData.text = "123Test";
    adptData.font = f;
    adptData.color = color;
    adptData.lineSpacing = 10;
    adptData.rotation = 10;

    auto cvtData = PermissionConfig::instance()->convertAdapterWaterMarkData(adptData);
    EXPECT_EQ(data.type(), cvtData.type());
    EXPECT_EQ(data.layout(), cvtData.layout());
    EXPECT_EQ(data.text(), cvtData.text());
    EXPECT_EQ(data.font(), cvtData.font());
    EXPECT_EQ(data.color(), cvtData.color());
    EXPECT_EQ(data.lineSpacing(), cvtData.lineSpacing());
    EXPECT_EQ(data.rotation(), cvtData.rotation());
}

#endif  // VERSION CHECK

#endif
