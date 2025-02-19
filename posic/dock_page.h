#ifndef DOCK_PAGE_H
#define DOCK_PAGE_H

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QProcess>
#include "ripple_button.h"

class DockPage : public QWidget
{
    Q_OBJECT

public:
    explicit DockPage(QWidget *parent = nullptr);

public slots:
    void applyTheme(bool isLight);

private:
    QFrame *mainCard;
    QLabel *titleLabel;
    QFrame *fullDockCard;
    QFrame *centerDockCard;
    RippleButton *nextButton;
    RippleButton *backButton;
    QProcess *process;
    bool isFullSelected = false;
    
    void setupUI();
    QGraphicsEffect* createShadowEffect();
    QFrame* createDockCard(const QString &title, const QString &imagePath, bool isFull);
    bool eventFilter(QObject *watched, QEvent *event) override;
    void updateDockSelection(bool isFull);
    void applyDockStyle(bool isFull);
    void executeDockCommands(bool isFull);

signals:
    void nextClicked();
    void backClicked();
    void dockStyleSelected(bool isFull);
    void finished();
};

#endif // DOCK_PAGE_H 