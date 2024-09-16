// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imageanimation.h"
#include "unionimage/unionimage.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QPainter>
#include <QPointer>
#include <QSharedPointer>
#include <QTime>
#include <QTimer>
#include <QScreen>
#include <QObject>
#include <QDesktopWidget>
#include <QPalette>
#include <QMutex>
#include <QApplication>

#include <cmath>

float GaussFunction(double max, float mu, float sigma, float x)
{
    return static_cast<float>(max * std::exp(static_cast<double>(-(x - mu) * (x - mu) / 2 * sigma * sigma)));
}

class LoopQueue
{
public:
    LoopQueue(const QString &beginPath, const QStringList &list);
    inline const QString first()const;
    inline const QString second()const;
    inline const QString last()const;
    inline int index() const;
    inline const QString next();
//    inline const QString pre();
    inline const QString jumpTonext();
    inline const QString jumpTopre();
    inline const QString current()const;
    inline void changeOrder(bool order);
private:
    inline void AddIndex();
private:
    QVector<QString> loop_paths;
    QMutex queueMutex;
    bool loop_order;
    char m_padding[3] = {'0', '0', '0'}; //填充占位,使数据结构内存对齐
    int loop_pindex;
};

class LibImageAnimationPrivate : public QWidget
{
    Q_OBJECT

public:
    enum AnimationType {
        FadeEffect = 0,             //图像1渐渐变淡,图像2渐渐显现
        BlindsEffect = 1,           //百叶窗效果
        FlipRightToLeft = 2,        //图像从右向左翻转
        OutsideToInside = 3,        //从外到内水平分割
        MoveLeftToRightEffect = 4,  //图像1从左至右退出可视区域,同时图像2从左至右进入可视区域
        MoveRightToLeftEffect = 5,  //图像1从左至右退出可视区域,同时图像2从左至右进入可视区域
        MoveBottomToUpEffect = 6,   //图像1从下至上退出可视区域,同时图像2从下至上进入可视区域
        MoveUpToBottomEffect = 7,   //图像1从上至下退出可视区域,同时图像2从上至下进入可视区域
        MoveBottomToLeftUpEffect = 8//图像1不动,同时图像2从右下到左上
    };

protected:
    explicit LibImageAnimationPrivate(LibImageAnimation *qq = nullptr);
    ~LibImageAnimationPrivate();
    void effectPainter(QPainter *painter, const QRect &rect);
    void forwardPainter(QPainter *painter, const QRect &rect);
    void retreatPainter(QPainter *painter, const QRect &rect);
    void keepStaticPainter(QPainter *painter, const QRect &rect);
    /**
    ****************************************************************************************************************
    Effects
    ****************************************************************************************************************
    */
    void fadeEffect(QPainter *painter, const QRect &rect, float factor, const QPixmap &pixmap1, const QPixmap &pixmap2);
    void blindsEffect(QPainter *painter, const QRect &rect, float factor, const QPixmap &pixmap1, const QPixmap &pixmap2);
    void flipRightToLeft(QPainter *painter, const QRect &rect, float factor, const QPixmap &pixmap1, const QPixmap &pixmap2);
    void outsideToInside(QPainter *painter, const QRect &rect, float factor, const QPixmap &pixmap1, const QPixmap &pixmap2);
    void moveLeftToRightEffect(QPainter *painter, const QRect &rect, float factor, const QPixmap &pixmap1, const QPixmap &pixmap2);

    //新增获取当前图片路径
    const QString getCurrentPath();
    /**
    ****************************************************************************************************************
    */
private:
    float m_factor;                   //动画因子(0 - 1.0之间变化)
    float m_funval;
    QString m_imageName1;             //图片1路径名称
    QString m_imageName2;             //图片2路径名称
    QPixmap m_pixmap1;                //图片1
    QPixmap m_pixmap2;                //图片2
    AnimationType m_animationType;    //动画效果类型
    bool m_isAnimationIng = false;    //正在播动画

public:

    void setSlideModel(LibImageAnimation::SlideModel model)
    {
        m_SliderModel = model;
    }

    void setPlayOrStatue(LibImageAnimation::PlayOrStatue statue)
    {
        m_PlayOrStatue = statue;
    }

