/*
    SPDX-FileCopyrightText: 2009 Matthew Woehlke <mw_triad@users.sourceforge.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kcolorutilsdemo.h"
#include <KColorUtils>
#include <kcolorscheme.h>

#include <QApplication>

KColorUtilsDemo::KColorUtilsDemo(QWidget *parent)
    : QWidget(parent)
    , _leOutImg(128, 128, QImage::Format_RGB32)
    , _mtMixOutImg(128, 16, QImage::Format_RGB32)
    , _mtTintOutImg(128, 16, QImage::Format_RGB32)
{
    _noUpdate = true;
    setupUi(this);
    _noUpdate = false;

    inputSpinChanged();
    targetSpinChanged();
}

void KColorUtilsDemo::inputChanged()
{
    qreal hue;
    qreal chroma;
    qreal luma;
    KColorUtils::getHcy(inColor->color(), &hue, &chroma, &luma);
    ifHue->setValue(hue);
    ifChroma->setValue(chroma);
    ifLuma->setValue(luma);
    ifGray->setValue(qGray(inColor->color().rgb()));

    lumaChanged();
    mixChanged();
    shadeChanged();
}

void KColorUtilsDemo::lumaChanged()
{
    QColor base = inColor->color();

    for (int y = 0; y < 128; ++y) {
        qreal k = qreal(y - 64) / 64.0;

        for (int x = 0; x < 128; ++x) {
            qreal c;

            QColor r;
            if (leOpLighten->isChecked()) {
                c = qreal(128 - x) / 64.0;
                r = KColorUtils::lighten(base, k, c);
            } else if (leOpDarken->isChecked()) {
                c = qreal(x) / 64.0;
                r = KColorUtils::darken(base, -k, c);
            } else {
                c = qreal(x - 64) / 64.0;
                r = KColorUtils::shade(base, k, c);
            }
            _leOutImg.setPixel(x, y, r.rgb());
        }
    }

    leOut->setImage(_leOutImg);
}

void KColorUtilsDemo::mixChanged()
{
    QColor base = inColor->color();
    QColor target = mtTarget->color();

    for (int x = 0; x < 128; ++x) {
        qreal k = qreal(x) / 128.0;

        QRgb m = KColorUtils::mix(base, target, k).rgb();
        QRgb t = KColorUtils::tint(base, target, k).rgb();

        for (int y = 0; y < 16; ++y) {
            _mtMixOutImg.setPixel(x, y, m);
            _mtTintOutImg.setPixel(x, y, t);
        }
    }

    mtMixOut->setImage(_mtMixOutImg);
    mtTintOut->setImage(_mtTintOutImg);
}

void setBackground(QWidget *widget, const QColor &color)
{
    QPalette palette = widget->palette();
    palette.setColor(widget->backgroundRole(), color);
    widget->setPalette(palette);

    QString name = color.name();
    name += " (" + QString::number(color.red()) + ", " + QString::number(color.green()) + ", " + QString::number(color.blue()) + QLatin1Char(')');
    widget->setToolTip(name);
}

#define SET_SHADE(_n, _c, _cn, _ch) setBackground(ss##_n, KColorScheme::shade(_c, KColorScheme::_n##Shade, _cn, _ch));

void KColorUtilsDemo::shadeChanged()
{
    qreal cn = ssContrast->value();
    qreal ch = ssChroma->value();

    QColor base = inColor->color();
    setBackground(ssOut, base);
    setBackground(ssBase, base);
    SET_SHADE(Light, base, cn, ch);
    SET_SHADE(Midlight, base, cn, ch);
    SET_SHADE(Mid, base, cn, ch);
    SET_SHADE(Dark, base, cn, ch);
    SET_SHADE(Shadow, base, cn, ch);
}

void updateSwatch(KColorButton *s, const QSpinBox *r, const QSpinBox *g, const QSpinBox *b)
{
    s->setColor(QColor(r->value(), g->value(), b->value()));
}

void updateSpins(const QColor &c, QSpinBox *r, QSpinBox *g, QSpinBox *b)
{
    r->setValue(c.red());
    g->setValue(c.green());
    b->setValue(c.blue());
}

void KColorUtilsDemo::inputSpinChanged()
{
    if (_noUpdate) {
        return;
    }
    _noUpdate = true;

    updateSwatch(inColor, inRed, inGreen, inBlue);
    inputChanged();

    _noUpdate = false;
}

void KColorUtilsDemo::targetSpinChanged()
{
    if (_noUpdate) {
        return;
    }
    _noUpdate = true;

    updateSwatch(mtTarget, mtRed, mtGreen, mtBlue);
    mixChanged();

    _noUpdate = false;
}

void KColorUtilsDemo::inputSwatchChanged(const QColor &color)
{
    if (_noUpdate) {
        return;
    }
    _noUpdate = true;

    updateSpins(color, inRed, inGreen, inBlue);
    inputChanged();

    _noUpdate = false;
}

void KColorUtilsDemo::targetSwatchChanged(const QColor &color)
{
    if (_noUpdate) {
        return;
    }
    _noUpdate = true;

    updateSpins(color, mtRed, mtGreen, mtBlue);
    mixChanged();

    _noUpdate = false;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    KColorUtilsDemo *d = new KColorUtilsDemo;
    d->show();
    return app.exec();
}

#include "moc_kcolorutilsdemo.cpp"
