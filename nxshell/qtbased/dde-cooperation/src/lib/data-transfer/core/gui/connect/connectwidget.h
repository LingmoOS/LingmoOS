#ifndef CONNECTWIDGET_H
#define CONNECTWIDGET_H
#ifdef __linux__
#    include <QFrame>

class QLabel;
class QHBoxLayout;
class QPushButton;
class ConnectWidget : public QFrame
{
    Q_OBJECT

public:
    ConnectWidget(QWidget *parent = nullptr);
    ~ConnectWidget();

    void initConnectLayout();

public slots:
    void nextPage();
    void backPage();
    void themeChanged(int theme);

private:
    void initUI();

private:
    QLabel *ipLabel = nullptr;
    QLabel *ipLabel1 = nullptr;
    QLabel *WarnningLabel = nullptr;
    QHBoxLayout *connectLayout = nullptr;
    int remainingTime = 300;
    QPushButton *backButton = nullptr;
    QLabel *separatorLabel = nullptr;
};
#endif
#endif