    void setPathList(const QString &first, const QStringList &list);
    void startAnimation();
    void startSingleNextAnimation();
    void startSinglePreAnimation();
    void startStatic();
    void endSlide()
    {
        if (m_staticTimer) {
            m_staticTimer->stop();
        }
        if (m_continuousanimationTimer) {
            m_continuousanimationTimer->stop();
        }
    }

public slots:
    void onContinuousAnimationTimer();
    void onStaticTimer();

private:
    //设置动画因子
//    inline void setFactor(float factor_bar)
//    {
//        m_factor = factor_bar;
//    }

    //设置图片1+图片2路径名称并适应widget
    void setImage1(const QString &imageName1_bar);
    void setImage2(const QString &imageName2_bar);

    //设置图片1+图片2
//    void setPixmap1(const QPixmap &pixmap1_bar)
//    {
//        m_pixmap1 = pixmap1_bar;
//    }
//    void setPixmap2(const QPixmap &pixmap2_bar)
//    {
//        m_pixmap2 = pixmap2_bar;
//    }
    //设置动画类型
//    void setAnimationType(const AnimationType &animationType_bar)
//    {
//        m_animationType = animationType_bar;
//    }

private:
    char m_padding2[3] = {'0', '0', '0'};              //填充占位,使数据结构内存对齐
    QSharedPointer<LoopQueue> queue;
    QPointer<QTimer> m_singleanimationTimer;
    QPointer<QTimer> m_continuousanimationTimer;
    QPointer<QTimer> m_staticTimer;
    QPointer<QRect> m_rect;
    QPoint centrePoint;
//    int beginX;
//    int beginY;
//    int finalX;
//    int finalY;

    LibImageAnimation *const q_ptr;
    LibImageAnimation::SlideModel m_SliderModel = LibImageAnimation::AutoPlayModel;
    LibImageAnimation::PlayOrStatue m_PlayOrStatue = LibImageAnimation::PlayStatue;
    Q_DECLARE_PUBLIC(LibImageAnimation)
};

/**
 ****************************************************************************************************************
 *  LoopQueue
 ****************************************************************************************************************
 */

LoopQueue::LoopQueue(const QString &beginPath, const QStringList &list): loop_order(true), loop_pindex(0)
{
    Q_UNUSED(m_padding);
    loop_paths.clear();
    int newfirst = list.indexOf(beginPath);
    QVector<QString> temp;
    QList<QString>::const_iterator i = list.begin();
    for (int j = 0; i != list.end() && j < newfirst; j++) {
        temp.append(*i);
        i.operator++();
    }
    for (; i != list.end();) {
        loop_paths.append(*i);
        i.operator++();
    }
    loop_paths.append(temp);
}

const QString LoopQueue::first() const
{
    return loop_paths.first();
}

const QString LoopQueue::second() const
{
    return loop_paths[1];
}

const QString LoopQueue::last() const
{
    return loop_paths.last();
}

int LoopQueue::index() const
{
    return loop_pindex;
}

const QString LoopQueue::next()
{
    if (loop_pindex + 1 >= loop_paths.size()) {
        return loop_paths.first();
    } else {
        return loop_paths[loop_pindex + 1];
    }
}

//const QString LoopQueue::pre()
//{
//    if (loop_pindex - 1 < 0) {
//        return loop_paths.last();
//    } else {
//        return loop_paths[loop_pindex - 1];
//    }
//}

const QString LoopQueue::jumpTonext()
{
    changeOrder(true);
    AddIndex();
    if (loop_paths.size() > loop_pindex)
        return loop_paths[loop_pindex];
    else
        return QString();
}

const QString LoopQueue::jumpTopre()
{
    changeOrder(false);
    AddIndex();
    return loop_paths[loop_pindex];
}

const QString LoopQueue::current() const
{
    return loop_paths[loop_pindex];
}

void LoopQueue::changeOrder(bool order)
{
    loop_order = order;
}

void LoopQueue::AddIndex()
{
    QMutexLocker locker(&queueMutex);
    if (loop_order) {
        loop_pindex++;
        if (loop_pindex >= loop_paths.size()) {
            loop_pindex = 0;
        }
    } else {
        loop_pindex--;
        if (loop_pindex < 0) {
            loop_pindex = loop_paths.size() - 1;
        }
    }
}

