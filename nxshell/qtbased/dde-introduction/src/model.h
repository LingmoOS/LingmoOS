// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QJsonObject>
#include <QDebug>
#include <QPixmap>
#include <QCollator>

//图标结构体
struct IconStruct {
    QString Id;//图标id
    QString Path;//图标路径
    bool Deletable;//是否可删除的标志位
    QString Pixmap;//图标名称

    //重载==
    bool operator ==(const IconStruct &icon) const {
        return Id == icon.Id;
    }

    //重载<
    bool operator <(const IconStruct &icon) const {
        return Id < icon.Id;
    }

    //友元函数，重定义<<，打印IconStruct中信息
    friend QDebug operator <<(QDebug debug, const IconStruct &icon) {
        debug << QString("{Id=%1,Path=%2,Deletable=%3}").arg(icon.Id)
                                                        .arg(icon.Path)
                                                        .arg(icon.Deletable);
        return debug;
    }

    //获取json中图标信息
    static IconStruct fromJson(const QJsonObject &object) {
          IconStruct Icon;
          Icon.Id = object["Id"].toString();
          Icon.Path = object["Path"].toString();
          Icon.Deletable = object["Deletable"].toBool();
          Icon.Pixmap = object["Pixmap"].toString();

          return Icon;
    }
};

//数据存储模型类
class Model : public QObject
{
    Q_OBJECT
public:
    static Model* Instance();

    //桌面样式枚举值，EfficientMode：高效模式 FashionMode：时尚模式
    enum DesktopMode {
        FashionMode,
        EfficientMode
    };

    //运行模式枚举值，WM_2D：普通模式，WM_3D：特效模式
    enum WMType {
        WM_NOMAL,
        WM_2D,
        WM_3D
    };

    //主题图标枚举值，Big：大图标，Small：小图标
    enum IconType {
        Big,
        Small
    };

    //获取当前主题图标数据，如果有返回找到的数据，反之，返回空的数据
    IconStruct currentIcon();
    //获取存储主题图标的列表
    inline QList<IconStruct> iconList() { return m_iconList; }
    //获取运行模式的类型，特效或者普通
    inline WMType wmType() const { return m_wmType; }
    //获取桌面样式的类型，高效或者时尚
    inline DesktopMode desktopMode() const { return m_desktopMode; }

signals:
    //图标变化信号
    void iconChanged(const IconStruct &icon) const;
    //图标添加信号
    void iconAdded(const IconStruct &icon) const;
    //图标删除信号
    void iconRemoved(const IconStruct &icon) const;
    //运行模式变化信号
    void wmTypeChanged(WMType type) const;
    //桌面样式变化信号
    void desktopModeChanged(DesktopMode mode) const;

public slots:
    //添加图标槽
    void addIcon(const IconStruct &icon);
    //删除图标槽
    void removeIcon(const IconStruct &icon);
    //设置当前图标名
    void setCurrentIcon(const QString &icon);
    //设置运行模式类型
    void setWmType(WMType wmType);
    //设置桌面样式类型
    void setDesktopMode(DesktopMode desktopMode);

private:
    explicit Model(QObject *parent = nullptr);

private:
    //存储主题图标列表
    QList<IconStruct> m_iconList;
    //当前图标名称
    QString m_currentIcon;
    //运行模式类型
    WMType m_wmType {WMType::WM_NOMAL};
    //桌面样式类型
    DesktopMode m_desktopMode = FashionMode;
};

#endif // MODEL_H
