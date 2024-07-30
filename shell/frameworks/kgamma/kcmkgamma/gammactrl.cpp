/*
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *
 *  SPDX-FileCopyrightText: 2001 Michael v.Ostheim <MvOstheim@web.de>
 */

#include "gammactrl.h"

#include <QHBoxLayout>
#include <qlabel.h>
#include <qlineedit.h>
#include <qstring.h>

#include "displaynumber.h"
#include "xvidextwrap.h"

GammaCtrl::GammaCtrl(QWidget *parent, XVidExtWrap *xvid, int channel, const QString &mingamma, const QString &maxgamma, const QString &setgamma)
    : QWidget(parent)
{
    int maxslider = (int)((maxgamma.toDouble() - mingamma.toDouble() + 0.0005) * 20);
    int setslider = (int)((setgamma.toDouble() - mingamma.toDouble() + 0.0005) * 20);
    setslider = (setslider > maxslider) ? maxslider : setslider;
    setslider = (setslider < 0) ? 0 : setslider;

    suspended = false;
    changed = false;
    ming = mingamma.toFloat();
    mgamma = mingamma;
    oldpos = setslider;
    gchannel = channel;
    xv = xvid;

    QHBoxLayout *layout = new QHBoxLayout(this);

    slider = new QSlider(Qt::Horizontal, this);
    slider->setTickPosition(QSlider::TicksBelow);
    slider->setRange(0, maxslider);
    slider->setTickInterval(2);
    slider->setValue(setslider);
    layout->addWidget(slider);
    connect(slider, &QAbstractSlider::valueChanged, this, QOverload<int>::of(&GammaCtrl::setGamma));
    connect(slider, &QAbstractSlider::sliderPressed, this, &GammaCtrl::pressed);

    textfield = new DisplayNumber(this, 4, 2);
    textfield->setText(setgamma);
    layout->addWidget(textfield);
}

GammaCtrl::~GammaCtrl()
{
}

/** set gamma, slider and textfield */
void GammaCtrl::setGamma(const QString &gamma)
{
    int sliderpos;

    sliderpos = (int)((gamma.toDouble() - mgamma.toDouble() + 0.0005) * 20);
    changed = true;
    slider->setValue(sliderpos);

    setGamma(sliderpos);
    if (suspended) {
        suspended = false;
        textfield->setDisabled(false);
    }
}

/** set slider and textfield */
void GammaCtrl::setControl(const QString &gamma)
{
    int sliderpos;

    sliderpos = (int)((gamma.toDouble() - mgamma.toDouble() + 0.0005) * 20);
    setCtrl(sliderpos);
}

/** Return the current gamma value with precision prec */
QString GammaCtrl::gamma(int prec)
{
    QString gammatext;
    gammatext.setNum(xv->getGamma(gchannel) + 0.0005, 'f', prec);

    return (gammatext);
}

/** Slot: set gamma and textfield */
void GammaCtrl::setGamma(int sliderpos)
{
    if (sliderpos != oldpos || changed) {
        xv->setGamma(gchannel, ming + (float)(slider->value()) * 0.05);
        textfield->setNum(xv->getGamma(gchannel));
        oldpos = sliderpos;
        changed = false;
        Q_EMIT gammaChanged(sliderpos);
    }
}

/** Slot: set slider and textfield */
void GammaCtrl::setCtrl(int sliderpos)
{
    if (suspended) {
        suspended = false;
        textfield->setDisabled(false);
    }
    oldpos = sliderpos;
    slider->setValue(sliderpos);
    textfield->setNum(xv->getGamma(gchannel));
}

/** Slot: disable textfield */
void GammaCtrl::suspend()
{
    if (!suspended) {
        suspended = true;
        textfield->setDisabled(true);
    }
}

/** Slot: Change status of GammaCtrl when pressed */
void GammaCtrl::pressed()
{
    if (suspended) {
        suspended = false;
        textfield->setDisabled(false);
        changed = true;
        setGamma(slider->value());
    }
}

#include "moc_gammactrl.cpp"
