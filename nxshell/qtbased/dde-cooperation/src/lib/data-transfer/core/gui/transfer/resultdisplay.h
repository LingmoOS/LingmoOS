#ifndef RESULTDISPLAYWIDGET_H
#define RESULTDISPLAYWIDGET_H

#include <QFrame>
#include "../type_defines.h"
class QTextBrowser;
class QLabel;
class ResultWindow : public ProcessDetailsWindow
{
    Q_OBJECT
public:
    ResultWindow(QFrame *parent = nullptr);
    ~ResultWindow() override;
    void updateContent(const QString &name, const QString &type, bool success);
    void changeTheme(int theme);

private:
    void init();
};

class ResultDisplayWidget : public QFrame
{
    Q_OBJECT

public:
    ResultDisplayWidget(QWidget *parent = nullptr);
    ~ResultDisplayWidget();
public slots:
    void themeChanged(int theme);
    void addResult(QString name, bool success, QString reason);
    void clear();
    void setStatus(bool success);

private:
    void initUI();
    void nextPage();

private:
    ResultWindow *resultWindow { nullptr };
    QString processText;
    QLabel *iconLabel { nullptr };
    QLabel *titileLabel { nullptr };
    QLabel *tiptextlabel { nullptr };
};

#endif
