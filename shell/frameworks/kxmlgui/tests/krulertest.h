/* -*- c++ -*- */

#ifndef krulertest_h
#define krulertest_h

#include <QCheckBox>
#include <QFrame>
#include <QRadioButton>

class KRuler;
class QWidget;
class QGridLayout;
class QGroupBox;
class QLabel;
class QSpinBox;
class QDoubleSpinBox;

class MouseWidget : public QFrame
{
    Q_OBJECT
public:
    MouseWidget(QWidget *parent = nullptr);

Q_SIGNALS:
    void newXPos(int);
    void newYPos(int);
    void newWidth(int);
    void newHeight(int);

protected:
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void resizeEvent(QResizeEvent *) override;

private:
    bool mouseButtonDown;
};

class KRulerTest : public QWidget
{
    Q_OBJECT
public:
    KRulerTest();
    ~KRulerTest() override;

private Q_SLOTS:
    void slotNewWidth(int);
    void slotNewHeight(int);

    void slotSetTinyMarks(bool);
    void slotSetLittleMarks(bool);
    void slotSetMediumMarks(bool);
    void slotSetBigMarks(bool);
    void slotSetEndMarks(bool);
    void slotSetRulerPointer(bool);

    void slotSetRulerLength(int);
    void slotFixRulerLength(bool);
    void slotSetMStyle(int);
    void slotUpdateShowMarks();
    void slotCheckLength(bool);

    void slotSetRotate(double);
    void slotSetXTrans(double);
    void slotSetYTrans(double);

private:
    KRuler *hruler, *vruler;
    QGridLayout *layout;
    QFrame *mainframe = nullptr;
    MouseWidget *bigwidget = nullptr;
    QFrame *miniwidget = nullptr;

    QLabel *mouse_message;
    QGroupBox *showMarks, *lineEdit, *vertrot;
    QCheckBox *showTM, *showLM, *showMM, *showBM, *showEM, *showPT, *fixLen;
    QSpinBox *beginMark, *endMark, *lengthInput;
    QDoubleSpinBox *transX, *transY, *rotV;
    QGroupBox *metricstyle;
    QRadioButton *pixelmetric, *inchmetric, *mmmetric, *cmmetric, *mmetric;
};
#endif
