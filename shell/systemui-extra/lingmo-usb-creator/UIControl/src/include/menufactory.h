#ifndef MENUFACTORY_H
#define MENUFACTORY_H

#include <QObject>
#include <QMenu>
#include <QToolButton>
class MenuFactory : public QObject
{
    Q_OBJECT
public:
    explicit MenuFactory(QObject *parent = nullptr);

signals:
private:
    QMenu *m_menu = nullptr;
    QMenu *themeMenu = nullptr;
    QAction *actionHelp;
    QAction *actionAbout;
    QAction *actionTheme;
    QAction *actionsetting;
    QAction *actionQuit;
    QAction *autoTheme;
    QAction *lightTheme;
    QAction *darkTheme;
public:
    QToolButton *menuButton = nullptr;
private:
    void init();

};

#endif // MENUFACTORY_H
