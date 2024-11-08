#ifndef CUSTOMMESSAGEBOX_H
#define CUSTOMMESSAGEBOX_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QGSettings>
class CustomMessageBox : public QDialog {
    Q_OBJECT

public:
    explicit CustomMessageBox(int type,QWidget *parent = nullptr);
    ~CustomMessageBox() Q_DECL_OVERRIDE;
    QString m_text;
    QString getText();
    QGSettings *styleGsettings = nullptr;
    int m_type;
    void SetText(QString text);
    QLabel *textLabel = nullptr;
protected:
    void paintEvent(QPaintEvent *) override;
    void showEvent(QShowEvent *event) override;
signals:
    void yesClicked();
    void noClicked();
};

#endif // CUSTOMMESSAGEBOX_H
