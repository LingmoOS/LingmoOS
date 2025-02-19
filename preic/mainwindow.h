#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>

class WelcomePage;
class InstallPage;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QStackedWidget *stackedWidget;
    WelcomePage *welcomePage;
    InstallPage *installPage;
    QPropertyAnimation *fadeInAnimation;
    void setupAnimations();
};

#endif // MAINWINDOW_H 