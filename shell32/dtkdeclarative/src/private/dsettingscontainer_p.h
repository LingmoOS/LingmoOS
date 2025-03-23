// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSETTINGSCONTAINER_P_H
#define DSETTINGSCONTAINER_P_H

#include <dtkdeclarative_global.h>

#include <QQmlParserStatus>
#include <QQmlComponent>
#include <private/qqmlobjectmodel_p.h>
#include "dconfigwrapper_p.h"

DQUICK_BEGIN_NAMESPACE

class SettingsOption : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString key READ key WRITE setKey NOTIFY keyChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QVariant value READ value WRITE setValue RESET resetValue NOTIFY valueChanged)
    Q_PROPERTY(QQmlComponent *delegate READ delegate WRITE setDelegate NOTIFY delegateChanged)
    Q_CLASSINFO("DefaultProperty", "delegate")
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(SettingsOption)
#endif

public:
    explicit SettingsOption(QObject *parent = nullptr);
    virtual ~SettingsOption() override;

    QString key() const;
    void setKey(QString key);
    QString name() const;
    void setName(QString name);
    QVariant value() ;
    void setValue(QVariant value);
    void resetValue();

    QQmlComponent *delegate() const;
    void setDelegate(QQmlComponent *delegate);
    void setConfig(DConfigWrapper *config);

    static SettingsOption *qmlAttachedProperties(QObject *object);

Q_SIGNALS:
    void keyChanged(QString key);
    void nameChanged(QString name);
    void valueChanged(QVariant value);
    void delegateChanged();

private Q_SLOTS:
    void onConfigValueChanged();

private:
    QString m_key;
    QString m_name;
    QVariant m_value;
    bool m_valueInitialized = false;
    QQmlComponent *m_delegate = nullptr;
    DConfigWrapper *m_config = nullptr;
};

class SettingsGroup : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString key READ key WRITE setKey NOTIFY keyChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(int level READ level CONSTANT)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(int index READ index NOTIFY indexChanged)
    Q_PROPERTY(QQmlListProperty<DTK_QUICK_NAMESPACE::SettingsOption> options READ options NOTIFY optionsChanged)
    Q_PROPERTY(QQmlListProperty<DTK_QUICK_NAMESPACE::SettingsGroup> children READ children NOTIFY childrenChanged)
    Q_PROPERTY(QQmlComponent *background READ background WRITE setBackground NOTIFY backgroundChanged)
    Q_CLASSINFO("DefaultProperty", "options")
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(SettingsGroup)
    QML_ATTACHED(SettingsGroup)
#endif

public:
    explicit SettingsGroup(QObject * parent = nullptr);
    virtual ~SettingsGroup() override;

    QString key() const;
    void setKey(const QString &key);
    QString name() const;
    void setName(QString name);
    int level() const;
    void setLevel(int level);
    bool visible() const;
    void setVisible(bool visible);
    QQmlListProperty<DTK_QUICK_NAMESPACE::SettingsOption> options();
    QQmlListProperty<DTK_QUICK_NAMESPACE::SettingsGroup> children();
    QQmlComponent *background() const;
    void setBackground(QQmlComponent *background);
    void setConfig(DConfigWrapper *config);
    SettingsGroup *parentGroup() const;
    void setParentGroup(SettingsGroup *parentGroup);
    int index() const;
    void setIndex(const int index);

    static SettingsGroup *qmlAttachedProperties(QObject *object);

Q_SIGNALS:
    void keyChanged(QString key);
    void nameChanged(QString name);
    void visibleChanged(bool visible);
    void optionsChanged();
    void childrenChanged();
    void backgroundChanged();
    void indexChanged(int index);

private:

    QString m_key;
    QString m_name;
    int m_level = -1;
    bool m_visible = true;
    int m_index;
    QList<SettingsOption*> m_options;
    QList<SettingsGroup*> m_children;
    QQmlComponent * m_background = nullptr;
    SettingsGroup* m_parentGroup = nullptr;
};

class SettingsContainer;
class SettingsInstanceModelPrivate;
class SettingsInstanceModel : public QQmlInstanceModel
{
    Q_OBJECT
public:
    explicit SettingsInstanceModel(SettingsContainer *container);

public:
    virtual int count() const override;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    virtual QVariant variantValue(int index, const QString &name) override;
    virtual ReleaseFlags release(QObject *object, ReusableFlag reusableFlag = NotReusable) override;
#else
    virtual QString stringValue(int index, const QString &name) override;
    virtual ReleaseFlags release(QObject *object) override;
#endif
    virtual void setWatchedRoles(const QList<QByteArray> &roles) override;
    virtual QQmlIncubator::Status incubationStatus(int index) override;
    virtual int indexOf(QObject *object, QObject *objectContext) const override;

