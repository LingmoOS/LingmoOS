// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ATTRIBUTEITEMWIDGET_H
#define ATTRIBUTEITEMWIDGET_H

#include "globaldefine.h"

#include <QWidget>
#include <QMap>
#include <QLabel>
#include <QDebug>

#include <QComboBox>
#include <QSpinBox>
#include <QMetaType>
#include <QCheckBox>


class QSpacerItem;
class QHBoxLayout;

class CSpinBox;
class SeperatorLine;
class CColorPickWidget;
#ifdef USE_DTK
#include <DIconButton>
#include <DBlurEffectWidget>
DWIDGET_USE_NAMESPACE
#endif

Q_DECLARE_METATYPE(QMargins)

//所有图元都可以拥有的属性：画笔颜色、画笔宽度、填充颜色
enum EComAttri {
    ETitle,

    EBrushColor,
    EPenColor,
    EBorderWidth,  //可设置为0px
    EPenStyle,

    EPenWidth,    //不支持0px

    ERectRadius,

    EStarAnchor,
    EStarInnerOuterRadio,

    EPolygonSides,

    EStreakBeginStyle,
    EStreakEndStyle,
    EStreakStyle,

    EFontColor,
    EFontFamily,
    EFontWeightStyle,
    EFontSize,

    EImageLeftRot,
    EImageRightRot,
    EImageHorFilp,
    EImageVerFilp,
    EImageAdaptScene,

    EGroupWgt,

    EPictureAttri,
    EBlurAttri,
    EFullFontAttri,
    ECutToolAttri,
    EEraserWidth,

    EPenWidthProperty,
    EArrowWidthProperty,
    EEraserWidthProperty,
    ERectWidthProperty,

    ERectProperty,
    ERotProperty,
    EOrderProperty,
    EStyleProper,
    EEnablePenStyle,
    EEnableBrushStyle,
    EEBlurStyle,
    EPenBrushColor,
    EEnablePenBrushStyle,
    EUserAttri = 1000
};

#define AttriWidgetVar "variant"
#define ChildAttriWidget "childAttriWidget"
#define ParentAttriWidget "parentAttriWidget"

struct DRAWLIB_EXPORT SAttri {
    int      attri = -1;
    QVariant var;
    SAttri(int atr = -1, const QVariant &vr = QVariant()): attri(atr), var(vr) {}
    bool operator < (const SAttri &other)
    {
        return (attri < other.attri);
    }
    bool operator == (const SAttri &other)
    {
        return (attri == other.attri && var == other.var);
    }

    friend QDebug operator<<(QDebug debug, const SAttri &c)
    {
        QDebugStateSaver saver(debug);
        debug.nospace() << '(' << c.attri << ", " << c.var << ')';

        return debug;
    }
};

class DRAWLIB_EXPORT SAttrisList: public QList<SAttri>
{
public:
    SAttrisList(const QList<int> &list = QList<int>());

    SAttrisList insected(const SAttrisList &other);
    SAttrisList unionAtrri(const SAttrisList &other);

    bool        haveAttribution(int attri);
    SAttri getAttribution(int attri);
};


class DRAWLIB_EXPORT AttributeWgt: public QWidget
{
    Q_OBJECT
public:
    AttributeWgt(int attri = -1, QWidget *parent = nullptr);

    void setAttribution(int arrti);
    int  attribution();

    void addHSeparateLine();
    void addVSeparateLine();
    virtual void  setVar(const QVariant &var);
    virtual QSize recommendedSize()const;

signals:
    void attriChanged(const QVariant var, int phase = EChanged);

protected:
    bool event(QEvent *e) override;

private:
    int _attri = -1;
};

//可用于颜色相关的控件如EPenColor，EBrushColor，EFontColor
class DRAWLIB_EXPORT ColorSettingButton: public AttributeWgt
{
    Q_OBJECT

public:
    enum EColorFill {EFillArea, EFillBorder};

    ColorSettingButton(const QString &text = "color",
                       QWidget *parent = nullptr, bool autoConnect = true);


    void  setVar(const QVariant &var) override;

    CColorPickWidget *colorPick();
    QColor getColor();

signals:
    void colorChanged(const QColor &color, int phase = EChanged);

public slots:
    void setColor(const QColor &color, int phase = EChanged);
    void setDefaultButtonSize(const QSize &sz);
    void setColorFill(EColorFill fillPos);

protected:
    void  paintEvent(QPaintEvent *event) override;
    void  mousePressEvent(QMouseEvent *event) override;
public:

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void paintFillArea(QPainter *painter);
    void paintFillBorder(QPainter *painter);
    void paintBackground(QPainter *painter);

private:
    QString     _text;
    QColor      _color;
    EColorFill  _fillPos = EFillArea;
    QSize       _defaultButtonSize = QSize(24, 24);
    const int   _space = 6;

    bool        _connectedColorPicker = false;
};
class ColorPanel;
class DRAWLIB_EXPORT ColorSettingWgt : public AttributeWgt
{
    Q_OBJECT
public:
    ColorSettingWgt(QWidget *parent = nullptr);
    void  setVar(const QVariant &var) override;
    const ColorPanel *panel();
signals:
    void colorChanged(const QColor &color, int phase = EChanged);
public slots:
    void setColor(const QColor &color, int phase = EChanged);
private:
    ColorPanel      *m_colorPanel;
    QColor      _color;
};

class DRAWLIB_EXPORT ComboBoxSettingWgt: public AttributeWgt
{
    Q_OBJECT
public:
    ComboBoxSettingWgt(const QString &text = "", QWidget *parent = nullptr);

    void setText(const QString &text);

    QComboBox *comboBox();

    void setComboBox(QComboBox *pBox);
private:
    Q_SLOT void onCurrentChanged(int index);

private:
    QComboBox *_comBox = nullptr;
    QLabel    *_lab    = nullptr;
};

class DRAWLIB_EXPORT SpinBoxSettingWgt: public AttributeWgt
{
    Q_OBJECT

public:
    SpinBoxSettingWgt(const QString &text = "", QWidget *parent = nullptr);

    void  setVar(const QVariant &var) override;

    CSpinBox *spinBox();

    QSize recommendedSize()const override;

signals:
    void widthChanged(int width);

private:
    CSpinBox *_spinBox = nullptr;
    QLabel   *_lab     = nullptr;
};

class DRAWLIB_EXPORT CheckBoxSettingWgt: public AttributeWgt
{
    Q_OBJECT

public:
    CheckBoxSettingWgt(const QString &text = "", QWidget *parent = nullptr);

    void  setVar(const QVariant &var) override;

    QCheckBox *checkBox();

signals:
    void checkChanged(bool bChecked);
    void checkStatusChanged(int status);
    void checkEnable(bool bChecked);

private:
    QCheckBox *_checkBox = nullptr;
};

#endif // ATTRIBUTEITEMWIDGET_H
