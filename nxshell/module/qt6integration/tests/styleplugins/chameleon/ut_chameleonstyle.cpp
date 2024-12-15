/*
 * SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#include <gtest/gtest.h>

#include <DTabBar>
#include <DStyleOptionButton>
#include <DSpinBox>
#include <DStyleOptionButton>

#include <QToolButton>
#include <QTableView>
#include <QMenu>
#include <QCalendarWidget>
#include <QTreeView>
#include <QListView>
#include <QLineEdit>
#include <QRadioButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QStatusBar>
#include <QScrollBar>
#include <QPushButton>
#include <QHeaderView>
#include <QComboBox>
#include <QProgressBar>

#include "chameleonstyle.h"

DWIDGET_USE_NAMESPACE;
using namespace chameleon;

class TestForStyleHintParam : public ::testing::TestWithParam<int>
{
protected:
    void SetUp() override;
    void TearDown() override;

    ChameleonStyle *style;
    QWidget *testWidget;
};

void TestForStyleHintParam::SetUp()
{
    style = new ChameleonStyle;
    testWidget = new QWidget;
}

void TestForStyleHintParam::TearDown()
{
    delete style;
    delete testWidget;
}

INSTANTIATE_TEST_SUITE_P(ChameleonStyle, TestForStyleHintParam, ::testing::Range(int(ChameleonStyle::SH_EtchDisabledText), int(ChameleonStyle::SH_SpinBox_ButtonsInsideFrame)));

TEST_P(TestForStyleHintParam, styleHint)
{
    QStyleOption opt;
    opt.initFrom(testWidget);

    testWidget->setStyle(style);

    int param = GetParam();
    // 测试在调用默认参数时是否会崩溃
    style->styleHint(ChameleonStyle::StyleHint(param));
    // 测试调用有参时是否会崩溃
    style->styleHint(ChameleonStyle::StyleHint(param), &opt, testWidget);
}

class TestForPixelMetricParam : public ::testing::TestWithParam<quint64>
{
protected:
    void SetUp() override;
    void TearDown() override;

    ChameleonStyle *style;
    QWidget *testWidget;
};

void TestForPixelMetricParam::SetUp()
{
    style = new ChameleonStyle;
    testWidget = new QWidget;
}

void TestForPixelMetricParam::TearDown()
{
    delete style;
    delete testWidget;
}

INSTANTIATE_TEST_SUITE_P(ChameleonStyle, TestForPixelMetricParam, ::testing::Range(quint64(ChameleonStyle::PM_FocusBorderWidth), quint64(ChameleonStyle::PM_ButtonMinimizedSize)));

TEST_P(TestForPixelMetricParam, pixelMetric)
{
    QStyleOption opt;
    opt.initFrom(testWidget);

    testWidget->setStyle(style);

    quint64 param = GetParam();
    // 测试在调用默认参数时是否有潜在崩溃风险
    style->pixelMetric(QStyle::PixelMetric(param));
    // 测试调用有参时是否有潜在崩溃风险
    style->pixelMetric(QStyle::PixelMetric(param), &opt, testWidget);
}

class TestForQtDrawControlParam : public ::testing::TestWithParam<int>
{
protected:
    void SetUp() override;
    void TearDown() override;

    ChameleonStyle *style;
    QWidget *testWidget;
};

INSTANTIATE_TEST_SUITE_P(ChameleonStyle, TestForQtDrawControlParam, ::testing::Range(int(QStyle::CE_PushButton), int(QStyle::CE_ShapedFrame)));

TEST_P(TestForQtDrawControlParam, drawControl)
{
    QStyleOption opt;
    opt.initFrom(testWidget);

    // 测试Qt原生drawControl参数调用时是否有潜在崩溃风险
    QPixmap surface(QSize(200, 200));
    QPainter painter(&surface);
    int param = GetParam();

    // DStyle中存在的元素值在DTKWIDGET中使用 这里不再测试调用
    style->drawControl(QStyle::ControlElement(param), &opt, &painter);
}

void TestForQtDrawControlParam::SetUp()
{
    style = new ChameleonStyle;
    testWidget = new QWidget;
}

void TestForQtDrawControlParam::TearDown()
{
    delete style;
    delete testWidget;
}

struct DrawComplexControl_Param {
    explicit DrawComplexControl_Param(QStyle::ComplexControl cc, QStyleOptionComplex *opt)
        : cc(cc)
        , opt(opt)
    {
    }

    void releaseOption()
    {
        switch (cc) {
        case QStyle::CC_SpinBox:
            delete qstyleoption_cast<QStyleOptionSpinBox *>(opt);
            break;
        case QStyle::CC_ToolButton:
            delete qstyleoption_cast<QStyleOptionToolButton *>(opt);
            break;
        case QStyle::CC_Slider:
            delete qstyleoption_cast<QStyleOptionSlider *>(opt);
            break;
        case QStyle::CC_ComboBox:
            delete qstyleoption_cast<QStyleOptionComboBox *>(opt);
            break;
        default:
            delete opt;
        }

        opt = nullptr;
    }

    QStyle::ComplexControl cc;
    QStyleOptionComplex *opt;
};

class TestForQtDrawComplexControlParam : public ::testing::TestWithParam<DrawComplexControl_Param>
{
protected:
    void SetUp() override;
    void TearDown() override;

    ChameleonStyle *style;
    QWidget *testWidget;
};

INSTANTIATE_TEST_SUITE_P(ChameleonStyle, TestForQtDrawComplexControlParam, ::testing::Values(DrawComplexControl_Param(QStyle::CC_SpinBox, new QStyleOptionSpinBox), DrawComplexControl_Param(QStyle::CC_ToolButton, new QStyleOptionToolButton), DrawComplexControl_Param(QStyle::CC_Slider, new QStyleOptionSlider), DrawComplexControl_Param(QStyle::CC_ComboBox, new QStyleOptionComboBox)));

TEST_P(TestForQtDrawComplexControlParam, drawComplexControl)
{
    DrawComplexControl_Param param = GetParam();

    QPixmap surface(QSize(200, 200));
    QPainter p(&surface);

    param.opt->initFrom(testWidget);
    // 测试调用默认参数时函数是否会崩溃
    style->drawComplexControl(param.cc, param.opt, &p);
    param.releaseOption();
}

void TestForQtDrawComplexControlParam::SetUp()
{
    style = new ChameleonStyle;
    testWidget = new QWidget;
}

void TestForQtDrawComplexControlParam::TearDown()
{
    delete style;
    delete testWidget;
}

class TestForQtStandardPixmapParam : public ::testing::TestWithParam<int>
{
};

INSTANTIATE_TEST_SUITE_P(ChameleonStyle, TestForQtStandardPixmapParam, ::testing::Range(int(QStyle::SP_TitleBarMenuButton), int(QStyle::SP_LineEditClearButton)));

TEST_P(TestForQtStandardPixmapParam, StandardPixmap)
{
    ChameleonStyle *style = new ChameleonStyle;
    int param = GetParam();

    qDebug() << "QStyle::standardIcon:" << param << " isNull: " << (style->standardIcon(QStyle::StandardPixmap(param)).isNull());
    qDebug() << "QStyle::standardPixmap:" << param << " isNull: " << (style->standardPixmap(QStyle::StandardPixmap(param)).isNull());

    if (param < int(QStyle::CC_MdiControls))
        qDebug() << "DStyle::hitTestComplexControl" << param << style->hitTestComplexControl(QStyle::ComplexControl(param), nullptr, {0, 0});

    delete style;
}

#define ASSERT_DrawFuncHasData(DrawFunc) \
    painter->begin(&pixmap); \
    DrawFunc(); \
    painter->end(); \
    ASSERT_FALSE(pixmap.isNull()); \
    ASSERT_TRUE(testPixmapHasData()); \
    pixmap.fill(Qt::green)

#define ASSERT_DrawFuncNoData(DrawFunc) \
    painter->begin(&pixmap); \
    DrawFunc(); \
    painter->end(); \
    ASSERT_FALSE(pixmap.isNull()); \
    ASSERT_FALSE(testPixmapHasData()); \
    pixmap.fill(Qt::green)

#define INIT_TESTWIDGET(ClassName) \
    if (testWidget) \
        delete testWidget; \
    testWidget = new ClassName; \
    testWidget->setGeometry(0, 0, 100, 100)

class TestDrawUtilIntegration;
class TestForDrawUtil : public ::testing::Test
{
    friend TestDrawUtilIntegration;

protected:
    void SetUp();
    void TearDown();
    bool testPixmapHasData();

    ChameleonStyle *style;
    QWidget *testWidget = nullptr;
    QPixmap pixmap;
    QPainter *painter;
};

class TestDrawUtilIntegration
{
public:
    TestDrawUtilIntegration(TestForDrawUtil *parent)
        : m_parent(parent)
    {
    }

protected:
    inline QPainter *painter() const
    {
        return m_parent->painter;
    }

    inline QWidget *testWidget() const
    {
        return m_parent->testWidget;
    }

    inline ChameleonStyle *style() const
    {
        return m_parent->style;
    }

    TestForDrawUtil *m_parent;
};

void TestForDrawUtil::SetUp()
{
    style = new ChameleonStyle;
    pixmap = QPixmap(QSize(200, 200));
    painter = new QPainter;

    // 以异色作为初始颜色防止默认随机颜色的干扰
    pixmap.fill(Qt::green);
}

void TestForDrawUtil::TearDown()
{
    delete style;
    delete painter;
    delete testWidget;
}

bool TestForDrawUtil::testPixmapHasData()
{
    QImage image = pixmap.toImage();

    image.reinterpretAsFormat(QImage::Format_RGB32);
    const QRgb *bits = reinterpret_cast<const QRgb *>(image.constBits());
    const QRgb *end = bits + image.sizeInBytes() / sizeof(QRgb);
    return !std::all_of(bits, end, [](QRgb r) { return r == QColor(Qt::green).rgb(); });
}

TEST_F(TestForDrawUtil, drawShadow)
{
    auto drawShaowFunc = [&]() {
        style->drawShadow(painter, QRect(0, 0, 20, 20), Qt::black);
    };

    ASSERT_DrawFuncHasData(drawShaowFunc);
}

TEST_F(TestForDrawUtil, drawBorder)
{
    auto drawBorderFunc = [&]() {
        QStyleOption opt;
        opt.initFrom(testWidget);

        // 测试调用是否存在异常崩溃并且测试函数的绘制操作是否有效
        style->drawBorder(painter, &opt, testWidget);
    };

    INIT_TESTWIDGET(QWidget);
    ASSERT_DrawFuncHasData(drawBorderFunc);
}

TEST_F(TestForDrawUtil, drawCalenderEllipse)
{
    auto drawCalenderEllipseFunc = [&]() {
        // 测试绘制操作是否存在数据
        style->drawCalenderEllipse(painter, QRect(0, 0, 50, 50), 5);
    };

    INIT_TESTWIDGET(QWidget);
    ASSERT_DrawFuncHasData(drawCalenderEllipseFunc);
}

TEST_F(TestForDrawUtil, drawButtonBoxButton)
{
    class TestButtonBoxButtonDrawFunc : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawButtonBoxButonBeginning()
        {
            DStyleOptionButtonBoxButton button;
            button.init(testWidget());
            button.orientation = Qt::Horizontal;
            button.position = DStyleOptionButtonBoxButton::Beginning;

            style()->drawButtonBoxButton(&button, painter(), m_DefaultRect, m_radius);
        }

        void testDrawButtonBoxButtonMiddle()
        {
            DStyleOptionButtonBoxButton button;
            button.init(testWidget());
            button.orientation = Qt::Horizontal;
            button.position = DStyleOptionButtonBoxButton::Middle;

            style()->drawButtonBoxButton(&button, painter(), m_DefaultRect, m_radius);
        }

        void testDrawButtonBoxButtonEnd()
        {
            DStyleOptionButtonBoxButton button;
            button.init(testWidget());
            button.orientation = Qt::Horizontal;
            button.position = DStyleOptionButtonBoxButton::End;

            style()->drawButtonBoxButton(&button, painter(), m_DefaultRect, m_radius);
        }

        void testDrawButtonBoxButtonOnlyOne()
        {
            DStyleOptionButtonBoxButton button;
            button.init(testWidget());
            button.orientation = Qt::Horizontal;
            button.position = DStyleOptionButtonBoxButton::OnlyOne;
            style()->drawButtonBoxButton(&button, painter(), m_DefaultRect, m_radius);
        }

        void testDrawButtonBoxButtonVerticalBeginning()
        {
            DStyleOptionButtonBoxButton button;
            button.init(testWidget());
            button.orientation = Qt::Vertical;
            button.position = DStyleOptionButtonBoxButton::Beginning;
            style()->drawButtonBoxButton(&button, painter(), m_DefaultRect, m_radius);
        }

    private:
        QRect m_DefaultRect = {0, 0, 50, 50};
        int m_radius = 5;
    };

    INIT_TESTWIDGET(QWidget);
    TestButtonBoxButtonDrawFunc drawFuncInstance(this);

    // 多场景触发绘制 测试数据是否存在 并 异于初始数据
    ASSERT_DrawFuncHasData(drawFuncInstance.testDrawButtonBoxButonBeginning);
    ASSERT_DrawFuncHasData(drawFuncInstance.testDrawButtonBoxButtonMiddle);
    ASSERT_DrawFuncHasData(drawFuncInstance.testDrawButtonBoxButtonEnd);
    ASSERT_DrawFuncHasData(drawFuncInstance.testDrawButtonBoxButtonOnlyOne);
    ASSERT_DrawFuncHasData(drawFuncInstance.testDrawButtonBoxButtonVerticalBeginning);
}

TEST_F(TestForDrawUtil, drawButtonDownArrow)
{
    auto drawButtonDownArrowFunc = [&]() {
        QStyleOptionButton opt;
        opt.initFrom(testWidget);

        style->drawButtonDownArrow(&opt, painter, testWidget);
    };

    INIT_TESTWIDGET(QWidget);
    ASSERT_DrawFuncHasData(drawButtonDownArrowFunc);
}

TEST_F(TestForDrawUtil, drawSpinBox)
{
    auto drawSpinBoxPlusMinusFunc = [&]() {
        QStyleOptionSpinBox opt;
        opt.initFrom(testWidget);
        opt.buttonSymbols = QAbstractSpinBox::PlusMinus;

        style->drawSpinBox(&opt, painter, testWidget);
    };

    auto drawSpinBoxUpDownArrows = [&]() {
        QStyleOptionSpinBox opt;
        opt.initFrom(testWidget);
        opt.buttonSymbols = QAbstractSpinBox::UpDownArrows;

        style->drawSpinBox(&opt, painter, testWidget);
    };

    INIT_TESTWIDGET(QWidget);
    testWidget->setProperty("_d_dtk_spinBox", true);
    ASSERT_DrawFuncHasData(drawSpinBoxPlusMinusFunc);
    ASSERT_DrawFuncHasData(drawSpinBoxUpDownArrows);
}

TEST_F(TestForDrawUtil, drawMenuBarItem)
{
    auto drawMenuBarItemMarginAndExclusive = [&]() {
        QStyleOptionMenuItem opt;
        opt.initFrom(testWidget);
        opt.menuItemType = QStyleOptionMenuItem::Margin;
        opt.checkType = QStyleOptionMenuItem::Exclusive;
        opt.text = "测试1";
        opt.icon = style->standardIcon(QStyle::SP_DirIcon);
        opt.menuRect = QRect(20, 20, 60, 40);
        opt.maxIconWidth = 20;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        opt.reservedShortcutWidth = 2;
#else
        opt.tabWidth = 2;
#endif
        QRect ret;
        style->drawMenuBarItem(&opt, ret, painter, testWidget);
    };

    auto drawMenuBarItemNormalAndExclusive = [&]() {
        QStyleOptionMenuItem opt;
        opt.initFrom(testWidget);
        opt.menuItemType = QStyleOptionMenuItem::Normal;
        opt.checkType = QStyleOptionMenuItem::Exclusive;
        opt.text = "测试2";
        opt.icon = style->standardIcon(QStyle::SP_DirIcon);
        opt.menuRect = QRect(20, 20, 60, 40);
        opt.maxIconWidth = 20;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        opt.reservedShortcutWidth = 2;
#else
        opt.tabWidth = 2;
#endif
        opt.state |= QStyle::State_MouseOver;

        QRect ret;
        style->drawMenuBarItem(&opt, ret, painter, testWidget);
    };

    INIT_TESTWIDGET(QWidget);
    ASSERT_DrawFuncHasData(drawMenuBarItemMarginAndExclusive);
    ASSERT_DrawFuncHasData(drawMenuBarItemNormalAndExclusive);
}

TEST_F(TestForDrawUtil, drawMenuItemBackground)
{
    class TestMenuItemBackgroundDrawFunc : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawNormalBackground()
        {
            QStyleOption opt;
            opt.initFrom(testWidget());
            opt.state |= QStyle::State_Selected;

            style()->drawMenuItemBackground(&opt, painter(), QStyleOptionMenuItem::Normal);
        }

        void testDrawDefaultItemBackground()
        {
            QStyleOption opt;
            opt.initFrom(testWidget());
            opt.state |= QStyle::State_MouseOver;

            style()->drawMenuItemBackground(&opt, painter(), QStyleOptionMenuItem::DefaultItem);
        }

        void testDrawSunMenuBackground()
        {
            QStyleOption opt;
            opt.initFrom(testWidget());
            opt.state |= QStyle::State_Sunken;

            style()->drawMenuItemBackground(&opt, painter(), QStyleOptionMenuItem::SubMenu);
        }
    };

    INIT_TESTWIDGET(QWidget);
    TestMenuItemBackgroundDrawFunc drawFuncInstance(this);
    ASSERT_DrawFuncHasData(drawFuncInstance.testDrawNormalBackground);
    ASSERT_DrawFuncHasData(drawFuncInstance.testDrawDefaultItemBackground);
    ASSERT_DrawFuncHasData(drawFuncInstance.testDrawSunMenuBackground);
}

TEST_F(TestForDrawUtil, drawMenuItem)
{
    class TestMenuItemDrawFunc : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawNorMenuItem()
        {
            QStyleOptionMenuItem opt;
            opt.initFrom(testWidget());
            opt.menuItemType = QStyleOptionMenuItem::Normal;

            style()->drawMenuItem(&opt, painter(), testWidget());
        }

        void testDrawSeparatorMenuItem()
        {
            QStyleOptionMenuItem opt;
            opt.initFrom(testWidget());
            opt.menuItemType = QStyleOptionMenuItem::Separator;

            style()->drawMenuItem(&opt, painter(), testWidget());
        }

        void testDrawSubMenuItem()
        {
            QStyleOptionMenuItem opt2;
            opt2.initFrom(testWidget());
            opt2.menuItemType = QStyleOptionMenuItem::SubMenu;

            style()->drawMenuItem(&opt2, painter(), testWidget());
        }
    };

    INIT_TESTWIDGET(QWidget);
    TestMenuItemDrawFunc drawFuncInstance(this);
    ASSERT_DrawFuncHasData(drawFuncInstance.testDrawNorMenuItem);
    ASSERT_DrawFuncHasData(drawFuncInstance.testDrawSeparatorMenuItem);
    ASSERT_DrawFuncHasData(drawFuncInstance.testDrawSubMenuItem);
}

TEST_F(TestForDrawUtil, drawTabBar)
{
    class TestTabBarDrawUtil : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawRoundedEstOnlyOneTab()
        {
            QStyleOptionTab tab;
            tab.initFrom(testWidget());
            tab.state |= (QStyle::State_Selected | QStyle::State_Enabled);
            tab.shape = QTabBar::RoundedEast;
            tab.position = QStyleOptionTab::OnlyOneTab;
            tab.selectedPosition = QStyleOptionTab::NotAdjacent;
            tab.cornerWidgets = QStyleOptionTab::NoCornerWidgets;
            tab.text = "test 1";

            style()->drawTabBar(painter(), &tab, testWidget());
        }

        void testDrawTriangularNorthMiddleTab()
        {
            QStyleOptionTab tab;
            tab.initFrom(testWidget());
            tab.state |= QStyle::State_Enabled;
            tab.shape = QTabBar::TriangularNorth;
            tab.position = QStyleOptionTab::Middle;
            tab.selectedPosition = QStyleOptionTab::NextIsSelected;
            tab.cornerWidgets = QStyleOptionTab::RightCornerWidget;
            tab.text = "test 2";
            tab.icon = QIcon::fromTheme("icon_Layout");
            tab.iconSize = QSize(16, 16);

            style()->drawTabBar(painter(), &tab, testWidget());
        }

        void testDrawRoundedWestBeginningTab()
        {
            QStyleOptionTab tab;
            tab.initFrom(testWidget());
            tab.state |= QStyle::State_Enabled;
            tab.shape = QTabBar::RoundedWest;
            tab.position = QStyleOptionTab::Beginning;
            tab.selectedPosition = QStyleOptionTab::PreviousIsSelected;
            tab.cornerWidgets = QStyleOptionTab::NoCornerWidgets;
            tab.text = "test 3";
            tab.icon = style()->standardIcon(QStyle::SP_DirIcon);
            tab.iconSize = QSize(16, 16);

            style()->drawTabBar(painter(), &tab, testWidget());
        }
    };

    INIT_TESTWIDGET(QTabBar);
    TestTabBarDrawUtil tabbarDrawInstance(this);
    ASSERT_DrawFuncHasData(tabbarDrawInstance.testDrawRoundedEstOnlyOneTab);
    ASSERT_DrawFuncHasData(tabbarDrawInstance.testDrawTriangularNorthMiddleTab);
    ASSERT_DrawFuncHasData(tabbarDrawInstance.testDrawRoundedWestBeginningTab);
}

TEST_F(TestForDrawUtil, drawTabBarLabel)
{
    class TestTabBarLabelDrawUtil : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawRoundedEastOnlyOneTabLabel()
        {
            QStyleOptionTab tab;
            tab.initFrom(testWidget());
            tab.state |= (QStyle::State_Selected | QStyle::State_Enabled);
            tab.shape = QTabBar::RoundedEast;
            tab.position = QStyleOptionTab::OnlyOneTab;
            tab.selectedPosition = QStyleOptionTab::NotAdjacent;
            tab.cornerWidgets = QStyleOptionTab::NoCornerWidgets;
            tab.text = "test 1";

            style()->drawTabBarLabel(painter(), &tab, testWidget());
        }

        void testDrawTriangularNorthTabLabel()
        {
            QStyleOptionTab tab;
            tab.initFrom(testWidget());
            tab.state |= QStyle::State_Enabled;
            tab.shape = QTabBar::TriangularNorth;
            tab.text = "test 2";

            style()->drawTabBarLabel(painter(), &tab, testWidget());
        }

        void testDrawTriangularNorthOnlyOneTabLabel()
        {
            QStyleOptionTab tab;
            tab.initFrom(testWidget());
            tab.state |= (QStyle::State_Selected | QStyle::State_Enabled);
            tab.shape = QTabBar::TriangularNorth;
            tab.position = QStyleOptionTab::OnlyOneTab;
            tab.selectedPosition = QStyleOptionTab::NotAdjacent;
            tab.cornerWidgets = QStyleOptionTab::NoCornerWidgets;
            tab.text = "test 3";

            style()->drawTabBarLabel(painter(), &tab, testWidget());
        }

        void testDrawTriangularWestOnlyOneTabLabel()
        {
            QStyleOptionTab tab;
            tab.initFrom(testWidget());
            tab.state |= (QStyle::State_Selected | QStyle::State_Enabled);
            tab.shape = QTabBar::TriangularWest;
            tab.position = QStyleOptionTab::OnlyOneTab;
            tab.selectedPosition = QStyleOptionTab::NotAdjacent;
            tab.cornerWidgets = QStyleOptionTab::NoCornerWidgets;
            tab.text = "test 4";

            style()->drawTabBarLabel(painter(), &tab, testWidget());
        }
    };

    INIT_TESTWIDGET(DTabBar);
    TestTabBarLabelDrawUtil drawFuncInstance(this);
    testWidget->setProperty("_d_dtk_tabbartab_type", true);
    ASSERT_DrawFuncHasData(drawFuncInstance.testDrawRoundedEastOnlyOneTabLabel);
    ASSERT_DrawFuncHasData(drawFuncInstance.testDrawTriangularNorthTabLabel);
    testWidget->setProperty("_d_dtk_tabbar_alignment", 0);
    ASSERT_DrawFuncHasData(drawFuncInstance.testDrawTriangularNorthOnlyOneTabLabel);
    testWidget->setProperty("_d_dtk_tabbar_alignment", 1);
    ASSERT_DrawFuncHasData(drawFuncInstance.testDrawTriangularWestOnlyOneTabLabel);
}

TEST_F(TestForDrawUtil, drawTabBarCloseButton)
{
    INIT_TESTWIDGET(QTabBar);
    QTabBar *tabbar = qobject_cast<QTabBar *>(testWidget);
    ASSERT_TRUE(tabbar);

    tabbar->addTab("测试1");
    tabbar->addTab("测试2");
    tabbar->addTab("测试3");

    auto drawEnableAndSelectedCloseButton = [&]() {
        QWidget *btn1 = new QWidget(tabbar);
        btn1->resize(20, 20);
        tabbar->setTabButton(0, QTabBar::LeftSide, btn1);

        QStyleOption opt;
        opt.initFrom (btn1);
        opt.state = QStyle::State_Enabled | QStyle::State_Selected;

        style->drawTabBarCloseButton(painter, &opt, btn1);
    };

    auto drawNormalCloseButton = [&]() {
        QWidget *btn2 = new QWidget(tabbar);
        btn2->resize(20, 20);
        tabbar->setTabButton(1, QTabBar::RightSide, btn2);

        QStyleOption opt;
        opt.initFrom(btn2);

        style->drawTabBarCloseButton(painter, &opt, btn2);
    };

    ASSERT_DrawFuncHasData(drawEnableAndSelectedCloseButton);
    ASSERT_DrawFuncHasData(drawNormalCloseButton);
}

TEST_F(TestForDrawUtil, drawTabBarScrollButton)
{
    class TestTabBarScrollButtonDrawUtil : public TestDrawUtilIntegration
    {
    public:
        TestTabBarScrollButtonDrawUtil(TestForDrawUtil *parent)
            : TestDrawUtilIntegration(parent)
        {
            tabbar = qobject_cast<QTabBar *>(testWidget());
            toolButton = new QToolButton(tabbar);
        }

        void testDownArrowDrawUtil()
        {
            toolButton->setArrowType(Qt::DownArrow);

            QStyleOptionToolButton opt;
            opt.initFrom(toolButton);
            opt.text = "test 1";
            opt.arrowType = Qt::DownArrow;

            style()->drawTabBarScrollButton(painter(), &opt, toolButton);
        }

        void testUpArrowDrawUtil()
        {
            toolButton->setArrowType(Qt::UpArrow);

            QStyleOptionToolButton opt;
            opt.initFrom(toolButton);
            opt.text = "test 2";
            opt.arrowType = Qt::UpArrow;

            ASSERT_TRUE(tabbar);
            tabbar->setShape(QTabBar::TriangularEast);

            style()->drawTabBarScrollButton(painter(), &opt, toolButton);
        }

        void testLeftArrowDrawUtil()
        {
            toolButton->setArrowType(Qt::LeftArrow);

            QStyleOptionToolButton opt;
            opt.initFrom(toolButton);
            opt.text = "test 3";
            opt.arrowType = Qt::LeftArrow;

            ASSERT_TRUE(tabbar);
            tabbar->setShape(QTabBar::TriangularEast);
            style()->drawTabBarScrollButton(painter(), &opt, toolButton);
        }

        void testRightArrowDrawUtil()
        {
            toolButton->setArrowType(Qt::RightArrow);

            QStyleOptionToolButton opt;
            opt.initFrom(toolButton);
            opt.text = "test 4";
            opt.arrowType = Qt::RightArrow;

            tabbar->setShape(QTabBar::TriangularEast);
            style()->drawTabBarScrollButton(painter(), &opt, toolButton);
        }

    private:
        QToolButton *toolButton;
        QTabBar *tabbar;
    };

    INIT_TESTWIDGET(QTabBar);
    TestTabBarScrollButtonDrawUtil drawUtilInstance(this);

    ASSERT_DrawFuncHasData(drawUtilInstance.testDownArrowDrawUtil);
    ASSERT_DrawFuncHasData(drawUtilInstance.testUpArrowDrawUtil);
    ASSERT_DrawFuncHasData(drawUtilInstance.testLeftArrowDrawUtil);
    ASSERT_DrawFuncHasData(drawUtilInstance.testRightArrowDrawUtil);
}

TEST_F(TestForDrawUtil, drawComboBox)
{
    class TestComboBoxDrawUtil : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawNormalComboBox()
        {
            QStyleOptionComboBox opt;
            opt.initFrom(testWidget());
            opt.editable = true;
            opt.popupRect = QRect(0, 0, 30, 30);
            opt.frame = true;
            opt.currentText = "test 1";
            opt.currentIcon = style()->standardIcon(QStyle::SP_DirIcon);
            opt.iconSize = QSize(16, 16);
            opt.state = QStyle::State_Enabled | QStyle::State_MouseOver;

            style()->drawComboBox(painter(), &opt, testWidget());
        }

        void testDrawArrowFrameEditAndPopupComboBox()
        {
            QStyleOptionComboBox opt;
            opt.initFrom(testWidget());
            opt.editable = false;
            opt.frame = false;
            opt.currentText = "test 2";
            opt.currentIcon = style()->standardIcon(QStyle::SP_DirIcon);
            opt.iconSize = QSize(16, 16);
            opt.subControls |= QStyle::SC_ComboBoxArrow | QStyle::SC_ComboBoxFrame | QStyle::SC_ComboBoxEditField | QStyle::SC_ComboBoxListBoxPopup;

            style()->drawComboBox(painter(), &opt, testWidget());
        }

        void testDrawArrowFrameComboBox()
        {
            QStyleOptionComboBox opt;
            opt.initFrom(testWidget());

            opt.editable = false;
            opt.frame = false;
            opt.currentText = "test 3";
            opt.currentIcon = style()->standardIcon(QStyle::SP_DirIcon);
            opt.iconSize = QSize(16, 16);
            opt.subControls |= QStyle::SC_ComboBoxArrow | QStyle::SC_ComboBoxFrame;

            style()->drawComboBox(painter(), &opt, testWidget());
        }

        void testDrawEditPopupFocusComboBox()
        {
            QStyleOptionComboBox opt;
            opt.initFrom(testWidget());

            opt.currentText = "test 4";
            opt.currentIcon = style()->standardIcon(QStyle::SP_DirIcon);
            opt.iconSize = QSize(16, 16);
            opt.state = QStyle::State_Enabled | QStyle::State_Selected | QStyle::State_HasFocus;
            opt.subControls |= QStyle::SC_ComboBoxEditField | QStyle::SC_ComboBoxListBoxPopup;

            style()->drawComboBox(painter(), &opt, testWidget());
        }
    };

    INIT_TESTWIDGET(QWidget);

    TestComboBoxDrawUtil drawUtilInstance(this);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawNormalComboBox);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawArrowFrameEditAndPopupComboBox);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawArrowFrameComboBox);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawEditPopupFocusComboBox);
}

TEST_F(TestForDrawUtil, drawComboBoxLabel)
{
    class TestComboBoxLabelDrawUtil : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawEnableAndHoverComboBoxLabel()
        {
            QStyleOptionComboBox opt;
            opt.initFrom(testWidget());

            opt.editable = true;
            opt.popupRect = QRect(0, 0, 30, 30);
            opt.frame = true;
            opt.currentText = "test 1";
            opt.currentIcon = QIcon::fromTheme("icon_Layout");
            opt.iconSize = QSize(16, 16);
            opt.state = QStyle::State_Enabled | QStyle::State_MouseOver;

            style()->drawComboBoxLabel(painter(), &opt, testWidget());
        }

        void testDrawArrowFrameEditComboBoxLabel()
        {
            QStyleOptionComboBox opt;
            opt.initFrom(testWidget());

            opt.editable = false;
            opt.popupRect = QRect(0, 0, 30, 30);
            opt.frame = false;
            opt.currentText = "test 2";
            opt.currentIcon = QIcon::fromTheme("icon_Layout");
            opt.iconSize = QSize(16, 16);
            opt.subControls |= QStyle::SC_ComboBoxArrow | QStyle::SC_ComboBoxFrame | QStyle::SC_ComboBoxEditField | QStyle::SC_ComboBoxListBoxPopup;

            style()->drawComboBoxLabel(painter(), &opt, testWidget());
        }

        void testDrawArrowFrameSunkenComboBoxLabel()
        {
            QStyleOptionComboBox opt;
            opt.initFrom(testWidget());

            opt.rect = QRect(20, 20, 100, 50);
            opt.currentText = "test 3";
            opt.currentIcon = {};
            opt.iconSize = {};
            opt.subControls |= QStyle::SC_ComboBoxArrow | QStyle::SC_ComboBoxFrame;
            opt.state = QStyle::State_Enabled | QStyle::State_Sunken;
            style()->drawComboBoxLabel(painter(), &opt, testWidget());
        }

        void testDrawFocusAndSelectComboBoxLabel()
        {
            QStyleOptionComboBox opt;
            opt.initFrom(testWidget());

            opt.currentText = "测试3";
            opt.currentIcon = QIcon::fromTheme("icon_Layout");
            opt.iconSize = QSize(16, 16);
            opt.state = QStyle::State_Enabled | QStyle::State_Selected | QStyle::State_HasFocus;
            opt.subControls |= QStyle::SC_ComboBoxEditField | QStyle::SC_ComboBoxListBoxPopup;

            style()->drawComboBoxLabel(painter(), &opt, testWidget());
        }
    };

    INIT_TESTWIDGET(QWidget);

    TestComboBoxLabelDrawUtil drawUtilInstance(this);

    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawEnableAndHoverComboBoxLabel);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawArrowFrameEditComboBoxLabel);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawArrowFrameSunkenComboBoxLabel);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawFocusAndSelectComboBoxLabel);
}

TEST_F(TestForDrawUtil, drawSliderHandle)
{
    class TestSliderHandleDrawUtil : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawNoTicksSliderHandle()
        {
            QStyleOptionSlider slider;
            slider.initFrom(testWidget());
            slider.tickPosition = QSlider::NoTicks;

            style()->drawSliderHandle(&slider, defaultRect, painter(), testWidget());
        }

        void testDrawHorizontalLeftTicksSliderHandle()
        {
            QStyleOptionSlider slider;
            slider.initFrom(testWidget());
            slider.orientation = Qt::Horizontal;
            slider.tickPosition = QSlider::TicksLeft;

            style()->drawSliderHandle(&slider, defaultRect, painter(), testWidget());
        }

        void testDrawHorizontalRightTicksSliderHandle()
        {
            QStyleOptionSlider slider;
            slider.initFrom(testWidget());
            slider.orientation = Qt::Horizontal;
            slider.tickPosition = QSlider::TicksRight;

            style()->drawSliderHandle(&slider, defaultRect, painter(), testWidget());
        }

        void testDrawVerticalLeftTicksSliderHandle()
        {
            QStyleOptionSlider slider;
            slider.initFrom(testWidget());
            slider.orientation = Qt::Vertical;
            slider.tickPosition = QSlider::TicksLeft;

            style()->drawSliderHandle(&slider, defaultRect, painter(), testWidget());
        }

        void testDrawVerticalRightTicksSliderHandle()
        {
            QStyleOptionSlider slider;
            slider.initFrom(testWidget());
            slider.orientation = Qt::Vertical;
            slider.tickPosition = QSlider::TicksRight;

            style()->drawSliderHandle(&slider, defaultRect, painter(), testWidget());
        }

    private:
        QRectF defaultRect = QRectF(0, 0, 50, 50);
    };

    INIT_TESTWIDGET(QWidget);

    TestSliderHandleDrawUtil drawUtilInstance(this);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawNoTicksSliderHandle);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawHorizontalLeftTicksSliderHandle);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawHorizontalRightTicksSliderHandle);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawVerticalLeftTicksSliderHandle);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawVerticalRightTicksSliderHandle);
}

TEST_F(TestForDrawUtil, drawSliderHandleFocus)
{
    class TestSliderHandleFocusDrawUtil : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawHorizontalNoticksSliderHandleFocus()
        {
            QStyleOptionSlider opt;
            opt.initFrom(testWidget());
            opt.orientation = Qt::Horizontal;
            opt.tickPosition = QSlider::NoTicks;
            opt.minimum = 0;
            opt.maximum = 100;

            style()->drawSliderHandleFocus(&opt, defaultRect, painter(), nullptr);
        }

        void testDrawHorizontalLeftTicksSliderHandleFocus()
        {
            QStyleOptionSlider opt;
            opt.initFrom(testWidget());
            opt.orientation = Qt::Horizontal;
            opt.tickPosition = QSlider::TicksLeft;
            opt.minimum = 0;
            opt.maximum = 100;

            style()->drawSliderHandleFocus(&opt, defaultRect, painter(), nullptr);
        }

        void testDrawHorizontalRightTicksSliderHandleFocus()
        {
            QStyleOptionSlider opt;
            opt.initFrom(testWidget());
            opt.orientation = Qt::Horizontal;
            opt.tickPosition = QSlider::TicksRight;
            opt.minimum = 0;
            opt.maximum = 100;

            style()->drawSliderHandleFocus(&opt, defaultRect, painter(), nullptr);
        }

        void testDrawVerticalLeftTicksSliderHandleFocus()
        {
            QStyleOptionSlider opt;
            opt.initFrom(testWidget());
            opt.orientation = Qt::Vertical;
            opt.tickPosition = QSlider::TicksLeft;
            opt.minimum = 0;
            opt.maximum = 100;

            style()->drawSliderHandleFocus(&opt, defaultRect, painter(), nullptr);
        }

        void testDrawVerticalRightTicksSliderHandleFocus()
        {
            QStyleOptionSlider opt;
            opt.initFrom(testWidget());
            opt.orientation = Qt::Vertical;
            opt.tickPosition = QSlider::TicksRight;
            opt.minimum = 0;
            opt.maximum = 100;

            style()->drawSliderHandleFocus(&opt, defaultRect, painter(), nullptr);
        }

    private:
        QRectF defaultRect = QRect(0, 0, 20, 20);
    };

    INIT_TESTWIDGET(QWidget);

    TestSliderHandleFocusDrawUtil drawUtilInstance(this);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawHorizontalNoticksSliderHandleFocus);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawHorizontalLeftTicksSliderHandleFocus);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawHorizontalRightTicksSliderHandleFocus);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawVerticalLeftTicksSliderHandleFocus);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawVerticalRightTicksSliderHandleFocus);
}

TEST_F(TestForDrawUtil, drawIcon)
{
    auto drawIconUtil = [&]() {
        QStyleOption opt;
        opt.state |= QStyle::State_Enabled;
        opt.state |= QStyle::State_Selected;

        QRect rect(0, 0, 10, 10);
        style->drawIcon(&opt, painter, rect, QIcon::fromTheme("icon_Layout"), true);
    };

    ASSERT_DrawFuncHasData(drawIconUtil);
}

TEST_F(TestForDrawUtil, tabLayout)
{
    INIT_TESTWIDGET(QWidget);

    QStyleOptionTab tab;
    tab.initFrom(testWidget);
    tab.iconSize = QSize(32, 32);
    tab.shape = QTabBar::RoundedEast;
    tab.icon = QIcon::fromTheme("icon_Layout");

    QRect textRect, iconRect;
    style->tabLayout(&tab, testWidget, &textRect, &iconRect);
    ASSERT_TRUE(textRect.isValid());
    ASSERT_TRUE(iconRect.isValid());
}

TEST_F(TestForDrawUtil, drawTableViewItem)
{
    auto drawNormalTableViewItem = [&]() {
        QStyleOptionViewItem opt;
        opt.initFrom(testWidget);
        opt.backgroundBrush = Qt::darkCyan;

        style->drawTableViewItem(QStyle::PE_PanelItemViewItem, &opt, painter, testWidget);
    };

    auto drawEnableItemRadiusTableView = [&]() {
        QStyleOptionViewItem opt;
        opt.initFrom(testWidget);
        opt.backgroundBrush = Qt::darkCyan;
        opt.state |= QStyle::State_Selected;
        opt.showDecorationSelected = true;

        testWidget->setProperty("_d_dtk_enable_tableviewitem_radius", true);
        style->drawTableViewItem(QStyle::PE_PanelItemViewItem, &opt, painter, testWidget);
    };

    INIT_TESTWIDGET(QTableView);
    ASSERT_DrawFuncHasData(drawNormalTableViewItem);
    ASSERT_DrawFuncHasData(drawEnableItemRadiusTableView);
}

TEST_F(TestForDrawUtil, drawMenuItemRedPoint)
{
    INIT_TESTWIDGET(QMenu);
    QMenu *menu = qobject_cast<QMenu *>(testWidget);
    ASSERT_TRUE(menu);

    auto drawNormalMenuItemRedPoint = [&]() {
        QAction *action = new QAction(menu);
        action->setText("test 1");
        action->setProperty("_d_menu_item_redpoint", true);
        menu->addAction(action);

        QStyleOptionMenuItem opt;
        opt.initFrom(menu);
        opt.menuItemType = QStyleOptionMenuItem::Normal;
        opt.rect = menu->actionGeometry(action);

        style->drawMenuItemRedPoint(&opt, painter, menu);
    };

    auto drawSubMenuRedPoint = [&]() {
        QAction *actionMenu = new QAction(menu);
        QMenu *submenu = new QMenu("test menu");
        submenu->setProperty("_d_menu_item_redpoint", true);
        actionMenu->setMenu(submenu);
        menu->addAction(actionMenu);

        QStyleOptionMenuItem opt;
        opt.initFrom(menu);
        opt.menuItemType = QStyleOptionMenuItem::Normal;
        opt.menuItemType = QStyleOptionMenuItem::SubMenu;
        opt.rect = menu->actionGeometry(actionMenu);

        style->drawMenuItemRedPoint(&opt, painter, menu);
    };

    ASSERT_DrawFuncHasData(drawNormalMenuItemRedPoint);
    ASSERT_DrawFuncHasData(drawSubMenuRedPoint);
}

TEST_F(TestForDrawUtil, drawPrimitivePanelItemViewItem)
{
    class TestPanelItemPrimitiveDrawUtil : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawSelectedViewItemPanel()
        {
            QStyleOption opt;
            opt.initFrom(testWidget());
            opt.state |= QStyle::State_Selected;

            style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter(), testWidget()->findChild<QTableView *>("qt_calendar_calendarview"));
        }

        void testDrawHoverAndSelectedViewItemBeginningPanel()
        {
            QStyleOptionViewItem tableviewOption;
            tableviewOption.initFrom(testWidget());
            tableviewOption.showDecorationSelected = true;
            tableviewOption.state |= (QStyle::State_Selected | QStyle::State_MouseOver);
            tableviewOption.viewItemPosition = QStyleOptionViewItem::Beginning;

            style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &tableviewOption, painter(), testWidget());
        }

        void testDrawEndItemViewPanel()
        {
            QStyleOptionViewItem tableviewOption;
            tableviewOption.initFrom(testWidget());
            tableviewOption.showDecorationSelected = true;
            tableviewOption.state |= (QStyle::State_Selected | QStyle::State_MouseOver);
            tableviewOption.viewItemPosition = QStyleOptionViewItem::End;

            style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &tableviewOption, painter(), testWidget());
        }

        void testDrawNoDecorationViemItemPanel()
        {
            QStyleOptionViewItem tableviewOption;
            tableviewOption.initFrom(testWidget());

            tableviewOption.state |= (QStyle::State_Selected | QStyle::State_MouseOver);
            tableviewOption.viewItemPosition = QStyleOptionViewItem::End;
            tableviewOption.showDecorationSelected = false;
            tableviewOption.backgroundBrush = Qt::darkRed;

            style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &tableviewOption, painter(), testWidget());
        }

        void testDrawSelectedDecorationListViewItemPanel()
        {
            QStyleOptionViewItem listViewOption;
            listViewOption.initFrom(testWidget());

            listViewOption.state |= QStyle::State_Selected;
            listViewOption.showDecorationSelected = true;

            style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &listViewOption, painter(), testWidget());
        }
    };

    INIT_TESTWIDGET(QCalendarWidget);
    TestPanelItemPrimitiveDrawUtil drawUtilInstance(this);

    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawSelectedViewItemPanel);

    INIT_TESTWIDGET(QTreeView);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawHoverAndSelectedViewItemBeginningPanel);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawEndItemViewPanel);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawNoDecorationViemItemPanel);

    INIT_TESTWIDGET(QListView);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawSelectedDecorationListViewItemPanel);
}

TEST_F(TestForDrawUtil, drawPrimitivePanelLineEdit)
{
    class TestPanelLineEditDrawUtil : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawFlatFocusLineEdit()
        {
            QStyleOptionFrame frameOption;
            frameOption.initFrom(testWidget());

            frameOption.features |= QStyleOptionFrame::Flat;
            frameOption.state |= QStyle::State_HasFocus;

            style()->drawPrimitive(QStyle::PE_PanelLineEdit, &frameOption, painter(), testWidget());
        }

        void testDrawRoundedLineEdit()
        {
            QStyleOptionFrame frameOption;
            frameOption.initFrom(testWidget());

            frameOption.state |= QStyle::State_HasFocus;
            frameOption.features |= QStyleOptionFrame::Rounded;
            frameOption.lineWidth = 5;

            style()->drawPrimitive(QStyle::PE_PanelLineEdit, &frameOption, painter(), testWidget());
        }

        void testDrawHasOpacityLineEdit()
        {
            QStyleOption option;
            option.initFrom(testWidget());

            testWidget()->setProperty("_d_dtk_lineedit_opacity", true);
            style()->drawPrimitive(QStyle::PE_PanelLineEdit, &option, painter(), testWidget());
        }

        void testDrawHasParentAndOpacityLineEdit()
        {
            DSpinBox spinbox;
            spinbox.resize(200, 200);
            spinbox.setProperty("_d_dtk_lineedit_opacity", true);
            testWidget()->setParent(&spinbox);

            QStyleOption option;
            option.initFrom(testWidget());

            style()->drawPrimitive(QStyle::PE_PanelLineEdit, &option, painter(), testWidget());
            testWidget()->setParent(nullptr);
        }
    };

    INIT_TESTWIDGET(QLineEdit);

    TestPanelLineEditDrawUtil drawUtilInstance(this);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawFlatFocusLineEdit);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawRoundedLineEdit);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawHasOpacityLineEdit);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawHasParentAndOpacityLineEdit);
}

TEST_F(TestForDrawUtil, drawPrimitiveIndicatorRadioButton)
{
    auto drawNormalRadioButton = [&]() {
        QStyleOption radioButtonOption;
        radioButtonOption.initFrom(testWidget);

        radioButtonOption.state |= QStyle::State_On;
        style->drawPrimitive(QStyle::PE_IndicatorRadioButton, &radioButtonOption, painter, testWidget);
    };

    auto drawOffStateRadioButton = [&]() {
        QStyleOption radioButtonOption;
        radioButtonOption.initFrom(testWidget);

        radioButtonOption.state |= QStyle::State_Off;
        style->drawPrimitive(QStyle::PE_IndicatorRadioButton, &radioButtonOption, painter, testWidget);
    };

    INIT_TESTWIDGET(QRadioButton);

    ASSERT_DrawFuncHasData(drawNormalRadioButton);
    ASSERT_DrawFuncHasData(drawOffStateRadioButton);
}

TEST_F(TestForDrawUtil, drawPrimitiveIndicatorCheckBox)
{
    class TestIndicatorCheckBoxDrawUtil : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawNoChangeStateIndicatorCheckBox()
        {
            QStyleOption checkBoxOption;
            checkBoxOption.initFrom(testWidget());
            checkBoxOption.state |= QStyle::State_NoChange;

            style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &checkBoxOption, painter(), testWidget());
        }

        void testDrawStateOnIndicatorCheckBox()
        {
            QStyleOption checkBoxOption;
            checkBoxOption.initFrom(testWidget());
            checkBoxOption.state |= QStyle::State_On;

            style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &checkBoxOption, painter(), testWidget());
        }

        void testDrawNormalIndicatorCheckBox()
        {
            QStyleOption checkBoxOption;
            checkBoxOption.initFrom(testWidget());

            style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &checkBoxOption, painter(), testWidget());
        }
    };

    INIT_TESTWIDGET(QCheckBox);

    TestIndicatorCheckBoxDrawUtil drawUtilInstance(this);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawNoChangeStateIndicatorCheckBox);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawStateOnIndicatorCheckBox);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawNormalIndicatorCheckBox);
}

TEST_F(TestForDrawUtil, drawPrimitiveIndicatorTabClose)
{
    auto drawIndicatorTabClose = [&]() {
        QStyleOption opt;
        opt.initFrom(testWidget);

        style->drawPrimitive(QStyle::PE_IndicatorTabClose, &opt, painter, testWidget);
    };

    INIT_TESTWIDGET(QWidget);
    ASSERT_DrawFuncHasData(drawIndicatorTabClose);
}

TEST_F(TestForDrawUtil, drawPrimitiveFrameTabWidget)
{
    auto drawFrameTabWidget = [&]() {
        QStyleOption opt;
        opt.initFrom(testWidget);

        style->drawPrimitive(QStyle::PE_FrameTabWidget, &opt, painter, testWidget);
    };

    INIT_TESTWIDGET(QWidget);
    ASSERT_DrawFuncHasData(drawFrameTabWidget);
}

TEST_F(TestForDrawUtil, drawPrimitiveIndicatorItemViewItemCheck)
{
    class TestIndicatorItemViewItemCheckDrawUtil : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawNoChangeStateItemViewItemCheck()
        {
            QStyleOption option;
            option.initFrom(testWidget());
            option.state |= QStyle::State_NoChange;

            style()->drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &option, painter(), testWidget());
        }

        void testDrawOnStateItemViewItemCheck()
        {
            QStyleOption option;
            option.initFrom(testWidget());
            option.state |= (QStyle::State_On | QStyle::State_Enabled);

            style()->drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &option, painter(), testWidget());
        }

        void testDrawOffStateAndNoUncheckedItemViewItemCheck()
        {
            QStyleOption option;
            option.initFrom(testWidget());
            option.state |= QStyle::State_Off;
            option.state &= ~QStyle::State_On;

            style()->drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &option, painter(), testWidget());
        }

        void testDrawOffStateAndHasUncheckedItemViewItemCheck()
        {
            QStyleOption option;
            option.initFrom(testWidget());
            option.state |= QStyle::State_Off;
            option.state &= ~QStyle::State_On;

            testWidget()->setProperty("_d_dtk_UncheckedItemIndicator", true);
            style()->drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &option, painter(), testWidget());
        }
    };

    INIT_TESTWIDGET(QWidget);

    TestIndicatorItemViewItemCheckDrawUtil drawUtilInstance(this);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawNoChangeStateItemViewItemCheck);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawOnStateItemViewItemCheck);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawOffStateAndNoUncheckedItemViewItemCheck);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawOffStateAndHasUncheckedItemViewItemCheck);
}

TEST_F(TestForDrawUtil, drawPrimitivePanelMenu)
{
    auto drawPanelMenuUtilWithValidWindowColor = [&]() {
        QStyleOption option;
        option.initFrom(testWidget);

        ASSERT_TRUE(option.palette.window().color().isValid());
        style->drawPrimitive(QStyle::PE_PanelMenu, &option, painter, testWidget);
    };

    INIT_TESTWIDGET(QWidget);
    ASSERT_DrawFuncHasData(drawPanelMenuUtilWithValidWindowColor);
}

TEST_F(TestForDrawUtil, drawPrimitiveFrame)
{
    auto drawFrameUtil = [&]() {
        QStyleOptionFrame option;
        option.initFrom(testWidget);

        option.features |= QStyleOptionFrame::Rounded;
        option.lineWidth = 1;

        style->drawPrimitive(QStyle::PE_Frame, &option, painter, testWidget);
    };

    auto drawFrameUtilWithNoLine = [&]() {
        QStyleOptionFrame option;
        option.initFrom(testWidget);

        option.features |= QStyleOptionFrame::Rounded;
        option.lineWidth = 0;
        painter->setBackground(option.palette.window());

        style->drawPrimitive(QStyle::PE_Frame, &option, painter, testWidget);
    };

    INIT_TESTWIDGET(QFrame);
    ASSERT_DrawFuncHasData(drawFrameUtil);
    ASSERT_DrawFuncHasData(drawFrameUtilWithNoLine);
}

TEST_F(TestForDrawUtil, drawPrimitivePanelTipLabel)
{
    // PE_PanelTipLabel return directly.
    auto drawPanelTipLabel = [&]() {
        style->drawPrimitive(QStyle::PE_PanelTipLabel, nullptr, nullptr, nullptr);
    };

    ASSERT_DrawFuncNoData(drawPanelTipLabel);
}

TEST_F(TestForDrawUtil, drawPrimitiveFrameGroupBox)
{
    auto drawFrameGroupBoxFlatFeature = [&]() {
        QStyleOptionFrame option;
        option.initFrom(testWidget);

        option.features |= QStyleOptionFrame::Flat;
        // Flat feature return directly.
        style->drawPrimitive(QStyle::PE_FrameGroupBox, &option, painter, testWidget);
    };

    auto drawFrameNormalFrameGroupBox = [&]() {
        QStyleOption option;
        option.initFrom(testWidget);

        style->drawPrimitive(QStyle::PE_FrameGroupBox, &option, painter, testWidget);
    };

    INIT_TESTWIDGET(QFrame);
    ASSERT_DrawFuncNoData(drawFrameGroupBoxFlatFeature);
    INIT_TESTWIDGET(QGroupBox);
    ASSERT_DrawFuncHasData(drawFrameNormalFrameGroupBox);
}

TEST_F(TestForDrawUtil, drawPrimitiveIndicatorArrowRightAndLeft)
{
    auto drawIndicatorArrowRightUtil = [&]() {
        QStyleOption option;
        option.initFrom(testWidget);

        style->drawPrimitive(QStyle::PE_IndicatorArrowRight, &option, painter, testWidget);
    };

    auto drawIndicatorArrowLeftUtil = [&]() {
        QStyleOption option;
        option.initFrom(testWidget);

        style->drawPrimitive(QStyle::PE_IndicatorArrowLeft, &option, painter, testWidget);
    };

    INIT_TESTWIDGET(QWidget);
    ASSERT_DrawFuncHasData(drawIndicatorArrowRightUtil);
    ASSERT_DrawFuncHasData(drawIndicatorArrowLeftUtil);
}

TEST_F(TestForDrawUtil, drawPrimitiveFrameTabBarBase)
{
    auto drawFrameTabBarBaseUtil = [&]() {
        // Only return directly.
        style->drawPrimitive(QStyle::PE_FrameTabBarBase, nullptr, nullptr, nullptr);
    };

    ASSERT_DrawFuncNoData(drawFrameTabBarBaseUtil);
}

TEST_F(TestForDrawUtil, drawPrimitiveIndicatorBranch)
{
    class TestIndicatorBranchDrawUtil : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawChildrenSelectedStateUtil()
        {
            QStyleOption option;
            option.initFrom(testWidget());
            option.state |= QStyle::State_Children | QStyle::State_Selected;
            option.state &= ~QStyle::State_Open;

            style()->drawPrimitive(QStyle::PE_IndicatorBranch, &option, painter(), testWidget());
        }

        void testDrawChildrenStateUtil()
        {
            QStyleOption option;
            option.initFrom(testWidget());
            option.state |= QStyle::State_Children;
            option.state &= ~QStyle::State_Open;

            style()->drawPrimitive(QStyle::PE_IndicatorBranch, &option, painter(), testWidget());
        }

        void testDrawChildrenOpenSelectedStateUtil()
        {
            QStyleOption option;
            option.initFrom(testWidget());
            option.state |= QStyle::State_Children | QStyle::State_Open | QStyle::State_Selected;

            style()->drawPrimitive(QStyle::PE_IndicatorBranch, &option, painter(), testWidget());
        }

        void testDrawChildrenOpenStateUtil()
        {
            QStyleOption option;
            option.initFrom(testWidget());
            option.state |= QStyle::State_Children | QStyle::State_Open;

            style()->drawPrimitive(QStyle::PE_IndicatorBranch, &option, painter(), testWidget());
        }
    };

    INIT_TESTWIDGET(QTreeView);
    TestIndicatorBranchDrawUtil drawUtilInstance(this);

    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawChildrenSelectedStateUtil);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawChildrenStateUtil);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawChildrenOpenSelectedStateUtil);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawChildrenOpenStateUtil);
}

TEST_F(TestForDrawUtil, drawPrimitivePanelItemViewRow)
{
    class TestPanelItemViewRowDrawUtil : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawPanelItemViewInQCalendarWidget()
        {
            QWidget *tableView = testWidget()->findChild<QTableView *>("qt_calendar_calendarview");
            ASSERT_TRUE(tableView);

            QStyleOption option;
            option.initFrom(tableView);

            style()->drawPrimitive(QStyle::PE_PanelItemViewRow, &option, painter(), tableView);
        }

        void testDrawPanelItemViewActiveQTreeView()
        {
            QTreeView *treeView = qobject_cast<QTreeView *>(testWidget());
            ASSERT_TRUE(treeView);

            QStyleOptionViewItem option;
            option.initFrom(treeView);
            option.state |= QStyle::State_Enabled | QStyle::State_Active;

            style()->drawPrimitive(QStyle::PE_PanelItemViewRow, &option, painter(), treeView);
        }

        void testDrawPanelItemViewInactiveEndQTreeView()
        {
            QTreeView *treeView = qobject_cast<QTreeView *>(testWidget());
            ASSERT_TRUE(treeView);

            QStyleOptionViewItem option;
            option.initFrom(treeView);
            option.state |= (QStyle::State_Enabled | QStyle::State_Selected | QStyle::State_MouseOver);
            option.state &= ~QStyle::State_Active;
            option.viewItemPosition = QStyleOptionViewItem::End;

            style()->drawPrimitive(QStyle::PE_PanelItemViewRow, &option, painter(), treeView);
        }

        void testDrawPanelItemViewInactiveValidQTreeView()
        {
            QTreeView *treeView = qobject_cast<QTreeView *>(testWidget());
            ASSERT_TRUE(treeView);

            QStyleOptionViewItem option;
            option.initFrom(treeView);
            option.state |= (QStyle::State_Enabled | QStyle::State_Selected | QStyle::State_MouseOver);
            option.state &= ~QStyle::State_Active;
            option.viewItemPosition = QStyleOptionViewItem::Middle;

            style()->drawPrimitive(QStyle::PE_PanelItemViewRow, &option, painter(), treeView);
        }

        void testDrawPanelItemViewInactiveAlternateQTreeView()
        {
            QTreeView *treeView = qobject_cast<QTreeView *>(testWidget());
            ASSERT_TRUE(treeView);

            QStyleOptionViewItem option;
            option.initFrom(treeView);
            option.state |= QStyle::State_Enabled;
            option.state &= ~QStyle::State_Active;
            option.features |= QStyleOptionViewItem::Alternate;
            option.viewItemPosition = QStyleOptionViewItem::Middle;

            style()->drawPrimitive(QStyle::PE_PanelItemViewRow, &option, painter(), treeView);
        }

        void testDrawQListViewPanelItemRow()
        {
            QListView *listView = qobject_cast<QListView *>(testWidget());
            ASSERT_TRUE(listView);

            QStyleOptionViewItem option;
            option.initFrom(listView);
            option.state |= QStyle::State_Enabled;
            option.features |= QStyleOptionViewItem::Alternate;
            option.viewItemPosition = QStyleOptionViewItem::Middle;

            style()->drawPrimitive(QStyle::PE_PanelItemViewRow, &option, painter(), listView);
        }

    };

    INIT_TESTWIDGET(QCalendarWidget);
    TestPanelItemViewRowDrawUtil drawUtilInstance(this);
    ASSERT_DrawFuncNoData(drawUtilInstance.testDrawPanelItemViewInQCalendarWidget);

    INIT_TESTWIDGET(QTreeView);
    ASSERT_DrawFuncNoData(drawUtilInstance.testDrawPanelItemViewActiveQTreeView);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawPanelItemViewInactiveEndQTreeView);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawPanelItemViewInactiveValidQTreeView);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawPanelItemViewInactiveAlternateQTreeView);

    INIT_TESTWIDGET(QListView);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawQListViewPanelItemRow);
}

TEST_F(TestForDrawUtil, drawPrimitiveFrameStatusBarItem)
{
    auto drawStatusbarItemUtil = [&]() {
        // return directly
        style->drawPrimitive(QStyle::PE_FrameStatusBarItem, nullptr, nullptr, nullptr);
    };

    ASSERT_DrawFuncNoData(drawStatusbarItemUtil);
}

TEST_F(TestForDrawUtil, drawPrimitivePanelStatusBar)
{
    auto drawPanelStatusBarUtil = [&]() {
        QStyleOption option;
        option.initFrom(testWidget);

        style->drawPrimitive(QStyle::PE_PanelStatusBar, &option, painter, testWidget);
    };

    INIT_TESTWIDGET(QStatusBar);
    ASSERT_DrawFuncHasData(drawPanelStatusBarUtil);
}

TEST_F(TestForDrawUtil, drawControlCheckBoxAndRadioButton)
{
    class TestCheckBoxAndRadioButtonDrawUtil : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawCheckBoxUtil()
        {
            QStyleOptionButton option;
            option.initFrom(testWidget());
            option.state |= QStyle::State_HasFocus;
            option.text = "test 1";
            option.features |= QStyleOptionButton::DefaultButton;

            style()->drawControl(QStyle::CE_CheckBox, &option, painter(), testWidget());
        }

        void testDrawRadioButtonUtil()
        {
            QStyleOptionButton option;
            option.initFrom(testWidget());
            option.state |= QStyle::State_HasFocus;
            option.text = "test 1";

            style()->drawControl(QStyle::CE_RadioButton, &option, painter(), testWidget());
        }
    };

    TestCheckBoxAndRadioButtonDrawUtil drawUtilInstance(this);
    INIT_TESTWIDGET(QCheckBox);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawCheckBoxUtil);
    INIT_TESTWIDGET(QRadioButton);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawRadioButtonUtil);
}

TEST_F(TestForDrawUtil, drawControlScrollBarSlider)
{
    class TestScrollBarSliderDrawUtil : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawNormalScrollBarSlider()
        {
            QStyleOptionSlider option;
            option.initFrom(testWidget());
            option.state |= QStyle::State_Horizontal | QStyle::State_MouseOver;
            option.state |= QStyle::QStyle::State_MouseOver;
            style()->drawControl(QStyle::CE_ScrollBarSlider, &option, painter(), testWidget());
        }

        void testDrawNormalVerticalScrollBarSlider()
        {
            QStyleOptionSlider option;
            option.initFrom(testWidget());
            option.state |= QStyle::QStyle::State_MouseOver;
            style()->drawControl(QStyle::CE_ScrollBarSlider, &option, painter(), testWidget());
        }

        void testDrawHorizontalSliderWithSpaceProperty()
        {
            QStyleOptionSlider option;
            option.initFrom(testWidget());
            option.state |= QStyle::State_Horizontal;
            option.state |= QStyle::QStyle::State_MouseOver;

            testWidget()->setProperty("_d_slider_spaceLeft", 10);
            testWidget()->setProperty("_d_slider_spaceRight", 10);
            testWidget()->setProperty("_d_slider_spaceUp", 10);
            testWidget()->setProperty("_d_slider_spaceDown", 10);
            style()->drawControl(QStyle::CE_ScrollBarSlider, &option, painter(), testWidget());
        }

        void testDrawVerticalSliderWithSpaceProperty()
        {
            QStyleOptionSlider option;
            option.initFrom(testWidget());
            option.state |= (QStyle::State_MouseOver | QStyle::State_Sunken);

            style()->drawControl(QStyle::CE_ScrollBarSlider, &option, painter(), testWidget());
        }
    };

    INIT_TESTWIDGET(QScrollBar);
    TestScrollBarSliderDrawUtil drawUtilInstance(this);

    QScrollBar *bar = qobject_cast<QScrollBar*> (drawUtilInstance.testWidget());
    bar->setProperty("_d_dtk_slider_always_show", true);

    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawNormalScrollBarSlider);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawNormalVerticalScrollBarSlider);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawHorizontalSliderWithSpaceProperty);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawVerticalSliderWithSpaceProperty);
}

TEST_F(TestForDrawUtil, drawControlMenuBarItem)
{
    auto drawMenuBarItemUtil = [&]() {
        QStyleOptionMenuItem option;
        option.initFrom(testWidget);

        option.menuItemType = QStyleOptionMenuItem::Margin;
        option.checkType = QStyleOptionMenuItem::Exclusive;
        option.text = "test 1";
        option.icon = style->standardIcon(QStyle::SP_DirIcon);
        option.menuRect = QRect(20, 20, 60, 40);
        option.maxIconWidth = 20;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        option.reservedShortcutWidth = 2;
#else
        option.tabWidth = 2;
#endif

        style->drawControl(QStyle::CE_MenuBarItem, &option, painter, testWidget);
    };

    INIT_TESTWIDGET(QWidget);
    ASSERT_DrawFuncHasData(drawMenuBarItemUtil);
}

TEST_F(TestForDrawUtil, drawControlMenuItem)
{
    auto drawNormalMenuItemUtil = [&]() {
        QStyleOptionMenuItem opt;
        opt.initFrom(testWidget);
        opt.menuItemType = QStyleOptionMenuItem::Normal;
        style->drawControl(QStyle::CE_MenuItem, &opt, painter, testWidget);
    };

    INIT_TESTWIDGET(QWidget);
    ASSERT_DrawFuncHasData(drawNormalMenuItemUtil);
}

TEST_F(TestForDrawUtil, drawControlPushButtonAndBevel)
{
    class TestPushButtonAndBevelUtil : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawFocusPushButtonUtil()
        {
            QStyleOptionButton option;
            option.initFrom(testWidget());

            option.state |= QStyle::State_HasFocus;
            option.text = "test 1";
            style()->drawControl(QStyle::CE_PushButton, &option, painter(), testWidget());
        }

        void testDrawPushButtonBevelDefaultButton()
        {
            QStyleOptionButton option;
            option.initFrom(testWidget());

            option.features |= QStyleOptionButton::DefaultButton;
            option.text = "test 2";
            style()->drawControl(QStyle::CE_PushButtonBevel, &option, painter(), testWidget());
        }

        void testDrawPushButtonBevelAutoDefaultButton()
        {
            QStyleOptionButton option;
            option.initFrom(testWidget());

            option.features |= QStyleOptionButton::AutoDefaultButton;
            option.text = "test 3";
            style()->drawControl(QStyle::CE_PushButtonBevel, &option, painter(), testWidget());
        }
    };

    INIT_TESTWIDGET(QPushButton);
    TestPushButtonAndBevelUtil drawUtilInstance(this);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawFocusPushButtonUtil);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawPushButtonBevelDefaultButton);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawPushButtonBevelAutoDefaultButton);
}

TEST_F(TestForDrawUtil, drawControlTabBarTab)
{
    class TestTabBarTabDrawUtil : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawTabBarTabShapeUtil()
        {
            QStyleOptionTab tab;
            tab.initFrom(testWidget());
            tab.state |= (QStyle::State_Selected | QStyle::State_Enabled);
            tab.shape = QTabBar::RoundedEast;
            tab.position = QStyleOptionTab::OnlyOneTab;
            tab.selectedPosition = QStyleOptionTab::NotAdjacent;
            tab.cornerWidgets = QStyleOptionTab::NoCornerWidgets;
            tab.text = "test 1";

            style()->drawControl(QStyle::CE_TabBarTabShape, &tab, painter(), testWidget());
        }

        void testDrawTabBarTabLabelUtil()
        {
            QStyleOptionTab tab;
            tab.initFrom(testWidget());
            tab.state |= (QStyle::State_Selected | QStyle::State_Enabled);
            tab.shape = QTabBar::RoundedEast;
            tab.position = QStyleOptionTab::Beginning;
            tab.selectedPosition = QStyleOptionTab::NotAdjacent;
            tab.cornerWidgets = QStyleOptionTab::NoCornerWidgets;
            tab.text = "test 2";

            style()->drawControl(QStyle::CE_TabBarTabLabel, &tab, painter(), testWidget());
        }

        void testDrawTabBarTabHorizontalSelectedUtil()
        {
            QStyleOptionTab tab;
            tab.initFrom(testWidget());
            tab.state |= (QStyle::State_Selected | QStyle::State_Enabled);
            tab.shape = QTabBar::RoundedNorth;
            tab.position = QStyleOptionTab::Middle;
            tab.selectedPosition = QStyleOptionTab::NotAdjacent;
            tab.cornerWidgets = QStyleOptionTab::NoCornerWidgets;
            tab.text = "test 3";

            style()->drawControl(QStyle::CE_TabBarTab, &tab, painter(), testWidget());
        }

        void testDrawTabBarTabVerticalCheckedUtil()
        {
            QStyleOptionTab tab;
            tab.initFrom(testWidget());
            tab.state |= (QStyle::State_Selected | QStyle::State_Enabled);

            tab.shape = QTabBar::TriangularWest;
            tab.position = QStyleOptionTab::Beginning;
            tab.selectedPosition = QStyleOptionTab::NotAdjacent;
            tab.cornerWidgets = QStyleOptionTab::NoCornerWidgets;
            tab.text = "test 4";

            testWidget()->setProperty("_d_dtk_tabbartab_type", true);
            style()->drawControl(QStyle::CE_TabBarTab, &tab, painter(), testWidget());
        }

        void testDrawTabBarTabVerticalCheckedAndOffStateUtil()
        {
            QStyleOptionTab tab;
            tab.initFrom(testWidget());
            tab.state |= QStyle::State_Enabled;

            tab.shape = QTabBar::RoundedSouth;
            tab.position = QStyleOptionTab::Beginning;
            tab.selectedPosition = QStyleOptionTab::NotAdjacent;
            tab.cornerWidgets = QStyleOptionTab::NoCornerWidgets;
            tab.text = "test 4";

            testWidget()->setProperty("_d_dtk_tabbartab_type", true);
            style()->drawControl(QStyle::CE_TabBarTab, &tab, painter(), testWidget());
        }
    };

    INIT_TESTWIDGET(QTabBar);
    TestTabBarTabDrawUtil drawUtilInstance(this);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawTabBarTabShapeUtil);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawTabBarTabLabelUtil);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawTabBarTabHorizontalSelectedUtil);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawTabBarTabVerticalCheckedUtil);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawTabBarTabVerticalCheckedAndOffStateUtil);
}

TEST_F(TestForDrawUtil, drawControlRubberBand)
{
    auto drawRunnerBandUtil = [&]() {
        QStyleOptionRubberBand option;
        option.initFrom(testWidget);

        style->drawControl(QStyle::CE_RubberBand, &option, painter, testWidget);
    };

    INIT_TESTWIDGET(QWidget);
    ASSERT_DrawFuncHasData(drawRunnerBandUtil);
}

TEST_F(TestForDrawUtil, drawControlHeader)
{
    auto drawHeaderUtil = [&]() {
        QStyleOptionHeader header;
        header.initFrom(testWidget);

        header.sortIndicator = QStyleOptionHeader::SortUp;
        style->drawControl(QStyle::CE_Header, &header, painter, testWidget);
    };

    INIT_TESTWIDGET(QHeaderView(Qt::Horizontal));
    ASSERT_DrawFuncHasData(drawHeaderUtil);
}

TEST_F(TestForDrawUtil, drawShapedFrameUtil)
{
    class TestShapedFrameDrawUtil : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;

        void testBoxPlainShapedFrame()
        {
            QStyleOptionFrame option;
            option.initFrom(testWidget());

            option.frameShape = QFrame::Box;
            option.lineWidth = 3;
            option.midLineWidth = 2;
            style()->drawControl(QStyle::CE_ShapedFrame, &option, painter(), testWidget());
        }

        void testBoxSunkenShapedFrame()
        {
            QStyleOptionFrame option;
            option.initFrom(testWidget());

            option.state = QStyle::State_Sunken;
            option.frameShape = QFrame::Box;
            option.lineWidth = 2;
            option.midLineWidth = 3;
            style()->drawControl(QStyle::CE_ShapedFrame, &option, painter(), testWidget());
        }

        void testBoxRaisedShapedFrame()
        {
            QStyleOptionFrame option;
            option.initFrom(testWidget());

            option.state = QStyle::State_Raised;
            option.frameShape = QFrame::Box;
            option.lineWidth = 2;
            option.midLineWidth = 3;
            style()->drawControl(QStyle::CE_ShapedFrame, &option, painter(), testWidget());
        }

        void testStyledPanelShapedFrame()
        {
            QStyleOptionFrame option;
            option.initFrom(testWidget());

            option.frameShape = QFrame::StyledPanel;
            option.lineWidth = 2;
            option.midLineWidth = 3;
            style()->drawControl(QStyle::CE_ShapedFrame, &option, painter(), testWidget());
        }

        void testStyledPanelSunkenFrame()
        {
            QStyleOptionFrame option;
            option.initFrom(testWidget());

            option.state = QStyle::State_Sunken;
            option.frameShape = QFrame::StyledPanel;
            option.lineWidth = 2;
            option.midLineWidth = 3;
            style()->drawControl(QStyle::CE_ShapedFrame, &option, painter(), nullptr);
        }

        void testPanelPlainFrame()
        {
            QStyleOptionFrame option;
            option.initFrom(testWidget());

            option.frameShape = QFrame::Panel;
            option.lineWidth = 2;
            option.midLineWidth = 3;
            style()->drawControl(QStyle::CE_ShapedFrame, &option, painter(), testWidget());
        }

        void testPanelSunkenFrame()
        {
            QStyleOptionFrame option;
            option.initFrom(testWidget());

            option.state = QStyle::State_Sunken;
            option.frameShape = QFrame::Panel;
            option.lineWidth = 2;
            option.midLineWidth = 3;
            style()->drawControl(QStyle::CE_ShapedFrame, &option, painter(), testWidget());
        }

        void testWinPanelSunkenFrame()
        {
            QStyleOptionFrame option;
            option.initFrom(testWidget());

            option.state = QStyle::State_Sunken;
            option.frameShape = QFrame::WinPanel;
            option.lineWidth = 2;
            option.midLineWidth = 3;
            style()->drawControl(QStyle::CE_ShapedFrame, &option, painter(), testWidget());
        }

        void testWinPanelPlainFrame()
        {
            QStyleOptionFrame option;
            option.initFrom(testWidget());

            option.frameShape = QFrame::WinPanel;
            option.lineWidth = 2;
            option.midLineWidth = 3;
            style()->drawControl(QStyle::CE_ShapedFrame, &option, painter(), testWidget());
        }

        void testHLinePlainFrame()
        {
            QStyleOptionFrame option;
            option.initFrom(testWidget());

            option.frameShape = QFrame::HLine;
            option.lineWidth = 2;
            option.midLineWidth = 3;
            style()->drawControl(QStyle::CE_ShapedFrame, &option, painter(), testWidget());
        }

        void testVLineSunkenFrame()
        {
            QStyleOptionFrame option;
            option.initFrom(testWidget());

            option.state = QStyle::State_Sunken;
            option.frameShape = QFrame::VLine;
            option.lineWidth = 2;
            option.midLineWidth = 3;
            style()->drawControl(QStyle::CE_ShapedFrame, &option, painter(), testWidget());
        }
    };

    INIT_TESTWIDGET(QFrame);
    TestShapedFrameDrawUtil drawUtilInstance(this);
    ASSERT_DrawFuncHasData(drawUtilInstance.testBoxPlainShapedFrame);
    ASSERT_DrawFuncHasData(drawUtilInstance.testBoxSunkenShapedFrame);
    ASSERT_DrawFuncHasData(drawUtilInstance.testBoxRaisedShapedFrame);
    ASSERT_DrawFuncHasData(drawUtilInstance.testStyledPanelShapedFrame);
    ASSERT_DrawFuncHasData(drawUtilInstance.testStyledPanelSunkenFrame);
    ASSERT_DrawFuncHasData(drawUtilInstance.testPanelPlainFrame);
    ASSERT_DrawFuncHasData(drawUtilInstance.testPanelSunkenFrame);
    ASSERT_DrawFuncHasData(drawUtilInstance.testWinPanelSunkenFrame);
    ASSERT_DrawFuncHasData(drawUtilInstance.testWinPanelPlainFrame);
    ASSERT_DrawFuncHasData(drawUtilInstance.testHLinePlainFrame);
    ASSERT_DrawFuncHasData(drawUtilInstance.testVLineSunkenFrame);
}

TEST_F(TestForDrawUtil, drawControlComboBoxLabel)
{
    auto drawComboBoxLabelUtil = [&]() {
        QStyleOptionComboBox option;
        option.initFrom(testWidget);

        option.currentText = "test 1";
        style->drawControl(QStyle::CE_ComboBoxLabel, &option, painter, testWidget);
    };

    INIT_TESTWIDGET(QComboBox);
    ASSERT_DrawFuncHasData(drawComboBoxLabelUtil);
}

TEST_F(TestForDrawUtil, drawControlPushButtonLabel)
{
    class TestPushButtonLabelDrawUtil : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawWarningButtonLabel()
        {
            QStyleOptionButton option;
            option.initFrom(testWidget());

            option.features |= QStyleOptionButton::ButtonFeature(DStyleOptionButton::WarningButton);
            option.text = "test 1";
            option.icon = style()->standardIcon(QStyle::SP_FileIcon);
            option.direction = Qt::RightToLeft;
            option.state |= QStyle::State_On | QStyle::State_Sunken;
            option.features |= QStyleOptionButton::HasMenu;

            style()->drawControl(QStyle::CE_PushButtonLabel, &option, painter(), testWidget());
        }

        void testDrawSuggestButtonLabel()
        {
            QStyleOptionButton option;
            option.initFrom(testWidget());

            option.features |= QStyleOptionButton::ButtonFeature(DStyleOptionButton::SuggestButton);
            option.text = "test 1";
            option.icon = style()->standardIcon(QStyle::SP_FileIcon);
            option.direction = Qt::LeftToRight;
            option.state |= QStyle::State_On | QStyle::State_Sunken;
            option.features |= QStyleOptionButton::HasMenu;

            style()->drawControl(QStyle::CE_PushButtonLabel, &option, painter(), testWidget());
        }
    };

    INIT_TESTWIDGET(QPushButton);
    TestPushButtonLabelDrawUtil drawUtilInstance(this);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawWarningButtonLabel);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawSuggestButtonLabel);
}

TEST_F(TestForDrawUtil, drawControlProgressBar)
{
    class TestProgressBarDrawUtil : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawHorizontalProgressBar()
        {
            QStyleOptionProgressBar option;
            option.initFrom(testWidget());

            option.state |= QStyle::State_Horizontal;
            option.progress = 80;
            option.minimum = 0;
            option.maximum = 100;

            style()->drawControl(QStyle::CE_ProgressBar, &option, painter(), testWidget());
        }

        void testVerticalProgressBar()
        {
            QStyleOptionProgressBar option;
            option.initFrom(testWidget());

            option.state &= ~QStyle::State_Horizontal;
            option.progress = 50;
            option.minimum = 0;
            option.maximum = 100;

            style()->drawControl(QStyle::CE_ProgressBar, &option, painter(), testWidget());
        }
    };

    INIT_TESTWIDGET(QProgressBar);
    TestProgressBarDrawUtil drawUtilInstance(this);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawHorizontalProgressBar);
    ASSERT_DrawFuncHasData(drawUtilInstance.testVerticalProgressBar);
}

TEST_F(TestForDrawUtil, drawControlProgressBarGroove)
{
    class TestProgressBarGrooveDrawUtil : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testProgressBarGrooveUtil()
        {
            QStyleOptionProgressBar option;
            option.initFrom(testWidget());

            option.state |= QStyle::State_Horizontal;
            option.progress = 50;
            option.minimum = 0;
            option.maximum = 100;

            style()->drawControl(QStyle::CE_ProgressBarGroove, &option, painter(), testWidget());
        }
    };

    INIT_TESTWIDGET(QProgressBar);
    TestProgressBarGrooveDrawUtil drawUtilInstance(this);
    ASSERT_DrawFuncHasData(drawUtilInstance.testProgressBarGrooveUtil);
}

TEST_F(TestForDrawUtil, drawControlProgressBarContents)
{
    class TestProgressBarContentsDrawUtil : public TestDrawUtilIntegration
    {
    public:
        using TestDrawUtilIntegration::TestDrawUtilIntegration;
        void testDrawProgressBarTextVisibleUtl()
        {
            QStyleOptionProgressBar option;
            option.initFrom(testWidget());

            option.state |= QStyle::State_Horizontal;
            option.progress = 50;
            option.minimum = 0;
            option.maximum = 100;
            option.textVisible = true;
            option.text = "test 1";

            style()->drawControl(QStyle::CE_ProgressBarContents, &option, painter(), testWidget());
        }

        void testDrawProgressBarUtl()
        {
            QStyleOptionProgressBar option;
            option.initFrom(testWidget());

            option.state |= QStyle::State_Horizontal;
            option.progress = 100;
            option.minimum = 0;
            option.maximum = 100;
            option.rect = QRect(0, 0, 6, 4);

            style()->drawControl(QStyle::CE_ProgressBarContents, &option, painter(), testWidget());
        }

        void testDrawCenterProgressBarLabel()
        {
            QStyleOptionProgressBar option;
            option.initFrom(testWidget());

            option.state |= QStyle::State_Horizontal;
            option.progress = 50;
            option.minimum = 0;
            option.maximum = 100;
            option.rect = QRect(0, 0, 80, 40);
            option.text = "test 2";
            option.fontMetrics = testWidget()->fontMetrics();
            option.textAlignment = Qt::AlignCenter;

            style()->drawControl(QStyle::CE_ProgressBarLabel, &option, painter(), testWidget());
        }

        void testDrawRightProgressBarLabel()
        {
            QStyleOptionProgressBar option;
            option.initFrom(testWidget());

            option.state |= QStyle::State_Horizontal;
            option.progress = 10;
            option.minimum = 0;
            option.maximum = 100;
            option.rect = QRect(0, 0, 80, 40);
            option.text = "test 3";
            option.textAlignment = Qt::AlignRight;

            style()->drawControl(QStyle::CE_ProgressBarLabel, &option, painter(), testWidget());
        }

        void testDrawLeftProgressBarLabel()
        {
            QStyleOptionProgressBar option;
            option.initFrom(testWidget());

            option.state |= QStyle::State_Horizontal;
            option.progress = 90;
            option.minimum = 0;
            option.maximum = 100;
            option.rect = QRect(0, 0, 80, 40);
            option.text = "test 4";
            option.textAlignment = Qt::AlignLeft;

            style()->drawControl(QStyle::CE_ProgressBarLabel, &option, painter(), testWidget());
        }
    };

    INIT_TESTWIDGET(QProgressBar);
    TestProgressBarContentsDrawUtil drawUtilInstance(this);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawProgressBarTextVisibleUtl);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawProgressBarUtl);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawCenterProgressBarLabel);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawRightProgressBarLabel);
    ASSERT_DrawFuncHasData(drawUtilInstance.testDrawLeftProgressBarLabel);
}
