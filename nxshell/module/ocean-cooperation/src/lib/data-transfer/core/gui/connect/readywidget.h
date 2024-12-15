#ifndef READYWIDGET_H
#define READYWIDGET_H

#include <QFrame>

class QLineEdit;
class QPushButton;
class QLabel;
class ReadyWidget : public QFrame
{
    Q_OBJECT
public:
    ReadyWidget(QWidget *parent = nullptr);
    ~ReadyWidget();
    void clear();

public slots:
    void nextPage();
    void backPage();
    void onLineTextChange();
    void connectFailed();
private:
    void initUI();
    void tryConnect();
    void setnextButEnable(bool enabel);
    QLineEdit *ipInput{ nullptr };
    QLineEdit *captchaInput{ nullptr };
    QPushButton *nextButton{ nullptr };
    QLabel *tiptextlabel{ nullptr };

    QTimer *timer{ nullptr };
};

#endif // READYWIDGET_H
