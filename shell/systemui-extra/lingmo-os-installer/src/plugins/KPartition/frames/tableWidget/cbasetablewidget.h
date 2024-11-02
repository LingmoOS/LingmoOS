#ifndef CBASETABLEWIDGET_H
#define CBASETABLEWIDGET_H

#include "cbasetableitem.h"
#include "cbasetabledelegate.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QEvent>
#include <QList>

class CBaseTableItem;//类的前置声明，下面要使用

class CBaseTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    /*explicit 声明为显式构造 不能在隐式转换中使用，规定只传一个参数的构造函数会定义
     *一个隐式转换，隐式转换常带来逻辑错误，所以尽量声明显式*/
    explicit CBaseTableWidget(QWidget *parent=0);
    //构造指定行数和列数的table,并按顺序初始化单元格
    explicit CBaseTableWidget(int rowNum,int columnNum,QList <CBaseTableItem *> items,bool rowInit=true,QWidget *parent = 0);

    ~CBaseTableWidget();

    void initTableProperty();//初始化一些QTableWidget的属性

    void setItemContent(int row,int column,CBaseTableItem *item);//设置Item内容的函数
    void setItemValue(int row,int column,QString text);//设置item文本
    void setColWidthRowHeight(int columnWidth,int rowHeight);//设置表格的列宽和行高，所有行列都一致

    void setNormalStyle(QString bgColor,QString textColor,QString borderColor="",QString borderStyle="",QString borderWidth="");//设置正常的样式
    void setPressStyle(QString bgColor,QString textColor,QString borderColor="orange",QString borderStyle="solid",QString borderWidth="2px 0px 2px 0px");//设置鼠标按下的样式
    void setPressStyle(QString style);//设置按下的样式
    void setReleaseStyle(QString bgColor, QString textColor, QString borderColor="blue", QString borderStyle="solid", QString borderWidth="2px 0px 2px 0px");//设置鼠标释放后的样式
    void setReleaseStyle(QString style);//设置释放的样式

    void setSelectCell(int row=0,int column=0);//根据表格设置的selectionBehavior，选择某行/某列/某单元格
    void setHeaderVisible(bool v);//设置表格头部是否显示
    void setHeaderLabels(QStringList &labels,bool isHorizontal,QFont font=QFont());//设置表头文本显示
    bool getItemCheckstate(int row,int column);//获取某行列的复选框状态

    QString getItemText(int row,int column);//获取选中行指定列值
    QWidget *getItemWidget(int row,int column,int num=1);//获取选中行指定列控件
    void insertRows(QList<CBaseTableItem *> items);
    void addRows(int rowNum,int columnNum,QList <CBaseTableItem *> items,bool rowInit=true);
protected:
    virtual void mousePressEvent(QMouseEvent *e);//鼠标按下事件处理函数
    virtual void mouseReleaseEvent(QMouseEvent *e);//鼠标释放事件处理函数
    bool eventFilter(QObject *, QEvent *);//事件过滤器处理函数
//    void paintEvent(QPaintEvent* event) override;
signals:
    void pressedSignal();//鼠标按下发出的信号
    void releasedSignal();

public slots:

    void tablePress();//鼠标按下事件发射信号的响应槽
    void tableRelease();//鼠标释放事件发射信号的响应槽
//    void itemChangedSlot(QTableWidgetItem *item);//changed 信号的响应槽

private:
    QString pressStyle;//存放设置的鼠标按下的样式
    QString releaseStyle;//存放设置的鼠标释放的样式
public:
    int rows;
    int cols;
    int rowHeigh;
    int colWidth;

};

#endif // CBASETABLEWIDGET_H
