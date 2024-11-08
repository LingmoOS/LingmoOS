/*
 * QRoundProgressBar - a circular progress bar Qt widget.
 *
 * Sintegrial Technologies (c) 2015-now
 *
 * The software is freeware and is distributed "as is" with the complete source codes.
 * Anybody is free to use it in any software projects, either commercial or non-commercial.
 * Please do not remove this copyright message and remain the name of the author unchanged.
 *
 * It is very appreciated if you produce some feedback to us case you are going to use
 * the software.
 *
 * Please send your questions, suggestions, and information about found issues to the
 *
 * sintegrial@gmail.com
 *
 */


#include "QRoundProgressBar.h"

#include <QtGui/QPainter>
#define COLOR_BLUE "#3790FA"
#define COLOR_LIGHT_BLUE "#DDEBFF"
#define COLOR_BLUE_DARK "#3B4251"
QColor QRoundProgressBar::baseColorAfter= QColor(227,236,248);//theme::baseColorAfter;//
QColor QRoundProgressBar::baseColorFront=QColor(255, 255, 255);
QRoundProgressBar::QRoundProgressBar(QWidget *parent) :
    QWidget(parent)
//    m_min(0), m_max(25*60),
//    m_value(0),
//    m_nullPosition(PositionTop),
//    m_barStyle(StyleDonut),
//    m_outlinePenWidth(1),
//    m_dataPenWidth(1),
//    m_rebuildBrush(false),
//    m_format("%p%"),
//    m_decimals(0),
//    m_updateFlags(UF_PERCENT)
{
    menu=new menuModule(this);
  //  this->setWindowFlags(Qt::CustomizeWindowHint /*| Qt::FramelessWindowHint*//*|Qt::SubWindow*/);
    //this->setFocusPolicy(Qt::ClickFocus);

//    if(theme::themetype==1){
//        baseColorAfter=QColor(43,44,46);
//    }else{
//        baseColorAfter=QColor(227,236,248);
//    }
}

void QRoundProgressBar::paintEvent(QPaintEvent* /*event*/)
{
    QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.save();

        m_rotateAngle = 360*m_persent/100;

        int side = qMin(width(), height());
        QRectF outRect(0, 0, side, side);
        QRectF inRect(10, 10, side - 20, side - 20);
        QString valueStr = QString("%1%").arg(QString::number(m_persent));

        //画外圆
        QPen pen = p.pen();
        p.setPen(Qt::NoPen);
        bool ii=menu->isDarkTheme();
        if(menu->isDarkTheme()){
            p.setBrush(QBrush(QColor(43,44,46)));
        }else{
            p.setBrush(QBrush(QColor(COLOR_LIGHT_BLUE)));
        }
        p.drawEllipse(outRect);
        p.setBrush(QBrush(QColor(COLOR_BLUE)));

        //画遮罩
        p.setPen(Qt::NoPen);
        if(menu->m_isDarktheme==true){
            p.setBrush(QBrush(QColor(COLOR_BLUE_DARK)));
        }else{
            p.setBrush(palette().window().color());
        }
        p.drawEllipse(inRect);

        //画圆弧
        pen.setCapStyle(Qt::RoundCap);
        pen.setColor(QColor(COLOR_BLUE));
        qreal penWidth = 5;
        pen.setWidthF(penWidth);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawArc(QRectF(5 + penWidth/2 , 5 + penWidth/2, side - 10 - penWidth, side - 10 - penWidth), (90-m_rotateAngle)*16, m_rotateAngle*16);
        // p.drawPie(QRectF(5 , 5, side - 10, side - 10), (90-m_rotateAngle)*16, m_rotateAngle*16);

        // 画进度圆点（白圈套篮圈，白圈有个篮圈模糊边）
        p.save();
        QPainterPath path;
        path.moveTo(inRect.center());
        path.arcTo(QRectF(5 + penWidth/2 , 5 + penWidth/2, side - 10 - penWidth, side - 10 - penWidth), 90, -m_rotateAngle);
        QPointF currentPosition = path.currentPosition();
        QColor color(QColor(COLOR_BLUE));
        color.setAlphaF(0.5);
        p.setPen(color);
        p.setBrush(QBrush(QColor(Qt::white)));
        p.drawEllipse(currentPosition, 6, 6);
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(QColor(COLOR_BLUE)));
        p.drawEllipse(currentPosition, 3.5, 3.5);
        p.restore();

        //画文字
        QFont f = this->font();
        f.setBold(true);
        f.setPixelSize(25);
        p.setFont(f);
        p.setPen(QColor(COLOR_BLUE));
        p.drawText(inRect, Qt::AlignCenter, valueStr);
        p.restore();

}
void QRoundProgressBar::setPersent(int persent)
{
    QString ii=QString::number(persent);
    if(ii.isEmpty()){
        return;
    }else{
    m_persent = persent;
    repaint();
    }
}
//    double outerRadius = qMin(width(), height());
//    QRectF baseRect(1, 1, outerRadius-2, outerRadius-2);

