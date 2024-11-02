#include "musicslider.h"
#include "UI/base/widgetstyle.h"
#include "UIControl/player/player.h"
#include <QApplication>

MusicSlider::MusicSlider(QWidget *parent):QSlider(parent),m_isPlaying(false)
{
    //现在进度条样式已基本符合要求，但是两端稍微有点瑕疵，暂搁置
    //逻辑也稍微有点问题，如果不选择歌曲，进度条应该禁止操作
    //所以将音乐播放进度条独立出来，方便协作和扩展

//    this->installEventFilter(this);
    initStyle();//初始化样式
}

void MusicSlider::initStyle()
{
    this->setFixedHeight(22);
    this->setOrientation(Qt::Horizontal);
//    this->setMinimum(0);
//    this->setMaximum(1000);

    changeStyleColor();
}

void MusicSlider::changeStyleColor()
{
    QColor highlight = QApplication::palette().highlight().color();
    QColor shadow = QApplication::palette().midlight().color();
    QString highlightStr = QString("#%1%2%3").arg(highlight.red(),2,16,QChar('0')).arg(highlight.green(),2,16,QChar('0')).arg(highlight.blue(),2,16,QChar('0'));
    QString shadowStr = QString("#%1%2%3").arg(shadow.red(),2,16,QChar('0')).arg(shadow.green(),2,16,QChar('0')).arg(shadow.blue(),2,16,QChar('0'));

//    qDebug() << "highlightStr: " << highlightStr
//             << "shadowStr: " << shadowStr;

    QString style;

    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ"){
        if (WidgetStyle::themeColor == 1) {
            style = QString("QSlider::handle:horizontal{height:6px;background-color:transparent;}"
                            "QSlider::groove:horizontal{left:0px;right:0px;height:2px;background-color:%2;}"
                            "QSlider::add-page:horizontal{background:%2;background-color:#333333;}" //设置滑块未滑过部分
                            "QSlider::sub-page:horizontal{background:%1;}").arg(shadowStr).arg(highlightStr); // //设置滑块已滑过部分
        } else {
            style = QString("QSlider::handle:horizontal{height:6px;background-color:transparent;}"
                            "QSlider::groove:horizontal{left:0px;right:0px;height:2px;background-color:%2;}"
                            "QSlider::add-page:horizontal{background:%2;}" //设置滑块未滑过部分
                            "QSlider::sub-page:horizontal{background:%1;}").arg(shadowStr).arg(highlightStr);
        }
    }else{
        if (WidgetStyle::themeColor == 1) {
            style = QString("QSlider::handle:horizontal{height:6px;background-color:transparent;}"
                            "QSlider::groove:horizontal{left:0px;right:0px;height:2px;background-color:%2;}"
                            "QSlider::add-page:horizontal{background:%2;background-color:#333333;}" //设置滑块未滑过部分
                            "QSlider::sub-page:horizontal{background:%1;}").arg(highlightStr).arg(shadowStr); // //设置滑块已滑过部分
        } else {
            style = QString("QSlider::handle:horizontal{height:6px;background-color:transparent;}"
                            "QSlider::groove:horizontal{left:0px;right:0px;height:2px;background-color:%2;}"
                            "QSlider::add-page:horizontal{background:%2;}" //设置滑块未滑过部分
                            "QSlider::sub-page:horizontal{background:%1;}").arg(highlightStr).arg(shadowStr);
        }
    }

    this->setStyleSheet(style);
}

void MusicSlider::isPlaying(bool isPlaying)
{
    m_isPlaying = isPlaying;
}

void MusicSlider::mousePressEvent(QMouseEvent *event)
{
    if(m_isPlaying == false)
    {
        return QSlider::mousePressEvent(event);
    }
    this->blockSignals(true);
    if (event->button() == Qt::LeftButton) //判断左键
    {
        QString systemLang = QLocale::system().name();
        if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ"){
            int value =this->maximum() - ((this->maximum() - this->minimum()) * ((double)event->x())) / (this->width());
            setValue(value);
        }else{
            int value = this->minimum() + ((this->maximum() - this->minimum()) * ((double)event->x())) / (this->width());
            setValue(value);
        }
    }

}

