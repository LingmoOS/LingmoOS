#ifndef CBASETABLEDELEGATE_H
#define CBASETABLEDELEGATE_H

#include <QObject>
#include <QApplication>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QSpinBox>

//使用自定义代理渲染view item
#define USE_CUSTOM_PAINT
//使用自定义代理编辑器
#define USE_CUSTOM_EDITOR

class CBasetableDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    QPen pen;

    CBasetableDelegate(QObject *parent=0);
    ~CBasetableDelegate();


#ifdef USE_CUSTOM_EDITOR
    /*提供自定义的输入功能,需要表格设置为可编辑状态,且表项非cellWidget*/
    //创建自定义的编辑部件，双击表项就会展示这里创建的部件
    QWidget *createEditor(QWidget * parent, const QStyleOptionViewItem & option,
                             const QModelIndex & index) const;
    //从model中取数据填到代理编辑器中
    void setEditorData(QWidget *editor, const QModelIndex &index) const ;
    //将编辑器设置的数据写入到model中
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const ;
    //设置编辑器的显示位置以及样式
    void updateEditorGeometry(QWidget *editor,const QStyleOptionViewItem &option,
                              const QModelIndex &index) const ;
    //判断表项是否需要自定义代理编辑器
    bool isNeedCustomEdit(const QModelIndex &index) const;
#endif

#ifdef USE_CUSTOM_PAINT
    /*view 表项外观渲染*/
    //绘制渲染view item(表项)外观,可以调用QStyle中的函数绘制，也可以使用QPainter的函数绘制
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex &index) const ;
    //判断表项是否需要自定义代理渲染绘制
    bool isNeedCustomPainter(const QModelIndex &index) const;
#endif


signals:

public slots:

private:

};

#endif // CBASETABLEDELEGATE_H
