#ifndef BLANKPAGE_H
#define BLANKPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDragEnterEvent>
#include <QDropEvent>

#include "blankpage.h"

class BlankPage : public QWidget
{
    Q_OBJECT

public:
    BlankPage(QWidget *parent = nullptr);
    ~BlankPage();

    void setWidgetUi();      /* 初始化组件 */
    void initLayout();       /* 设置组件样式 */
    void initGsetting();
    void changeTheme();
    void changeSearchText();       /* 更改文案 */
    void changeBlankText();       /* 更改文案 */

protected:
    void paintEvent(QPaintEvent *event) override;           /* 绘制事件 */

private:
    QLabel *m_blankLabel;
    QLabel *m_blankTextLabel;
    QVBoxLayout *m_vLayout = nullptr;
    QHBoxLayout *m_hLayout = nullptr;

};
#endif // FONTWIDGET_H
