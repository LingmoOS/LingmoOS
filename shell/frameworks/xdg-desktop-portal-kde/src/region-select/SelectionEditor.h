#pragma once

#include <QKeyEvent>
#include <QMouseEvent>
#include <QObject>
#include <QQuickItem>

class QQuickView;

class SelectionEditorPrivate;

class SelectionEditor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QRect rect READ rect NOTIFY rectChanged FINAL)
    Q_PROPERTY(bool isDragging READ isDragging NOTIFY isDraggingChanged FINAL)

public:
    explicit SelectionEditor(QObject *parent = nullptr);
    ~SelectionEditor() override;

    bool isDragging() const;
    QRect rect() const;

    Q_SCRIPTABLE void dragStart(const QString &screenName, int x, int y);
    Q_SCRIPTABLE void setMousePosition(const QString &screenName, int x, int y);
    Q_SCRIPTABLE void dragRelease(const QString &screenName, int x, int y);
    Q_SCRIPTABLE void dragReset();
    Q_SCRIPTABLE void reject();

    bool exec();

Q_SIGNALS:
    void rectChanged();
    void emptyChanged();
    void isDraggingChanged();

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event);

    void showViews();

private:
    void accept();

    SelectionEditorPrivate *d;
    QQmlEngine *m_engine;
    QList<QQuickView *> m_views;
    QEventLoop m_execLoop;
};