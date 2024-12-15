#ifndef QMLWEBVIEWWIDGET_H
#define QMLWEBVIEWWIDGET_H

#include "types.h"

#include <QQuickPaintedItem>
#include <QWidget>
#include <DWidget>

DWIDGET_USE_NAMESPACE

//#define USE_INNER
class TimeLineView;
class QmlCustomInternalWidget : public QWidget
{
    friend class QmlWidget;
    Q_OBJECT
private:
    explicit QmlCustomInternalWidget(QQuickPaintedItem *parent);

public slots:
protected:
    QPaintEngine * paintEngine() const ;
    void paintEvent(QPaintEvent * event) ;

private:
    QQuickPaintedItem *m_qquickContainer;
    TimeLineView *m_timeView { nullptr };
};


class QmlWidget : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(int viewType READ viewType WRITE setViewType NOTIFY viewTypeChanged)
    Q_PROPERTY(int filterType READ filterType WRITE setFilterType NOTIFY filterTypeChanged)
public:
    explicit QmlWidget(QQuickItem *parent = nullptr);
    void paint(QPainter *painter);

    void setViewType(int viewType);
    int viewType();

    void setFilterType(int filterType);
    int filterType();

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void navigateToMonth(const QString& month);
    Q_INVOKABLE QVariantList allUrls();
    Q_INVOKABLE void unSelectAll();
    Q_INVOKABLE void selectUrls(const QStringList& urls);

public slots:
    void setFocus(bool arg);

Q_SIGNALS:
    void viewTypeChanged();
    void filterTypeChanged();
private:
    void initWidget();
private:
#ifdef USE_INNER
    QmlCustomInternalWidget *m_view;
#else
    DWidget *m_view = nullptr;
#endif

    QWidget *m_lastHoveredWidget = nullptr;
protected:
    virtual void geometryChanged(const QRectF & newGeometry,
                                 const QRectF & oldGeometry);
    bool event(QEvent * e) ;
protected slots:
    void updateGeometry();

private:
    int m_viewType = Types::WidgetViewUnknown; //0:日视图 1:已导入视图
    int m_filterType = Types::All; // 0:所有 1:照片 2:视频
    bool m_disableHoverEvent {false}; // 日视图和已导入视图在刷新时，鼠标悬停事件路由不能执行，否则会访问之前已析构的QWidget控件，导致程序崩溃
};

#endif // QMLWEBVIEWWIDGET_H
