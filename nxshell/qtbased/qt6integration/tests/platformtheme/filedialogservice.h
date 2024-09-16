/*
 * SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */
#ifndef FILEDIALOGSERVICE_H
#define FILEDIALOGSERVICE_H
#include <QDBusContext>
#include <QDBusObjectPath>
#include <QDBusConnection>

class FileDialogService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.filemanager.filedialog")
    Q_CLASSINFO("D-Bus Introspection", ""
                                       "  <interface name=\"com.deepin.filemanager.filedialog\">\n"
                                       "    <property access=\"readwrite\" type=\"s\" name=\"directory\"/>\n"
                                       "    <property access=\"readwrite\" type=\"s\" name=\"directoryUrl\"/>\n"
                                       "    <property access=\"readwrite\" type=\"as\" name=\"nameFilters\">\n"
                                       "      <annotation value=\"QStringList\" name=\"org.qtproject.QtDBus.QtTypeName\"/>\n"
                                       "    </property>\n"
                                       "    <property access=\"readwrite\" type=\"i\" name=\"filter\"/>\n"
                                       "    <property access=\"readwrite\" type=\"i\" name=\"viewMode\"/>\n"
                                       "    <property access=\"readwrite\" type=\"i\" name=\"acceptMode\"/>\n"
                                       "    <property access=\"read\" type=\"b\" name=\"windowActive\"/>\n"
                                       "    <property access=\"readwrite\" type=\"i\" name=\"heartbeatInterval\"/>\n"
                                       "    <property access=\"readwrite\" type=\"u\" name=\"windowFlags\"/>\n"
                                       "    <property access=\"readwrite\" type=\"b\" name=\"hideOnAccept\"/>\n"
                                       "    <method name=\"selectFile\">\n"
                                       "      <arg direction=\"in\" type=\"s\" name=\"filename\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"selectedFiles\">\n"
                                       "      <annotation value=\"QStringList\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
                                       "      <arg direction=\"out\" type=\"as\" name=\"filelist\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"selectUrl\">\n"
                                       "      <arg direction=\"in\" type=\"s\" name=\"url\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"selectedUrls\">\n"
                                       "      <annotation value=\"QStringList\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
                                       "      <arg direction=\"out\" type=\"as\" name=\"urllist\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"selectNameFilter\">\n"
                                       "      <arg direction=\"in\" type=\"s\" name=\"filter\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"selectedNameFilter\">\n"
                                       "      <arg direction=\"out\" type=\"s\" name=\"filter\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"selectNameFilterByIndex\">\n"
                                       "      <arg direction=\"in\" type=\"i\" name=\"index\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"selectedNameFilterIndex\">\n"
                                       "      <arg direction=\"out\" type=\"i\" name=\"index\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"setFileMode\">\n"
                                       "      <arg direction=\"in\" type=\"i\" name=\"fileMode\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"setLabelText\">\n"
                                       "      <arg direction=\"in\" type=\"i\" name=\"label\"/>\n"
                                       "      <arg direction=\"in\" type=\"s\" name=\"text\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"labelText\">\n"
                                       "      <arg direction=\"in\" type=\"i\" name=\"label\"/>\n"
                                       "      <arg direction=\"out\" type=\"s\" name=\"text\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"setOptions\">\n"
                                       "      <arg direction=\"in\" type=\"i\" name=\"options\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"setOption\">\n"
                                       "      <arg direction=\"in\" type=\"i\" name=\"option\"/>\n"
                                       "      <arg direction=\"in\" type=\"b\" name=\"on\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"options\">\n"
                                       "      <arg direction=\"out\" type=\"i\" name=\"options\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"testOption\">\n"
                                       "      <arg direction=\"in\" type=\"i\" name=\"option\"/>\n"
                                       "      <arg direction=\"out\" type=\"b\" name=\"on\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"setCurrentInputName\">\n"
                                       "      <arg direction=\"in\" type=\"s\" name=\"name\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"show\"/>\n"
                                       "    <method name=\"open\"/>\n"
                                       "    <method name=\"hide\"/>\n"
                                       "    <method name=\"accept\"/>\n"
                                       "    <method name=\"reject\"/>\n"
                                       "    <method name=\"winId\">\n"
                                       "      <arg direction=\"out\" type=\"t\" name=\"windowId\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"setWindowTitle\">\n"
                                       "      <arg direction=\"in\" type=\"s\" name=\"title\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"deleteLater\"/>\n"
                                       "    <method name=\"activateWindow\"/>\n"
                                       "    <method name=\"makeHeartbeat\"/>\n"
                                       "    <method name=\"addCustomWidget\">\n"
                                       "      <arg direction=\"in\" type=\"i\" name=\"type\"/>\n"
                                       "      <arg direction=\"in\" type=\"s\" name=\"data\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"getCustomWidgetValue\">\n"
                                       "      <arg direction=\"in\" type=\"i\" name=\"type\"/>\n"
                                       "      <arg direction=\"in\" type=\"s\" name=\"text\"/>\n"
                                       "      <arg direction=\"out\" type=\"v\" name=\"value\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"allCustomWidgetsValue\">\n"
                                       "      <arg direction=\"in\" type=\"i\" name=\"type\"/>\n"
                                       "      <arg direction=\"out\" type=\"a{sv}\" name=\"values\"/>\n"
                                       "      <annotation value=\"QMap&lt;QString,QVariant&gt;\" name=\"org.qtproject.QtDBus.QtTypeName.Out0\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"beginAddCustomWidget\"/>\n"
                                       "    <method name=\"endAddCustomWidget\"/>\n"
                                       "    <method name=\"addDisableUrlScheme\">\n"
                                       "      <arg direction=\"in\" type=\"s\" name=\"type\"/>\n"
                                       "    </method>\n"
                                       "    <method name=\"setAllowMixedSelection\">\n"
                                       "      <arg direction=\"in\" type=\"b\" name=\"on\"/>\n"
                                       "    </method>\n"
                                       "    <signal name=\"finished\">\n"
                                       "      <arg direction=\"out\" type=\"i\" name=\"result\"/>\n"
                                       "    </signal>\n"
                                       "    <signal name=\"accepted\"/>\n"
                                       "    <signal name=\"rejected\"/>\n"
                                       "    <signal name=\"windowActiveChanged\"/>\n"
                                       "    <signal name=\"destroyed\"/>\n"
                                       "    <signal name=\"selectionFilesChanged\"/>\n"
                                       "    <signal name=\"currentUrlChanged\"/>\n"
                                       "    <signal name=\"directoryChanged\"/>\n"
                                       "    <signal name=\"directoryUrlChanged\"/>\n"
                                       "    <signal name=\"selectedNameFilterChanged\"/>\n"
                                       "  </interface>\n"
                                       "")
    Q_PROPERTY(int acceptMode READ acceptMode WRITE setAcceptMode)
    Q_PROPERTY(QString directory READ directory WRITE setDirectory)
    Q_PROPERTY(QString directoryUrl READ directoryUrl WRITE setDirectoryUrl)
    Q_PROPERTY(int filter READ filter WRITE setFilter)
    Q_PROPERTY(int heartbeatInterval READ heartbeatInterval WRITE setHeartbeatInterval)
    Q_PROPERTY(bool hideOnAccept READ hideOnAccept WRITE setHideOnAccept)
    Q_PROPERTY(QStringList nameFilters READ nameFilters WRITE setNameFilters)
    Q_PROPERTY(int viewMode READ viewMode WRITE setViewMode)
    Q_PROPERTY(bool windowActive READ windowActive)
    Q_PROPERTY(uint windowFlags READ windowFlags WRITE setWindowFlags)

