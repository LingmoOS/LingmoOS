#ifndef DEBINFOWIDGET_H
#define DEBINFOWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QStandardPaths>
#include <QDebug>
#include <QVariant>
#include <QFile>
#include <QIcon>
#include <QPixmap>
#include <QGSettings>

#define ORG_LINGMO_STYLE_FONT            "org.lingmo.style"
#define GSETTING_FONT_KEY              "systemFontSize"

class debInfoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit debInfoWidget(QString appName,QString appVersion,QWidget *parent = nullptr);


    QString      m_appName;
    QString      m_appVersion;
    QVBoxLayout  *debInfoLayout;
    /*app  icon name  layout*/
    QWidget      *IconAndAppNameWidget;
    QVBoxLayout  *IconAndAppNameLayout;
    QHBoxLayout  *deb_nameLayout;
    QLabel       *debIconLabel;

    QHBoxLayout  * Namelyout;
    QHBoxLayout  * Versionlyout;
    /*deb version name layout*/
    QLabel       *debAppNameLabel;
    QWidget      *debNameAndVersionWidget;
    QHBoxLayout  *debNameAndVersionLayout;
    QLabel       *debNameLabel;
    QLabel       *debNameLabel_pkg;
    QLabel       *debVersionLabel;
    QWidget      *nameVersionWidget;
    QVBoxLayout  *nameVersionVLayout;
    QGSettings   *m_pGsettingFontSize;
    QString      getIconPath(QString appName);
    QString      setLabelStringBody(QString text, QLabel *lable);
    QString      SpliteText(QFontMetrics font, QString text,int nlablesize);
    QString      getAppCnName(QString appName);
    void         resetAppInfo(QString appName, QString appVersion);
    void         fontGsettingInit();
    void         setLabelText();
    void         setdebnamelable();
    void         setdebNamelable();
    QFont        getSystemFont(int size);

};

#endif // DEBINFOWIDGET_H
