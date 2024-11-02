#include "customservicenamedlg.h"

#include <QDebug>
#include <QMessageBox>
#include <QIcon>

#include "../controls/mydialog.h"
#include "../macro.h"
#include "../util.h"

CustomServiceNameDlg::CustomServiceNameDlg(const QString &title, const QString &message, QWidget *parent)
    : MyDialog(title, "", parent)
{
    this->setFixedSize(372,160);
    m_nameLineEdit = new QLineEdit(this);
    addContent(m_nameLineEdit);

    addButton(tr("OK"), true);
    addButton(tr("Cancel"));

    connect(this,
            &CustomServiceNameDlg::buttonClicked,
            this,
            &CustomServiceNameDlg::onButtonClicked);
}

CustomServiceNameDlg::~CustomServiceNameDlg()
{
}

void CustomServiceNameDlg::onButtonClicked(int index, const QString &)
{
    if (index == 0) {
        m_strCustomName = m_nameLineEdit->text();
        setResult(QMessageBox::Ok);
    } else {
        setResult(QMessageBox::Cancel);
    }
}

// 显示事件
void CustomServiceNameDlg::showEvent(QShowEvent *)
{
    // 聚焦输入框
    m_nameLineEdit->setFocus();
}

QString CustomServiceNameDlg::getCustomServiceName()
{
    return m_strCustomName;
}

void CustomServiceNameDlg::setCustomServiceName(QString strSvcName)
{
    m_strCustomName = strSvcName;
}

int CustomServiceNameDlg::getResult()
{
    return m_nResult;
}

void CustomServiceNameDlg::setResult(int nRet)
{
    m_nResult = nRet;
}
