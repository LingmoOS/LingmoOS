#include "cbasetableitem.h"

CBaseTableItem::CBaseTableItem(ItemContentType itemContentType,QString text,QString iconAddr,QWidget *widget1,QWidget *widget2)
{
    this->text=text;//由参数对item对象传递数据
    this->iconAddr=iconAddr;
    this->widget1=widget1;
    this->widget2=widget2;
    this->itemContentType=itemContentType;

}

void CBaseTableItem::setItemType(CBaseTableItem::ItemContentType itemContentType){this->itemContentType = itemContentType;}

void CBaseTableItem::setItemText(QString text){this->text = text;}

void CBaseTableItem::setItemIcon(QString iconAddr){this->iconAddr = iconAddr;}

void CBaseTableItem::setItemWidget1(QWidget *widget1){this->widget1 = widget1;}

void CBaseTableItem::setItemWidget2(QWidget *widget2){this->widget2 = widget2;}



