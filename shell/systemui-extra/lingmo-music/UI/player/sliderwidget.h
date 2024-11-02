#ifndef SLIDERWIDGET_H
#define SLIDERWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QHBoxLayout>
#include <QSlider>
#include <QEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QFocusEvent>
#include <QX11Info>

class SliderWidget : public QDialog
{
    Q_OBJECT

public:
    explicit SliderWidget(QWidget *parent = nullptr);

    QSlider *vSlider = nullptr;

    void changeVolumePos(int posX, int posY, int width, int height);
    void initColor();
    void initUi();
    void setColor(QColor c){m_color = c;}
    void setRadius(int radius){m_radius = radius;repaint();}

protected:
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;   //鼠标滑块点击  事件过滤器
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent *event) override;

private:
    int volunmPosX;
    int volunmPosY;
    int volunmPosWidth;
    int volunmPosHeight;

    QColor m_color;
    int m_radius;

    QHBoxLayout *HLayout = nullptr;
};


#endif // SLIDERWIDGET_H