/**
 ****************************************************************************************************************
 *  ImageAnimationPrivate
 ****************************************************************************************************************
 */

LibImageAnimationPrivate::LibImageAnimationPrivate(LibImageAnimation *qq) :
    m_factor(0.0f), m_funval(0.0f), m_animationType(AnimationType::BlindsEffect), queue(nullptr),
    m_singleanimationTimer(nullptr), m_continuousanimationTimer(nullptr), m_staticTimer(nullptr),  q_ptr(qq)
{
    Q_UNUSED(m_padding2);
}

LibImageAnimationPrivate::~LibImageAnimationPrivate()
{

}

void LibImageAnimationPrivate::effectPainter(QPainter *painter, const QRect &rect)
{
    if (m_pixmap1.isNull() || m_pixmap2.isNull()) {
        return;
    } else if (!m_isAnimationIng) {
        painter->drawPixmap(0, 0, m_pixmap2);
        return;
    }
    centrePoint = rect.center();
    switch (m_animationType) {
    case 0:
        fadeEffect(painter, rect, m_factor, m_pixmap1, m_pixmap2);
        break;
    case 1:
        blindsEffect(painter, rect, m_factor, m_pixmap1, m_pixmap2);
        break;
    case 2:
        flipRightToLeft(painter, rect, m_factor, m_pixmap1, m_pixmap2);
        break;
    case 3:
        outsideToInside(painter, rect, m_factor, m_pixmap1, m_pixmap2);
        break;
    case 4:
        moveLeftToRightEffect(painter, rect, m_factor, m_pixmap1, m_pixmap2);
        break;
    default:
        break;
    }
    painter->end();
}

void LibImageAnimationPrivate::forwardPainter(QPainter *painter, const QRect &rect)
{
    Q_UNUSED(rect);
    if (m_pixmap1.isNull() || m_pixmap2.isNull()) {
        return;
    }
    Q_Q(LibImageAnimation);
    if (!m_continuousanimationTimer && !m_staticTimer) {
        setImage1(m_imageName2);
        setImage2(queue->jumpTonext());
        painter->drawPixmap(0, 0, m_pixmap1);
        q->setPaintTarget(LibImageAnimation::KeepStatic);
        return;
    }
    if (m_continuousanimationTimer) {
        m_continuousanimationTimer->stop();
        m_continuousanimationTimer->setInterval(0);
        m_factor = 0.0f;
        painter->drawPixmap(0, 0, m_pixmap2);
        q->setPaintTarget(LibImageAnimation::KeepStatic);
        m_continuousanimationTimer->deleteLater();
    }
    if (m_staticTimer) {
        if (m_continuousanimationTimer)
            if (m_continuousanimationTimer->timerId() >= 0)
                killTimer(m_continuousanimationTimer->timerId());
    }
    q->update();
}

void LibImageAnimationPrivate::retreatPainter(QPainter *painter, const QRect &rect)
{
    Q_UNUSED(rect);
    if (m_pixmap1.isNull() || m_pixmap2.isNull()) {
        return;
    }
    Q_Q(LibImageAnimation);
    if (!m_continuousanimationTimer && !m_staticTimer) {
        setImage1(m_imageName2);
        setImage2(queue->jumpTopre());
        painter->drawPixmap(0, 0, m_pixmap1);
        q->setPaintTarget(LibImageAnimation::KeepStatic);
        return;
    }

    //动画播放中
    if (m_continuousanimationTimer) {
        m_continuousanimationTimer->stop();
        m_continuousanimationTimer->setInterval(0);
        m_factor = 0.0f;
        setImage2(queue->jumpTopre());
        painter->drawPixmap(0, 0, m_pixmap2);
        q->setPaintTarget(LibImageAnimation::KeepStatic);
        m_continuousanimationTimer->deleteLater();
    }
    if (m_staticTimer) {
        if (m_continuousanimationTimer->timerId() >= 0)
            killTimer(m_continuousanimationTimer->timerId());
        if (m_staticTimer->timerId() >= 0)
            killTimer(m_staticTimer->timerId());
    }
}

