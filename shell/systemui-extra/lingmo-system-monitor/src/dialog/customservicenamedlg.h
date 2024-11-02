#ifndef CUSTOMSERVICENAMEDLG_H
#define CUSTOMSERVICENAMEDLG_H

#include <QDialog>
#include <QLineEdit>
#include <QGSettings>

#include "../controls/mydialog.h"

class QVBoxLayout;

class CustomServiceNameDlg : public MyDialog
{
    Q_OBJECT
public:
    explicit CustomServiceNameDlg(const QString &title, const QString &message, QWidget *parent = 0);
    ~CustomServiceNameDlg();

    /**
     * @brief getCustomServiceName 获取用户输入的子服务名称
     * @return 子服务名称
     */
    QString getCustomServiceName();

    /**
     * @brief setCustomServiceName 设置输入的子服务名称
     * @param strSvcName 子服务名称
     */
    void setCustomServiceName(QString strSvcName);

    /**
     * @brief getResult 获取确认结果
     * @return 确认结果
     */
    int getResult();

    /**
     * @brief setResult 设置确认结果
     * @param nRet 确认结果
     */
    void setResult(int nRet);

public slots:
    /**
     * @brief onButtonClicked 按钮点击
     * @param index 按钮下标
     */
    void onButtonClicked(int index, const QString &);

protected:
    /**
     * @brief showEvent 显示事件
     * @param event 显示事件
     */
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private:
    QString m_strCustomName = "";
    int m_nResult = 0;
    QLineEdit *m_nameLineEdit = nullptr;
};

#endif // CUSTOMSERVICENAMEDLG_H
