#ifndef MYTOOLBUTTON_H
#define MYTOOLBUTTON_H

#include <QToolButton>
#include <QPainter>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QtWidgets>

const QString IS_SELECT = "IS_SELECT";
class MyToolButton : public QToolButton
{
     Q_OBJECT
public:
     MyToolButton();
    ~MyToolButton();
     void selectChanged();
     void defaultStyle();
     void requestCustomContextMenu(const QPoint &pos);
     void initGsettings();
public:
    QAction *deleteAct = nullptr;
    QAction *renameAct = nullptr;
    QString buttonListName = "";
Q_SIGNALS:
    void playall(QString text);
    void renamePlayList(QString text);
    void removePlayList(QString text);

    void selectButtonChanged(QString text);
public Q_SLOTS:

private:
    int transparency = 0;

};


#endif
