#include "cbasetablewidget.h"
#include <QDebug>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMouseEvent>

//控制是否对单元格的控件在表格上添加事件过滤
#define WIDGET_EVENT_FILTER

CBaseTableWidget::CBaseTableWidget(int rowNum, int columnNum, QList<CBaseTableItem *> items, bool rowInit, QWidget *parent):
    QTableWidget(rowNum,columnNum,parent)
{

    initTableProperty();//初始化表格属性
    addRows(rowNum, columnNum, items, rowInit);
    //信号与槽连接
    connect(this,SIGNAL(pressedSignal()),this,SLOT(tablePress()));
   /*    connect(this,SIGNAL(releasedSignal()),this,SLOT(tableRelease()));
    connect(this,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(itemChangedSlot(QTableWidgetItem *)));//连接时不加具体参数*/
    //初始化一组item选中的样式
//    pressStyle="::item:selected {"
//            "background-color: orange;"
//            "color: black;}";
//    releaseStyle="::item:selected {"
//            "background-color: orange;"
//            "color: black;}";
}

void CBaseTableWidget::addRows(int rowNum, int columnNum, QList<CBaseTableItem *> items, bool rowInit)
{
    int itemSize = items.size();
    if(rowInit)
    {
        //如果当前行列不足以存放items,则自动添加行
        while(rowNum*columnNum < itemSize)
        {
            rowNum++;
            this->setRowCount(rowNum);
        }
        for(int i=0;i<rowNum;i++)//按行初始化item
        {
            for(int j=0;j<columnNum&&(i*columnNum+j)<itemSize;j++)
            {
                CBaseTableItem * tmpItem= items.at(i*columnNum+j);//获取list中对应的item
                setItemContent(i,j,tmpItem);//在指定的行列设置item
            }
        }
    }
    else
    {
        //如果当前行列不足以存放items,则自动添加列
        while(rowNum*columnNum < itemSize)
        {
            columnNum++;
            this->setColumnCount(columnNum);
        }
        for(int i=0;i<columnNum;i++)//按列初始化item
        {
            for(int j=0;j<rowNum&&(i*rowNum+j)<itemSize;j++)
            {
                CBaseTableItem * tmpItem= items.at(i*rowNum+j);//获取list中对应的item
                setItemContent(j,i,tmpItem);//在指定的行列设置item
            }
        }
    }
}

CBaseTableWidget::CBaseTableWidget(QWidget *parent):
    QTableWidget(parent)
{
    initTableProperty();
    //信号与槽连接
    //点击单元格触发的信号与槽
    connect(this,SIGNAL(pressedSignal()),this,SLOT(tablePress()));
//    connect(this,SIGNAL(releasedSignal()),this,SLOT(tableRelease()));
//    connect(this,SIGNAL(itemChanged(QTableWidgetItem*)),this,SLOT(itemChangedSlot(QTableWidgetItem *)));//连接时不加具体参数*/
    //初始化一组item选中的样式
//    pressStyle="::item:selected {"
//            "background-color: orange;"
//            "color: black;}";//对鼠标不同行为初始化样式
//    releaseStyle="::item:selected {"
//            "background-color: orange;"
//            "color: black;}";
}

CBaseTableWidget::~CBaseTableWidget()
{

}

void CBaseTableWidget::insertRows(QList<CBaseTableItem *> items)
{
    rows = items.length() / this->columnCount();
//    this->insertRow(rowCount());
    this->setRowCount(rows);
    cols = this->columnCount();

    for(int i=0;i<rows;i++)//按行初始化item
    {
        this->setRowHeight(i, this->rowHeight(0));
        this->insertRow(rowCount()-1);
        for(int j=0;j<cols&&(i*cols+j)<items.length();j++)
        {
            CBaseTableItem * tmpItem= items.at(i*cols+j);//获取list中对应的item
            setItemContent(i,j,tmpItem);//在指定的行列设置item
        }
    }
    this->resize(cols * this->columnWidth(0), (rows) * this->rowHeight(0));
}

