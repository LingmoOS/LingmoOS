#ifndef SCHEDULEMARK_H
#define SCHEDULEMARK_H

#include <QWidget>
#include <QLabel>
#include <QPainter>
#include <QMouseEvent>
#include <QEvent>
#include <QDebug>
#include <QHBoxLayout>
#include <QMenu>
#include <QDateTime>
#include <QAction>
#include "schedulewidget.h"
#include "mylabel.h"
#include "schedulestruct.h"

class ScheduleMark : public QWidget
{
    Q_OBJECT
public:
    //颜色要对应主题选择
    enum COLOR_TYPE {
        ALL_DAY_CORLOR = 0,    // 全天类颜色
        SEVERAL_DAY_COLOR = 1, // 连续类颜色
        STANDAR_DAY_COLOR = 2, // 标准类颜色
    };
    explicit ScheduleMark(QWidget *parent = nullptr, const MarkInfo &info = {});
    MyLabel *m_label = nullptr;//显示的文字信息。未来可以用在还有...条日程中
    MarkInfo m_scheduleMarkInfo;
    //CSchceduleDlg *dlg = nullptr;
     void mousePressEvent(QMouseEvent *mouseEvent) override;
     void mouseMoveEvent(QMouseEvent *mouseEvent) override;
     QMenu *ExtraSchedule = nullptr;
     void ExtraMenu(QVector<MarkInfo> marks);
     QVector<MarkInfo> allmarks;
     QVector<MarkInfo> v_info;
     void setVectorMarkInfo(QVector<MarkInfo> infos);
protected:
    void drawBackGround();
    void drawBg(QPainter *);
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *keyEvent) override;
    //void mouseMoveEvent(QMouseEvent *mouseEvent) override;


private:

    void rightClickedMenuRequest();

signals:

    void editMarkSignal();
    void addMarkInfoSignal(const MarkInfo &info);
    void markClearRequest();
    void scheduleClicked(MarkInfo info);

public slots:
    //void addMark(const MarkInfo &info);
    void editScheduleSlot();
    void deleteScheduleSlot();
    void cancelScheduleEditSlot(); //取消编辑
    void addMarkInfoSlot(const MarkInfo &info);
    void updateMarkInfoSlot(const MarkInfo &info);
    void slotupdateInfo(const MarkInfo &info);

    void slotClicked(MarkInfo info);
    void slot(MarkInfo info);
};

#endif // SCHEDULEMARK_H