void LibImageAnimationPrivate::keepStaticPainter(QPainter *painter, const QRect &rect)
{
    Q_UNUSED(rect);
    painter->drawPixmap(0, 0, m_pixmap2);
}

void LibImageAnimationPrivate::fadeEffect(QPainter *painter, const QRect &rect, float factor, const QPixmap &pixmap1, const QPixmap &pixmap2)
{
    factor = factor + FACTOR_STEP > 1.0f ? 1.0f : factor;
    int alpha = static_cast<int>(255 * (1.0f - factor));
    QPixmap alphaPixmap(rect.size());
    alphaPixmap.fill(Qt::transparent);


    QPainter p1(&alphaPixmap);
    p1.setCompositionMode(QPainter::CompositionMode_Source);
    p1.drawPixmap(0, 0, pixmap1);
    p1.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p1.fillRect(alphaPixmap.rect(), QColor(0, 0, 0, alpha));
    p1.end();

    painter->drawPixmap(0, 0, alphaPixmap);
    alpha = 255 - alpha;
    alphaPixmap.fill(Qt::transparent);
    QPainter p2(&alphaPixmap);
    p2.setCompositionMode(QPainter::CompositionMode_Source);
    p2.drawPixmap(0, 0, pixmap2);
    p2.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p2.fillRect(alphaPixmap.rect(), QColor(0, 0, 0, alpha));
    p2.end();
    painter->drawPixmap(0, 0, alphaPixmap);
}

void LibImageAnimationPrivate::blindsEffect(QPainter *painter, const QRect &rect, float factor, const QPixmap &pixmap1, const QPixmap &pixmap2)
{
    Q_UNUSED(rect);
    factor = factor + FACTOR_STEP > 1.0f ? 1.0f : factor;
    int i, n, dh, ddh;
    painter->drawPixmap(0, 0, pixmap1);
    n = 10;
    dh = pixmap2.height() / n;
    ddh = static_cast<int>(factor * dh);
    if (ddh < 1) {
        ddh = 1;
    }
    for (i = 0; i < n; i++) {
        painter->drawPixmap(0, 0 + i * dh, pixmap2, 0, i * dh, pixmap2.width(), ddh);
    }
}

void LibImageAnimationPrivate::flipRightToLeft(QPainter *painter, const QRect &rect, float factor, const QPixmap &pixmap1, const QPixmap &pixmap2)
{
    int w, h;
    float rot;
    QTransform trans;

    w = rect.width();
    h = rect.height();

    rot = factor * 90.0f;
    trans.translate(static_cast<qreal>(w * (1 - factor)), h / 2);
    trans.rotate(static_cast<qreal>(rot), Qt::YAxis);
    trans.translate(-w, -h / 2);

    painter->setTransform(trans);
    painter->drawPixmap(0, 0, pixmap1);
    painter->resetTransform();

    trans.reset();
    rot = 90 * (factor - 1);
    trans.translate(static_cast<qreal>(w * (1 - factor)), h / 2);
    trans.rotate(static_cast<qreal>(rot), Qt::YAxis);
    trans.translate(0, -h / 2);

    painter->setTransform(trans);
    painter->drawPixmap(0, 0, pixmap2);
    painter->resetTransform();
}

void LibImageAnimationPrivate::outsideToInside(QPainter *painter, const QRect &rect, float factor, const QPixmap &pixmap1, const QPixmap &pixmap2)
{
    int   w, h, x3, y3, dh, ddh;
    w = rect.width();
    h = rect.height();
    painter->drawPixmap(0, 0, pixmap1);
    dh = pixmap2.height() / 2;
    ddh = static_cast<int>(factor * dh);
    if (ddh < 1) {
        ddh = 1;
    }
    painter->drawPixmap(0, 0, pixmap2, 0, 0, pixmap2.width(), ddh);
    x3 = (w - pixmap2.width()) / 2;
    y3 = static_cast<int>(dh * (1.0f - factor) + h / 2);
    if (y3 != h / 2)
        y3 += 1;
    painter->drawPixmap(x3, y3, pixmap2, 0, pixmap2.height() - ddh, pixmap2.width(), ddh);
}

