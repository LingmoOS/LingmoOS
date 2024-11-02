#ifndef COLORDIALOG_H
#define COLORDIALOG_H

#include <QDialog>
#include <QRadioButton>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QRegExp>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QPainterPath>
#include "colorsquare.h"
#include "gradientslider.h"
#include "view/sizedate.h"
#include "maskwidget.h"

class ColorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ColorDialog(QWidget *parent = nullptr);
    ~ColorDialog();

    QLineEdit *m_colorLineEdit;
    QColor color() const;

protected:
    void paintEvent(QPaintEvent *event);

private:
    QVBoxLayout *m_Layout;

    ColorSquare *m_colorSquare = nullptr;
    GradientSlider *m_gradientSlider = nullptr;

    QHBoxLayout *m_topLayout;
    QFrame *m_frame1;
    QPushButton *m_closeBtn;
    QSpacerItem *m_TopSpacer;

    QHBoxLayout *m_midLayout;
    QFrame *m_frame2;
    QPushButton *m_colorPicker;

    QHBoxLayout *m_bottomLayout;
    QFrame *m_frame3;

    QVBoxLayout *m_hexLayout;
    QLabel *m_hexLabel;

    QVBoxLayout *m_redLayout;
    QSpinBox *m_spinBoxR;
    QLabel *m_labelR;

    QVBoxLayout *m_greenLayout;
    QSpinBox *m_spinBoxG;
    QLabel *m_labelG;

    QVBoxLayout *m_blueLayout;
    QSpinBox *m_spinBoxB;
    QLabel *m_labelB;

    enum CheckedColor
    {
        H,S,V,R,G,B
    }checkedColor;

    QSpinBox*           spinAlpha;
    GradientSlider*     sliderAlpha;

    QRadioButton*       radioHue;
    QSpinBox*           spinHue;
    GradientSlider*     sliderHue;

    QRadioButton*       radioSat;
    QSpinBox*           spinSat;
    GradientSlider*     sliderSat;

    QRadioButton*       radioVal;
    QSpinBox*           spinVal;
    GradientSlider*     sliderVal;

    QRadioButton*       radioRed;
    QSpinBox*           spinRed;
    GradientSlider*     sliderRed;

    QRadioButton*       radioGreen;
    QSpinBox*           spinGreen;
    GradientSlider*     sliderGreen;

    QRadioButton*       radioBlue;
    QSpinBox*           spinBlue;
    GradientSlider*     sliderBlue;

    void initLayout();
    void setupInit();
    void signalsBind();
    void drawSlider();
    void setHorizontalSlider();
    void SetColor(QColor color);

private Q_SLOTS:
    void updateWidgetsSlot();
    void setRgbSlot();
    void SetHsvSlot();
    void colorSelectedSolt();

Q_SIGNALS:
    void colorChanged(QColor);
    void checkedChanged(char);
    void colorSelected(QColor);
};

#endif // COLORDIALOG_H
