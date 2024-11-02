#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <knotifier.h>

using namespace kdk;
class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private Q_SLOTS:
    void onActionInvoked(quint32 id, QString actionKey);

private:
    int m_id;
};
#endif // WIDGET_H
