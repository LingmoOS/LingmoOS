#include "krulertest.h"

#include <KRuler>

#include <QApplication>
#include <QButtonGroup>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QSpinBox>

/*
void
MyCheckBox::mouseReleaseEvent(QMouseEvent *e )
{
  QButton::mouseReleaseEvent(e);
  if ();
}
*/

MouseWidget::MouseWidget(QWidget *parent)
    : QFrame(parent)
{
}

void MouseWidget::mousePressEvent(QMouseEvent *e)
{
    mouseButtonDown = true;
    Q_EMIT newXPos(e->x());
    Q_EMIT newYPos(e->y());
}

void MouseWidget::mouseReleaseEvent(QMouseEvent *)
{
    mouseButtonDown = false;
}

void MouseWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (mouseButtonDown) {
        Q_EMIT newXPos(e->x());
        Q_EMIT newYPos(e->y());
    }
}

void MouseWidget::resizeEvent(QResizeEvent *r)
{
    Q_EMIT newWidth(r->size().width());
    Q_EMIT newHeight(r->size().height());
}

KRulerTest::KRulerTest()
    : QWidget(nullptr)
{
    QVBoxLayout *topLayout = new QVBoxLayout(this);
    mainframe = new QFrame(this);
    topLayout->addWidget(mainframe);

    layout = new QGridLayout(mainframe);

    miniwidget = new QFrame(mainframe);
    miniwidget->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
    bigwidget = new MouseWidget(mainframe);
    bigwidget->setFrameStyle(QFrame::WinPanel | QFrame::Sunken);

    //  QRect bwrect = bigwidget->frameRect();
    //  qDebug("big rect: top%i left%i bottom%i right%i",
    //    bwrect.top(), bwrect.left(), bwrect.bottom(), bwrect.right());
    hruler = new KRuler(Qt::Horizontal, mainframe);
    //  hruler->setRange( bwrect.left(), bwrect.right() );
    hruler->setRange(0, 1000);
    //  hruler->setOffset( bwrect.left() - bigwidget->frameRect().left() );
    hruler->setOffset(0);

    vruler = new KRuler(Qt::Vertical, mainframe);
    vruler->setOffset(0);
    vruler->setRange(0, 1000);

    connect(bigwidget, &MouseWidget::newXPos, hruler, &KRuler::slotNewValue);
    connect(bigwidget, &MouseWidget::newYPos, vruler, &KRuler::slotNewValue);
    connect(bigwidget, &MouseWidget::newWidth, this, &KRulerTest::slotNewWidth);
    connect(bigwidget, &MouseWidget::newHeight, this, &KRulerTest::slotNewHeight);

    layout->addWidget(miniwidget, 0, 0);
    layout->addWidget(hruler, 0, 1);
    layout->addWidget(vruler, 1, 0);
    layout->addWidget(bigwidget, 1, 1);

    mouse_message = new QLabel(QStringLiteral("Press and hold mouse button\nfor pointer movement"), bigwidget);
    mouse_message->adjustSize();
    mouse_message->move(4, 4);

    showMarks = new QGroupBox(QStringLiteral("Show which marks ?"), bigwidget);
    showMarks->setFixedSize(140, 160);
    showMarks->move(330, 4);
    showTM = new QCheckBox(QStringLiteral("show tiny marks"), showMarks);
    showTM->adjustSize();
    showTM->move(5, 15);
    showTM->setChecked(true);
    connect(showTM, &QCheckBox::toggled, this, &KRulerTest::slotSetTinyMarks);
    showLM = new QCheckBox(QStringLiteral("show little marks"), showMarks);
    showLM->adjustSize();
    showLM->move(5, 35);
    showLM->setChecked(true);
    connect(showLM, &QCheckBox::toggled, this, &KRulerTest::slotSetLittleMarks);
    showMM = new QCheckBox(QStringLiteral("show medium marks"), showMarks);
    showMM->adjustSize();
    showMM->move(5, 55);
    showMM->setChecked(true);
    connect(showMM, &QCheckBox::toggled, this, &KRulerTest::slotSetMediumMarks);
    showBM = new QCheckBox(QStringLiteral("show big marks"), showMarks);
    showBM->adjustSize();
    showBM->move(5, 75);
    showBM->setChecked(true);
    connect(showBM, &QCheckBox::toggled, this, &KRulerTest::slotSetBigMarks);
    showEM = new QCheckBox(QStringLiteral("show end marks"), showMarks);
    showEM->adjustSize();
    showEM->move(5, 95);
    showEM->setChecked(true);
    connect(showEM, &QCheckBox::toggled, this, &KRulerTest::slotSetEndMarks);
    showPT = new QCheckBox(QStringLiteral("show ruler pointer"), showMarks);
    showPT->adjustSize();
    showPT->move(5, 115);
    showPT->setChecked(true);
    connect(showPT, &QCheckBox::toggled, this, &KRulerTest::slotSetRulerPointer);
    fixLen = new QCheckBox(QStringLiteral("fix ruler length"), showMarks);
    fixLen->adjustSize();
    fixLen->move(5, 135);
    fixLen->setChecked(true);
    connect(fixLen, &QCheckBox::toggled, this, &KRulerTest::slotFixRulerLength);
    connect(fixLen, &QCheckBox::toggled, this, &KRulerTest::slotCheckLength);

    lineEdit = new QGroupBox(QStringLiteral("Value of begin/end"), bigwidget);
    lineEdit->setFixedSize(140, 80);
    lineEdit->move(330, 4 + 160);
    beginMark = new QSpinBox(lineEdit);
    beginMark->setValue(0);
    beginMark->setRange(-1000, 1000);
    beginMark->move(5, 15);
    beginMark->setFixedSize(beginMark->sizeHint());
    connect(beginMark, qOverload<int>(&QSpinBox::valueChanged), hruler, &KRuler::slotNewOffset);
    connect(beginMark, qOverload<int>(&QSpinBox::valueChanged), vruler, &KRuler::slotNewOffset);
    endMark = new QSpinBox(lineEdit);
    endMark->setValue(0);
    endMark->setRange(-1000, 1000);
    endMark->move(5, 35);
    endMark->setFixedSize(endMark->sizeHint());
    connect(endMark, qOverload<int>(&QSpinBox::valueChanged), hruler, &KRuler::slotEndOffset);
    connect(endMark, qOverload<int>(&QSpinBox::valueChanged), vruler, &KRuler::slotEndOffset);
    lengthInput = new QSpinBox(lineEdit);
    lengthInput->setValue(0);
    lengthInput->setRange(-1000, 1000);
    lengthInput->move(5, 55);
    lengthInput->setFixedSize(lengthInput->sizeHint());
    connect(lengthInput, qOverload<int>(&QSpinBox::valueChanged), hruler, &KRuler::slotEndOffset);
    connect(lengthInput, qOverload<int>(&QSpinBox::valueChanged), vruler, &KRuler::slotEndOffset);

    vertrot = new QGroupBox(QStringLiteral("Value of rotate translate for Vert."), bigwidget);
    vertrot->setFixedSize(140, 80);
    vertrot->move(330, 4 + 160 + 80 + 4);
    transX = new QDoubleSpinBox(vertrot);
    transX->setValue(0.0);
    transX->setRange(-1000, 1000);
    transX->setSingleStep(1);
    transX->move(5, 15);
    transX->setFixedSize(transX->sizeHint());
    // transX->setLabel("transx", AlignLeft);
    connect(transX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &KRulerTest::slotSetXTrans);
    transY = new QDoubleSpinBox(vertrot);
    transY->setValue(-12.0);
    transY->setRange(-1000, 1000);
    transY->setSingleStep(1);
    transY->move(5, 35);
    transY->setFixedSize(transY->sizeHint());
    // transY->setLabel("transy", AlignLeft);
    connect(transY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &KRulerTest::slotSetYTrans);
    rotV = new QDoubleSpinBox(vertrot);
    rotV->setValue(90.0);
    rotV->setRange(-1000, 1000);
    rotV->setSingleStep(1);
    rotV->move(5, 55);
    rotV->setFixedSize(rotV->sizeHint());
    // rotV->setLabel("rot", AlignLeft);
    connect(rotV, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &KRulerTest::slotSetRotate);

    metricstyle = new QGroupBox(QStringLiteral("metric styles"), bigwidget);

    QButtonGroup *metricstyleButtons = new QButtonGroup(bigwidget);

    metricstyle->setFixedSize(100, 120);
    metricstyle->move(330 - 110, 4);
    pixelmetric = new QRadioButton(QStringLiteral("pixel"), metricstyle);
    pixelmetric->adjustSize();
    pixelmetric->move(5, 15);
    metricstyleButtons->addButton(pixelmetric, (int)KRuler::Pixel);
    inchmetric = new QRadioButton(QStringLiteral("inch"), metricstyle);
    inchmetric->adjustSize();
    inchmetric->move(5, 35);
    metricstyleButtons->addButton(inchmetric, (int)KRuler::Inch);
    mmmetric = new QRadioButton(QStringLiteral("millimeter"), metricstyle);
    mmmetric->adjustSize();
    mmmetric->move(5, 55);
    metricstyleButtons->addButton(mmmetric, (int)KRuler::Millimetres);
    cmmetric = new QRadioButton(QStringLiteral("centimeter"), metricstyle);
    cmmetric->adjustSize();
    cmmetric->move(5, 75);
    metricstyleButtons->addButton(cmmetric, (int)KRuler::Centimetres);
    mmetric = new QRadioButton(QStringLiteral("meter"), metricstyle);
    mmetric->adjustSize();
    mmetric->move(5, 95);
    metricstyleButtons->addButton(mmetric, (int)KRuler::Metres);
    connect(metricstyleButtons, &QButtonGroup::buttonClicked, this, [this, metricstyleButtons](QAbstractButton *button) {
        slotSetMStyle(metricstyleButtons->id(button));
    });

    slotUpdateShowMarks();
}

