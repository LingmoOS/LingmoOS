#include "theme.h"
#include <QByteArray>
#include <QVariant>
/**
 * QT主题
 */
#define THEME_QT_SCHEMA "org.lingmo.style"
#define MODE_QT_KEY "style-name"
QString theme::backcolor_black=QString("rgba(63, 63, 63, 1)");
QString theme::backcolor_black2=QString("rgba(0, 63, 63, 1)");
QString theme::backcolor_withe=QString("rgba(250, 250, 250, 1)");
QString theme::backcolor_withe2=QString("rgba(216, 216, 216, 1)");

QString theme::pre_backcolor=QString("rgba(250, 250, 250, 1)");
QString theme::back_backcolor=QString("rgba(216, 216, 216, 1)");
QColor theme::backcolcr=QColor(250,250,250);
QColor theme::textcolcr=QColor(255,255,255,192);
QColor theme::textcolcr45=QColor(255,255,255,115);
//圆环
QColor theme::baseColorFront=QColor(41,135,236);
QColor theme::baseColorAfter=QColor(220,220,220);

QColor theme::pointColor=QColor(250,250,250);

//主题
int theme::themetype=0;
theme::theme(QObject *parent) : QObject(parent)
{
    if (QGSettings::isSchemaInstalled("org.lingmo.style")){
        qtSettings = new QGSettings("org.lingmo.style");
    }
    QString currentThemeMode =qtSettings->get("style-name").toString();
    if ("lingmo-white" == currentThemeMode) {
        pre_backcolor=QString("rgba(255, 255, 255, 1)");
        back_backcolor=QString("rgba(216, 217, 218, 1)");
        backcolcr=QColor(255,255,255);
        textcolcr=QColor(38,38,38,192);
        textcolcr45=QColor(38,38,38,115);
        baseColorFront=QColor(111,145,255);
        baseColorAfter=QColor(227,236,248);

        pointColor=QColor(227,236,248);

        themetype=0;
    }else if ("lingmo-light" == currentThemeMode) {
        pre_backcolor=QString("rgba(255, 255, 255, 1)");
        back_backcolor=QString("rgba(216, 217, 218, 1)");
        backcolcr=QColor(255,255,255);
        textcolcr=QColor(38,38,38,192);
        textcolcr45=QColor(38,38,38,115);
        baseColorFront=QColor(111,145,255);
        baseColorAfter=QColor(227,236,248);

        pointColor=QColor(227,236,248);
        themetype=0;
    }else if ("lingmo-dark" == currentThemeMode) {
        pre_backcolor=QString("rgba(51, 52, 54, 1)");
        back_backcolor=QString("rgba(28, 28, 30, 1)");
        backcolcr=QColor(51,52,54);
        textcolcr=QColor(255,255,255,191);

        textcolcr45=QColor(255,255,255,115);
        baseColorFront=QColor(70,159,255);
        baseColorAfter=QColor(43,44,46);

        pointColor=QColor(43,44,46);


        themetype=1;
    }else if ("lingmo-black" == currentThemeMode) {
        pre_backcolor=QString("rgba(51, 52, 54, 1)");
        back_backcolor=QString("rgba(28, 28, 30, 1)");
        backcolcr=QColor(51,52,54);
        textcolcr=QColor(255,255,255,191);
        textcolcr45=QColor(255,255,255,115);
        baseColorFront=QColor(70,159,255);
        baseColorAfter=QColor(43,44,46);

        pointColor=QColor(43,44,46);

        themetype=1;
    }else if ("" == currentThemeMode) {
        pre_backcolor=QString("rgba(255, 255, 255, 1)");
        back_backcolor=QString("rgba(216, 217, 218, 1)");
        backcolcr=QColor(255,255,255);
        baseColorFront=QColor(111,145,255);
        baseColorAfter=QColor(227,236,248);

        pointColor=QColor(227,236,248);
        themetype=0;
    } else{
        pre_backcolor=QString("rgba(255, 255, 255, 1)");
        back_backcolor=QString("rgba(216, 217, 218, 1)");
        backcolcr=QColor(255,255,255);

        baseColorFront=QColor(111,145,255);
        baseColorAfter=QColor(227,236,248);

        pointColor=QColor(227,236,248);
        themetype=0;
    }
    connect(qtSettings, &QGSettings::changed, this, [=]{
        QString currentThemeMode = qtSettings->get("style-name").toString();
        if ("lingmo-white" == currentThemeMode) {
            pre_backcolor=QString("rgba(255, 255, 255, 1)");
            back_backcolor=QString("rgba(216, 217, 218, 1)");
            backcolcr=QColor(255,255,255);
            textcolcr=QColor(38,38,38,192);

            textcolcr45=QColor(38,38,38,115);
            baseColorFront=QColor(111,145,255);
            baseColorAfter=QColor(227,236,248);

            pointColor=QColor(227,236,248);
            themetype=0;
        }else if ("lingmo-light" == currentThemeMode) {
            pre_backcolor=QString("rgba(255, 255, 255, 1)");
            back_backcolor=QString("rgba(216, 217, 218, 1)");
            backcolcr=QColor(255,255,255);
            textcolcr=QColor(38,38,38,192);
            textcolcr45=QColor(38,38,38,115);
            baseColorFront=QColor(111,145,255);
            baseColorAfter=QColor(227,236,248);

            pointColor=QColor(227,236,248);
            themetype=0;
        }else if ("lingmo-dark" == currentThemeMode) {
            pre_backcolor=QString("rgba(51, 52, 54, 1)");
            back_backcolor=QString("rgba(28, 28, 30, 1)");
            backcolcr=QColor(51,52,54);
            textcolcr=QColor(255,255,255,191);
            textcolcr45=QColor(255,255,255,115);
            baseColorFront=QColor(70,159,255);
            baseColorAfter=QColor(43,44,46);

            pointColor=QColor(43,44,46);

            themetype=1;
        }else if ("lingmo-black" == currentThemeMode) {
            pre_backcolor=QString("rgba(51, 52, 54, 1)");
            back_backcolor=QString("rgba(28, 28, 30, 1)");
            backcolcr=QColor(/*63,63,63*/51, 52, 54);
            textcolcr=QColor(255,255,255,191);
            textcolcr45=QColor(255,255,255,115);
            baseColorFront=QColor(70,159,255);
            baseColorAfter=QColor(43,44,46);
            pointColor=QColor(43,44,46);
            themetype=1;
        }else {
            pre_backcolor=QString("rgba(250, 250, 250, 1)");
            back_backcolor=QString("rgba(216, 217, 218, 1)");
            backcolcr=QColor(255,255,255);
            textcolcr=QColor(38,38,38,192);
            textcolcr45=QColor(38,38,38,115);
            baseColorFront=QColor(111,145,255);
            baseColorAfter=QColor(227,236,248);
            pointColor=QColor(227,236,248);
            themetype=0;
        }
    });
}
theme::~theme()
{
    delete qtSettings;
}
