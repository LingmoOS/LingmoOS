#include "schedulemark.h"
#include "schedulewidget.h"
#include "schedulewidget.h"
//#include "dayitem.h"
#include <QMenu>
#include <QAction>
#include "calendardatabase.h"
//#include "leftwidget.h"
//#include "rightwidget.h"
//标志类，和Markinfo一起用
ScheduleMark::ScheduleMark(QWidget *parent, const MarkInfo &info) : QWidget(parent)
{

    m_scheduleMarkInfo = info;//MarkInfo是一个结构体 里面包含了日程的信息，开始日期 结束日期，内容是主要的 如果要和标记同样显示，那么主题色也要包含
    m_label = new MyLabel(this);
    m_label->setText(info.m_descript);
    QHBoxLayout *hlayout = new QHBoxLayout;
//    CSchceduleDlg *dlg = new CSchceduleDlg(0, this, true);
//    connect(dlg, &CSchceduleDlg::updateMarkInfoSignal, this, &ScheduleMark::updateMarkInfoSlot);//回车传来的
//    connect(dlg,&CSchceduleDlg::addToDatabase,this,&ScheduleMark::addMarkInfoSlot);
    hlayout->addWidget(m_label);
    hlayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hlayout);
    connect(this, &ScheduleMark::addMarkInfoSignal, this, &ScheduleMark::addMarkInfoSlot);
    connect(this,&ScheduleMark::scheduleClicked,this,&ScheduleMark::slotClicked);
//    connect(dlg,&CSchceduleDlg::updateInfo,this,&ScheduleMark::slotupdateInfo);
}
/**
 * @brief ScheduleMark::slotClicked
 * 用于生成菜单 
 * @param info
 */
void ScheduleMark::slotClicked(MarkInfo info)
{
    if(info.isExtra == true){
        qDebug()<<"=======ScheduleMark========mousePressEvent 该生成一个菜单了";
        if(m_scheduleMarkInfo.isExtra == true){
            qDebug()<<"该生成一个菜单了";
            ExtraSchedule = new QMenu(this);
            for (int i = 0; i < v_info.size(); i++) {
              qDebug()<<v_info.size()<<"hello";
              QAction *sch1 = new QAction(v_info.at(i).m_descript);
              ExtraSchedule->addAction(sch1);
              connect(sch1,&QAction::triggered,[=](){
                  slot(info);
              });
            }

            ExtraSchedule->exec(QCursor::pos());
        }
    }else{

    }
}
/**
 * @brief ScheduleMark::setVectorMarkInfo
 * @param infos
 */
void ScheduleMark::setVectorMarkInfo(QVector<MarkInfo> infos)
{
   v_info = infos;
}
void ScheduleMark::mouseMoveEvent(QMouseEvent *mouseEvent)
{
    setToolTip(m_scheduleMarkInfo.m_descript);
}
/**
 * @brief ScheduleMark::slot
 * 点击菜单中的某一项 这里用的是数组记录所有日程 点击后期望的是弹出日程信息弹窗 但是triggered没有参数传过来 这可咋整
 */
void ScheduleMark::slot(MarkInfo info)
{
   CSchceduleDlg *dlg = new CSchceduleDlg(0,this,true);
   dlg->m_textEdit->setText(info.m_descript);
   dlg->show();
}

void ScheduleMark::paintEvent(QPaintEvent *event)
{
    //绘制准备工作,启用反锯齿
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    drawBg(&painter);
}

void ScheduleMark::drawBg(QPainter *painter)
{
    painter->save();
    //根据当前类型选择对应的颜色
    QColor bgColor = QColor(0, 255, 49, 100);
    painter->setPen(QColor(255, 255, 255));
    painter->setBrush(bgColor);
    //painter->drawRect(rect());
    /* change
      if(RightWidget::Chronon::Day)
    {
        QRect rect = this->rect();
        rect.setWidth(100);
        rect.setHeight(500);
        painter->drawRect(rect);
        painter->restore();

        qDebug()<<"日视图要生成一个Mark";
    }*/
    QPen pen(QColor(0, 255, 255), 5);
    painter->setPen(pen);
    painter->drawLine(QPoint(0, 0), QPoint(0, height()));

    painter->restore();
}
/**
 * @brief ScheduleMark::mousePressEvent
 * Mark 以后也要替换成 日程的可拖拽item类 
 */
void ScheduleMark::mousePressEvent(QMouseEvent *mouseEvent)
{

    //return QWidget::mousePressEvent(mouseEvent);
    if (mouseEvent->button() == Qt::RightButton) {
        if (mouseEvent->type() == QEvent::MouseButtonPress) {
            qDebug() << "右键点击";
            rightClickedMenuRequest(); //右击弹窗:1编辑、2删除
        }
    }else if(mouseEvent->button() == Qt::LeftButton){

        emit scheduleClicked(m_scheduleMarkInfo);
    }
}
/**
 * @brief ScheduleMark::ExtraMenu
 * @param marks
 *
 */
