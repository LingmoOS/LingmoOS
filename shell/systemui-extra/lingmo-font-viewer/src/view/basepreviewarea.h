#ifndef BASEWIDGET_H
#define BASEWIDGET_H

#include <QWidget>
#include <QEvent>
#include <QLabel>
#include <QSlider>
#include <QLineEdit>
#include <QHBoxLayout>

#include "globalsizedata.h"

/* 预览区域 */
class BasePreviewArea : public QWidget
{
    Q_OBJECT
public:
    /* 默认显示字体大小 ，宽度 ，高度 ，间距 */
    BasePreviewArea(QString value = "24", int width = 620, int height = 50, int space = 16);
    ~BasePreviewArea();

    void init(void);
    void previewFocusOut();   /* 输入框失去焦点 */
    bool eventFilter(QObject* watched, QEvent* event) override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_value;
    int m_width;
    int m_height;
    int m_size;
    int m_space;

    QLineEdit *m_previewEdit = nullptr;    /* 预览消息编辑框 */
    QSlider *m_fontSizeSlider = nullptr;   /* 横向进度条 */
    QLabel *m_valueLab = nullptr;          /* 显示当前字体大小 */
    QHBoxLayout *m_hlayout = nullptr;

signals:
    void sigFontSize(int size);
    void sigPreviewValue(QString text);

private slots:
    void slotChangSize(int size);          /* 字号发生改变 */
    void slotPreviewValue(QString text);   /* 预览信息改变 */

public slots:
    void slotChangFont(QString fontName);  /* 改变字体 */
    void slotChangeFontName();             /* 改变字体名称 */
    void slotChangeFaltSize();
    void slotChangePCSize();


};

#endif // BASEWIDGET_H
