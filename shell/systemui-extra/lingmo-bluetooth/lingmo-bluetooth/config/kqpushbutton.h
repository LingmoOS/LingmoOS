#ifndef KQPUSHBUTTON_H
#define KQPUSHBUTTON_H

#include <QObject>
#include <QPushButton>
#include <QEvent>

class KQPushButton : public QPushButton
{
    Q_OBJECT
public:
    explicit KQPushButton(const QString &text, QWidget *parent = nullptr);
    explicit KQPushButton(QWidget *parent = nullptr);

protected:
    bool event(QEvent *e);
signals:
    void hovered(bool);
};

#endif // KQPUSHBUTTON_H
