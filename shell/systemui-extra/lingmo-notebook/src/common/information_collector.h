#ifndef INFORMATION_COLLECTOR_H
#define INFORMATION_COLLECTOR_H

#include <QString>

class InformationCollector
{
public:
    enum P {
        OrderList,      /* 有序列表 */
        UnorderList,    /* 无序列表 */
        Bold,           /* 加粗 */
        Italics,        /* 斜体 */
        Underline,      /* 下划线 */
        Deleteline,     /* 删除线 */
        FontSize,       /* 字号 */
        FontColor,      /* 字体颜色 */
        InsertPicture,  /* 插入图片 */
        InterfaceColor, /* 界面配色 */
        DeleteCurrent,  /* 删除当前 */
        UiTop,          /* UI 置顶 */
        ListMode,       /* 列表模式 */
        IconMode,       /* 图标模式 */
        NewNote,        /* 新建便签 */
        Search,         /* 搜索 */
        Delete,         /* 删除 */
        ModeChange,     /* 模式切换 */
    };
    static InformationCollector& getInstance()
    {
        static InformationCollector instance;
        return instance;
    }
    void addPoint(P point) const;
    void addMessage(const QString &str) const;

private:
    void sendMessage(const QString &key, const QString &value) const;

    InformationCollector(/* args */);
    InformationCollector(InformationCollector const&);
    InformationCollector& operator=(InformationCollector const&);
    ~InformationCollector();
};

#endif // INFORMATION_COLLECTOR_H