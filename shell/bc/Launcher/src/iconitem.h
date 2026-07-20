#ifndef ICONITEM_H
#define ICONITEM_H

#include <QQuickItem>
#include <QPixmap>
#include <QPointer>

class IconItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QVariant source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(int paintedWidth READ paintedWidth NOTIFY paintedSizeChanged)
    Q_PROPERTY(int paintedHeight READ paintedHeight NOTIFY paintedSizeChanged)

public:
    explicit IconItem(QQuickItem *parent = nullptr);

    void setSource(const QVariant &source);
    QVariant source() const;

    int paintedWidth() const;
    int paintedHeight() const;

    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *updatePaintNodeData) override;
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;

    void componentComplete() override;
    void updatePolish() override;

    Q_INVOKABLE void refresh();

private:
    void loadPixmap();

signals:
    void sourceChanged();
    void paintedSizeChanged();

private:
    QVariant m_source;
    QPixmap m_iconPixmap;
};

#endif // ICONITEM_H