void MusicSlider::mouseMoveEvent(QMouseEvent *event)
{
    if(m_isPlaying == false)
    {
        return QSlider::mousePressEvent(event);
    }

    auto rang = this->width();

    if(rang == 0)
    {
        return;
    }

    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ"){
        auto value =this->maximum() - ((this->maximum() - this->minimum()) * ((double)event->x())) / (this->width());
        setValue(value);
    }else{
        auto value = this->minimum() + ((this->maximum() - this->minimum()) * ((double)event->x())) / (this->width());
        setValue(value);
    }
}

void MusicSlider::mouseReleaseEvent(QMouseEvent *event)
{
    if(m_isPlaying == false)
    {
        return QSlider::mousePressEvent(event);
    }
    this->blockSignals(false);
    QSlider::mouseReleaseEvent(event);

    int range = this->maximum() - minimum();
    Q_ASSERT(range != 0);
    if(value() <= range)
    {
        int position = value() * playController::getInstance().getPlayer()->duration() / range;
        playController::getInstance().getPlayer()->setPosition(position);
    }

}

void MusicSlider::enterEvent(QEvent *event)
{
    QColor highlight = QApplication::palette().highlight().color();
    QColor shadow = QApplication::palette().midlight().color();
    QString highlightStr = QString("#%1%2%3").arg(highlight.red(),2,16,QChar('0')).arg(highlight.green(),2,16,QChar('0')).arg(highlight.blue(),2,16,QChar('0'));
    QString shadowStr = QString("#%1%2%3").arg(shadow.red(),2,16,QChar('0')).arg(shadow.green(),2,16,QChar('0')).arg(shadow.blue(),2,16,QChar('0'));

    QString imageUrl(":/img/default/point.png)"); // 只用蓝色纯色图片，也可实现不同主题色，很奇特

    QString style;

    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ"){
        if(WidgetStyle::themeColor == 1) {
            style = QString("QSlider::handle:horizontal{width:12px; height:12px; margin-top: -5px; margin-left: 0px; margin-bottom: -5px; margin-right: 0px; border-image:url(%3);}"
                            "QSlider::groove:horizontal{left:0px;right:0px;position:absolute; height:2px;background-color:transparent;}"
                            "QSlider::add-page:horizontal{background:%2;}" //设置滑块未滑过部分
                            "QSlider::sub-page:horizontal{background:%1;}").arg(shadowStr).arg(highlightStr).arg(imageUrl); // //设置滑块已滑过部分
        } else if (WidgetStyle::themeColor == 0){
            style = QString("QSlider::handle:horizontal{width:12px; height:12px; margin-top: -5px; margin-left: 0px; margin-bottom: -5px; margin-right: 0px; border-image:url(%3);}"
                            "QSlider::groove:horizontal{left:0px;right:0px;height:2px;background-color:transparent;}"
                            "QSlider::add-page:horizontal{background:%2;}"
                            "QSlider::sub-page:horizontal{background:%1;}").arg(shadowStr).arg(highlightStr).arg(imageUrl); // //设置滑块已滑过部分
        }
    }else{
        if(WidgetStyle::themeColor == 1) {
            style = QString("QSlider::handle:horizontal{width:12px; height:12px; margin-top: -5px; margin-left: 0px; margin-bottom: -5px; margin-right: 0px; border-image:url(%3);}"
                            "QSlider::groove:horizontal{left:0px;right:0px;position:absolute; height:2px;background-color:transparent;}"
                            "QSlider::add-page:horizontal{background:%2;}" //设置滑块未滑过部分
                            "QSlider::sub-page:horizontal{background:%1;}").arg(highlightStr).arg(shadowStr).arg(imageUrl); // //设置滑块已滑过部分
        } else if (WidgetStyle::themeColor == 0){
            style = QString("QSlider::handle:horizontal{width:12px; height:12px; margin-top: -5px; margin-left: 0px; margin-bottom: -5px; margin-right: 0px; border-image:url(%3);}"
                            "QSlider::groove:horizontal{left:0px;right:0px;height:2px;background-color:transparent;}"
                            "QSlider::add-page:horizontal{background:%2;}"
                            "QSlider::sub-page:horizontal{background:%1;}").arg(highlightStr).arg(shadowStr).arg(imageUrl); // //设置滑块已滑过部分
        }
    }
    this->setStyleSheet(style);

//    QSlider::enterEvent(event);
}

void MusicSlider::leaveEvent(QEvent *event)
{
    initStyle();
    QSlider::leaveEvent(event);
}

void MusicSlider::wheelEvent(QWheelEvent *e)
{
    QWidget::wheelEvent(e);
}
