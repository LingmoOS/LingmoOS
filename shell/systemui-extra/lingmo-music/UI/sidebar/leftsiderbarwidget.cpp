#include "leftsiderbarwidget.h"
#include <QDebug>
#include <QGSettings>
#include <QStyleOption>
#include <QPainterPath>
#include <QPainter>
#include "kwindoweffects.h"
#include "UI/base/widgetstyle.h"
#define PERSONALSIE_SCHEMA     "org.lingmo.control-center.personalise"
#define PERSONALSIE_TRAN_KEY   "transparency"
#define CONTAIN_PERSONALSIE_TRAN_KEY   "transparency"

LeftsiderbarWidget::LeftsiderbarWidget(QWidget *parent)
    : QWidget(parent)
{

    QGSettings *personalQgsettings = nullptr;
    if (QGSettings::isSchemaInstalled(PERSONALSIE_SCHEMA)) {
        personalQgsettings = new QGSettings(PERSONALSIE_SCHEMA, QByteArray(), this);
        transparency = personalQgsettings->get(PERSONALSIE_TRAN_KEY).toDouble() * 255;
        connect(personalQgsettings,&QGSettings::changed,this,[=](QString changedKey) {  //监听透明度变化
                        if (changedKey == CONTAIN_PERSONALSIE_TRAN_KEY) {
                           transparency = personalQgsettings->get(PERSONALSIE_TRAN_KEY).toDouble() * 255;
                           this->repaint();
                        }
                });
    } else {
        personalQgsettings = nullptr;
        qDebug()<<PERSONALSIE_SCHEMA<<" not installed";
    }
}

LeftsiderbarWidget::~LeftsiderbarWidget()
{

}


void LeftsiderbarWidget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing); // 反锯齿
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect(), 0, 0);
    /* 开启背景模糊效果（毛玻璃）可以使用主题的QMe */
    KWindowEffects::enableBlurBehind(this->winId(), true, QRegion(rectPath.toFillPolygon().toPolygon()));

    p.setPen(Qt::NoPen);

    QColor color = palette().color(QPalette::Window);
    color.setAlpha(transparency);
    QPalette pal(this->palette());

    if(WidgetStyle::themeColor == 1 ){
       pal.setColor(QPalette::Window,QColor(38, 38, 38, 0));
       this->setPalette(pal);
    }else if(WidgetStyle::themeColor == 0){
           pal.setColor(QPalette::Window,QColor(255, 255, 255, 0));
           this->setPalette(pal);
       }


    QStyleOption opt;
    opt.init(this);

    QBrush brush =QBrush(color);
    p.setBrush(brush);
    p.drawRoundedRect(opt.rect,0,0);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QWidget::paintEvent(event);
}
