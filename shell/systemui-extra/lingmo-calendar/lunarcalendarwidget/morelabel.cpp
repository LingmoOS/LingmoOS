#include "morelabel.h"
#include <QMouseEvent>
#include <QDebug>
#include <QGSettings>
#include "calendarcolor.h"
ShowMoreLabel::ShowMoreLabel(QWidget *parent) : QLabel(parent)
{
//    const QByteArray i("org.lingmo.style");
//    if (QGSettings::isSchemaInstalled(i)){
//    StyleSetting = new QGSettings(i);
    color1 = CalendarColor::CalendarColor::getThemeColor(CalendarColor::CLICKED);
//    }
    setStyleSheet(QString("QLabel { color: %1; }").arg(color1.name()));
    QFont font;
    font.setPointSize(16); // 设置字体大小为 14 像素
    setFont(font); // 应用字体到 QLabel

}

ShowMoreLabel::~ShowMoreLabel()
{
//    if(StyleSetting){
//        delete StyleSetting;
//        StyleSetting = nullptr;
//    }
}
void ShowMoreLabel::settext(const QString &text)
{
    m_text  = text;
}
void ShowMoreLabel::mousePressEvent(QMouseEvent *event)
{
    if(m_text == tr("More")){
       Q_EMIT clicked();
    }else if(m_text == tr("Collapse")){
        Q_EMIT backClicked();
    }


}
