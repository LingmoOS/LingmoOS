#ifndef WELCOME_MANAGER_H
#define WELCOME_MANAGER_H

#include <QStackedWidget>
#include "welcome_window.h"
#include "theme_page.h"
#include "dock_page.h"
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include "finish_page.h"
#include <QEvent>
#include <QLabel>

class WelcomeManager : public QStackedWidget
{
    Q_OBJECT

public:
    explicit WelcomeManager(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void nextPage();
    void slideToPage(QWidget* page);

private:
    WelcomeWindow *welcomePage;
    ThemePage *themePage;
    DockPage *dockPage;
    FinishPage *finishPage;
    QLabel *betaLabel;
};

#endif // WELCOME_MANAGER_H 