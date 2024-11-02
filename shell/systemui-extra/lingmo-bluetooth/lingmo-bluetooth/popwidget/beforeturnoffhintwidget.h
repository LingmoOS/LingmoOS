#ifndef BEFORETURNOFFHINTWIDGET_H
#define BEFORETURNOFFHINTWIDGET_H

#include <QDialog>
#include <QLabel>
#include <QGSettings>

namespace Ui {
class beforeTurnOffHintWidget;
}

class BeforeTurnOffHintWidget : public QDialog
{
    Q_OBJECT

public:
    explicit BeforeTurnOffHintWidget(QWidget *parent = nullptr);
    ~BeforeTurnOffHintWidget();

private:
    Ui::beforeTurnOffHintWidget *ui;
    QGSettings *styleGSettings;
    QLabel *textLabel;

private:
    void initConnect();

signals:
    void buttonPressed(bool confirm);
};

#endif // BEFORETURNOFFHINTWIDGET_H