void LibImageAnimationPrivate::moveLeftToRightEffect(QPainter *painter, const QRect &rect, float factor, const QPixmap &pixmap1, const QPixmap &pixmap2)
{
    int x, y, w;
    w = rect.width();
//    h = rect.height();
    x = static_cast<int>(0 + w * factor);
    y = 0;
    painter->drawPixmap(x, y, pixmap1);
    x = static_cast<int>(0 + w * (factor - 1));
    y = 0;
    painter->drawPixmap(x, y, pixmap2);
}

const QString LibImageAnimationPrivate::getCurrentPath()
{
    return queue->current();
}

void LibImageAnimationPrivate::setPathList(const QString &first, const QStringList &list)
{
    queue = QSharedPointer<LoopQueue>(new LoopQueue(first, list));
    setImage1(queue->last());
    setImage2(queue->first());
}

void LibImageAnimationPrivate::startAnimation()
{
    qsrand(static_cast<uint>(QTime(0, 0, 0).secsTo(QTime::currentTime())));
    m_animationType = static_cast<AnimationType>(qrand() % (3));
    if (!m_continuousanimationTimer) {
        m_continuousanimationTimer = new QTimer(this);
        m_factor = 0.0f;
        m_funval = 0.0f;
        connect(m_continuousanimationTimer, &QTimer::timeout, this, &LibImageAnimationPrivate::onContinuousAnimationTimer);
    }
    m_factor = 0.0f;
    m_funval = 0.0f;
    m_isAnimationIng = true;
    m_continuousanimationTimer->start(UPDATE_RATE);
}

void LibImageAnimationPrivate::startSingleNextAnimation()
{
    if (m_isAnimationIng) {
        m_isAnimationIng = false;
    } else {
        setImage1(m_imageName2);
        setImage2(queue->jumpTonext());
        startAnimation();
    }
}

void LibImageAnimationPrivate::startSinglePreAnimation()
{
    if (m_isAnimationIng) {
        m_isAnimationIng = false;
    } else {
        setImage1(m_imageName2);
        setImage2(queue->jumpTopre());
        startAnimation();
    }
}

void LibImageAnimationPrivate::startStatic()
{
    if (!m_staticTimer) {
        m_staticTimer = new QTimer(this);
        m_staticTimer->setSingleShot(true);
        connect(m_staticTimer, &QTimer::timeout, this, &LibImageAnimationPrivate::onStaticTimer);
    }
    m_isAnimationIng = false;
    m_staticTimer->start(SLIDER_TIME);
}

void LibImageAnimationPrivate::onContinuousAnimationTimer()
{
    Q_Q(LibImageAnimation);
    m_funval += FACTOR_STEP;
    m_factor += GaussFunction(0.25, 0.5f, 5, m_funval);
    if (m_factor + 0.005f > 1)
        m_factor = 1.0f;
    if (m_funval > 1.0f) {
        m_isAnimationIng = false;
        if (m_PlayOrStatue == LibImageAnimation::PlayStatue && m_SliderModel == LibImageAnimation::AutoPlayModel) {
            m_continuousanimationTimer->stop();
            m_funval = 0.0f;
            m_factor = 0.0f;
            startStatic();
        }
    } else {
        m_continuousanimationTimer->start(UPDATE_RATE);
        q->update();
    }
}

void LibImageAnimationPrivate::onStaticTimer()
{
    qDebug() << "ImageAnimationPrivate::onStaticTimer m_PlayOrStatue = " << LibImageAnimation::PlayStatue;
    qDebug() << "ImageAnimationPrivate::onStaticTimer m_SliderModel = " << LibImageAnimation::AutoPlayModel;

    if (m_PlayOrStatue == LibImageAnimation::PlayStatue && m_SliderModel == LibImageAnimation::AutoPlayModel) {
        qsrand(static_cast<uint>(QTime(0, 0, 0).secsTo(QTime::currentTime())));
        m_animationType = static_cast<AnimationType>(qrand() % (3));
        setImage1(m_imageName2);
        setImage2(queue->jumpTonext());
        startAnimation();
    }
}

