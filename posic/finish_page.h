#ifndef FINISH_PAGE_H
#define FINISH_PAGE_H

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include "ripple_button.h"

class FinishPage : public QWidget
{
    Q_OBJECT

public:
    explicit FinishPage(QWidget *parent = nullptr);

public slots:
    void applyTheme(bool isLight);

private:
    QFrame *mainCard;
    QLabel *titleLabel;
    QLabel *messageLabel;
    RippleButton *finishButton;
    
    void setupUI();
    QGraphicsEffect* createShadowEffect();

signals:
    void finished();
};

#endif // FINISH_PAGE_H 