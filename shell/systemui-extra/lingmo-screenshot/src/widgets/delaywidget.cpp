#include "delaywidget.h"
#include  <QPainter>
#include  <QFont>

#include <QGSettings>
#define LINGMO_STYLE_SCHEMA          "org.lingmo.style"
#define STYLE_NAME                 "styleName"
#define STYLE_NAME_KEY_DARK        "lingmo-dark"
#define STYLE_NAME_KEY_DEFAULT     "lingmo-default"
#define STYLE_NAME_KEY_BLACK       "lingmo-black"
#define STYLE_NAME_KEY_LIGHT       "lingmo-light"
#define STYLE_NAME_KEY_WHITE       "lingmo-white"

DelayWidget::DelayWidget(QWidget *parent) :
    QWidget(parent)
{
    setWindowFlags(Qt::WindowStaysOnTopHint
                   | Qt::FramelessWindowHint
                   | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setGeometry(100, 100, 118, 118);
    label = new  QLabel(this);

    QFont font;
    font.setPixelSize(62);
    font.setFamily("Arial");
    label->setFont(font);
    label->setAlignment(Qt::AlignCenter);
    label->setFixedSize(118, 118);
    label->setProperty("useSystemStyleBlur", true);
    listenToGsettings();
}

DelayWidget::~DelayWidget()
{
}

void DelayWidget::updateText(int time)
{
    QString num = QString::number(time);
    label->setText(num);
}

void DelayWidget::listenToGsettings()
{
    const QByteArray styleID(LINGMO_STYLE_SCHEMA);
    QStringList stylelist;
    if (QGSettings::isSchemaInstalled(styleID)) {
        QGSettings *styleLINGMO = new QGSettings(styleID, QByteArray(), this);
        stylelist << STYLE_NAME_KEY_DARK << STYLE_NAME_KEY_BLACK;
        // <<STYLE_NAME_KEY_DEFAULT;
        if (stylelist.contains(styleLINGMO->get(STYLE_NAME).toString())) {
            label->setStyleSheet("QLabel {"
                                 "border-width: 0px; "
                                 "border-radius: 18px; "
                                 "background-color:rgb(25,25,25,235);}");
        } else {
            label->setStyleSheet("QLabel {"
                                 "border-width: 0px; "
                                 "border-radius: 18px; "
                                 "background-color:rgb(225,225,225,235);}");
        }
        connect(styleLINGMO, &QGSettings::changed, this, [=](const QString &key)
        {
            if (key == STYLE_NAME) {
                if (stylelist.contains(styleLINGMO->get(
                                           STYLE_NAME).toString())) {
                    label->setStyleSheet("QLabel {"
                                         "border-width: 0px; "
                                         "border-radius: 18px; "
                                         "background-color:rgb(25,25,25,235);}");
                } else {
                    label->setStyleSheet("QLabel {"
                                         "border-width: 0px; "
                                         "border-radius: 18px; "
                                         "background-color:rgb(225,225,225,235);}");
                }
            }
        });
    }
}