void LibImageAnimationPrivate::setImage1(const QString &imageName1_bar)
{
    m_imageName1 = imageName1_bar;
    QImage tImg;
    QString errMsg;
    LibUnionImage_NameSpace::loadStaticImageFromFile(imageName1_bar, tImg, errMsg);
    QPixmap p1 = QPixmap::fromImage(tImg);
    int beginX = 0, beginY = 0;
    int number = QApplication::desktop()->screenNumber(q_ptr);
    if (number < 0) {
        number = 0;
    }
    // 多屏显示问题，定位当前屏幕
    if (p1.width() >= p1.height()) {
        m_pixmap1 = QPixmap(QGuiApplication::screens().at(number)->geometry().size());
        QPainter pa1(&m_pixmap1);
        m_pixmap1.fill(QColor("#252525"));
        p1 = p1.scaledToWidth(QGuiApplication::screens().at(number)->geometry().size().width());
        if (p1.height() > QGuiApplication::screens().at(number)->geometry().size().height()) {
            p1 = p1.scaledToHeight(QGuiApplication::screens().at(number)->geometry().size().height());
        }
        // 多屏显示去除x偏移
        centrePoint = q_ptr->getCurScreenGeometry().center();
        beginX = centrePoint.x() - p1.width() / 2;
        beginX = beginX > 0 ? beginX : 0;
        beginY = centrePoint.y() - p1.height() / 2;
        beginY = beginY > 0 ? beginY : 0;
        pa1.drawPixmap(beginX, beginY, p1);
        pa1.end();
    } else {
        m_pixmap1 = QPixmap(QGuiApplication::screens().at(number)->geometry().size());
        QPainter pa1(&m_pixmap1);
        m_pixmap1.fill(QColor("#252525"));
        p1 = p1.scaledToHeight(QGuiApplication::screens().at(number)->geometry().size().height() + 8);
        if (p1.width() > QGuiApplication::screens().at(number)->geometry().size().width()) {
            p1 = p1.scaledToWidth(QGuiApplication::screens().at(number)->geometry().size().width());
        }
        // 多屏显示去除x偏移
        centrePoint = q_ptr->getCurScreenGeometry().center();
        beginX = centrePoint.x() - p1.width() / 2;
        beginX = beginX > 0 ? beginX : 0;
        beginY = centrePoint.y() - p1.height() / 2;
        beginY = beginY > 0 ? beginY : 0;
        pa1.drawPixmap(beginX, beginY, p1);
        pa1.end();
    }
}

void LibImageAnimationPrivate::setImage2(const QString &imageName2_bar)
{
    m_imageName2 = imageName2_bar;
    int beginX = 0, beginY = 0;
    QImage tImg;
    QString errMsg;
    LibUnionImage_NameSpace::loadStaticImageFromFile(imageName2_bar, tImg, errMsg);
    QPixmap p2 = QPixmap::fromImage(tImg);
    // 双屏下或者多屏下
    int number = QApplication::desktop()->screenNumber(q_ptr);
    if (number < 0) {
        number = 0;
    }
    if (p2.width() >= p2.height()) {
        m_pixmap2 = QPixmap(QGuiApplication::screens().at(number)->geometry().size());
        QPainter pa2(&m_pixmap2);
        m_pixmap2.fill(QColor("#252525"));
        p2 = p2.scaledToWidth(QGuiApplication::screens().at(number)->geometry().size().width());
        if (p2.height() > QGuiApplication::screens().at(number)->geometry().size().height()) {
            p2 = p2.scaledToHeight(QGuiApplication::screens().at(number)->geometry().size().height());
        }
        // 多屏显示下，要去除x偏移
        centrePoint = q_ptr->getCurScreenGeometry().center();
        beginX = centrePoint.x() - p2.width() / 2;
        beginX = beginX > 0 ? beginX : 0;
        beginY = centrePoint.y() - p2.height() / 2;
        beginY = beginY > 0 ? beginY : 0;
        pa2.drawPixmap(beginX, beginY, p2);
        pa2.end();
    } else {
        m_pixmap2 = QPixmap(QGuiApplication::screens().at(number)->geometry().size());
        QPainter pa2(&m_pixmap2);
        m_pixmap2.fill(QColor("#252525"));
        p2 = p2.scaledToHeight(QGuiApplication::screens().at(number)->geometry().size().height() + 8);
        if (p2.width() > QGuiApplication::screens().at(number)->geometry().size().width()) {
            p2 = p2.scaledToWidth(QGuiApplication::screens().at(number)->geometry().size().width());
        }
        // 多屏显示下，要去除x偏移
        centrePoint = q_ptr->getCurScreenGeometry().center();
        beginX = centrePoint.x() - p2.width() / 2;
        beginX = beginX > 0 ? beginX : 0;
        beginY = centrePoint.y() - p2.height() / 2;
        beginY = beginY > 0 ? beginY : 0;
        pa2.drawPixmap(beginX, beginY, p2);
        pa2.end();
    }
}