//    QImage buffer(outerRadius, outerRadius, QImage::Format_ARGB32_Premultiplied);
//    QPainter p(&buffer);
//    p.setRenderHint(QPainter::Antialiasing);

//    // data brush
//    rebuildDataBrushIfNeeded();

//    // background
//    drawBackground(p, buffer.rect());

//    // base circle
//    drawBase(p, baseRect);

//    // data circle
//    double arcStep = 360.0 / (m_max - m_min) * m_value;
//    drawValue(p, baseRect, m_value, arcStep);

//    // center circle
//    double innerRadius(0);
//    QRectF innerRect;
//    calculateInnerRect(baseRect, outerRadius, innerRect, innerRadius);
//    drawInnerBackground(p, innerRect);

//    // text
//    drawText(p, innerRect, innerRadius, m_value);

//    // finally draw the bar
//    p.end();

//    QPainter painter(this);
//    painter.fillRect(baseRect, palette().background());
//    painter.drawImage(0,0, buffer);
//}

//void QRoundProgressBar::drawBackground(QPainter &p, const QRectF &baseRect)
//{
//    p.save();
//    p.setBrush(QBrush(theme::backcolcr));
//    p.fillRect(baseRect,QBrush(theme::backcolcr));
//    p.restore();
//}

//void QRoundProgressBar::drawBase(QPainter &p, const QRectF &baseRect)
//{
//    switch (m_barStyle)
//    {
//    case StyleDonut:
//        //p.setPen(QPen(palette().shadow().color(), m_outlinePenWidth));
//        //p.setBrush(palette().base());//画两圆中间区域颜色
//        p.setBrush(QBrush(QColor(240,248,255/*,1*/)));
//        p.setPen(QColor(250,250,250));
//        p.drawEllipse(baseRect);
//        break;

//    case StylePie:
//        p.setPen(QPen(palette().base().color(), m_outlinePenWidth));
////        p.setBrush(palette().base());
//        p.drawEllipse(baseRect);
//        break;

//    case StyleLine:
//    {
//        //1-25   p.setPen(QPen(baseColorAfter, m_outlinePenWidth));/*palette().base().color()*//*QColor(227,236,248)*/
//        p.setPen(QPen(theme::baseColorAfter, m_outlinePenWidth));

//        p.setBrush(Qt::NoBrush);


//        p.setRenderHint(QPainter::Antialiasing, true);

////        // 设置渐变色
////        QLinearGradient linear(QPointF(80, 80), QPointF(150, 150));
////        linear.setColorAt(0, Qt::black);
////        linear.setColorAt(1, Qt::white);
////        // 设置显示模式
////        linear.setSpread(QGradient::PadSpread);
////        p.setBrush(linear);

//        p.drawEllipse(baseRect.adjusted(m_outlinePenWidth/2, m_outlinePenWidth/2, -m_outlinePenWidth/2, -m_outlinePenWidth/2));
//        break;

//    }
//    default:;
//    }

//}

//void QRoundProgressBar::drawValue(QPainter &p, const QRectF &baseRect, double value, double arcLength)
//{
//    // nothing to draw
//    if (value == m_min)
//        return;

