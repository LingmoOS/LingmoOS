#ifndef THEME_PAGE_H
#define THEME_PAGE_H

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QProcess>
#include "ripple_button.h"

class ThemePage : public QWidget
{
    Q_OBJECT

public:
    explicit ThemePage(QWidget *parent = nullptr);

private:
    QFrame *mainCard;
    QLabel *titleLabel;
    QFrame *lightThemeCard;
    QFrame *darkThemeCard;
    RippleButton *nextButton;
    QString selectedTheme;
    QProcess *process;
    
    void setupUI();
    QGraphicsEffect* createShadowEffect();
    QFrame* createThemeCard(const QString &title, const QString &imagePath, bool isLight);
    bool eventFilter(QObject *watched, QEvent *event) override;
    void updateThemeSelection(bool isLight);
    void applyTheme(bool isLight);
    void executeThemeCommands(bool isLight);

signals:
    void nextClicked();
    void themeSelected(bool isLight);
};

#endif // THEME_PAGE_H 