void CBaseTableWidget::initTableProperty()
{
    this->setObjectName("CBaseTableWidget");
//    this->setGridStyle(Qt::SolidLine);//设置网格线样式
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//关闭水平滚动条
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    this->setFocusPolicy(Qt::NoFocus);//设置无焦点
    this->setSelectionMode(QAbstractItemView::SingleSelection);//设置选中的模式
    this->setSelectionBehavior(QAbstractItemView::SelectRows);//设置整行选中
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);//设置表格不可编辑
    this->setAlternatingRowColors(false);//设置行颜色交替　需要设置样式表alternate-background-color选项
    this->setShowGrid(false);//设置不显示网格
    this->setItemDelegate(new CBasetableDelegate(this));//为表格设置委托类
    this->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);


}

void CBaseTableWidget::setItemContent(int row,int column,CBaseTableItem *item)
{
    if(item->itemContentType == CBaseTableItem::TEXT)//只有文本
    {
        this->removeCellWidget(row,column);//移除原有的控件,不然有控件的话无法设置文本
        //该函数内部会自动清理旧的QTableWidgetItem，这里直接new即可
        this->setItem(row,column,new QTableWidgetItem(item->text));
        this->item(row,column)->setTextAlignment(Qt::AlignCenter);
    }
//    else if(item->itemContentType == BaseTableItem::CHECKBOX_TEXT)//复选框加文本
//    {
//        this->removeCellWidget(row,column);//移除原有的控件,不然有控件的话无法设置文本
//        //注意通过该方式设置的复选框没有状态变化的信号发出，点击复选框并不能选择表项
//        QTableWidgetItem * checkBoxItem =new QTableWidgetItem(item->text);
//        checkBoxItem->setCheckState(Qt::Unchecked);//给item本身设置复选框
//        this->setItem(row,column,checkBoxItem);

//    }
    else if(item->itemContentType == CBaseTableItem::ICON_TEXT)//图标加文本
    {
        //移除原有的控件,不然有控件的话无法设置文本
        this->removeCellWidget(row,column);
        this->setItem(row,column,new QTableWidgetItem(QIcon(item->iconAddr),item->text));
        this->item(row,column)->setTextAlignment(Qt::AlignCenter);
    }
    else if(item->itemContentType == CBaseTableItem::WIDGET)//单元格添加一个控件
    {
        QWidget *widget = new QWidget(this);//添加一个控件也需要将控件放在一个有布局(栅格布局)的widget上加入
        //样式表具有子部件继承性和重设回到默认性，所以为了子部件不使用表格的背景图片，也为子部件设置空的背景图
        //widget->setStyleSheet("border-image: url();");//设置自缩放的背景图片样式为空，避免继承表格的背景图样式
        //widget->setAttribute(Qt::WA_TranslucentBackground);//设置透明
        //垂直布局无法对固定大小的部件居中操作
        QGridLayout *layout = new QGridLayout(widget);//因为默认的setCellWidget()不能设置控件的对齐方式,需要用上层的widget与布局调节
        layout->setAlignment(Qt::AlignLeft | Qt::AlignHCenter);
        layout->setContentsMargins(2,2,2,2);
#ifdef WIDGET_EVENT_FILTER
        item->widget1->setObjectName("widget1");//为控件设置objectname，为用findchild查找控件对象
        item->widget1->installEventFilter(this);//控件在表格上安装过滤器
#endif
        layout->addWidget(item->widget1,0,0,Qt::AlignHCenter);
        /*该函数会对旧的widget调用deletelater,因为在布局中的部件会被设置成布局的子部件，这样deletelator就
         *有可能清理掉item->widget1对象，产生野指针*/
        this->setCellWidget(row,column,widget);

    }
    else if(item->itemContentType == CBaseTableItem::WIDGET_WIDGET)//单元格添加两个控件
    {
        QWidget *widget = new QWidget(this);
        //widget->setStyleSheet("border-image: url();");//设置自缩放的背景图片样式为空，避免继承表格的背景图样式
        QHBoxLayout *layout= new QHBoxLayout(widget);//创建水平布局
        layout->setContentsMargins(2,0,2,0);
        layout->setSpacing(2);
#ifdef WIDGET_EVENT_FILTER
        item->widget1->setObjectName("widget1");//为控件设置objectname，为用findchild查找控件对象
        item->widget2->setObjectName("widget2");
        item->widget1->installEventFilter(this);//控件在表格上安装事件过滤器
        item->widget2->installEventFilter(this);
#endif
        layout->addWidget(item->widget1);//布局上添加控件
        layout->addWidget(item->widget2);
        /*该函数会对旧的widget调用deletelater,因为在布局中的部件会被设置成布局的子部件，这样deletelator就
         *有可能清理掉item->widget1和widget2对象，产生野指针*/
        this->setCellWidget(row,column,widget);
        this->item(row,column)->setTextAlignment(Qt::AlignCenter);
    }

}

