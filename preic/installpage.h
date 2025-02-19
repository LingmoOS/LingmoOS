#ifndef INSTALLPAGE_H
#define INSTALLPAGE_H

#include <QWidget>

class QPushButton;
class QLabel;

class InstallPage : public QWidget
{
    Q_OBJECT

public:
    explicit InstallPage(QWidget *parent = nullptr);

private:
    QPushButton *installButton;
    QPushButton *tryButton;
    void setupUI();
    
    QLabel *titleLabel;
    QLabel *subtitleLabel;
    QLabel *installTitle;
    QLabel *installDesc;
    QLabel *tryText;
    QLabel *tryDesc;

private slots:
    void updateText();
};

#endif // INSTALLPAGE_H 