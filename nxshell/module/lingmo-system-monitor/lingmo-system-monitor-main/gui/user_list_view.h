#ifndef USER_LIST_VIEW_H
#define USER_LIST_VIEW_H

#include <DListView>
#include <DMenu>
DWIDGET_USE_NAMESPACE
class UserListView : public DListView
{
    Q_OBJECT
public:
    explicit UserListView(QWidget *parent = nullptr);
    void resetStatus(const QModelIndex &index);


Q_SIGNALS:
    void signalRightButtonClicked(const QPoint &p);

protected:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override;
    void keyPressEvent(QKeyEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

private:
    int m_currentIndex;
};

#endif // USER_LIST_VIEW_H