//ImageAnimation

LibImageAnimation::LibImageAnimation(QWidget *parent) :
    QWidget(parent), current_target(EffectPlay), d_ptr(new LibImageAnimationPrivate(this))
{
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setAttribute(Qt::WA_StyledBackground, true);
    QPalette pal(palette());
    pal.setColor(QPalette::Background, Qt::black);
    setAutoFillBackground(true);
    setPalette(pal);
}

LibImageAnimation::~LibImageAnimation()
{
    Q_D(LibImageAnimation);
    delete d;
}

void LibImageAnimation::startSlideShow(const QString &beginPath, const QStringList &pathlist)
{
    Q_D(LibImageAnimation);
    setPaintTarget(EffectPlay);
    d->setPathList(beginPath, pathlist);
//    d->startAnimation();
    d->setPlayOrStatue(LibImageAnimation::PlayStatue);
    d->setSlideModel(LibImageAnimation::AutoPlayModel);
    d->startStatic();
}

void LibImageAnimation::endSlider()
{
    Q_D(LibImageAnimation);
    d->endSlide();
}

void LibImageAnimation::playAndNext()
{
    Q_D(LibImageAnimation);
    d->setPlayOrStatue(LibImageAnimation::PlayStatue);
    d->setSlideModel(LibImageAnimation::ManualPlayModel);
    setPaintTarget(EffectPlay);
    d->startSingleNextAnimation();
}

void LibImageAnimation::playAndPre()
{
    Q_D(LibImageAnimation);
    d->setPlayOrStatue(LibImageAnimation::PlayStatue);
    d->setSlideModel(LibImageAnimation::ManualPlayModel);
    setPaintTarget(EffectPlay);
    d->startSinglePreAnimation();
}

void LibImageAnimation::pauseAndNext()
{
    Q_D(LibImageAnimation);
    d->setPlayOrStatue(LibImageAnimation::StopStatue);
    d->setSlideModel(LibImageAnimation::ManualPlayModel);
    setPaintTarget(SkipToNext);
    d->startSingleNextAnimation();
    update();
}

void LibImageAnimation::ifPauseAndContinue()
{
    Q_D(LibImageAnimation);
    d->setPlayOrStatue(LibImageAnimation::PlayStatue);
    d->setSlideModel(LibImageAnimation::AutoPlayModel);
    setPaintTarget(EffectPlay);
    d->setImage1(d->m_imageName2);
    d->setImage2(d->queue->jumpTonext());
    d->startAnimation();
}

const QString LibImageAnimation::currentPath()
{
    Q_D(LibImageAnimation);
    return d->getCurrentPath();
}

const QRect LibImageAnimation::getCurScreenGeometry()
{
    int number = QApplication::desktop()->screenNumber(this);
    if (number < 0) {
        number = 0;
    }
    QRect tempRect = QGuiApplication::screens().at(number)->geometry();
    tempRect.setRect(0, 0, tempRect.width(), tempRect.height());
    return tempRect;
}

void LibImageAnimation::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
    Q_D(LibImageAnimation);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QRect tempRect = getCurScreenGeometry();
    switch (current_target) {
    case EffectPlay: {

        d->effectPainter(&painter, tempRect);
        break;
    }
    case SkipToNext: {
        d->forwardPainter(&painter, tempRect);
        break;
    }
    case TurnBackPre: {
        d->retreatPainter(&painter, tempRect);
        break;
    }
    case KeepStatic: {
        d->keepStaticPainter(&painter, tempRect);
        break;
    }
    }
}

void LibImageAnimation::setPaintTarget(PaintTarget target)
{
    current_target = target;
}

#include "imageanimation.moc"
