// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef CREATELVWIDGET_H
#define CREATELVWIDGET_H

#include "partitioninfo.h"
#include "utils.h"
#include "customcontrol/partchartshowing.h"
#include "partedproxy/dmdbushandler.h"

#include <DDialog>
#include <DLabel>
#include <DSlider>
#include <DLineEdit>
#include <DComboBox>
#include <DFrame>
#include <DPalette>
#include <DApplicationHelper>
#include <DPushButton>
#include <DFontSizeManager>
#include <DIconButton>
#include <DMessageManager>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolTip>
#include <QRegExp>
#include <QRegExpValidator>

DWIDGET_USE_NAMESPACE

typedef struct STRUCTLV {
    STRUCTLV()
    {
        m_size = 0.0;
        m_lvName = m_fstype = m_lvSize = "";
        m_lvByteSize = 0;
        m_blast = false;
        m_password = m_passwordHint = m_dmName = "";
        m_encryption = CRYPT_CIPHER::NOT_CRYPT;
        m_isEncryption = false;
    }
    double m_size;
    QString m_lvName;
    QString m_fstype;
    bool m_blast;
    QString m_lvSize;
    long long m_lvByteSize;
    QString m_password;
    QString m_passwordHint;
    QString m_dmName;
    CRYPT_CIPHER m_encryption;
    bool m_isEncryption;
} stLV;

/**
 * @class createLVWidget
 * @brief 创建逻辑卷类
 */

class CreateLVWidget : public DDialog
{
    Q_OBJECT
public:
    explicit CreateLVWidget(QWidget *parent = nullptr);

    int partCount(){return m_patrinfo.count();}

private:
    /**
     * @brief 初始化界面
     */
    void initUi();

    /**
     * @brief 磁盘信息区域展示窗口
     */
    void topFrameSetting();

    /**
     * @brief 中间图形绘制区域展示窗口
     */
    void midFrameSetting();

    /**
     * @brief 逻辑卷页下端展示窗口
     */
    void botFrameSetting();

    /**
     * @brief 逻辑卷信息显示窗口
     */
    void partInfoShowing();

    /**
     * @brief 初始化连接
     */
    void initConnection();

    /**
     * @brief 给对应页面控件设置逻辑卷信息数据
     */
    void recPartitionInfo();

    /**
     * @brief 计算已用逻辑卷大小的和
     */
    double sumValue();  //已用逻辑卷大小的和

    /**
     * @brief 页面可操作页面是否使能
     * @param flag 选中分区标志
     * @param isExceed 是否存在空闲分区
     */
    void setEnable(const int &flag, const bool &isExceed);

    /**
     * @brief 设置可操作控件使能
     * @param isTrue 是否使能
     */
    void setControlEnable(const bool &isTrue);

    /**
     * @brief 设置颜色
     * @param isOk 判断设置对应颜色
     */
    void setLabelColor(const bool &isOk);//颜色

    /**
     * @brief 正则表达式，分区名称和分区大小进行输入时不能输入特殊字符
     */
    void setRegValidator();//正则表达

    /**
     * @brief 设置选中空闲逻辑卷的状态以及值
     */
    void setSelectUnallocatesSpace();//选中空闲逻辑卷的状态

    /**
     * @brief 设置逻辑卷信息操作窗口控件的样式
     * @param isExceed 是否存在空闲空间
     */
    void setAddOrRemResult(const bool &isExceed);

    /**
     * @brief 获取逻辑卷名称
     */
    QString lvName();

signals:

private slots:

    /**
     * @brief 滑动条数值改变响应的槽函数
     * @param value 逻辑卷数值
     */
    void onSliderValueChanged(int value);

    /**
     * @brief 设置滑动条数据
     */
    void onSetSliderValue();

    /**
     * @brief 添加逻辑卷
     */
    void onAddPartition();//"+"新建逻辑卷

    /**
     * @brief 删除逻辑卷
     */
    void onRemovePartition();//"-"删除逻辑卷

    /**
     * @brief 确定按钮点击响应的槽函数
     */
    void onApplyButton();

    /**
     * @brief 复原按钮点击响应的槽函数
     */
    void onRevertButton();

    /**
     * @brief 取消按钮点击响应的槽函数
     */
    void onCancelButton();

    /**
     * @brief 显示选中逻辑卷的信息
     * @param flag 选中分区标志
     * @param num 选中的第几个分区
     * @param posX 鼠标点击的X位置
     */
    void onShowSelectPathInfo(const int &flag, const int &num, const int &posX);//点击显示tip

    /**
     * @brief 显示选中分区的信息
     * @param hover 选中分区标志
     * @param num 选中的第几个分区
     * @param posX 鼠标点击的X位置
     */
    void onShowTip(const int &hover, const int &num, const int &posX);//悬停显示tip

    /**
     * @brief 下拉框单位切换
     * @param index 当前选择index
     */
    void onComboxCurTextChange(int index); //下拉框单位切换

    /**
     * @brief 设置滑动条和输入大小不可用
     */
    void onJudgeLastPartition();

    /**
     * @brief 下拉框分区格式切换
     * @param text 当前选择文本
     */
    void onComboxFormatTextChange(const QString &text); //下拉框分区格式切换

protected:
    /**
     * @brief event:事件变化
     * @param event事件
     * @return 布尔
     */
    bool event(QEvent *event) override;

    /**
     * @brief 键盘按下事件
     * @param event事件
     */
    void keyPressEvent(QKeyEvent *event) override;

private:
    QWidget *m_mainFrame;
    DFrame *m_topFrame;
    DFrame *m_midFrame;
    DFrame *m_botFrame;
    DLabel *m_partInfoLabel;
    DLabel *m_partDoLabel;
    DLabel *m_allMemory;
    DLabel *m_deviceFormate;
    DLabel *m_deviceNameLabel;
    DLabel *m_deviceName;
    DLabel *m_selectedPartition;
    DLabel *m_partNameLabel;
    DIconButton *m_addButton;
    DIconButton *m_remButton;
    QWidget *m_partWidget;
    DPushButton *m_applyBtn;
    DPushButton *m_cancleBtn;
    DPushButton *m_reveBtn;
    PartChartShowing *m_partChartWidget;
    DLineEdit *m_partNameEdit;
    DComboBox *m_partFormateCombox;
    DLineEdit *m_partSizeEdit;
    DComboBox *m_partComboBox;
    DSlider *m_slider;
    QString m_currentEditSize;
    int m_number = -1;
    double m_total = 0.00;
    double m_totalSize = 0.00;
    int m_flag = -1;
    int m_value = 0;
    int m_block = 0;
    QVector<double> m_sizeInfo;
    QVector<QString> m_partName;
    QVector<stLV> m_patrinfo;
    bool m_isExceed = true;
    QStringList m_lstLVName;
    int m_peSize = 4;
    DLabel *m_partFormateLabel;
    DLabel *m_encryptionInfo;
    DLabel *m_emptyLabel;
    DScrollArea *m_scrollArea;
};

#endif // CREATELVWIDGET_H
