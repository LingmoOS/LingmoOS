#ifndef KDRAGWIDGET_H
#define KDRAGWIDGET_H

#include "gui_g.h"
#include "kpushbutton.h"
#include <QLabel>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>

namespace kdk {

/**
 * @defgroup DragModule
 */
class KDragWidgetPrivate;

/**
 * @brief 拖动文件或文件夹，可识别并获取文件或文件夹路径
 * @since 2.3
 */
class GUI_EXPORT KDragWidget :public QWidget
{
    Q_OBJECT

public:
    KDragWidget(QWidget*parent=nullptr);

    /**
     * @brief iconButton
     * @return 返回iconbutton
     */
    KPushButton *iconButton();

    /**
     * @brief textLabel
     * @return 返回textlabel
     */
    QLabel *textLabel();

    /**
     * @brief fileDialog
     * @return 返回QFiledialog
     */
    QFileDialog *fileDialog();

    /**
     * @brief 将文件对话框中使用的过滤器设置为给定的过滤器。
     * @param filter
     */
    void setNameFilter(const QString &filter);

    /**
     * @brief 设置文件对话框中使用的过滤器。
     * @param filters
     */
    void setNameFilters(const QStringList &filters);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    void dragEnterEvent(QDragEnterEvent* event);
    void dragLeaveEvent(QDragLeaveEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent* event);

Q_SIGNALS:
    void getPath(QString);

private:
    Q_DECLARE_PRIVATE(KDragWidget)
    KDragWidgetPrivate* const d_ptr;
};

}
#endif // KDRAGWIDGET_H
