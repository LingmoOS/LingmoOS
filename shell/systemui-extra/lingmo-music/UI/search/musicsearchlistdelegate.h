#include <QScopedPointer>
#include <QStyledItemDelegate>
#include <QListView>

class MusicSearchListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit MusicSearchListDelegate(QWidget *parent = Q_NULLPTR);
    ~MusicSearchListDelegate() override;

Q_SIGNALS:
    void SearchClear() const;

protected:
    virtual void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override;

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
private:
    QPixmap m_shadowImg;
};
