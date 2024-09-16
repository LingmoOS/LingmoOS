// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sprayguntool.h"
#include "pagescene.h"
#include "pageview.h"
#include "pagecontext.h"
#include "attributemanager.h"
#include "cundoredocommand.h"
#include "layeritem.h"

#include <QToolButton>
#include <QStack>
#include <QtMath>
#include <QPainter>
#include <QTimer>
#include <QGraphicsScene>

class SprayGunTool::SprayGunTool_private
{
public:
    explicit SprayGunTool_private(SprayGunTool *qq): q(qq)
    {
        m_size = QSize(90, 90);
        m_totalTimes = 20;
        m_times = 0;
        m_radius = 20;
        resetFlags();
    }
    SprayGunTool *q;
//    void setParam(QSize size, int count, int radius)
//    {
//        m_size = size;
//        m_totalTimes = count;
//        m_times = 0;
//        m_radius = radius;
//        resetFlags();
//    }

    void resetFlags()
    {
        m_createFlags.clear();
        for (int i = 0; i < m_size.height(); i++) {
            QVector<int> t;
            for (int j = 0; j < m_size.width(); j++) {
                t << 0;
            }
            m_createFlags << t;
        }
    }


    QVector<QPoint> getRandArr()
    {
        QVector<QPoint> rs;
        srand(QTime::currentTime().msec());
        int num = m_size.width() * m_size.height() / m_totalTimes;
        if (m_times == m_totalTimes - 1) {
            for (int i = 0; i < m_createFlags.size(); ++i) {
                for (int j = 0; j < m_createFlags.at(i).size(); ++j) {
                    if (m_createFlags[i][j] == 0) {
                        rs.push_back(QPoint(i, j));
                        m_createFlags[i][j] = 1;
                    }
                }
            }
        } else {
            while (num > 0) {
                int xPos = rand() % m_size.width();
                int yPos = rand() % m_size.height();
                if (0 == m_createFlags[xPos][yPos]) {
                    rs.push_back(QPoint(xPos, yPos));
                    m_createFlags[xPos][yPos] = 1;
                    --num;
                }
            }
        }

        ++m_times;
        if (m_times == m_totalTimes) {
            m_times = 0;
            resetFlags();
        }

        return rs;
    }

    QVector<QPoint> getPoints(QPoint pos)
    {
        int r = m_radius;
        QPainterPath path;
        auto tPos = pos;
        path.addEllipse(QRect(tPos.x() - r, tPos.y() - r, 2 * r, 2 * r));
        path.closeSubpath();

        auto arr = getRandArr();
        QVector<QPoint> set;
        for (auto var : arr) {
            if (!path.contains(var + pos + QPoint(-r, -r))) {
                continue;
            }
            set.push_back(var + pos + QPoint(-r, -r));
        }

        return set;
    }

    void drawPointsToImage(QPicture &img, QPoint pos, QColor color)
    {
        QVector<QPoint> paintPoints = getPoints(pos);
        QPainter painter(&img);
        QPen pen(m_color);
        painter.setPen(pen);

        for (QPoint p : paintPoints) {
            painter.drawPoint(p);
        }

        painter.end();
    }

    void setPenAttribute(QColor color, int width)
    {
        m_color = color;
        m_radius = width;
        m_totalTimes = m_radius;

        if (m_size.width() < 2 * m_radius || m_size.height() < 2 * m_radius || m_size.width() > 3 * m_radius || m_size.height() > 3 * m_radius) {
            m_size.setWidth(2 * m_radius + 2);
            m_size.setHeight(2 * m_radius + 2);
        }
        m_times = 0;
        resetFlags();
    }
    //QPoint  m_pos;//当前位置
    QSize   m_size;//获取随机点的宽高
    int     m_totalTimes;//一共要获取次数
    QVector<QVector<int>>   m_createFlags;//获取的标志
    int     m_times;//当前获取次数
    int     m_radius;//获取点的半径
    QColor  m_color;
    RasterItem *m_layer = nullptr;
    RasterPaintCompsitor _rasterPaintCompsitor;


};

