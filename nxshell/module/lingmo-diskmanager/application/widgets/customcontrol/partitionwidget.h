// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef PARTITIONWIDGET_H
#define PARTITIONWIDGET_H

#include "partitioninfo.h"
#include "utils.h"
#include "partchartshowing.h"
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

typedef struct STRUCTPART {
    STRUCTPART()
    {
        m_size = 0.0;
        m_count = 0;
        m_labelName = m_fstype = "";
        m_blast = false;
        m_password = m_passwordHint = "";
        m_encryption = CRYPT_CIPHER::NOT_CRYPT;
        m_isEncryption = false;
    }
    double m_size;
    Sector m_count;
    QString m_labelName;
    QString m_fstype;
    bool m_blast;
    QString m_password;
    QString m_passwordHint;
    CRYPT_CIPHER m_encryption;
    bool m_isEncryption;
} stPart;

/**
 * @class PartitionWidget
 * @brief 创建分区类
 */

class PartitionWidget : public DDialog
{
    Q_OBJECT

public:
    explicit PartitionWidget(QWidget *parent = nullptr);
    ~PartitionWidget() override;

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
     * @brief 分区页下端展示窗口
     */
    void botFrameSetting();

    /**
     * @brief 分区信息显示窗口
     */
    void partInfoShowing();

    /**
     * @brief 初始化连接
     */
    void initConnection();

    /**
     * @brief 给对应页面控件设置分区信息数据
     */
    void recPartitionInfo();

    /**
     * @brief 计算已用分区大小的和
     */
    double sumValue();  //已用分区大小的和

    /**
     * @brief 是否超出最大可建分区个数
     * @return 超出返回true，没超出返回false
     */
    bool maxAmountPrimReached();

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
     * @brief 设置选中空闲分区的状态以及值
     */
    void setSelectUnallocatesSpace();//选中空闲分区的状态

    /**
     * @brief 设置分区信息操作窗口控件的样式
     * @param isExceed 是否存在空闲空间
     */
    void setAddOrRemResult(const bool &isExceed);

signals:

private slots:

    /**
     * @brief 滑动条数值改变响应的槽函数
     * @param value 分区数值
     */
    void onSliderValueChanged(int value);

    /**
     * @brief 设置滑动条数据
     */
    void onSetSliderValue();

    /**
     * @brief 设置分区名称
     */
    void onSetPartName(); //选个设置或修改分区名称

    /**
     * @brief 当前文本改变
     */
    void onTextChanged(const QString &text);

    /**
     * @brief 添加分区
     */
    void onAddPartition();//"+"新建分区

    /**
     * @brief 删除分区
     */
    void onRemovePartition();//"-"删除分区

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
     * @brief 显示选中分区的信息
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
    QVector<stPart> m_patrinfo;
    bool m_isExceed = true;
    DLabel *m_partFormateLabel;
    DLabel *m_encryptionInfo;
    DLabel *m_emptyLabel;
    DScrollArea *m_scrollArea;
};

#endif // PARTITIONWIDGET_H
