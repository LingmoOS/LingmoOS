#ifndef TITLEWIDGET_H
#define TITLEWIDGET_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QFileDialog>
#include <QCheckBox>
#include <QMenu>
#include <QProcess>
#include <QTextCodec>
#include <QtXml/QDomDocument>
#include <qxmlstream.h>
#include <QMessageBox>
#include <QMenu>
#include <QDebug>
#include <QFileInfo>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <polkit-qt5-1/polkitqt1-authority.h>
#include <QDBusInterface>
#include <QDBusReply>
#include <QPainter>
#include <QTranslator>
#include <QStackedWidget>
#include <QToolButton>
#include "picturetowhite.h"

#define ORG_LINGMO_STYLE_FONT            "org.lingmo.style"
#define GSETTING_FONT_KEY              "systemFontSize"

class titleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit titleWidget(QWidget *parent = nullptr);


    QHBoxLayout    *m_pTitleLayout;

    QLabel         *m_pLogoLabel;
    QLabel         *m_pNameLabel;

    QPushButton    *m_pClostBtn;
    QPushButton    *m_pMinBtn;
    QToolButton    *m_pSetBtn;


    QGSettings     *m_pGsettings;
    PictureToWhite *m_pPictureToWhite;

    QIcon          m_WhiteIconCloseBtn;
    QIcon          m_WhiteIconMinBtn;
    QIcon          m_WhiteIconSetBtn;
    QGSettings   *m_pGsettingFontSize;

    QMenu* m_menu;

     QFont getSystemFont();
//    QAction actionIpSet;
//    QAction actionAuthorize;

signals:
    void set_closewidget(bool status);

    void setopenmenu();

    void menuitemclicked(QString itemname);

public slots:
    void closeBtnClicked();
};

#endif // TITLEWIDGET_H
