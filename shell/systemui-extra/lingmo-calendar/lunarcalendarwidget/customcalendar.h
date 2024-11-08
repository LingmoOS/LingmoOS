#ifndef CUSTOMCALENDAR_H
#define CUSTOMCALENDAR_H

#include <QWidget>
#include <QCalendarWidget>
#include <QPushButton>
#include <QLabel>

class CustomCalendar : public QCalendarWidget
{
    Q_OBJECT
public:
    CustomCalendar(QWidget *parent = nullptr);
    ~CustomCalendar();
    QPixmap drawSymbolicColoredPixmap(const QPixmap &source);
    QPixmap loadSvg(const QString &path, int size);

protected:
    void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const;
    void mouseMoveEvent(QMouseEvent *e);
    void paintEvent(QPaintEvent *event);
signals:
};

#endif // CUSTOMCALENDAR_H