//    // for Line style
//    if (m_barStyle == StyleLine)
//    {
//        p.save();
//        QPen pen;//画圆角
//        pen.setWidth(m_dataPenWidth);
//        //pen.setColor(baseColorFront);//1-25
//        pen.setColor(QColor(41,135,236));
//        pen.setCapStyle(Qt::RoundCap);
//        p.setPen(pen);
//        p.setBrush(Qt::NoBrush);
//        //p.setPen(QPen(baseColorFront, m_dataPenWidth));/*palette().highlight().color()*//*QColor(111,145,255)*/
////        QRadialGradient adient()
////        p.setBrush(adient);


////        // 设置渐变色
////        QLinearGradient linear(QPointF(80, 80), QPointF(150, 150));
////        linear.setColorAt(0, Qt::black);
////        linear.setColorAt(1, Qt::white);
////        // 设置显示模式
////        linear.setSpread(QGradient::PadSpread);
////        p.setBrush(linear);
//        p.drawArc(baseRect.adjusted(m_outlinePenWidth/2, m_outlinePenWidth/2, -m_outlinePenWidth/2, -m_outlinePenWidth/2),
//                  m_nullPosition * 16,
//                  -arcLength * 16);
//        p.restore();
//        p.save();
//        //画白点
//        QPainterPath path;
//        //创建一个移动到位于以角度占据给定矩形的弧上。
//        //角度以度数指定。 可以使用负角指定顺时针弧。
//        //创建一个占据给定矩形的圆弧，从指定的 startAngle 开始并逆时针扩展sweepLength 度。
//        //角度以度数指定。 可以使用负角指定顺时针弧。
//        //请注意，如果弧的起点尚未连接，则此函数会将弧的起点连接到当前位置。
//        //添加圆弧后，当前位置为圆弧中的最后一个点。 要绘制一条返回第一个点的线，请使用 closeSubpath() 函数。
//        path.moveTo(baseRect.center());
//        path.arcTo(baseRect.adjusted(m_outlinePenWidth/2, m_outlinePenWidth/2, -m_outlinePenWidth/2, -m_outlinePenWidth/2), m_nullPosition, -(arcLength));
//        QPointF po = path.currentPosition();
//        p.setPen(Qt::NoPen);
//        p.setBrush(QBrush(QColor(Qt::white)));
//        p.drawEllipse(po,6,6);
//        QPainterPath path1;
//        path1.moveTo(baseRect.center());
//        path1.arcTo(baseRect.adjusted(m_outlinePenWidth/2, m_outlinePenWidth/2, -m_outlinePenWidth/2, -m_outlinePenWidth/2), m_nullPosition, -(arcLength));
//        QPointF po1=path1.currentPosition();
//        p.setPen(Qt::NoPen);
//        p.setBrush(QBrush(QColor(41,135,236)));
//        p.drawEllipse(po1,3,3);
//        p.restore();
//        return;
//    }

//    // for Pie and Donut styles
//    QPainterPath dataPath;
//    dataPath.setFillRule(Qt::WindingFill);

//    // pie segment outer
//    dataPath.moveTo(baseRect.center());
//    dataPath.arcTo(baseRect, m_nullPosition, -arcLength);
//    dataPath.lineTo(baseRect.center());

//    p.setBrush(palette().highlight());


//    p.setPen(QPen(palette().shadow().color(), m_dataPenWidth));
//    p.drawPath(dataPath);
//}

//void QRoundProgressBar::calculateInnerRect(const QRectF &/*baseRect*/, double outerRadius, QRectF &innerRect, double &innerRadius)
//{
//    // for Line style
//    if (m_barStyle == StyleLine)
//    {
//        innerRadius = outerRadius - m_outlinePenWidth;
//    }
//    else    // for Pie and Donut styles
//    {
//        innerRadius = outerRadius * 0.85;//进度条宽度
//    }

//    double delta = (outerRadius - innerRadius) / 2;
//    innerRect = QRectF(delta, delta, innerRadius, innerRadius);
//}