    void updateModel();

public:
    void insert(SettingsGroup *group);
    void remove(SettingsGroup *group);

protected:
    Q_DECLARE_PRIVATE(SettingsInstanceModel)
};

class SettingsContentModel : public SettingsInstanceModel
{
    Q_OBJECT
public:
    explicit SettingsContentModel(SettingsContainer *container = nullptr);

public:
    bool isValid() const override;
    virtual QObject *object(int index, QQmlIncubator::IncubationMode incubationMode) override;

private:
    QQmlComponent *contentBackgroundByGroup(const SettingsGroup *group);
    static const QByteArray groupComponentData();
    static const QByteArray optionsLayoutComponentData();
};

class SettingsNavigationModel : public SettingsInstanceModel {
    Q_OBJECT
public:
    SettingsNavigationModel(SettingsContainer *container = nullptr);

public:
    virtual bool isValid() const override;
    virtual QObject *object(int index, QQmlIncubator::IncubationMode incubationMode) override;
};

class SettingsContentModel;
class SettingsModelAttached;
class SettingsContainer : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(DConfigWrapper *config READ config WRITE setConfig NOTIFY configChanged)
    Q_PROPERTY(QQmlListProperty<DTK_QUICK_NAMESPACE::SettingsGroup> groups READ groups NOTIFY groupsChanged)
    Q_PROPERTY(SettingsContentModel *contentModel READ contentModel NOTIFY contentModelChanged)
    Q_PROPERTY(QQmlComponent *contentTitle READ contentTitle WRITE setContentTitle NOTIFY contentTitleChanged)
    Q_PROPERTY(QQmlComponent *contentBackground READ contentBackground WRITE setContentBackground NOTIFY contentBackgroundChanged)
    Q_PROPERTY(SettingsNavigationModel *navigationModel READ navigationModel NOTIFY navigationModelChanged)
    Q_PROPERTY(QQmlComponent *navigationTitle READ navigationTitle WRITE setNavigationTitle NOTIFY navigationTitleChanged)
    Q_CLASSINFO("DefaultProperty", "groups")
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QML_NAMED_ELEMENT(SettingsContainer)
#endif

public:
    explicit SettingsContainer(QObject *parent = nullptr);
    virtual ~SettingsContainer() override;

    DConfigWrapper *config() const;
    void setConfig(DConfigWrapper *config);
    QQmlListProperty<DTK_QUICK_NAMESPACE::SettingsGroup> groups();
    SettingsContentModel *contentModel() const;
    SettingsNavigationModel *navigationModel() const;
    QQmlComponent *contentTitle() const;
    void setContentTitle(QQmlComponent *delegate);
    QQmlComponent *navigationTitle() const;
    void setNavigationTitle(QQmlComponent *delegate);
    void setContentBackground(QQmlComponent * contentBackground);
    QQmlComponent *contentBackground() const;

    QVector<SettingsGroup *> groupList() const;
    QQmlContext *creationContext();

    virtual void classBegin() override;
    virtual void componentComplete() override;

public Q_SLOTS:
    void setGroupVisible(const QString &key, bool visible);
    bool groupVisible(const QString &key) const;
    void resetSettings();

private Q_SLOTS:
    void onGroupVisibleChanged(bool visible);

Q_SIGNALS:
    void groupsChanged();
    void navigationTitleChanged();
    void contentTitleChanged();
    void contentModelChanged();
    void navigationModelChanged();
    void configChanged();
    void contentBackgroundChanged();

private:
    QList<SettingsGroup*> m_groups;
    SettingsNavigationModel *m_navigationModel = nullptr;
    SettingsContentModel *m_contentModel = nullptr;
    QQmlComponent *m_contentTitle = nullptr;
    QQmlComponent *m_navigationTitle = nullptr;
    QQmlComponent * m_contentBackground = nullptr;
    DConfigWrapper *m_config = nullptr;
};

DQUICK_END_NAMESPACE

QML_DECLARE_TYPEINFO(DTK_QUICK_NAMESPACE::SettingsOption, QML_HAS_ATTACHED_PROPERTIES)
QML_DECLARE_TYPEINFO(DTK_QUICK_NAMESPACE::SettingsGroup, QML_HAS_ATTACHED_PROPERTIES)

#endif // DSETTINGSCONTAINER_P_H