SprayGunTool::SprayGunTool(QObject *parent): DrawFunctionTool(parent), SprayGunTool_d(new SprayGunTool_private(this))
    , m_timer(new QTimer(this)), m_img(nullptr), m_scene(nullptr)
{
    auto btn = toolButton();
    setWgtAccesibleName(btn, "Paint bucket tool button");
    btn->setToolTip(tr(""));
    btn->setIconSize(QSize(20, 20));
    btn->setFixedSize(QSize(37, 37));
    btn->setCheckable(true);
    btn->setIcon(QIcon::fromTheme("clear", QIcon(RCC_DRAWBASEPATH + "clear_20px.svg")));

    connect(this, &SprayGunTool::toolManagerChanged, this, [ = ](DrawBoardToolMgr * _t1, DrawBoardToolMgr * _t2) {
        Q_UNUSED(_t1)
        if (_t2 != nullptr) {
            auto board = _t2->drawBoard();
            connect(board, &DrawBoard::pageAdded, this, [ = ](Page * page) {
                page->context()->setDefaultAttri(EUserAttri + 2, 20);
            });
        }
    });

    connect(m_timer, &QTimer::timeout, this, [ = ] {
        //if (nullptr != m_img && !m_img->isNull())
        {
            RasterPaint pic;
            SprayGunTool_d->drawPointsToImage(pic, m_pos, Qt::NoPen);
            SprayGunTool_d->_rasterPaintCompsitor.merge(pic);
            m_scene->update();
        }
    });
}

int SprayGunTool::toolType() const
{
    return MoreTool + 2;
}


SAttrisList SprayGunTool::attributions()
{
    SAttrisList result;
    result << defaultAttriVar(EPenColor);
    result << defaultAttriVar(EUserAttri + 2);
    return result;
}

int SprayGunTool::minMoveUpdateDistance()
{
    return 0;
}

void SprayGunTool::funcStart(ToolSceneEvent *event)
{
    auto item = currentLayer(event);
    if (item == nullptr) {
        return;
    }

    QColor color = event->view()->page()->defaultAttriVar(EPenColor).value<QColor>();
    int sprayGunWidth = event->view()->page()->defaultAttriVar(EUserAttri + 2).value<int>();

    if (!color.isValid())
        return;

    if (item->type() == LayerItemType) {
        auto layer = static_cast<RasterItem *>(item);
        SprayGunTool_d->m_layer = layer;

        m_pos = layer->mapFromScene(event->pos()).toPoint();
        SprayGunTool_d->setPenAttribute(color, sprayGunWidth);
        SprayGunTool_d->_rasterPaintCompsitor.clear();
        m_timer->start(50);
        m_scene = event->scene();
    }

    event->scene()->update();
}

void SprayGunTool::funcUpdate(ToolSceneEvent *event, int decided)
{
    Q_UNUSED(decided)
    auto item = currentLayer(event);
    if (item == nullptr) {
        return;
    }

    QColor color = event->view()->page()->defaultAttriVar(EPenColor).value<QColor>();

    if (!color.isValid())
        return;

    if (item->type() == LayerItemType) {
        auto layer = static_cast<RasterItem *>(item);
        //QImage &img = layer->rImage();
        m_pos = layer->mapFromScene(event->pos()).toPoint();
    }

    event->scene()->update();
}

void SprayGunTool::funcFinished(ToolSceneEvent *event, int decided)
{
    Q_UNUSED(decided)
    auto item = currentLayer(event);
    if (item == nullptr) {
        return;
    }

    QColor color = event->view()->page()->defaultAttriVar(EPenColor).value<QColor>();

    if (!color.isValid())
        return;

    if (item->type() == LayerItemType) {
        auto layer = static_cast<RasterItem *>(item);
        QPicture rs = SprayGunTool_d->_rasterPaintCompsitor.result();
        layer->addPaint(rs);
    }

    m_img = nullptr;
    m_scene = nullptr;
    SprayGunTool_d->m_layer = nullptr;
    m_timer->stop();
}

void SprayGunTool::onStatusChanged(EStatus oldStatus, EStatus nowStatus)
{
    Q_UNUSED(oldStatus)
    Q_UNUSED(nowStatus)
}

void SprayGunTool::drawMore(QPainter *painter, const QRectF &rect, PageScene *scene)
{
    Q_UNUSED(rect)
    if (nullptr != scene->currentTopLayer()) {
        painter->setTransform(scene->currentTopLayer()->sceneTransform(), true);
        auto a = SprayGunTool_d->_rasterPaintCompsitor.result();
        painter->drawPicture(QPoint(0, 0), SprayGunTool_d->_rasterPaintCompsitor.result());
    }
}

PageItem *SprayGunTool::currentLayer(ToolSceneEvent *event)
{
    return event->scene()->currentTopLayer();
}

TOOLINTERFACE(SprayGunTool)
