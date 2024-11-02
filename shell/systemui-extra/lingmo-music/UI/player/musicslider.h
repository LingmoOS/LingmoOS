#ifndef MUSICSLIDER_H
#define MUSICSLIDER_H

#include <QObject>
#include <QSlider>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QWheelEvent>

class MusicSlider : public QSlider
{
    Q_OBJECT
public:
    MusicSlider(QWidget *parent);
    void isPlaying(bool isPlaying);
    void initStyle();
    void changeStyleColor();

protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void enterEvent(QEvent *event) Q_DECL_OVERRIDE;
    void leaveEvent(QEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *e) Q_DECL_OVERRIDE;
private:

private:
    bool m_isPlaying;
};

#endif // MUSICSLIDER_H
