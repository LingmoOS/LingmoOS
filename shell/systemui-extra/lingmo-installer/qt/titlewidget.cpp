#include "titlewidget.h"

titleWidget::titleWidget(QWidget *parent) : QWidget(parent)
{
    const QByteArray style_id(ORG_LINGMO_STYLE_FONT);
    m_pGsettingFontSize = new QGSettings(style_id);
    this->setFixedHeight(30);

    m_pPictureToWhite = new PictureToWhite();
    m_WhiteIconCloseBtn = m_pPictureToWhite->drawSymbolicColoredPixmap( \
                QIcon::fromTheme("window-close-symbolic").pixmap(QSize(30 , 30)));
    m_WhiteIconMinBtn = m_pPictureToWhite->drawSymbolicColoredPixmap( \
                QIcon::fromTheme("window-minimize-symbolic").pixmap(QSize(30 , 30)));

    m_WhiteIconSetBtn = m_pPictureToWhite->drawSymbolicColoredPixmap( \
                QIcon::fromTheme("open-menu-symbolic").pixmap(QSize(30 , 30)));


    m_pGsettings = new QGSettings("org.lingmo.style");

    m_pTitleLayout  = new QHBoxLayout;
    m_pTitleLayout->setContentsMargins(0, 0, 0, 0);
    m_pTitleLayout->setSpacing(0);

    m_pLogoLabel = new QLabel();
    m_pLogoLabel->setPixmap(QIcon::fromTheme("lingmo-installer").pixmap(QSize(24,24)));

    m_pNameLabel = new QLabel();
    m_pNameLabel->setText(tr("Installer"));
    m_pNameLabel->adjustSize();
    m_pClostBtn = new QPushButton(this);

    m_pClostBtn->setFlat(true);
    m_pClostBtn->setProperty("isWindowButton", 0x2);
    m_pClostBtn->setProperty("useIconHighlightEffect", 0x8);
    m_pClostBtn->setFixedSize(30,30);
    m_pClostBtn->setToolTip(tr("Close"));

    m_pMinBtn = new QPushButton();
    m_pMinBtn->setFlat(true);
    m_pMinBtn->setProperty("isWindowButton", 0x1);
    m_pMinBtn->setProperty("useIconHighlightEffect", 0x2);
    m_pMinBtn->setFixedSize(30,30);
    m_pMinBtn->setToolTip(tr("Min"));


    //open-menu-symbolic
    m_pSetBtn = new QToolButton();
//    m_pSetBtn->setFlat(true);

    m_pSetBtn->setFixedSize(30,30);

    m_pSetBtn->setProperty("isWindowButton", 0x1);
    m_pSetBtn->setProperty("useIconHighlightEffect", 0x2);
    m_pSetBtn->setPopupMode(QToolButton::InstantPopup);
    m_pSetBtn->setAutoRaise(true);
    m_pSetBtn->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    m_pSetBtn->setToolTip(tr("Menu"));


    m_pTitleLayout->addItem(new QSpacerItem(8, 1, QSizePolicy::Fixed));
    m_pTitleLayout->addWidget(m_pLogoLabel);
    m_pTitleLayout->addSpacerItem(new QSpacerItem(8, 1, QSizePolicy::Fixed));
    m_pTitleLayout->addWidget(m_pNameLabel);
    m_pTitleLayout->addSpacerItem(new QSpacerItem(366, 1, QSizePolicy::Expanding));
    m_pTitleLayout->addWidget(m_pSetBtn);
    m_pTitleLayout->addSpacerItem(new QSpacerItem(4, 1, QSizePolicy::Fixed));
    m_pTitleLayout->addWidget(m_pMinBtn);
    m_pTitleLayout->addSpacerItem(new QSpacerItem(4, 1, QSizePolicy::Fixed));
    m_pTitleLayout->addWidget(m_pClostBtn);
    m_pTitleLayout->addSpacerItem(new QSpacerItem(4, 1, QSizePolicy::Fixed));
    this->setLayout(m_pTitleLayout);


    //初始化下拉菜单
    m_menu = new QMenu();
    QList<QAction *> actions ;
//    QAction *actionTheme = new QAction(m_menu);
//    actionTheme->setText(tr("Theme"));
    QAction *actionAbout = new QAction(m_menu);
    actionAbout->setText(tr("About"));
    QAction *actionHelp = new QAction(m_menu);
    actionHelp->setText(tr("Help"));
    QAction *actionQuit = new QAction(m_menu);
    actionQuit->setText(tr("Quit"));

    actionAbout->setFont(getSystemFont());
    actionHelp->setFont(getSystemFont());
    actionQuit->setFont(getSystemFont());
//    m_pLogoLabel->setFont(getSystemFont());
//    m_pNameLabel->setFont(getSystemFont());
    actions << actionHelp << actionAbout<<actionQuit;
    m_menu->addActions(actions);
//    menu->addSeparator();

//    menu->setWindowFlags(menu->windowFlags() | Qt::FramelessWindowHint);
//    menu->setAttribute(Qt::WA_TranslucentBackground);

    m_pSetBtn->setMenu(m_menu);



//    connect(m_menu, &QMenu::triggered, this, [=](QAction * act){
//        QString tr = act->text();
//       // emit menuitemclicked (tr);
//        qDebug() << "aaaaaaaaaaaaaaaaa---->" << tr;
//    });


    if(m_pGsettings->get(STYLE_NAME).toString() == STYLE_NAME_KEY_LIGHT \
            || m_pGsettings->get(STYLE_NAME).toString() == STYLE_NAME_KEY_DEFAULT) {
        m_pClostBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
        m_pMinBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));

    }
    else {
        m_pClostBtn->setIcon(m_WhiteIconCloseBtn);
        m_pMinBtn->setIcon(m_WhiteIconMinBtn);
        m_pSetBtn->setIcon(m_WhiteIconSetBtn);
    }

    connect(m_pClostBtn,SIGNAL(clicked(bool)),this,SLOT(closeBtnClicked()));

    //m_pSetBtn->setMenu(menu);



    /*主题模式监听*/
    QStringList stylelist;
    stylelist << STYLE_NAME_KEY_DARK << STYLE_NAME_KEY_LIGHT << STYLE_NAME_KEY_DEFAULT \
              << STYLE_NAME_KEY_WHITE;

    connect (m_pGsettings, &QGSettings::changed, this, [=] (const QString &key) {
        //qDebug() << "12312312312";
        m_pLogoLabel->setPixmap(QIcon::fromTheme("lingmo-installer").pixmap(QSize(24,24)));
        if (key == STYLE_NAME) {
            if(stylelist.contains(m_pGsettings->get(STYLE_NAME).toString()) \
                    && (m_pGsettings->get(STYLE_NAME).toString() == STYLE_NAME_KEY_LIGHT \
                        || m_pGsettings->get(STYLE_NAME).toString() == STYLE_NAME_KEY_DEFAULT)) {
                m_pMinBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
                m_pClostBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
                m_pSetBtn->setIcon(QIcon::fromTheme("open-menu-symbolic"));
            }
            else {
                m_pMinBtn->setIcon(m_WhiteIconMinBtn);
                m_pClostBtn->setIcon(m_WhiteIconCloseBtn);
                m_pSetBtn->setIcon(m_WhiteIconSetBtn);

            }
        }
        if (key==GSETTING_FONT_KEY)
        {
            actionAbout->setFont(getSystemFont());
            actionHelp->setFont(getSystemFont());
            actionQuit->setFont(getSystemFont());
//            m_pLogoLabel->setFont(getSystemFont());
//            m_pNameLabel->setFont(getSystemFont());

        }
    });
    this->hide();

}
void titleWidget::closeBtnClicked()
{
    emit set_closewidget(true);
}


QFont titleWidget::getSystemFont()
{
    double fontsize = (m_pGsettingFontSize->get(GSETTING_FONT_KEY).toFloat()* 8/7);

    double lableBaseFontSize = 14 * 9/12;//魔鬼数字，自行处理
    double nowFontSize = lableBaseFontSize * double(fontsize) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    font.setBold(false);
    font.setPointSizeF(nowFontSize);
    return font;
}
