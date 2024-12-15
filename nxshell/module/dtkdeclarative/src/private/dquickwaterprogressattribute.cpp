// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dquickwaterprogressattribute_p.h"

#include <dobject_p.h>
#include <QQuickItem>
#include <QTimer>
#include <QGuiApplication>
#include <QtMath>

DQUICK_BEGIN_NAMESPACE

class WaterPopAttributePrivate : public QSharedData
{
public:
    qreal m_popSizeRatio = 0;
    qreal m_popXSpeed = 0;
    qreal m_popYSpeed = 0;
    qreal m_popX = 0;
    qreal m_popY = 0;
    qreal m_width = 0;
    qreal m_height = 0;
    qreal m_yOffset = 0;
};

WaterPopAttribute::WaterPopAttribute(qreal sizeRatio, qreal xSpeed, qreal ySpeed)
    : d(new WaterPopAttributePrivate)
{
    d->m_popSizeRatio = sizeRatio;
    d->m_popXSpeed = xSpeed;
    d->m_popYSpeed = ySpeed;
}

WaterPopAttribute::~WaterPopAttribute()
{

}

WaterPopAttribute::WaterPopAttribute(const WaterPopAttribute &other)
    : QObject(nullptr)
    , d(other.d)
{

}

WaterPopAttribute &WaterPopAttribute::operator=(const WaterPopAttribute &other)
{
    d = other.d;
    return *this;
}

qreal WaterPopAttribute::xSpeed() const
{
    return d->m_popXSpeed;
}

void WaterPopAttribute::setXSpeed(qreal popXSpeed)
{
    if (qFuzzyCompare(d->m_popXSpeed, popXSpeed))
        return;

    d->m_popXSpeed = popXSpeed;
    Q_EMIT xSpeedChanged();
}

qreal WaterPopAttribute::ySpeed() const
{
    return d->m_popYSpeed;
}

void WaterPopAttribute::setYSpeed(qreal popYSpeed)
{
    if (qFuzzyCompare(d->m_popYSpeed, popYSpeed))
        return;

    d->m_popYSpeed = popYSpeed;
    Q_EMIT ySpeedChanged();
}

qreal WaterPopAttribute::x() const
{
    return d->m_popX;
}

void WaterPopAttribute::setX(qreal x)
{
    if (qFuzzyCompare(d->m_popX, x))
        return;

    d->m_popX = x;
    Q_EMIT xChanged();
}

qreal WaterPopAttribute::y() const
{
    return d->m_popY;
}

void WaterPopAttribute::setY(qreal y)
{
    if (qFuzzyCompare(d->m_popY, y))
        return;

    d->m_popY = y;
    Q_EMIT yChanged();
}

qreal WaterPopAttribute::width() const
{
    return d->m_width;
}

void WaterPopAttribute::setWidth(qreal width)
{
    if (qFuzzyCompare(d->m_width, width))
        return;

    d->m_width = width;
    Q_EMIT widthChanged();
}

qreal WaterPopAttribute::height() const
{
    return d->m_height;
}

void WaterPopAttribute::setHeight(qreal height)
{
    if (qFuzzyCompare(d->m_height, height))
        return;

    d->m_height = height;
    Q_EMIT heightChanged();
}

qreal WaterPopAttribute::sizeRatio() const
{
    return d->m_popSizeRatio;
}

qreal WaterPopAttribute::yOffset() const
{
    return d->m_yOffset;
}

void WaterPopAttribute::setYOffset(qreal offset)
{
    if (qFuzzyCompare(d->m_yOffset, offset))
        return;

    d->m_yOffset = offset;
}

class DQuickWaterProgressAttributePrivate : public DCORE_NAMESPACE::DObjectPrivate
{
    D_DECLARE_PUBLIC(DQuickWaterProgressAttribute)
public:
    DQuickWaterProgressAttributePrivate(DQuickWaterProgressAttribute *qq)
    : DObjectPrivate(qq)
    , m_frontXOffset(100)
    , m_backXOffset(0)
    , m_timer(nullptr)
    , m_interval(33)
    , m_waterProgress(nullptr)
    {}

    ~DQuickWaterProgressAttributePrivate();
    void init();

private:
    qreal m_frontXOffset;
    qreal m_backXOffset;
    QList<WaterPopAttribute *> m_pops;
    QTimer *m_timer;
    int m_interval;
    QQuickItem * m_waterProgress;
    qreal m_imageWidth;
    qreal m_imageHeight;
};

DQuickWaterProgressAttributePrivate::~DQuickWaterProgressAttributePrivate()
{
    qDeleteAll(m_pops);
    m_pops.clear();
}

