#ifndef FUNCTIONWID_H
#define FUNCTIONWID_H

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListView>

#include "basepopuptitle.h"
#include "basepreviewarea.h"
#include "globalsizedata.h"

#include "knavigationbar.h"
using namespace kdk;

class FunctionWid : public QWidget
{
    Q_OBJECT
public:
    FunctionWid(QWidget *parent = nullptr);
    ~FunctionWid();

    void setWidgetUi();      /* 初始化组件 */
    void initLayout();       /* 初始化布局 */
    void setSlotConnet();    /* 关联信号与槽 */
    void setWidgetStyle();   /* 设置尺寸 */ 

private:
    BasePopupTitle *m_leftTitle = nullptr;            /* 标题栏 */

    KNavigationBar *m_funBar;

    QVBoxLayout *m_vLayout = nullptr;
    QHBoxLayout *m_hLayout = nullptr;
    QVBoxLayout *m_mainVLayout = nullptr;

    FontType m_fontType;

signals:
    void sigChangeFontType(FontType type);   /* 切换字体列表类型 */

private slots:
    void slotNaviClicked(const QModelIndex &index);   /* 切换字体列表 */

    void slotChangePCMode();
    void slotChangeFaltMode();
};

#endif // FUNCTIONWID_H
