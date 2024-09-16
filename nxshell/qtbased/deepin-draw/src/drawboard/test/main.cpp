// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gui/drawboard.h"
#include "gui/drawboardtoolmgr.h"
#include "drawboardattrimgr.h"
#include "drawtoolfactory.h"
#include "csidewidthwidget.h"
#include "gui/toolattrimgr.h"
//#include "widgets/colorWidget/ccolorpickwidget.h"
#include "widgets/colorWidget/private/cnumbersliderwidget.h"
#include "widgets/colorWidget/private/ccolorpanel.h"
#include "widgets/colorWidget/cpickcolorattriwidget.h"
#include "attribution/attributewidget.h"
#include <QApplication>

static int fontNums[] = {8, 10, 12, 14, 16, 18, 24, 36, 48, 60, 72, 100};
static int penWidthArr[] = {1, 2, 4, 6, 8, 10, 12};

int arrNum2SliderValue(int *arr, int count, int fontNum)
{
    for (int i = count - 1; i >= 0; --i) {
        if (arr[i] <= fontNum) {
            return i;
        }
    }

    return 0;
}

void installAttriWidgets(DrawBoard *pBoard)
{
    pBoard->setAttributionManager(new ToolAttriMgr(pBoard));

    //2.注册画笔宽度设置控件
    auto penWidth = new SideWidthWidget(pBoard);
    penWidth->setMinimumWidth(90);

    // task:1708 画笔粗细属性设置项去掉“0px”
    setWgtAccesibleName(penWidth->menuComboBox(), "Pen width combox");
    penWidth->menuComboBox()->removeItem(0);


    auto combox = new ComboBoxSettingWgt();
    combox->setAttribution(9999);
    combox->comboBox()->insertItem(0, "矩形");
    combox->comboBox()->insertItem(1, "圆形");
    combox->comboBox()->insertItem(2, "三角形");
    combox->comboBox()->insertItem(3, "多角星");
    combox->comboBox()->insertItem(4, "多边形");
    combox->comboBox()->insertItem(5, "线");
    QObject::connect(combox->comboBox(), QOverload<int>::of(&QComboBox::currentIndexChanged), combox, [ = ](int index) {
        pBoard->setDrawAttribution(/*EPenWidth*/9999, index);
    });
    pBoard->attributionManager()->installComAttributeWgt(9999, combox, 0);

    auto colorPickWidget = new ColorSettingWgt();
    setWgtAccesibleName(colorPickWidget, "colorPanel");
    pBoard->attributionManager()->installComAttributeWgt(EPenColor, colorPickWidget,  QColor(0, 0, 0, 0));

    auto fillColorPickWidget = new ColorSettingWgt();
    setWgtAccesibleName(fillColorPickWidget, "colorPanel1");
    pBoard->attributionManager()->installComAttributeWgt(EBrushColor, fillColorPickWidget,  QColor(0, 0, 0, 0));


    {
        auto penWidthNumSlider = new NumberSlider;
        penWidthNumSlider->setTextVisiable(false);
        penWidthNumSlider->setFlagDiameter(5);
        penWidthNumSlider->setRange(1, sizeof(penWidthArr) / sizeof(int));
        setWgtAccesibleName(penWidthNumSlider, "EPenWidthProperty");
        pBoard->attributionManager()->installComAttributeWgt(EPenWidth, penWidthNumSlider);
        QObject::connect(penWidthNumSlider, &NumberSlider::valueChanged, penWidthNumSlider, [ = ](int _t1, EChangedPhase _t2) {
            emit pBoard->attributionManager()->helper()->attributionChanged(EPenWidth, penWidthArr[_t1 - 1], _t2);
        });

        QObject::connect(pBoard->attributionManager()->helper(), &AttributionManagerHelper::updateWgt, penWidthNumSlider, [ = ](
        QWidget * _t1, const QVariant & _t2) {
            if (penWidthNumSlider == _t1) {
                QSignalBlocker bloker(penWidthNumSlider);
                int w = arrNum2SliderValue(penWidthArr, sizeof(penWidthArr) / sizeof(int), _t2.toInt());
                penWidthNumSlider->setValue(w + 1);
            }
        });
    }

    {
        auto vectorBorderWidthSetting = new NumberSlider();
        vectorBorderWidthSetting->setRange(0, 10);
        vectorBorderWidthSetting->setTextVisiable(false);
        vectorBorderWidthSetting->setFlagDiameter(5);
        setWgtAccesibleName(vectorBorderWidthSetting, "EArrowWidthProperty");
        pBoard->attributionManager()->installComAttributeWgt(EBorderWidth, vectorBorderWidthSetting);
        QObject::connect(vectorBorderWidthSetting, &NumberSlider::valueChanged, vectorBorderWidthSetting, [ = ](int _t1, EChangedPhase _t2) {
            emit pBoard->attributionManager()->helper()->attributionChanged(EBorderWidth, _t1, _t2);
        });

        QObject::connect(pBoard->attributionManager()->helper(), &AttributionManagerHelper::updateWgt, vectorBorderWidthSetting, [ = ](
        QWidget * _t1, const QVariant & _t2) {
            if (vectorBorderWidthSetting == _t1) {
                QSignalBlocker bloker(vectorBorderWidthSetting);
                vectorBorderWidthSetting->setValue(_t2.toInt());
            }
        });
    }

    {
        auto fontSizeSetting = new NumberSlider();
        fontSizeSetting->setTextVisiable(false);
        fontSizeSetting->setFlagDiameter(12);
        //fontSizeSetting->setRange(8, 100);
        fontSizeSetting->setRange(1, sizeof(fontNums) / sizeof(int));
        setWgtAccesibleName(fontSizeSetting, "EEraserWidthProperty");
        pBoard->attributionManager()->installComAttributeWgt(EFontSize, fontSizeSetting);
        QObject::connect(fontSizeSetting, &NumberSlider::valueChanged, fontSizeSetting, [ = ](int _t1, EChangedPhase _t2) {
            int fontNum = fontNums[_t1 - 1];
            emit pBoard->attributionManager()->helper()->attributionChanged(EFontSize, fontNum, _t2);
        });

        QObject::connect(pBoard->attributionManager()->helper(), &AttributionManagerHelper::updateWgt, fontSizeSetting, [ = ](
        QWidget * _t1, const QVariant & _t2) {
            if (fontSizeSetting == _t1) {
                QSignalBlocker bloker(fontSizeSetting);
                int sliderValue = arrNum2SliderValue(fontNums, sizeof(fontNums) / sizeof(int), _t2.toInt());
                fontSizeSetting->setValue(sliderValue + 1);

            }
        });
    }

    {
        auto eraseWidthSetting = new NumberSlider();
        eraseWidthSetting->setTextVisiable(false);
        eraseWidthSetting->setFlagDiameter(5);
        eraseWidthSetting->setRange(1, 500);
        eraseWidthSetting->setTickInterval(2);
        setWgtAccesibleName(eraseWidthSetting, "ERectWidthProperty");
        pBoard->attributionManager()->installComAttributeWgt(EEraserWidth, eraseWidthSetting);
        QObject::connect(eraseWidthSetting, &NumberSlider::valueChanged, eraseWidthSetting, [ = ](int _t1, EChangedPhase _t2) {
            emit pBoard->attributionManager()->helper()->attributionChanged(EEraserWidth, _t1, _t2);
        });

        QObject::connect(pBoard->attributionManager()->helper(), &AttributionManagerHelper::updateWgt, eraseWidthSetting, [ = ](
        QWidget * _t1, const QVariant & _t2) {
            if (eraseWidthSetting == _t1) {
                QSignalBlocker bloker(eraseWidthSetting);
                eraseWidthSetting->setValue(_t2.toInt());
            }
        });
    }

    {
        auto streakBeginStyle = new ComboBoxSettingWgt(QObject::tr("Start"));
        auto pStreakStartComboBox = new QComboBox;
        setWgtAccesibleName(pStreakStartComboBox, "Line start style combox");
        pStreakStartComboBox->setFixedSize(QSize(90, 36));
        pStreakStartComboBox->setIconSize(QSize(34, 20));
        pStreakStartComboBox->setFocusPolicy(Qt::NoFocus);

        pStreakStartComboBox->addItem(QIcon::fromTheme("ddc_none_arrow"), "");
        pStreakStartComboBox->addItem(QIcon::fromTheme("ddc_right_circle"), "");
        pStreakStartComboBox->addItem(QIcon::fromTheme("ddc_right_fill_circle"), "");
        pStreakStartComboBox->addItem(QIcon::fromTheme("ddc_right_arrow"), "");
        pStreakStartComboBox->addItem(QIcon::fromTheme("ddc_right_fill_arrow"), "");

        streakBeginStyle->setComboBox(pStreakStartComboBox);
        QObject::connect(pStreakStartComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), streakBeginStyle, [ = ](int index) {
            pBoard->setDrawAttribution(EStreakBeginStyle, index);
        });
        QObject::connect(pBoard->attributionManager()->helper(), &AttributionManagerHelper::updateWgt, streakBeginStyle, [ = ](QWidget * pWgt, const QVariant & var) {
            if (pWgt == streakBeginStyle) {
                QSignalBlocker bloker(pStreakStartComboBox);
                pStreakStartComboBox->setCurrentIndex(var.toInt());
            }
        });

        streakBeginStyle->setAttribution(EStreakBeginStyle);
        pBoard->attributionManager()->installComAttributeWgt(EStreakBeginStyle, streakBeginStyle, 0);
    }

    {
        auto streakEndStyle = new ComboBoxSettingWgt(QObject::tr("End"));
        streakEndStyle->setAttribution(EStreakEndStyle);
        auto pStreakEndComboBox = new QComboBox;
        setWgtAccesibleName(pStreakEndComboBox, "Line end style combox");
        pStreakEndComboBox->setFixedSize(QSize(90, 36));
        pStreakEndComboBox->setIconSize(QSize(34, 20));
        pStreakEndComboBox->setFocusPolicy(Qt::NoFocus);

        pStreakEndComboBox->addItem(QIcon::fromTheme("ddc_none_arrow"), "");
        pStreakEndComboBox->addItem(QIcon::fromTheme("ddc_left_circle"), "");
        pStreakEndComboBox->addItem(QIcon::fromTheme("ddc_left_fill_circle"), "");
        pStreakEndComboBox->addItem(QIcon::fromTheme("ddc_left_arrow"), "");
        pStreakEndComboBox->addItem(QIcon::fromTheme("ddc_left_fill_arrow"), "");

        streakEndStyle->setComboBox(pStreakEndComboBox);
        QObject::connect(pStreakEndComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), streakEndStyle, [ = ](int index) {
            pBoard->setDrawAttribution(EStreakEndStyle, index);
        });
        QObject::connect(pBoard->attributionManager()->helper(), &AttributionManagerHelper::updateWgt, streakEndStyle, [ = ](QWidget * pWgt, const QVariant & var) {
            if (pWgt == streakEndStyle) {
                QSignalBlocker bloker(pStreakEndComboBox);
                pStreakEndComboBox->setCurrentIndex(var.toInt());
            }
        });
        pBoard->attributionManager()->installComAttributeWgt(EStreakEndStyle, streakEndStyle, 0);
    }

    {
        auto eraseWidthSetting = new NumberSlider();
        eraseWidthSetting->setText(QObject::tr("Radius"));
        eraseWidthSetting->setTextVisiable(false);
        eraseWidthSetting->setFlagDiameter(10);
        eraseWidthSetting->setRange(5, 60);
        setWgtAccesibleName(eraseWidthSetting, "ERectWidthProperty");
        pBoard->attributionManager()->installComAttributeWgt(EUserAttri + 2, eraseWidthSetting);
        QObject::connect(eraseWidthSetting, &NumberSlider::valueChanged, eraseWidthSetting, [ = ](int _t1, EChangedPhase _t2) {
            emit pBoard->attributionManager()->helper()->attributionChanged(EUserAttri + 2, _t1, _t2);
        });

        QObject::connect(pBoard->attributionManager()->helper(), &AttributionManagerHelper::updateWgt, eraseWidthSetting, [ = ](
        QWidget * _t1, const QVariant & _t2) {
            if (eraseWidthSetting == _t1) {
                QSignalBlocker bloker(eraseWidthSetting);
                eraseWidthSetting->setValue(_t2.toInt());
            }
        });
    }
}

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication a(argc, argv);

    DrawBoard::loadToolPlugins();
    DrawBoard board;
    board.addPage();
    board.toolManager()->addTool(blur);
//    board.toolManager()->addTool(polygonalStar);
//    board.toolManager()->addTool(triangle);
//    board.toolManager()->addTool(polygon);
//    board.toolManager()->addTool(EDrawComItemTool);
    board.toolManager()->addTool(EOpenTool);
    board.toolManager()->addTool(EClearTool);
    board.toolManager()->addTool(MoreTool + 2);
    board.show();

    installAttriWidgets(&board);

    return a.exec();
}
