#include "myscrollarea.h"

myScrollArea::myScrollArea(QWidget *parent) : QScrollArea(parent)
{
    // QScrollArea本身主题有背景，避免干扰需要设置全透明，不使用主题的背景颜色,只有1种恰当方式, rgba(0,0,0,0)
    this->setStyleSheet("background-color: rgba(0, 0, 0, 0);");

    /** setStyleBar组件设置样式技巧：通过整体设置背景、大小等参数，之后可以通过::来指定控件中其他小部件， ：可以设置点击、垂直方向等用户手动操作的状态参数;
     * width 越小，border-radius也应该越小，否则设置不生效;
     * 通过margin-left和margin-right来控制滑块大小。
     */
    this->verticalScrollBar()->setStyleSheet("QScrollBar:vertical{width:16px; border-radius:4px;}" // 可设置可区分背景颜色进行调节，整个滑动条控件的宽度width = 滑块宽度 + margin-left + margin-right
                                             "QScrollBar::handle:vertical{background:rgba(0,0,0, 10%); width: 4px;border-radius: 2px; margin-left:6px;margin-right:6px;}" // 滑动条滑块handle两端变椭圆,和滑轨之间的间隙
                                             "QScrollBar::handle:vertical:hover{background:rgba(0,0,0, 15%); width: 8px; border-radius:4px;margin-left:4px;margin-right:4px;}" // 鼠标放到滑块上颜色加深
                                             "QScrollBar::sub-line:vertical{border: none; background: none; color:none;}" // 设置上箭头不可见
                                             "QScrollBar::add-line:vertical{border: none; background: none; color:none;}" // 设置下箭头不可见
                                             "QScrollBar::sub-page:vertical{background:none;}" // 滑块还没经过的滑轨区域的颜色
                                             "QScrollBar::add-page:vertical{background:none;}" // 滑块已经经过的滑轨区域的颜色
                                             "QScrollBar::up-arrow:vertical{background:none;}" // 上箭头图标
                                             "QScrollBar::down-arrow:vertical{background:none;}" // 下箭头图标
                                             );


}

myScrollArea::~myScrollArea()
{

}
