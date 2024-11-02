#ifndef PARTS_TITLE_H
#define PARTS_TITLE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPalette>
#include <QIcon>
#include "picturetowhite.h"

class parts_title : public QWidget
{
    Q_OBJECT

public:
   explicit parts_title(QWidget *parent = nullptr);
   void init();
   void create_interface();
   void create_interrupt();

private:
   QLabel *p_icon;
   QLabel *p_explain;
   QPushButton *p_minimize;
   QPushButton *p_close;
   QHBoxLayout *p_hlayout;
   PictureToWhite *p_picturetowhite;
   QGSettings  *p_gsettings;

   QIcon icon_minimize_reverse;
   QIcon icon_close_reverse;

signals:
   void sig_close_interface();
   void sig_minimize_interface();

public slots:
   void slot_close_interface();
   void slot_minimize_interface();
};

#endif // PARTS_TITLE_H
