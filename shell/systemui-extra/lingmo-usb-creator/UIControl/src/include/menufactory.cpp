#include "menufactory.h"

MenuFactory::MenuFactory(QObject *parent) : QObject(parent)
{
    init();
}
void MenuFactory::init()
{
    m_menu = new QMenu();
    menuButton = new QToolButton;
    menuButton->setMenu(m_menu);
    menuButton->setToolTip(tr("Options"));
    menuButton->setProperty("isWindowButton", 0x1);
    menuButton->setProperty("useIconHighlightEffect", 0x2);
    menuButton->setPopupMode(QToolButton::InstantPopup);
    menuButton->setFixedSize(30,30);
    menuButton->setIconSize(QSize(16, 16));
    menuButton->setAutoRaise(true);
    menuButton->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    QList<QAction *> actions ;
    actionTheme = new QAction(m_menu);
    actionTheme->setText(tr("Theme"));
    actionHelp = new QAction(m_menu);
    actionHelp->setText(tr("Help"));
    actionAbout = new QAction(m_menu);
    actionAbout->setText(tr("About"));
    actionsetting = new QAction(m_menu);
    actionsetting->setText(tr("Setting"));
    actionQuit = new QAction(m_menu);
    actionQuit->setText(tr("Quit"));
    actions<<actionAbout<<actionHelp<<actionQuit;
    m_menu->addActions(actions);
}