KRulerTest::~KRulerTest()
{
    delete layout;
    delete hruler;
    delete vruler;
    delete miniwidget;
    delete bigwidget;
    delete mainframe;
}

void KRulerTest::slotNewWidth(int width)
{
    hruler->setMaximum(width);
}

void KRulerTest::slotNewHeight(int height)
{
    vruler->setMaximum(height);
}

void KRulerTest::slotSetTinyMarks(bool set)
{
    hruler->setShowTinyMarks(set);
    vruler->setShowTinyMarks(set);
}

void KRulerTest::slotSetLittleMarks(bool set)
{
    hruler->setShowLittleMarks(set);
    vruler->setShowLittleMarks(set);
}

void KRulerTest::slotSetMediumMarks(bool set)
{
    hruler->setShowMediumMarks(set);
    vruler->setShowMediumMarks(set);
}

void KRulerTest::slotSetBigMarks(bool set)
{
    hruler->setShowBigMarks(set);
    vruler->setShowBigMarks(set);
}

void KRulerTest::slotSetEndMarks(bool set)
{
    hruler->setShowEndMarks(set);
    vruler->setShowEndMarks(set);
}

void KRulerTest::slotSetRulerPointer(bool set)
{
    hruler->setShowPointer(set);
    vruler->setShowPointer(set);
}

