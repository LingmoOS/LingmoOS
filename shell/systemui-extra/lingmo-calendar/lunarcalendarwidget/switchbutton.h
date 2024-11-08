#ifndef SWITCHBUTTON_H
#define SWITCHBUTTON_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QPainterPath>
#include <QEvent>
#include <QGSettings/QGSettings>


#define OFF_BG_DARK_COLOR "#404040"
#define OFF_HOVER_BG_DARK_COLOR "#666666"
#define ON_BG_DARK_COLOR "#3790FA"
#define ON_HOVER_BG_DARK_COLOR "#40A9FB"
#define DISABLE_DARK_COLOR "#474747"
#define DISABLE_RECT_DARK_COLOR "#6E6E6E"
#define ENABLE_RECT_DARK_COLOR "#FFFFFF"

#define OFF_BG_LIGHT_COLOR "#E0E0E0"
#define OFF_HOVER_BG_LIGHT_COLOR "#B3B3B3"
#define ON_BG_LIGHT_COLOR "#3790FA"
#define ON_HOVER_BG_LIGHT_COLOR "#40A9FB"
#define DISABLE_LIGHT_COLOR "#E9E9E9"
#define DISABLE_RECT_LIGHT_COLOR "#B3B3B3"
#define ENABLE_RECT_LIGHT_COLOR "#FFFFFF"
class SwitchButton : public QWidget
{
    Q_OBJECT

public:
    SwitchButton(QWidget *parent = 0);
    ~SwitchButton();

    void setChecked(bool checked);
    void setAnimation(bool on);

    bool isChecked();
    void setDisabledFlag(bool);
    bool getDisabledFlag();

protected:
    void mousePressEvent(QMouseEvent *);
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

    void drawBg(QPainter *painter);
    void drawSlider(QPainter *painter);
    void changeColor(const QString &themes);

private:
    bool checked; //切换的判断
    bool disabled;

    void animation(QPainter *painter);
    QRect rect;
    bool isMoving;    //滑块动作判断
    bool isAnimation; // 是否允许动画执行

    QColor bgColorOff;
    QColor bgColorOn;
    QColor bgHoverOnColor;
    QColor bgHoverOffColor;
    QColor bgColorDisabled;

    QColor sliderColorEnabled;
    QColor sliderColorDisabled;


    QColor rectColorEnabled;
    QColor rectColorDisabled;

    QColor sliderColorOff;
    QColor sliderColorOn;
    QGSettings *m_qtThemeSetting;
    QGSettings *m_gtkThemeSetting;

    int space;      //滑块离背景间隔
    int rectRadius; //圆角角度

    int mStep; //移动步长
    int mStartX;
    int mEndX;

    bool hover;
    QTimer *mTimer;


private Q_SLOTS:
    void updatevalue();


Q_SIGNALS:
    void checkedChanged(bool checked);
    void disabledClick();
};

#endif // SWITCHBUTTON_H
