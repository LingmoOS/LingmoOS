#include <QFont>
#include "parts_title.h"


parts_title::parts_title(QWidget *parent) : QWidget(parent)
{
    init();
    create_interface();
    create_interrupt();
}

void parts_title::init()
{
    p_icon = NULL;
    p_explain = NULL;
    p_minimize = NULL;
    p_close = NULL;
    p_hlayout = NULL;

    p_picturetowhite = new PictureToWhite();
    p_gsettings = new QGSettings("org.lingmo.style");

    icon_minimize_reverse = p_picturetowhite->drawSymbolicColoredPixmap(QIcon::fromTheme("window-minimize-symbolic").pixmap(QSize(24 , 24)));
    icon_close_reverse = p_picturetowhite->drawSymbolicColoredPixmap(QIcon::fromTheme("window-close-symbolic").pixmap(QSize(24 , 24)));

    return;
}

void parts_title::create_interface()
{
   /*图标*/
    p_icon = new QLabel(this);
    p_icon->setFixedSize(24 , 24);
    p_icon->setPixmap(QIcon::fromTheme("lingmo-installer").pixmap(QSize(24, 24)));
//   QPixmap icon("://image/install.png");
//   icon = icon.scaled(p_icon->width() , p_icon->height());
//   p_icon->setPixmap(icon);

   /*说明*/
   p_explain = new QLabel(this);
   p_explain->resize(170 , 20);
   p_explain->setText(tr("uninstaller"));

   /*最小化按钮*/
   p_minimize = new QPushButton(this);
   p_minimize->setFixedSize(30 , 30);
   p_minimize->setFlat(true);
   p_minimize->setProperty("isWindowButton", 0x1);
   p_minimize->setProperty("useIconHighlightEffect", 0x2);
   p_minimize->setFlat(true);
   p_minimize->setIconSize(QSize(16, 16));

   if (p_gsettings->get(STYLE_NAME).toString() == STYLE_NAME_KEY_LIGHT || p_gsettings->get(STYLE_NAME).toString() == STYLE_NAME_KEY_DEFAULT) {
      p_minimize->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
   } else {
      p_minimize->setIcon(icon_minimize_reverse);
   }

   /*关闭按钮*/
   p_close = new QPushButton(this);
   p_close->setFixedSize(30 , 30);
   p_close->setFlat(true);
   p_close->setProperty("isWindowButton", 0x2);
   p_close->setProperty("useIconHighlightEffect", 0x8);
   p_close->setFlat(true);
   p_close->setIconSize(QSize(16, 16));

   if (p_gsettings->get(STYLE_NAME).toString() == STYLE_NAME_KEY_LIGHT || p_gsettings->get(STYLE_NAME).toString() == STYLE_NAME_KEY_DEFAULT) {
      p_close->setIcon(QIcon::fromTheme("window-close-symbolic"));
   } else {
      p_close->setIcon(icon_close_reverse);
   }

   /*水平布局*/
   p_hlayout = new QHBoxLayout(this);
   p_hlayout->setMargin(0);
   p_hlayout->addSpacing(8);
   p_hlayout->addWidget(p_icon);
   p_hlayout->addSpacing(4);
   p_hlayout->addWidget(p_explain);
   p_hlayout->addStretch(274);
   p_hlayout->addWidget(p_minimize);
   p_hlayout->addSpacing(4);
   p_hlayout->addWidget(p_close);
   p_hlayout->addStretch(0);

   this->setLayout(p_hlayout);

   return;
}

void parts_title::create_interrupt()
{
   /*最小化*/
   connect(p_minimize , &QPushButton::clicked , this , &parts_title::slot_minimize_interface);

   /*关闭*/
   connect(p_close , &QPushButton::clicked , this , &parts_title::slot_close_interface);

   /*主题模式监听*/
   QStringList stylelist;
   stylelist << STYLE_NAME_KEY_DARK << STYLE_NAME_KEY_LIGHT << STYLE_NAME_KEY_DEFAULT;

   connect(p_gsettings , &QGSettings::changed , this ,
           [=] (const QString &key) {
              if (key == STYLE_NAME) {
                 if (stylelist.contains(p_gsettings->get(STYLE_NAME).toString()) && (p_gsettings->get(STYLE_NAME).toString() == STYLE_NAME_KEY_LIGHT || p_gsettings->get(STYLE_NAME).toString() == STYLE_NAME_KEY_DEFAULT)) {
                    p_minimize->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
                    p_close->setIcon(QIcon::fromTheme("window-close-symbolic"));
                 } else {
                    p_minimize->setIcon(icon_minimize_reverse);
                    p_close->setIcon(icon_close_reverse);
                 }
              }
           });

   return;
}

/*槽函数*/
void parts_title::slot_close_interface()
{
    emit sig_close_interface();
    return;
}

void parts_title::slot_minimize_interface()
{
    emit sig_minimize_interface();
    return;
}
