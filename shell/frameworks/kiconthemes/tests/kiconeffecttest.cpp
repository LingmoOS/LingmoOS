#include "kiconeffecttest.h"

#include <kiconeffect.h>
#include <kiconloader.h>

#include <KColorButton>

#include <QApplication>
#include <QGridLayout>
#include <QLabel>
#include <QSlider>

KIconEffectTestWidget::KIconEffectTestWidget(QWidget *parent)
    : QScrollArea(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    QWidget *frame = new QWidget(this);
    setWidget(frame);
    setWidgetResizable(true);
    QGridLayout *layout = new QGridLayout(frame);
    layout->setColumnStretch(1, 1);

    img = QImage(KIconLoader::global()->iconPath(QStringLiteral("application-x-cd-image"), -128));
    QImage tmp;
    QSlider *slider;

    tmp = img;
    KIconEffect::toGray(tmp, 0.0);
    lbl[0] = new QLabel(frame);
    lbl[0]->setPixmap(QPixmap::fromImage(tmp));
    layout->addWidget(lbl[0], 0, 0, 3, 1);
    layout->addWidget(new QLabel(QStringLiteral("Grayscale"), frame), 0, 1);
    slider = new QSlider(Qt::Horizontal, frame);
    slider->setRange(0, 100);
    connect(slider, &QAbstractSlider::valueChanged, this, &KIconEffectTestWidget::slotGray);
    layout->addWidget(slider, 1, 1);

    tmp = img;
    KIconEffect::toMonochrome(tmp, Qt::black, Qt::white, 0.0);
    lbl[1] = new QLabel(frame);
    lbl[1]->setPixmap(QPixmap::fromImage(tmp));
    layout->addWidget(lbl[1], 4, 0, 3, 1);
    layout->addWidget(new QLabel(QStringLiteral("Monochrome"), frame), 4, 1);
    slider = new QSlider(Qt::Horizontal, frame);
    slider->setRange(0, 100);
    connect(slider, &QAbstractSlider::valueChanged, this, &KIconEffectTestWidget::slotMonochrome);
    layout->addWidget(slider, 5, 1);

    tmp = img;
    KIconEffect::deSaturate(tmp, 0.0);
    lbl[2] = new QLabel(frame);
    lbl[2]->setPixmap(QPixmap::fromImage(tmp));
    layout->addWidget(lbl[2], 8, 0, 3, 1);
    layout->addWidget(new QLabel(QStringLiteral("Desaturate"), frame), 8, 1);
    slider = new QSlider(Qt::Horizontal, frame);
    slider->setRange(0, 100);
    connect(slider, &QAbstractSlider::valueChanged, this, &KIconEffectTestWidget::slotDesaturate);
    layout->addWidget(slider, 9, 1);

    tmp = img;
    KIconEffect::toGamma(tmp, 0.0);
    lbl[3] = new QLabel(frame);
    lbl[3]->setPixmap(QPixmap::fromImage(tmp));
    layout->addWidget(lbl[3], 12, 0, 3, 1);
    layout->addWidget(new QLabel(QStringLiteral("Gamma"), frame), 12, 1);
    slider = new QSlider(Qt::Horizontal, frame);
    slider->setRange(0, 100);
    connect(slider, &QAbstractSlider::valueChanged, this, &KIconEffectTestWidget::slotGamma);
    layout->addWidget(slider, 13, 1);

    tmp = img;
    colorizedColor = Qt::blue;
    colorizedValue = 0.0;
    KIconEffect::colorize(tmp, colorizedColor, colorizedValue);
    lbl[4] = new QLabel(frame);
    lbl[4]->setPixmap(QPixmap::fromImage(tmp));
    layout->addWidget(lbl[4], 16, 0, 4, 1);
    layout->addWidget(new QLabel(QStringLiteral("Colorize"), frame), 16, 1);
    slider = new QSlider(Qt::Horizontal, frame);
    slider->setRange(0, 100);
    connect(slider, &QAbstractSlider::valueChanged, this, &KIconEffectTestWidget::slotColorizeValue);
    layout->addWidget(slider, 17, 1);
    KColorButton *btn = new KColorButton(colorizedColor, frame);
    connect(btn, &KColorButton::changed, this, &KIconEffectTestWidget::slotColorizeColor);
    layout->addWidget(btn, 18, 1);

    tmp = img;
    KIconEffect::semiTransparent(tmp);
    lbl[5] = new QLabel(frame);
    lbl[5]->setPixmap(QPixmap::fromImage(tmp));
    layout->addWidget(lbl[5], 20, 0, 3, 1);
    layout->addWidget(new QLabel(QStringLiteral("Semitransparent"), frame), 20, 1);

    layout->setRowStretch(21, 1);
    frame->resize(frame->sizeHint());
}

void KIconEffectTestWidget::slotGray(int value)
{
    QImage tmp(img);
    KIconEffect::toGray(tmp, value * 0.01);
    lbl[0]->setPixmap(QPixmap::fromImage(tmp));
}

void KIconEffectTestWidget::slotMonochrome(int value)
{
    QImage tmp(img);
    KIconEffect::toMonochrome(tmp, Qt::black, Qt::white, value * 0.01);
    lbl[1]->setPixmap(QPixmap::fromImage(tmp));
}

void KIconEffectTestWidget::slotDesaturate(int value)
{
    QImage tmp(img);
    KIconEffect::deSaturate(tmp, value * 0.01);
    lbl[2]->setPixmap(QPixmap::fromImage(tmp));
}

void KIconEffectTestWidget::slotGamma(int value)
{
    QImage tmp(img);
    KIconEffect::toGamma(tmp, value * 0.01);
    lbl[3]->setPixmap(QPixmap::fromImage(tmp));
}

void KIconEffectTestWidget::slotColorizeColor(const QColor &c)
{
    colorizedColor = c;
    QImage tmp(img);
    KIconEffect::colorize(tmp, colorizedColor, colorizedValue);
    lbl[4]->setPixmap(QPixmap::fromImage(tmp));
}

void KIconEffectTestWidget::slotColorizeValue(int value)
{
    colorizedValue = value * 0.01f;
    QImage tmp(img);
    KIconEffect::colorize(tmp, colorizedColor, colorizedValue);
    lbl[4]->setPixmap(QPixmap::fromImage(tmp));
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KIconEffectTestWidget *w = new KIconEffectTestWidget;
    w->show();
    return app.exec();
}

#include "moc_kiconeffecttest.cpp"
