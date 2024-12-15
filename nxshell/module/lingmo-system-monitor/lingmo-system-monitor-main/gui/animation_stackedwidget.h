// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AnimationStackedWidget_H
#define AnimationStackedWidget_H

#include <QStackedWidget>
#include <QStackedLayout>
#include <QPropertyAnimation>

class AnimationStackedWidget : public QStackedWidget
{
    Q_OBJECT
    Q_PROPERTY(double offset READ offset WRITE setOffset)
public:
    enum MoveOrientation {LeftToRight, RightToLeft, TopToBottom, BottomToTop};
    enum AnimationOri {LR, TB}; //LR:Left Right  ,TB:Top  Bottom
public:
    explicit AnimationStackedWidget(const AnimationOri ori = LR, QWidget *parent = nullptr);
    ~AnimationStackedWidget() override;
    /**
     * @brief setDuration   设置动画持续时间
     * @param duration  动画时间
     */
    void setDuration(int duration = 10);
    /**
     * @brief IsRunning 动画是否正在运行
     * @return  动画师傅正在运行
     */
    bool IsRunning()const
    {
        return m_IsAnimation;
    }
    /**
     * @brief offset    获取移动偏移量
     * @return   移动偏移量
     */
    double offset() const;
    /**
     * @brief setOffset 设置移动偏移量
     * @param offset  移动偏移量
     */
    void setOffset(double offset);

protected:
    /**
     * @brief paintEvent   窗口绘制事件
     * @param e 绘制事件对象
     */
    void paintEvent(QPaintEvent *e) override;
signals:
    /**
     * @brief signalIsFinished     动画结束信号
     */
    void signalIsFinished();
public slots:
    /**
     * @brief animationFinished  动画切换完成
     */
    void animationFinished();
    /**
     * @brief setCurrent   设置当前窗口
     * @param index 索引
     */
    void setCurrent(int index);

    /**
     * @brief 设置当前窗口
     * @param widget
     */
    void setCurrent(QWidget *widget);

private:
    /**
     * @brief setCurrentWidget  根据索引设置当前窗口，开启动画绘制
     * @param index     索引
     * @param beginWidth 动画起始值
     */
    void setCurrentWidget(int &index, int beginWidth);
    /**
     * @brief getBeginValue 获取动画起始值
     * @return 返回动画起始值
     */
    int getBeginValue();
    /**
     * @brief getMoveOrientation  根据当前索引和下一个索引获取动画方向
     * @param currIndex 当前索引
     * @param nextIndex 设置的索引
     * @return  动画绘制方向
     */
    MoveOrientation getMoveOrientation(const int currIndex, const int nextIndex);
    /**
     * @brief paintCurrentWidget获取当前窗口的图片动态绘制
     * @param paint     绘制对象
     * @param nextIndex 窗口索引
     */
    void paintCurrentWidget(QPainter &paint, int currentIndex);
    /**
     * @brief paintNextWidget   获取下一个窗口的图片动态绘制
     * @param paint         绘制对象
     * @param nextIndex     窗口索引
     */
    void paintNextWidget(QPainter &paint, int nextIndex);
private:
    /**
     * @brief m_animationOri 动画方向--左右还是上下
     */
    AnimationOri m_animationOri{LR};
    /**
     * @brief m_Animation 动画对象
     */
    QPropertyAnimation *m_Animation = nullptr;
    /**
     * @brief m_Duration 动画时间(ms)
     */
    int m_Duration{0};
    /**
     * @brief m_IsAnimation 是否为动画状态
     */
    bool m_IsAnimation = false;
    /**
     * @brief m_offset 移动偏移量
     */
    double m_offset = 0;
    /**
     * @brief paintNext
     */
    int m_WidgetCount{0};
    /**
     * @brief m_NextIndex 下一个索引
     */
    int m_NextIndex{0};
    /**
     * @brief m_moveOri 移动方向
     */
    MoveOrientation m_moveOri = MoveOrientation::LeftToRight;
};

#endif // AnimationStackedWidget_H
