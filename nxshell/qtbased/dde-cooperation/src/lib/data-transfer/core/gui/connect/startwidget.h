#ifndef STARTWIDGET_H
#define STARTWIDGET_H

#include <QFrame>

class QPushButton;
class StartWidget : public QFrame
{
    Q_OBJECT

public:
    StartWidget(QWidget *parent = nullptr);
    ~StartWidget();

public slots:
    void nextPage();
    void themeChanged(int theme);

private:
    void initUI();
    QPushButton *nextButton { nullptr };
};

#endif
