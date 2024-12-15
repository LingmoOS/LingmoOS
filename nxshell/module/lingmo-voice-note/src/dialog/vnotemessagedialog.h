// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VNOTEMESSAGEDIALOG_H
#define VNOTEMESSAGEDIALOG_H

#include "vnotebasedialog.h"

#include <DPushButton>
#include <DWarningButton>
#include <DVerticalLine>

DWIDGET_USE_NAMESPACE;

class VNoteMessageDialog : public VNoteBaseDialog
{
    Q_OBJECT
public:
    explicit VNoteMessageDialog(int msgType , QWidget *parent = nullptr ,int notesCount =1 );

    enum MessageType {
        DeleteNote,
        AbortRecord,
        DeleteFolder,
        AsrTimeLimit,
        AborteAsr,
        VolumeTooLow,
        CutNote,
        SaveFailed, //保存失败
        NoPermission, //无权限
        VoicePathNoAvail, //语音路径无效
    };

protected:
    //初始化布局
    void initUI();
    //连接槽函数
    void initConnections();
    //设置文本
    void initMessage();
    //显示单按钮
    void setSingleButton(); //Need to be Optimzed

    /**
     * @author liuxinping  ut002764
     * @brief  focusInEvent 重写
     * @param
     * @return
     */
    void focusInEvent(QFocusEvent *event) override;
signals:

public slots:

protected:
    DLabel *m_pMessage {nullptr};
    DPushButton *m_cancelBtn {nullptr};
    DWarningButton *m_confirmBtn {nullptr};
    DVerticalLine *m_buttonSpliter {nullptr};
    //笔记数量
    int m_notesCount = 1;
    MessageType m_msgType;
};

#endif // VNOTEMESSAGEDIALOG_H
