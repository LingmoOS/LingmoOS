// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CTIMELINEEDIT_H
#define CTIMELINEEDIT_H

#include <DSpinBox>

DWIDGET_USE_NAMESPACE

//时间数字编辑器
class CTimeLineEdit : public DSpinBox
{
    Q_OBJECT
public:
    explicit CTimeLineEdit(int id = 0, QWidget *parent = nullptr);

    //设置控件数字，值将被限制在临界值范围内
    void setNum(int num);
    //设置控件数字，如果canCaryy为true且超过临界值将进行自动跳转时间
    void setNum(int num, bool canCarry);
    //设置数字显示范围
    void setRange(int min, int max);
    //设置步状态
    void setStepEnabled(CTimeLineEdit::StepEnabled);

signals:
    //数字改变信号
    void signalNumChange(int id, int num);
    //时间跳转信号
    void signalDateJump(int id, int num);

public slots:
    //编辑完成事件
    void slotEditingFinished();
    //编辑框编辑事件
    void slotTextEdited(const QString &value);

protected:
    //重写步状态
    StepEnabled stepEnabled() const override;
    //重写步事件
    void stepBy(int steps) override;

private:
    void initView();

private:
    int m_num = 0;      //当前显示的数字
    const int m_id = 0; //控件id
    CTimeLineEdit::StepEnabled m_stepEnable = CTimeLineEdit::StepUpEnabled|CTimeLineEdit::StepDownEnabled;
};

#endif // CTIMELINEEDIT_H
