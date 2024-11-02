#include <QFont>
#include "parts_result.h"

parts_result::parts_result(int i_result , QWidget *parent) : QWidget(parent)
{
   init();
   create_interface(i_result);
}
void parts_result::init()
{
   p_icon = NULL;
   p_result_txt = NULL;
   p_hlayout_1 = NULL;
}

void parts_result::create_interface(int i_result)
{
   this->setFixedSize(550 , 50);

   /*图片*/
   p_icon = new QLabel(this);
   p_icon->setFixedSize(48 , 48);

   if (i_result == 0) {
      /*加载成功图标*/
      QPixmap icon("://image/success.png");
      icon = icon.scaled(p_icon->width() , p_icon->height());
      p_icon->setPixmap(icon);
   } else {
      /*加载失败图标*/
      QPixmap icon("://image/unsuccess.png");
      icon = icon.scaled(p_icon->width() , p_icon->height());
      p_icon->setPixmap(icon);
   }

   /*显示文本*/
   p_result_txt = new QLabel(this);
   p_result_txt->resize(189 , 36);
   QFont font;
   font.setPixelSize(24);
   p_result_txt->setFont(font);
   if (i_result == 0) {
      p_result_txt->setText(tr("uninstall success"));
   } else {
      p_result_txt->setText(tr("uninstall fail"));
   }

   p_hlayout_1 = new QHBoxLayout;
   p_hlayout_1->setMargin(0);
   p_hlayout_1->addSpacing(170);
   p_hlayout_1->addWidget(p_icon);
   p_hlayout_1->addSpacing(16);
   p_hlayout_1->addWidget(p_result_txt);
   p_hlayout_1->addStretch(0);

   this->setLayout(p_hlayout_1);
   return;
}
