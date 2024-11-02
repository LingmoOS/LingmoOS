#include "widbox.h"
#include "kreinstall.h"
#include <QApplication>
#include <QDesktopWidget>
#include "xatom-helper.h"
//#include "qtsingleapplication.h"
#include "adaptscreeninfo.h"
#include <qlogging.h>
#include <stdio.h>
#include <stdlib.h>
#include <QGSettings>
#include <QLibraryInfo>
#include <QTranslator>
#include <QSettings>
#include <lingmosdk/lingmosdk-system/libkysysinfo.h>
#include <qlogging.h>
#include <QDir>
#include <singleapplication.h>
#include <lingmostylehelper/lingmostylehelper.h>
#include <signal.h>
#include <QPalette>
widbox::widbox(QString text,QWidget *parent) : QWidget(parent)
{
      gsettingInit();
      adaptScreenInfo adapt;
      int x = adapt.m_nScreen_x;
      int y = adapt.m_nScreen_y;
      int width = adapt.m_screenWidth;
      int height = adapt.m_screenHeight;
      MotifWmHints hints;
      hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
      hints.functions = MWM_FUNC_ALL;
      hints.decorations = MWM_DECOR_BORDER;
      XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
      setWindowTitle(tr("lingmo-installer"));
      //this->setWindowModality(Qt::ApplicationModal);
//      QPalette palette;
//      palette.setColor(QPalette::Window, Qt::white);
//      this->setPalette(palette);

      this->setFixedWidth(424);
      this->setMaximumHeight(120);
      this->move((width / 2 + x) - this->width() / 2 , (height / 2 + y )- this->height() / 2);
      KWindowSystem::setState(this->winId(),NET::SkipTaskbar |NET::SkipPager);

      windicon = new QLabel();
      windicon->setFixedSize(22,22);
      windicon->setPixmap(QIcon::fromTheme("dialog-error").pixmap(QSize(22,22)));


      widinfo = new QLabel();

      widinfo->setText(text);
      textname = text;
      //widinfo->setWordWrap(true);
      widinfo->setFixedWidth(300);

      widinfo->setFont(getSystemFont(14));
      QFontMetrics font1(widinfo->font());
      QString texq1 = setAutoWrap(font1,widinfo->text(),widinfo->width());
      widinfo->setText(texq1);



      widinfo->setWordWrap(true);


      buttonclose = new QPushButton(this);
      buttonclose->setFlat(true);
      buttonclose->setProperty("isWindowButton", 0x2);
      buttonclose->setProperty("useIconHighlightEffect", 0x8);
      buttonclose->setFixedSize(30,30);
      buttonclose->setToolTip(tr("Close"));
      buttonclose->setIcon(QIcon::fromTheme("window-close-symbolic"));
      connect(buttonclose,&QPushButton::clicked,this,[=]{
          this->close();
      });


      widbutton = new QPushButton();
      widbutton->setText(tr("OK"));
      widbutton->setFont(getSystemFont(14));
      connect(widbutton,&QPushButton::clicked,this,[=]{
          this->close();
      });

      layoutBox1 = new QHBoxLayout();
      layoutBox1->setContentsMargins(0,0,0,0);
      layoutBox1->setSpacing(0);


      layoutBox1->addStretch();
      layoutBox1->addWidget(buttonclose);
      layoutBox1->addSpacing(5);


      layoutBox2 = new QHBoxLayout();
      //layoutBox2->setContentsMargins(0,0,0,0);
      layoutBox2->setSpacing(0);

      layoutBox2->addSpacing(25);
      //layoutBox1->addSpacing(5);
      layoutBox2->addWidget(windicon);
      layoutBox2->addSpacing(9);
      layoutBox2->addWidget(widinfo);
      layoutBox2->addSpacing(25);

      layoutBox3 = new QHBoxLayout();
      layoutBox3->setContentsMargins(0,0,0,0);
      layoutBox3->setSpacing(0);

      layoutBox3->addStretch();
      layoutBox3->addWidget(widbutton);
      layoutBox3->addSpacing(24);


      Allbox = new QVBoxLayout();
      Allbox->setContentsMargins(0,0,0,0);
      Allbox->setSpacing(0);

      Allbox->addSpacing(5);
      Allbox->addLayout(layoutBox1);
      Allbox->addSpacing(6);
      Allbox->addLayout(layoutBox2);
      Allbox->addSpacing(5);
      Allbox->addLayout(layoutBox3);
      Allbox->addSpacing(20);


      this->setLayout(Allbox);

      this->show();

     // this->setFixedHeight(this->height());
      this->move((width / 2 + x) - this->width() / 2 , (height / 2 + y )- this->height() / 2);

}
QString widbox::setAutoWrap(const QFontMetrics& font, const QString& text, int nLabelSize)
{

    int nTextSize = font.width(text);
    if (nTextSize > nLabelSize)
    {
        int nPos = 0;
        long nOffset = 0;
        for (int i = 0; i < text.size(); i++)
        {
            nOffset += font.width(text.at(i));
            if (nOffset > nLabelSize)
            {
                nPos = i -1;
                break;
            }
        }

        QString qstrLeftData = text.left(nPos);
        QString qstrMidData = text.mid(nPos);
        return qstrLeftData + "\n" + setAutoWrap(font, qstrMidData, nLabelSize);
    }
    return text;

}

void widbox::gsettingInit()
{
    const QByteArray style_id(ORG_LINGMO_STYLE);
    m_pGsettingFontSize = new QGSettings(style_id);
    connect(m_pGsettingFontSize, &QGSettings::changed, this, [=] (const QString &key){
        if (key==GSETTING_KEY) {
            qDebug() << "8787878787--> ";
            widinfo->setFixedWidth(300);
            QFontMetrics font1(widinfo->font());
            QString texq1 = setAutoWrap(font1,textname,widinfo->width());
            widinfo->setText(texq1);
            widinfo->setFont(getSystemFont(14));
            widbutton->setFont(getSystemFont(14));
       }
    });
}

QFont widbox::getSystemFont(int size )
{
    double fontsize = (m_pGsettingFontSize->get(GSETTING_FONT_KEY).toFloat());

    double lableBaseFontSize = size * 9/12;//魔鬼数字，自行处理
    double nowFontSize = lableBaseFontSize * double(fontsize) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    if (size == 18){
        font.setFamily("Noto Sans CJK SC");
        font.setWeight(500);

    }
    font.setBold(false);
    font.setPointSizeF(nowFontSize);


    return font;
}
