#ifndef CBASETABLEITEM_H
#define CBASETABLEITEM_H
#include <QString>
#include <QWidget>

class CBaseTableItem
{
public:
    enum ItemContentType{
                    TEXT,  //只设置文本
                    CHECKBOX_TEXT,   //带复选框的文本
                    ICON_TEXT,   //带图标的文本
                    WIDGET,     //设置一个控件
                    WIDGET_WIDGET    //设置两个控件
    };
    CBaseTableItem(ItemContentType itemContentType)
    {
        this->itemContentType=itemContentType;
        this->widget1=NULL;
        this->widget2=NULL;
    }
    CBaseTableItem(ItemContentType itemContentType,QString text,QString iconAddr="",QWidget *widget1=NULL,QWidget *widget2=NULL);

    void setItemType(ItemContentType itemContentType);
    void setItemText(QString text);
    void setItemIcon(QString iconAddr);
    void setItemWidget1(QWidget *widget1);
    void setItemWidget2(QWidget *widget2);


    QString text;//存放单元格显示文本
    QString iconAddr;//存放图标路径
    QWidget *widget1;//存放部件
    QWidget *widget2;
    ItemContentType itemContentType;//存放item类型

};

#endif // CBASETABLEITEM_H
