#ifndef MULTITIFFTOOLWID_H
#define MULTITIFFTOOLWID_H

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QStyleOption>
class MultiTiffToolWid : public QWidget
{
    Q_OBJECT
public:
    explicit MultiTiffToolWid(QWidget *parent = nullptr);
    void themeStyle(QString theme);

private:
    void paintEvent(QPaintEvent *event);
    QColor m_backgroundColor;
    QString m_theme = "";
};

#endif // MULTITIFFTOOLWID_H
