// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BUTTONTOOL_H
#define BUTTONTOOL_H

#include "globaldefine.h"
#include "drawtool.h"

class DRAWLIB_EXPORT NCButtonTool: public DrawTool
{
    Q_OBJECT
public:
    NCButtonTool(int tp, QObject *parent = nullptr);

    int toolType() const override;

protected:
    void pressOnScene(ToolSceneEvent *event) final;
    void moveOnScene(ToolSceneEvent *event) final;
    void releaseOnScene(ToolSceneEvent *event) final;

    //SAttrisList attributions() override;

    void onStatusChanged(EStatus oldStatus, EStatus nowStatus) final;

    virtual void onClicked() {}

signals:
    void clicked();

private:
    const int _toolType;
};

class DRAWLIB_EXPORT UndoTool: public NCButtonTool
{
    Q_OBJECT
public:
    explicit UndoTool(QObject *parent = nullptr);

    void onClicked() override;

    void onCurrentPageChanged(Page *newPage) override;

private:
    QUndoStack *lastStack = nullptr;
};

class DRAWLIB_EXPORT RedoTool: public NCButtonTool
{
    Q_OBJECT
public:
    explicit RedoTool(QObject *parent = nullptr);

    void onClicked() override;

    void onCurrentPageChanged(Page *newPage) override;

private:
    QUndoStack *lastStack = nullptr;
};

class DRAWLIB_EXPORT ClearTool: public NCButtonTool
{
    Q_OBJECT
public:
    explicit ClearTool(QObject *parent = nullptr);

    void onClicked() override;
};

class DRAWLIB_EXPORT SaveTool: public NCButtonTool
{
    Q_OBJECT
public:
    explicit SaveTool(QObject *parent = nullptr);

    void onClicked() override;
};

class DRAWLIB_EXPORT CloseTool: public NCButtonTool
{
    Q_OBJECT
public:
    explicit CloseTool(QObject *parent = nullptr);

    void onClicked() override;
};

class DRAWLIB_EXPORT OpenTool: public NCButtonTool
{
    Q_OBJECT
public:
    explicit OpenTool(QObject *parent = nullptr);

    void onClicked() override;
};


#endif // BUTTONTOOL_H