void CBaseTableWidget::setItemValue(int row, int column, QString text)
{
    this->removeCellWidget(row,column);//移除原有的控件，不然有控件的话无法设置文本
    this->setItem(row,column,new QTableWidgetItem(text));
}

void CBaseTableWidget::setColWidthRowHeight(int columnWidth, int rowHeight)
{
    int rowNum = this->rowCount();//获取行数
    int colNum = this->columnCount();//获取列数
    for(int i=0;i<rowNum;i++)
    {
        this->setRowHeight(i,rowHeight);
    }
    for(int j=0;j<colNum;j++)
    {
        this->setColumnWidth(j,columnWidth);
    }
}


void CBaseTableWidget::setNormalStyle(QString bgColor, QString textColor, QString borderColor, QString borderStyle, QString borderWidth)
{
    this->setStyleSheet("::item {"
                        "background-color: "+bgColor+";"
                        "color: "+textColor+";"
                        "border-width:"+borderWidth+";"
                        "border-style:"+borderStyle+";"
                        "border-color:"+borderColor+";}"
                        );
}

void CBaseTableWidget::setPressStyle(QString bgColor,QString textColor,QString borderColor,QString borderStyle,QString borderWidth)
{
    pressStyle = "::item:selected {"
                        "background-color: "+bgColor+";"
                        "color: "+textColor+";"
                        "border-width:"+borderWidth+";"
                        "border-style:"+borderStyle+";"
                        "border-color:"+borderColor+";}";
}

void CBaseTableWidget::setPressStyle(QString style)
{
    pressStyle = style;
}

void CBaseTableWidget::setReleaseStyle(QString bgColor, QString textColor, QString borderColor, QString borderStyle, QString borderWidth)
{
    releaseStyle = "::item:selected{"
                        "background-color: "+bgColor+";"
                        "color: "+textColor+";"
                        "border-width:"+borderWidth+";"
                        "border-style:"+borderStyle+";"
                        "border-color:"+borderColor+";}";
}

void CBaseTableWidget::setReleaseStyle(QString style)
{
    releaseStyle = style;
}

void CBaseTableWidget::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        //在arm上，对鼠标按下事件处理中，设置样式使用了占空间较大的背景图片(800k左右)会比较费时，导致表格界面响应缓慢
        //this->setStyleSheet(pressStyle);//设置鼠标按下的样式
        emit pressedSignal();//发送按下的信号
        QTableWidget::mousePressEvent(e);//执行重写前的鼠标按下的处理操作
    }
}

void CBaseTableWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        //this->setStyleSheet(releaseStyle);//设置鼠标释放的样式
        emit releasedSignal();//发送释放的信号
        QTableWidget::mouseReleaseEvent(e);
    }
}

