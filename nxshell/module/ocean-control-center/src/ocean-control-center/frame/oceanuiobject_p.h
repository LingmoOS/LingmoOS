// SPDX-FileCopyrightText: 2024 - 2027 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DCCOBJECT_P_H
#define DCCOBJECT_P_H

#include "oceanuiobject.h"

#include <QVector>
#define DCC_HIOCEANN 0x80000000
#define DCC_DISABLED 0x40000000
#define DCC_CONFIG_HIOCEANN 0x20000000
#define DCC_CONFIG_DISABLED 0x10000000
// 0xA0000000 = 0x80000000|0x20000000 0x80000000为用户设置位 0x20000000为配置设置位
// 0x50000000 = 0x40000000|0x10000000 0x4000000为用户设置位 0x10000000为配置设置位
#define DCC_ALL_HIOCEANN 0xA0000000
#define DCC_ALL_DISABLED 0x50000000

#define DCC_MASK_BGTYPE 0x000000FF   // 背景类型
#define DCC_MASK_NOBGTYPE 0xFFFFFF00 // 背景类型

#define DCC_CUSTOM_DEFULT 0x08000000

#define DCC_CANSEARCH 0x04000000 // 不参与搜索

namespace oceanuiV25 {
class OceanUIObject::Private
{
public:
    static OceanUIObject::Private *FromObject(const OceanUIObject *obj);

    bool getFlagState(uint32_t flag) const;
    bool setFlagState(uint32_t flag, bool state);
    uint32_t getFlag() const;

    bool addChild(OceanUIObject::Private *child);
    bool addChild(OceanUIObject *child, bool updateParent = true);
    void removeChild(int index);
    void removeChild(OceanUIObject *child);
    void updatePos(OceanUIObject *child);
    const QVector<OceanUIObject *> &getChildren() const;
    int getIndex() const;

    inline OceanUIObject *getParent() const { return m_parent; }

    inline const QVector<OceanUIObject *> &getObjects() const { return m_objects; }

    OceanUIObject *getChild(int childPos) const;
    int getChildIndex(const OceanUIObject *child) const;
    void deleteSectionItem();

protected:
    explicit Private(OceanUIObject *obj);
    virtual ~Private();

    virtual inline void SetParent(OceanUIObject *anObject) { m_parent = anObject; }

private:
    // data property
    static void data_append(QQmlListProperty<QObject> *data, QObject *o);
    static qsizetype data_count(QQmlListProperty<QObject> *data);
    static QObject *data_at(QQmlListProperty<QObject> *data, qsizetype i);
    static void data_clear(QQmlListProperty<QObject> *data);

protected:
    qint8 m_badge;
    quint8 m_pageType;
    quint16 m_weight;
    quint32 m_flags;

    OceanUIObject *q_ptr;    // q指针
    OceanUIObject *m_parent; // 父项
    OceanUIObject *m_currentObject;
    QVector<OceanUIObject *> m_children; // 子项
    QVector<OceanUIObject *> m_objects;  // m_data中OceanUIObject(未保证有效(delete时未处理))
    QObjectList m_data;              // data属性，为qml能加子项
    QPointer<QQmlComponent> m_page;
    QPointer<QQuickItem> m_sectionItem; // Item
    QPointer<QQuickItem> m_parentItem;  // Item父项

    QString m_parentName;
    QString m_displayName;
    QString m_description;
    QString m_icon;    // 属性
    QUrl m_iconSource; // icon带相对路径处理
    friend class OceanUIObject;
};
} // namespace oceanuiV25
#endif // DCCOBJECT_P_H
