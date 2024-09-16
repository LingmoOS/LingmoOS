#ifndef QUICKPANELWIDGET_H
#define QUICKPANELWIDGET_H

#include "commoniconbutton.h"
#include <DLabel>
#include <QWidget>
#include <QTime>
#include <QTimer>
DWIDGET_USE_NAMESPACE

class QuickPanelWidget : public QWidget
{
    Q_OBJECT

public:
    enum WidgetState {
        WS_NORMAL,
        WS_ACTIVE,
    };

    enum FuctionType {
        SHOT,
        RECORD,
    };
public:
    QuickPanelWidget(QWidget *parent = nullptr);
    virtual ~QuickPanelWidget();

    void setIcon(const QIcon &icon);
    void setDescription(const QString &description);
    void setWidgetState(WidgetState state);

    /**
     * @brief changeType 切换截图或录屏模式
     * @param type
     */
    void changeType(int type);

    /**
     * @brief start:开始计时
     */
    void start();

    /**
     * @brief stop:停止计时
     */
    void stop();
    /**
     * @brief pause:暂停
     */
    void pause();
Q_SIGNALS:
    void clicked();

protected slots:
    void onTimeout();
    void refreshIcon();
protected:
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void initUI();

private:
    CommonIconButton *m_icon;
    DLabel *m_description;

    QTimer *m_timer;
    QTime m_baseTime;
    QString m_showTimeStr;
    int m_type;
};

#endif // QUICKPANELWIDGET_H
