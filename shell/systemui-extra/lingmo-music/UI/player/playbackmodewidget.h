#ifndef PLAYBACKMODEWIDGET_H
#define PLAYBACKMODEWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QIcon>
#include <QMouseEvent>
#include <QFocusEvent>
#include <QEvent>
#include <QDebug>
#include <QX11Info>

class PlayBackModeWidget : public QDialog
{
    Q_OBJECT
public:
    explicit PlayBackModeWidget(QWidget *parent = nullptr);
public:
    void changePlayModePos(int posX, int posY, int width, int height);
    void playModecolor();
public:
    QToolButton *loopBtn = nullptr;
    QToolButton *randomBtn = nullptr;
    QToolButton *sequentialBtn = nullptr;
    QToolButton *currentItemInLoopBtn = nullptr;
protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) Q_DECL_OVERRIDE;
Q_SIGNALS:

private:
    void initWidget();
private:
    int playModePosX;
    int playModePosY;
    int playModePosWidth;
    int playModePosHeight;
};

#endif // PLAYBACKMODEWIDGET_H