void KRulerTest::slotSetRulerLength(int len)
{
    hruler->setLength(len);
    vruler->setLength(len);
}

void KRulerTest::slotFixRulerLength(bool fix)
{
    hruler->setLengthFixed(fix);
    vruler->setLengthFixed(fix);
}

void KRulerTest::slotSetMStyle(int style)
{
    hruler->setRulerMetricStyle((KRuler::MetricStyle)style);
    vruler->setRulerMetricStyle((KRuler::MetricStyle)style);
    slotUpdateShowMarks();
}

void KRulerTest::slotUpdateShowMarks()
{
    showTM->setChecked(hruler->showTinyMarks());
    showLM->setChecked(hruler->showLittleMarks());
    showMM->setChecked(hruler->showMediumMarks());
    showBM->setChecked(hruler->showBigMarks());
    showEM->setChecked(hruler->showEndMarks());
}

void KRulerTest::slotCheckLength(bool fixlen)
{
    Q_UNUSED(fixlen);
    beginMark->setValue(hruler->offset());
    endMark->setValue(hruler->endOffset());
    lengthInput->setValue(hruler->length());
}

void KRulerTest::slotSetRotate(double d)
{
    Q_UNUSED(d);
#ifdef KRULER_ROTATE_TEST
    vruler->rotate = d;
    vruler->update();
    // debug("rotate %.1f", d);
#endif
}

void KRulerTest::slotSetXTrans(double d)
{
    Q_UNUSED(d);
#ifdef KRULER_ROTATE_TEST
    vruler->xtrans = d;
    vruler->update();
    // debug("trans x %.1f", d);
#endif
}

void KRulerTest::slotSetYTrans(double d)
{
    Q_UNUSED(d);
#ifdef KRULER_ROTATE_TEST
    vruler->ytrans = d;
    vruler->update();
    // debug("trans y %.1f", d);
#endif
}

/* --- MAIN -----------------------*/
int main(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication::setApplicationName(QStringLiteral("test"));
    QApplication *testapp;
    KRulerTest *window;

    testapp = new QApplication(argc, argv);
    testapp->setFont(QFont(QStringLiteral("Helvetica"), 12));

    window = new KRulerTest();
    window->setWindowTitle(QStringLiteral("KRulerTest"));
    window->resize(800, 600);
    window->show();
    return testapp->exec();
}
