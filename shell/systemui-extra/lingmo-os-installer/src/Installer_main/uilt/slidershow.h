#ifndef SLIDERSHOW_H
#define SLIDERSHOW_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QImage>
namespace KInstaller {
class SlideShow : public QWidget
{
    Q_OBJECT
public:
    explicit SlideShow(QWidget *parent = nullptr);
    void initUI();
    void initAllConnect();
    void addStyleSheet();
    void loadSlides();
    QImage addtransterstr(QImage &image, QRect rct, QString str,QFont font,QColor color);
signals:

public slots:
    void next();
    void prev();
    void start();
    void changeSlide(int index);
protected:
    void keyPressEvent(QKeyEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

public:
    QPushButton* m_next;
    QPushButton* m_prev;
    QTimer* timer;
    int num ;
    int m_currentIndex;
    QStackedWidget *sliderWidget;
};

class Slide : public QLabel
{
    Q_OBJECT
public:
    Slide(QWidget *parent = nullptr) : QLabel(parent)
    {
        this->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        this->setAlignment(Qt::AlignCenter);

    }
};

}
#endif // SLIDERSHOW_H