void DQuickWaterProgressAttributePrivate::init()
{
    D_Q(DQuickWaterProgressAttribute);
    m_timer = new QTimer(q);
    m_timer->setInterval(m_interval);

    m_pops.append(new WaterPopAttribute(0.07, -1.8, 0.6));
    m_pops.append(new WaterPopAttribute(0.08, 1.2, 1.0));
    m_pops.append(new WaterPopAttribute(0.11, 0.8, 1.6));

    QObject::connect(m_timer, &QTimer::timeout, q, [=]() {
        // move 60% per second
        double frontXDeta = 40.0 / (1000.0 / m_interval);
        // move 90% per second
        double backXDeta = 60.0 / (1000.0 / m_interval);

        qreal canvasWidth = m_waterProgress->width() * qApp->devicePixelRatio();
        q->setFrontXOffset(q->frontXOffset() - frontXDeta * canvasWidth / 100);
        q->setBackXOffset(q->backXOffset() + backXDeta * canvasWidth / 100);

        if (q->frontXOffset() > canvasWidth || q->frontXOffset() < (canvasWidth - q->imageWidth()))
            q->setFrontXOffset(canvasWidth);
        if (q->backXOffset() > q->imageWidth())
            q->setBackXOffset(0);

        // update pop
        // move 25% per second default
        qreal speed = 25 / (1000.0 / m_interval);
        int value = m_waterProgress->property(QByteArrayLiteral("value")).toInt();
        for (auto &pop : m_pops) {
            // yOffset 0 ~ 100;
            pop->setYOffset(pop->yOffset() + speed * pop->ySpeed());
            pop->setYOffset(pop->yOffset() > value ? 0 : pop->yOffset());
            qreal xOffset = qSin((pop->yOffset() * 0.01) * 2 * M_PI) * 18 * pop->xSpeed() + 50;
            pop->setY((1 - 0.01 * pop->yOffset()) * m_waterProgress->height());
            pop->setX(xOffset * 0.01 * m_waterProgress->width());
            pop->setWidth(pop->sizeRatio() * m_waterProgress->width());
            pop->setHeight(pop->sizeRatio() * m_waterProgress->height());
        }
    });

}

DQuickWaterProgressAttribute::DQuickWaterProgressAttribute(QObject *parent)
    : QObject(parent)
    , DObject(*new DQuickWaterProgressAttributePrivate(this))
{
    D_D(DQuickWaterProgressAttribute);
    d->init();
}

qreal DQuickWaterProgressAttribute::frontXOffset() const
{
    D_DC(DQuickWaterProgressAttribute);
    return d->m_frontXOffset;
}

void DQuickWaterProgressAttribute::setFrontXOffset(qreal frontXOffset)
{
    D_D(DQuickWaterProgressAttribute);
    if (qFuzzyCompare(d->m_frontXOffset, frontXOffset))
        return;

    d->m_frontXOffset = frontXOffset;
    Q_EMIT frontXOffsetChanged();
}

qreal DQuickWaterProgressAttribute::backXOffset() const
{
    D_DC(DQuickWaterProgressAttribute);
    return d->m_backXOffset;
}

void DQuickWaterProgressAttribute::setBackXOffset(qreal backXOffset)
{
    D_D(DQuickWaterProgressAttribute);
    if (qFuzzyCompare(d->m_backXOffset, backXOffset))
        return;

    d->m_backXOffset = backXOffset;
    Q_EMIT backXOffsetChanged();
}

QQmlListProperty<WaterPopAttribute> DQuickWaterProgressAttribute::pops()
{
    D_D(DQuickWaterProgressAttribute);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    return QQmlListProperty<WaterPopAttribute>(this, &d->m_pops);
#else
    return QQmlListProperty<WaterPopAttribute>(this, d->m_pops);
#endif
}

bool DQuickWaterProgressAttribute::running() const
{
    D_DC(DQuickWaterProgressAttribute);
    return d->m_timer->isActive();
}

void DQuickWaterProgressAttribute::setRunning(bool running)
{
    D_D(DQuickWaterProgressAttribute);
    if (d->m_timer->isActive() == running)
        return;

    if (running)
        d->m_timer->start();
    else
        d->m_timer->stop();
    Q_EMIT runningChanged();
}

QQuickItem *DQuickWaterProgressAttribute::waterProgress() const
{
    D_DC(DQuickWaterProgressAttribute);
    return d->m_waterProgress;
}

void DQuickWaterProgressAttribute::setWaterProgress(QQuickItem *waterProgress)
{
    D_D(DQuickWaterProgressAttribute);
    if (d->m_waterProgress == waterProgress)
        return;

    d->m_waterProgress = waterProgress;
    Q_EMIT waterProgressChanged();
}

qreal DQuickWaterProgressAttribute::imageWidth() const
{
    D_DC(DQuickWaterProgressAttribute);
    return d->m_imageWidth;
}

void DQuickWaterProgressAttribute::setImageWidth(qreal imageWidth)
{
    D_D(DQuickWaterProgressAttribute);
    if (qFuzzyCompare(d->m_imageWidth, imageWidth))
        return;

    d->m_imageWidth = imageWidth;
    Q_EMIT imageWidthChanged();
}

qreal DQuickWaterProgressAttribute::imageHeight() const
{
    D_DC(DQuickWaterProgressAttribute);
    return d->m_imageHeight;
}

void DQuickWaterProgressAttribute::setImageHeight(qreal imageHeight)
{
    D_D(DQuickWaterProgressAttribute);
    if (qFuzzyCompare(d->m_imageHeight, imageHeight))
        return;

    d->m_imageHeight = imageHeight;
    Q_EMIT imageHeightChanged();
}

DQUICK_END_NAMESPACE
