#ifndef BASESEARCHEDIT_H
#define BASESEARCHEDIT_H

#include <QWidget>
#include <QEvent>
#include <QLabel>
#include <QSlider>
#include <QLineEdit>
#include <QHBoxLayout>

/* 标题栏搜索 */
class BaseSearchEdit : public QWidget
{
    Q_OBJECT
public:
    /* 图标路径, 提示信息， 间距， 宽度， 高度 */
    BaseSearchEdit(QString iconPath = "edit-find-symbolic", QString value = tr("Search"));
    ~BaseSearchEdit();
    void init(void);
    bool eventFilter(QObject *watch,QEvent *e) override;
    void changeFontSize();   /* 改变字体大小 */
    void searchFocusOut();   /* 输入框失去焦点 */

private:

private:
    QString m_iconPath;
    QString m_value;
    int m_width;
    int m_height;
    int m_space;

    QLabel *m_iconLab = nullptr;   /* 放置图标 */
    QLineEdit *m_searchFontEdit = nullptr;
    QHBoxLayout *m_hlayout = nullptr;

signals:
    void sigSearchFont(QString searchText);
public slots:
    void slotSearchFont(QString searchText);   /* 搜索字体 */
};


#endif // BASESEARCHEDIT_H