bool CBaseTableWidget::eventFilter(QObject *target, QEvent *event)
{
    if(event->type()==QEvent::MouseButtonRelease)//事件类型
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);//将事件强制转换
        if(mouseEvent->button() == Qt::LeftButton)
        {
            int rowNum=this->rowCount();
            int colNum=this->columnCount();
            for(int i=0;i<rowNum;i++)
            {
                for(int j=0;j<colNum;j++)
                {
                    if(this->getItemWidget(i,j)!=NULL)//对所有有控件的单元格进行判断
                    {
                        //判断目标是否为监听的对象
                        if(target==getItemWidget(i,j)||target==getItemWidget(i,j,2))
                        {
                            setSelectCell(i,j);//设置选中行
                            return QTableWidget::eventFilter(target,event);//返回父类的事件过滤器
                        }
                    }
                }
            }
        }
    }
    return QTableWidget::eventFilter(target,event);//返回父类的事件过滤器
}

void CBaseTableWidget::setSelectCell(int row, int column)
{
    this->clearSelection();//清除之前所有的选择
    //单元格选中
    if(this->selectionBehavior() == QAbstractItemView::SelectItems)
    {
        this->setCurrentCell(row,column,QItemSelectionModel::Select);
    }
    //整行选中
    else if(this->selectionBehavior() == QAbstractItemView::SelectRows)
    {
        this->setCurrentCell(row,column,QItemSelectionModel::Select|QItemSelectionModel::Rows);
    }
    //整列选中
    else if(this->selectionBehavior() == QAbstractItemView::SelectColumns)
    {
        this->setCurrentCell(row,column,QItemSelectionModel::Select|QItemSelectionModel::Columns);
    }
    //this->setStyleSheet(releaseStyle);
}

void CBaseTableWidget::setHeaderVisible(bool v)
{
    this->horizontalHeader()->setVisible(v);//水平表头
    this->verticalHeader()->setVisible(v);//垂直表头
}

void CBaseTableWidget::setHeaderLabels(QStringList &labels, bool isHorizontal, QFont font)
{
    if(isHorizontal)//水平
    {
        this->setHorizontalHeaderLabels(labels);
        //设置表头字体，其他属性可参考QHeaderView类的方法
        this->horizontalHeader()->setFont(font);
    }
    else//垂直
    {
        this->setVerticalHeaderLabels(labels);
        this->verticalHeader()->setFont(font);
    }
}

bool CBaseTableWidget::getItemCheckstate(int row, int column)
{
    QTableWidgetItem * checkBoxItem = this->item(row,column);
    if(checkBoxItem != NULL)
    {
        if(checkBoxItem->checkState()==Qt::Checked)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    //不存在QTableWidgetItem
    else
    {
        return false;
    }
}

QString CBaseTableWidget::getItemText(int row, int column)
{
    if(item(row,column)!=NULL)//如果单元格存放的是item，返回其显示的文本值
    {
        //qDebug()<<"当前单元格文本值："<<item(row,column)->text();
        return item(row,column)->text();
    }
    else
    {
        return NULL;
    }
}

QWidget *CBaseTableWidget::getItemWidget(int row, int column,int num)
{
    if(cellWidget(row,column)!=NULL)
    {
        //设置控件的objname
        QString childObjName = QString("widget%1").arg(num);
        QWidget *itemWidget=this->cellWidget(row,column)->findChild<QWidget *>(childObjName);
        if(itemWidget==0)
        {
             return NULL;
        }
        else
        {
            /*返回的是QWidget类型指针，而具体存放的一般是其子类对象所以使用时需要用static_cast
             *强制转换为子类对象指针才能操作子类的相关功能，但这种强制转换不安全(将父类赋值给子类)
             *例:static_cast <BaseButton *>(baseTableWidget->getItemWidget(2,0));
             */
            return itemWidget;
        }
    }
    else
    {
        return NULL;
    }
}

void CBaseTableWidget::tablePress()
{
    //qDebug()<<"鼠标按下";
}

void CBaseTableWidget::tableRelease()
{
    //qDebug()<<"鼠标释放";
}

//void CBaseTableWidget::paintEvent(QPaintEvent *event)
//{
//    QPainter painter(this);
//    int width = cols * colWidth;
//    int heigh = rows * rowHeigh;
//    QRect rect = QRect(0, 0, width, heigh);
//    painter.drawRect(rect);
//}