void ScheduleMark::ExtraMenu(QVector<MarkInfo> marks)
{

}
void ScheduleMark::slotupdateInfo(const MarkInfo &info){
    qDebug()<<"开始更新";
    m_scheduleMarkInfo = info;
    qDebug()<<"info.m_descript="<<info.m_descript;
    m_label->setText(info.m_descript);
    emit DATABASE_INSTANCE.insertDataSignal(info);
}

void ScheduleMark::keyPressEvent(QKeyEvent *keyEvent)
{

    qDebug() << "获取删除键值";
    if (keyEvent->key() == Qt::Key_Delete) {
        qDebug() << "获取删除键值";
        this->deleteLater();
    }
}

void ScheduleMark::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    QRect rect = geometry();
    setFixedHeight(25);
}

void ScheduleMark::rightClickedMenuRequest()
{
    QMenu *m_menu = new QMenu(this);
    QAction *m_actionCalendar = new QAction(this);
    QAction *m_actionEdit = new QAction(this);
    QAction *m_actionCut = new QAction(this);
    QAction *m_actionCopy = new QAction(this);
    QAction *m_actionPaste = new QAction(this);
    QAction *m_actionDelete = new QAction(this);
    QAction *m_actionSend = new QAction(this);

    m_actionCalendar->setText(tr("Calendar"));
    m_actionEdit->setText(tr("Edit"));
    m_actionCut->setText(tr("Cut"));
    m_actionCopy->setText(tr("Copy"));
    m_actionPaste->setText(tr("Paste"));
    m_actionDelete->setText(tr("Delete"));
    m_actionPaste->setText(tr("Paste"));

    connect(m_actionEdit, &QAction::triggered, this, &ScheduleMark::editScheduleSlot);
    connect(m_actionDelete, &QAction::triggered, this, &ScheduleMark::deleteScheduleSlot);

    m_menu->addAction(m_actionCalendar);
    m_menu->addAction(m_actionEdit);
    m_menu->addAction(m_actionCut);
    m_menu->addAction(m_actionCopy);
    m_menu->addAction(m_actionPaste);
    m_menu->addAction(m_actionDelete); // 2020/12/12禁用
    m_menu->addAction(m_actionPaste);
    m_menu->exec(QCursor::pos());
}

void ScheduleMark::editScheduleSlot()
{
    QPoint weizhi = mapToGlobal(this->rect().center());
    qDebug() << "mark的相对位置:" << weizhi;
    CSchceduleDlg *dlg = new CSchceduleDlg(0, this, true);
    connect(dlg, &CSchceduleDlg::addManyDaysSignal, this, &ScheduleMark::addMarkInfoSlot);
    connect(dlg, &CSchceduleDlg::updateMarkInfoSignal, this, &ScheduleMark::updateMarkInfoSlot);//回车传来的
    connect(dlg, &CSchceduleDlg::cancelScheduleEditSignal, this, &ScheduleMark::cancelScheduleEditSlot);

    dlg->updateScheduleInfo(m_scheduleMarkInfo);
    dlg->move(weizhi.rx() + width() / 2, weizhi.ry() - height() * 6 - 10);
    dlg->show();
}
//以下代码是dayItem双击后传过来的生成小框的
void ScheduleMark::addMarkInfoSlot(const MarkInfo &info)
{
    qDebug()<<"进来了addMarkInfoSlot:"<<info.m_descript;//添加日程的时候点击enter发射了信号但是为什么不显示呢？
    //这是双击后出现后横框的代码 需要对日程内容同步更新
    //下面的宽度高度可以设置
    m_scheduleMarkInfo = info;
   // m_label->setText(info.m_descript);
    emit DATABASE_INSTANCE.insertDataSignal(info);
    qDebug() << "开始时间:" << info.m_markStartDate << "结束时间:" << info.m_markEndDate << "信息:" << info.m_descript
             << "开始:" << info.m_markStartTime << "结束:" << info.m_markEndTime << "宽度:" << info.m_markWidth
             << "高度:" << info.m_markHeight;
   // qDebug()<<m_label->text();
}
void ScheduleMark::updateMarkInfoSlot(const MarkInfo &info)
{
    qDebug()<<"走到了updateMarkInfoSlot";
    m_scheduleMarkInfo = info;
    qDebug()<<"传进来的info正常吗"<<m_scheduleMarkInfo.m_descript;
    m_label->setText(info.m_descript);
    m_label->update();
    m_label->repaint();
    emit DATABASE_INSTANCE.updateDataSignal(info);
}

void ScheduleMark::cancelScheduleEditSlot()
{
    if (m_scheduleMarkInfo.m_markId == "") {
        this->deleteLater();
    }
}

void ScheduleMark::deleteScheduleSlot()
{
    //删除日程相关
    qDebug() << "id" << m_scheduleMarkInfo.m_markId;
    emit DATABASE_INSTANCE.deleteDataSignal(m_scheduleMarkInfo);
    qDebug() << "开始时间:" << m_scheduleMarkInfo.m_markStartDate << "结束时间:" << m_scheduleMarkInfo.m_markEndDate
             << "信息:" << m_scheduleMarkInfo.m_descript;

    this->deleteLater();
}
