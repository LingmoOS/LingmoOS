//#ifndef DEBINFOWIDGET_H
//#define DEBINFOWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
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
#include <QProgressBar>
#include <QMessageBox>
#include <QProcess>
#include <QCoreApplication>
#include <QApt/Backend>
#include <QFileInfo>
#include <QStorageInfo>
#include <QApt/DebFile>
#include <QToolButton>

#define ORG_LINGMO_STYLE_FONT            "org.lingmo.style"
#define GSETTING_FONT_KEY              "systemFontSize"

class Batchinstall : public QWidget
{
    Q_OBJECT
public:
    explicit Batchinstall(QString debpath,QWidget *parent = nullptr);

    QString m_appname;
    QString m_appversion;
    QString m_debArch;
    QString zh_apname;

    QString m_debpath;
    QApt::DebFile * debfile;

    QProgressBar     *pProgressBar;
    QLabel           *progress;
    QWidget      *widgetall;
    QWidget      *nameVersionWidget;
    QWidget      *statuswidget;
    QLabel       *appnamelabel;
    QLabel       *appversionlabel;
    QLabel       *appiconlabel;
    QLabel       *m_appiconlabel;
    QToolButton  *deleteitem;
    QLabel       *appstatus;

    QHBoxLayout  *Layoutall;
    QHBoxLayout  *debiconLayout;
    QHBoxLayout  *deleteitemlayout;
    QHBoxLayout  *stauslayout;
    QHBoxLayout  *stauslayoutall;
    QVBoxLayout  *debNameAndVersioNLayout;
    QGSettings   *m_pGsettingFontSize;


    QString getDebInfoFromDeb(QString m_debPath, QString type);
    QString getAppCnName(QString appName);
    QString getIconPath(QString appName);
    QString setLabelStringBody(QString text, QLabel *lable);
    void removelistwidgetitem();
    QFont getSystemFont(int size);
signals:
    void deleteitemstring(QString msg);


};
