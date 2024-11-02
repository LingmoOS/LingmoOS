#ifndef SERVICEWIDGET_H
#define SERVICEWIDGET_H

#include "servicetableview.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGSettings>
#include <QSettings>

class ServiceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ServiceWidget(QSettings *svcSettings = nullptr, QWidget *parent = nullptr);
    virtual ~ServiceWidget();

    virtual void onWndClose();

public slots:

signals:
    void searchSignal(QString);
    void switchOnWidget();

protected:
    void paintEvent(QPaintEvent *) override;

private:
    void initUI();
    void initConnections();
    void initThemeMode();
    void onThemeFontChanged(float fFontSize);

private:
    QVBoxLayout *m_mainLayout = nullptr;
    QVBoxLayout *m_svcTableLayout = nullptr;

    QGSettings *m_qtSettings = nullptr;
    QSettings *m_svcSettings = nullptr;

    ServiceTableView *m_svcTabView = nullptr;
};

#endif // SERVICEWIDGET_H
