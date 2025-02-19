#ifndef WELCOME_WINDOW_H
#define WELCOME_WINDOW_H

#include <QWidget>
#include <QLabel>
#include <QFrame>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGraphicsEffect>

class WelcomeWindow : public QWidget
{
    Q_OBJECT

public:
    explicit WelcomeWindow(QWidget *parent = nullptr);

signals:
    void nextClicked();

private:
    QLabel *titleLabel;
    QLabel *subtitleLabel;
    QPushButton *nextButton;
    QVBoxLayout *mainLayout;
    QFrame *mainCard;
    
    void setupUI();
    QGraphicsEffect* createShadowEffect();
};

#endif // WELCOME_WINDOW_H 