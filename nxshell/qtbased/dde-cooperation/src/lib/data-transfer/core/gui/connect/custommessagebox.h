#ifndef CUSTOMMESSAGEBOX_H
#define CUSTOMMESSAGEBOX_H


#include <QDialog>
#include <QLabel>
#include <QPushButton>

class CustomMessageBox : public QDialog
{
    Q_OBJECT

public:
    CustomMessageBox(const QString &mainTitle, const QString &subTitle, QWidget *parent = nullptr);

    static bool SelectContinueTransfer();
protected:
    void paintEvent(QPaintEvent *event) override;
private:
    void initUi();

    QString message1;
    QString message2;
signals:
    void ok();
    void cancel();
};





#endif // CUSTOMMESSAGEBOX_H
