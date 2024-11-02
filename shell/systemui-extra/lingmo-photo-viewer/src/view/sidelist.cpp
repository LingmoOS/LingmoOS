#include "sidelist.h"
#include "global/interactiveqml.h"
#include "global/variable.h"
#include "controller/interaction.h"
#include "sizedate.h"
SideList::SideList(QWidget *parent) : kdk::KListView(parent)
{
    QPalette pal;
    //设置背景色
    pal.setColor(QPalette::Base, QColor(Qt::transparent));
    this->setPalette(pal);
}
void SideList::mouseReleaseEvent(QMouseEvent *e)
{
    Variable::g_nextOrPreviousTiff = false;
    //处理滑动，或拖出相册时，相册选中态和主界面显示的图片保持一致
    Interaction::getInstance()->changeImageFromClick(this->currentIndex());
    this->setCurrentIndex(this->currentIndex());
    operayteMode = OperayteMode::ChangeImage;
    InteractiveQml::getInstance()->setOperateWay(int(operayteMode));
    QWidget::mouseReleaseEvent(e);
}