//void QRoundProgressBar::drawInnerBackground(QPainter &p, const QRectF &innerRect)
//{
//    if (m_barStyle == StyleDonut)
//    {
//        //p.setBrush(QBrush(QColor(255,255,255)));
//        p.setBrush(QBrush(theme::backcolcr));
//        //p.setBrush(palette().alternateBase());
//        p.setPen(QColor(250,250,250));
//        p.drawEllipse(innerRect);
//    }
//    p.setBrush(Qt::NoBrush);

////    QPen pen;
////    pen.setColor(QColor(250,250,250));//227,236,248/*theme::baseColorAfter*//*QColor(227,236,248)*/
////    pen.setWidth(2);
////    pen.setCapStyle(Qt::RoundCap);
////    //pen.setStyle(Qt::DotLine);
////    QVector<double> dashes;
////    qreal space=1;
////    dashes<<space<<11<<space<<11<<space
////         <<11<<space<<11<<space<<11;
////    pen.setDashPattern(dashes);
////    p.setPen(pen);
////    p.drawEllipse(innerRect.x()+20,innerRect.y()+20,innerRect.width()-40,innerRect.height()-40);



//}

//void QRoundProgressBar::drawText(QPainter &p, const QRectF &innerRect, double innerRadius, double value)
//{
//    if (m_format.isEmpty())
//        return;

//    // !!! to revise
//    QFont f(font());
//    f.setPixelSize(innerRadius * qMax(0.05, (0.35 - (double)m_decimals * 0.08)));
//    QFont ff;//
//    ff.setPointSize(15);//
//    p.setFont(ff);//
//    //p.setFont(f);

//    //QPalette pe;
//    //pe.setColor(QPalette::ButtonText,QColor("#469BFF"));//设置字体 颜色大小
//    QRectF textRect(innerRect.x()-35,innerRect.y()+10,156,64);
//   // p.setPen(palette().text().color());
//    p.setPen(QColor(41,135,236));//QColor("#262626"));//
//    p.drawText(textRect, Qt::AlignCenter, valueToText(value));
//}

//QString QRoundProgressBar::valueToText(double value) const
//{
//    QString textToDraw(m_format);

//    if (m_updateFlags & UF_VALUE)
//        textToDraw.replace("%v", QString::number(value, 'f', m_decimals));

//    if (m_updateFlags & UF_PERCENT)
//    {
//        double procent = (value - m_min) / (m_max - m_min) * 100.0;
//        textToDraw.replace("%p", QString::number(procent, 'f', m_decimals));
//    }

//    if (m_updateFlags & UF_MAX)
//        textToDraw.replace("%m", QString::number(m_max - m_min + 1, 'f', m_decimals));

//    return textToDraw;
//}

//void QRoundProgressBar::valueFormatChanged()
//{
//    m_updateFlags = 0;

//    if (m_format.contains("%v"))
//        m_updateFlags |= UF_VALUE;

//    if (m_format.contains("%p"))
//        m_updateFlags |= UF_PERCENT;

//    if (m_format.contains("%m"))
//        m_updateFlags |= UF_MAX;

//    update();
//}

//void QRoundProgressBar::rebuildDataBrushIfNeeded()
//{
//    if (m_rebuildBrush)
//    {
//        m_rebuildBrush = false;

//        QConicalGradient dataBrush;
//        dataBrush.setCenter(0.5,0.5);
//        dataBrush.setCoordinateMode(QGradient::StretchToDeviceMode);

//        // invert colors
//        for (int i = 0; i < m_gradientData.count(); i++)
//        {
//            dataBrush.setColorAt(1.0 - m_gradientData.at(i).first, m_gradientData.at(i).second);
//        }

//        // angle
//        dataBrush.setAngle(m_nullPosition);

//        QPalette p(palette());
//        p.setBrush(QPalette::Highlight, dataBrush);
//        //p.setBrush(QPalette::Highlight,QBrush(QColor(240,248,255)));
//        setPalette(p);
//    }
//}
