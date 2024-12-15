// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MoveView_H
#define MoveView_H

#include <DWidget>
#include <DApplicationHelper>
DWIDGET_USE_NAMESPACE

struct VNoteFolder;
struct VNoteItem;
class MoveView : public DWidget
{
    Q_OBJECT
public:
    explicit MoveView(QWidget *parent = nullptr);
    //设置记事本数据
    void setFolder(VNoteFolder *folder);
    //设置笔记数据
    void setNote(VNoteItem *note);
    //设置笔记数据列表
    void setNoteList(const QList<VNoteItem *> &noteList);
    //设置笔记数量
    void setNotesNumber(int value);

protected slots:
    void onCompositeChange();

protected:
    //重写paint事件
    void paintEvent(QPaintEvent *e) override;
    //非特效模式绘制
    void paintNormal(QPaintEvent *e);
    //特效模式绘制
    void paintComposite(QPaintEvent *e);

private:
    VNoteFolder *m_folder {nullptr};
    VNoteItem *m_note {nullptr};
    //当前选中笔记列表
    QList<VNoteItem *> m_noteList {nullptr};
    //拖拽笔记数量
    int m_notesNumber = 0;
    //初始化背景图片
    QPixmap m_backGroundPixMap;
    bool m_hasComposite {false};
};

#endif // MoveView_H