public:
    inline int acceptMode() const
    { return m_acceptMode; }
    inline void setAcceptMode(int value)
    { m_acceptMode = value; }

    inline QString directory() const
    { return directoryUrl(); }
    inline void setDirectory(const QString &value)
    {
        setDirectoryUrl(value);
        Q_EMIT this->directoryChanged();
    }

    inline QString directoryUrl() const
    { return m_directoryUrl; }
    inline void setDirectoryUrl(const QString &value)
    {
        m_directoryUrl = value;
        Q_EMIT this->directoryUrlChanged();
    }

    inline int filter() const
    { return m_filter; }
    inline void setFilter(int value)
    {
        m_filter = value;
    }

    inline int heartbeatInterval() const
    { return m_heartbeatInterval; }
    inline void setHeartbeatInterval(int value)
    { m_heartbeatInterval = value; }

    inline bool hideOnAccept() const
    { return m_hideOnAccept; }
    inline void setHideOnAccept(bool value)
    {
        m_hideOnAccept = value;
    }

    inline QStringList nameFilters() const
    { return m_nameFilters; }
    inline void setNameFilters(const QStringList &value)
    { m_nameFilters = value; }

    inline int viewMode() const
    { return m_viewMode; }
    inline void setViewMode(int value)
    { m_viewMode = value; }

    inline bool windowActive() const
    { return m_windowActive; }

    inline uint windowFlags() const
    { return m_windowFlags; }
    inline void setWindowFlags(uint value)
    { m_windowFlags = value; }

public Q_SLOTS:
    void accept();
    void activateWindow();
    void addCustomWidget(int type, const QString &data);
    void addDisableUrlScheme(const QString &type);
    QMap<QString, QVariant> allCustomWidgetsValue(int type);
    void beginAddCustomWidget();
    void endAddCustomWidget();
    QDBusVariant getCustomWidgetValue(int type, const QString &text);
    void hide();
    QString labelText(int label);
    void makeHeartbeat();
    void open();
    int options();
    void reject();
    void selectFile(const QString &filename);
    void selectNameFilter(const QString &filter);
    void selectNameFilterByIndex(int index);
    void selectUrl(const QString &url);
    QStringList selectedFiles();
    QString selectedNameFilter();
    int selectedNameFilterIndex();
    QStringList selectedUrls();
    void setAllowMixedSelection(bool on);
    void setCurrentInputName(const QString &name);
    void setFileMode(int fileMode);
    void setLabelText(int label, const QString &text);
    void setOption(int option, bool on);
    void setOptions(int options);
    void setWindowTitle(const QString &title);
    void show();
    bool testOption(int option);
    qulonglong winId();

Q_SIGNALS: // SIGNALS
    void accepted();
    void currentUrlChanged();
    void destroyed();
    void directoryChanged();
    void directoryUrlChanged();
    void finished(int result);
    void rejected();
    void selectedNameFilterChanged();
    void selectionFilesChanged();
    void windowActiveChanged();

public:
    FileDialogService(const QString &path);
    ~FileDialogService();

    QString m_objPath;
    bool    m_online;
    bool    m_accepted;
    bool    m_rejected;
    bool    m_visible;
    QList<QUrl> m_selectedFiles;

    int m_acceptMode;
    QString m_directoryUrl;
    int m_filter;
    int m_heartbeatInterval;
    bool m_hideOnAccept;
    QStringList m_nameFilters;
    int m_viewMode;
    bool m_windowActive;
    uint m_windowFlags;





};
#endif  // FILEDIALOGSERVICE_H
