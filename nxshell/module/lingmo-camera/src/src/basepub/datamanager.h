// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QSet>

enum GridType
{
    Grid_None = 0, //不显示网格线
    Grid_Matts = 1, //田字格
    Grid_Thirds = 2 //九宫格
};

enum DeviceStatus {NOCAM, CAM_CANNOT_USE, CAM_CANUSE};
//enum MultiType {None, Ctrl, Shift, Both};

enum EncodeEnv
{
    FFmpeg_Env = 0,
    QCamera_Env = 1,
    GStreamer_Env = 2
};

class DataManager: public QObject
{
    Q_OBJECT
public:

    static DataManager *instance();

    QSet<int> m_setIndex;
    bool m_bTabEnter;//tab焦点Enter键触发
    uint m_tabIndex;//tab框位置索引
    Q_DISABLE_COPY(DataManager)

    /**
     * @brief getMultiType 返回ctrl多选或者shift多选
     */
//    MultiType getMultiType();

    /**
     * @brief getObject 获取控件
     * @param index
     * @return
     */
    QWidget *getObject(uint index);

    /**
     * @brief setCtrlMulti 设置ctrl多选
     * @param bCtrlMulti
     */
//    void setCtrlMulti(bool bCtrlMulti);
    /**
     * @brief setShiftMulti 设置shift连续多选
     * @param bShiftMultiSlt
     */
//    void setShiftMulti(bool bShiftMultiSlt);
    /**
     * @brief getbindexNow 获得当前索引
     */
//    int getindexNow();
    /**
     * @brief setindexNow 设置当前索引
     * @param indexNow
     */
//    void setindexNow(int indexNow);
    /**
     * @brief setLastIndex 设置上一次图片索引
     * @param index
     */
//    void setLastIndex(int index)
//    {
//        m_indexLast = index;
//    }

    /**
     * @brief getLastIndex 获取图片索引
     */
//    int getLastIndex()
//    {
//        return m_indexLast;
//    }

    /**
     * @brief clearIndex 清除所有选中索引
     */
    void clearIndex()
    {
        m_setIndex.clear();
    }

    /**
     * @brief insertIndex 插入索引
     * @param index 索引
     */
    void insertIndex(int index)
    {
        m_setIndex.insert(index);
    }

    /**
     * @brief getstrFileName 获得文件名
     */
    QString &getstrFileName();
    /**
     * @brief getdevStatus 获得设备状态
     */
    enum DeviceStatus getdevStatus();
    /**
     * @brief setdevStatus 设置设备状态
     */
    void setdevStatus(enum DeviceStatus devStatus);

    /**
     * @brief setNowTabIndex 设置当前Tab焦点窗口的Index索引
     * @param tabindex 索引
     */
    void setNowTabIndex(uint tabindex);

    /**
     * @brief getNowTabIndex 获取当前Tab焦点窗口的Index索引
     * @return
     */
    uint getNowTabIndex();

    /**
     * @brief setFFmpegEnv 设置当前是否具备FFmpeg环境
     * @return
     */
    void setEncodeEnv(EncodeEnv env);

    /**
     * @brief isFFmpegEnv 获取当前是否具备FFmpeg环境
     * @return
     */
    EncodeEnv encodeEnv();

    void setEncExists(bool status);
    bool encExists();

private:
    DataManager();
    static DataManager *m_dataManager;
    uint m_tabIndexNow;//当前tab索引
    QString m_strFileName;
    int m_videoCount;
    EncodeEnv m_encodeEnv;
    volatile enum DeviceStatus m_devStatus;
    bool m_H264EncoderExists;
};
#endif // DATAMANAGER_H
