#include "cbasetabledelegate.h"
#include <QDebug>

CBasetableDelegate::CBasetableDelegate(QObject *parent):
    QStyledItemDelegate(parent)
{

}

CBasetableDelegate::~CBasetableDelegate()
{

}
#ifdef USE_CUSTOM_EDITOR

QWidget *CBasetableDelegate::createEditor(QWidget *parent,
                                         const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    qDebug()<<"create editor "<<index.row()<<index.column();
    //可以只针对某个表项(或者某行某列)自定义代理编辑器
    if(isNeedCustomEdit(index))
    {
        QSpinBox *editor = new QSpinBox(parent);
        editor->setFrame(false);
        editor->setMinimum(0);
        editor->setMaximum(100);
        return editor;
    }
    //其他情况使用默认的代理编辑器
    else
    {
         return QStyledItemDelegate::createEditor(parent, option, index);
    }
}

void CBasetableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    qDebug()<<"set editor "<<index.row()<<index.column();
    /*因为该函数与createEditor等是一组不可分割的函数，所以需要使用相同的判断条件同步
     *使用自定义的代理编辑器或者默认的代理编辑器。
     */
    if(isNeedCustomEdit(index))
    {
        //将edit转换成上面创建的类型
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        //获取model中某个index索引下的对应角色的值
        int value = index.model()->data(index, Qt::EditRole).toInt();
        //设置编辑器显示的值
        spinBox->setValue(value);
    }
    //设置数据到默认的代理编辑器
    else
    {
        QStyledItemDelegate::setEditorData(editor,index);
    }

}

void CBasetableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                     const QModelIndex &index) const
{
    qDebug()<<"set model "<<index.row()<<index.column();
    /*因为该函数与createEditor等是一组不可分割的函数，所以需要使用相同的判断条件同步
     *使用自定义的代理编辑器或者默认的代理编辑器。
     */
    if(isNeedCustomEdit(index))
    {
        //将edit转换成上面创建的类型
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        //获取编辑器的值
        int value = spinBox->value();
        //将数据写入item model
        model->setData(index, value, Qt::EditRole);
        /*数据写入item model后,编辑结束,稍后清理编辑器内存.
         *也可以发送closeEditor()信号让view在不需要时自动清理
         */
        spinBox->deleteLater();
    }
    //从默认的代理编辑器取数据写入model
    else
    {
        QStyledItemDelegate::setModelData(editor,model,index);
    }
}

void CBasetableDelegate::updateEditorGeometry(QWidget *editor,
                                             const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //qDebug()<<"updateEditorGeometry "<<index.row()<<index.column();
    /*因为该函数与createEditor等是一组不可分割的函数，所以需要使用相同的判断条件同步
     *使用自定义的代理编辑器或者默认的代理编辑器。
     */
    if(isNeedCustomEdit(index))
    {
        //设置编辑器的大小和位置
        editor->setGeometry(option.rect);
        editor->setStyleSheet("border-width:2px 0px 2px 0px;"
                              "border-style:solid;"
                              "border-color:blue;");
    }
    //更新默认代理编辑器位置
    else
    {
        QStyledItemDelegate::updateEditorGeometry(editor,option,index);
    }
}

bool CBasetableDelegate::isNeedCustomEdit(const QModelIndex &index) const
{
    if(index.row()==0 && index.column()==0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
#endif
#ifdef USE_CUSTOM_PAINT

void CBasetableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
//    QPen oldPen = painter->pen();
//    painter->setPen(QPen(Qt::white, 1, Qt::SolidLine, Qt::RoundCap));
//    //painter->drawLine(option.rect.topRight(), option.rect.bottomRight());
//    painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());
//    painter->setPen(oldPen);


    //qDebug()<<"paint "<<index.row()<<index.column();
//    if(isNeedCustomPainter(index))
//    {
//        /*使用QStyle的函数绘制格式化字符串*/
        /*int num = index.model()->data(index, Qt::DisplayRole).toInt();
        //将item数据转换成带0x的十六进制文本
        QString hexText = QString("0x%1").arg(num,0,16);
        //获取项风格设置  const类型不能直接修改,拷贝一个副本修改
        QStyleOptionViewItem myOption = option;
        myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
        //绘制文本　如果继承自QItemDelegate类，则可以直接用drawDisplay()
        qApp->style()->drawItemText( painter, myOption.rect , myOption.displayAlignment,
                                     QApplication::palette(), true,hexText );*/

        /*使用QPainter绘制方框*/
        /*painter->setPen(QPen(QBrush(Qt::blue), 4));
        painter->drawRect(option.rect);*/


        /*为选中行的两边画线*/
//        if(option.state & QStyle::State_Selected)
//        {
//            painter->setPen(QPen(QBrush(Qt::blue), 2));
//            QPoint p1,p2;
//            if(index.column()==0)
//            {
//                p1= option.rect.topLeft();
//                p2 = option.rect.bottomLeft();
//                painter->drawLine(p1,p2);
//            }
//            else if(index.column()==1)
//            {
//                p1= option.rect.topRight();
//                p2 = option.rect.bottomRight();
//                painter->drawLine(p1,p2);
//            }
//        }
//    }
    //使用默认的代理绘制item
//    else
//    {
//        QStyledItemDelegate::paint(painter,option,index);
//    }
}

bool CBasetableDelegate::isNeedCustomPainter(const QModelIndex &index) const
{
    if(index.column()==0 || index.column()==1)
    {
        return true;
    }
    else
    {
        return false;
    }
}
#endif

