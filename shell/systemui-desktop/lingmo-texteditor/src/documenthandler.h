/****************************************************************************
 * *
 ** Copyright (C) 2017 The Qt Company Ltd.
 ** Contact: https://www.qt.io/licensing/
 **
 ** This file is part of the examples of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:BSD$
 ** Commercial License Usage
 ** Licensees holding valid commercial Qt licenses may use this file in
 ** accordance with the commercial license agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
 ** a written agreement between you and The Qt Company. For licensing terms
 ** and conditions see https://www.qt.io/terms-conditions. For further
 ** information use the contact form at https://www.qt.io/contact-us.
 **
 ** BSD License Usage
 ** Alternatively, you may use this file under the terms of the BSD license
 ** as follows:
 **
 ** "Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions are
 ** met:
 **   * Redistributions of source code must retain the above copyright
 **     notice, this list of conditions and the following disclaimer.
 **   * Redistributions in binary form must reproduce the above copyright
 **     notice, this list of conditions and the following disclaimer in
 **     the documentation and/or other materials provided with the
 **     distribution.
 **   * Neither the name of The Qt Company Ltd nor the names of its
 **     contributors may be used to endorse or promote products derived
 **     from this software without specific prior written permission.
 **
 **
 ** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 ** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 ** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 ** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 ** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 ** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 ** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 ** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 ** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 ** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

#ifndef DOCUMENTHANDLER_H
#define DOCUMENTHANDLER_H

#include <QAbstractListModel>
#include <QDebug>
#include <QFont>
#include <QObject>
#include <QTextCursor>
#include <QThread>
#include <QTimer>
#include <QUrl>

QT_BEGIN_NAMESPACE
class QFileSystemWatcher;
class QTextDocument;
class QQuickTextDocument;
QT_END_NAMESPACE

namespace KSyntaxHighlighting
{
class Repository;
class SyntaxHighlighter;
}

/**
 * @brief The AlertAction struct
 */
struct AlertAction {
    QString label;
    std::function<void()> action;
};

/**
 * @brief The DocumentAlert class
 */
class DocumentAlert : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString title MEMBER m_title CONSTANT FINAL)
    Q_PROPERTY(QString body MEMBER m_body CONSTANT FINAL)
    Q_PROPERTY(uint level MEMBER m_level CONSTANT FINAL)
    Q_PROPERTY(QStringList actionLabels READ actionLabels CONSTANT FINAL)

public:
    enum LEVEL : uint { INFO_LEVEL = 0, WARNING_LEVEL = 1, DANGER_LEVEL = 2 };

    DocumentAlert(const QString &title, const QString &body, const uint &level, const int &id, QObject *parent = nullptr)
        : QObject(parent)
    {
        this->m_title = title;
        this->m_body = body;
        this->m_level = level;
        this->m_id = id;
    }

    /**
     * @brief setIndex
     * @param index
     */
    void setIndex(const int &index)
    {
        this->m_index = index;
    }

    /**
     * @brief setActions
     * @param actions
     */
    void setActions(QVector<AlertAction> actions)
    {
        this->m_actions = actions;
    }

    /**
     * @brief getId
     * @return
     */
    int getId() const
    {
        return this->m_id;
    }

    /**
     * @brief operator ==
     * @param other
     * @param other2
     * @return
     */
    friend bool operator==(const DocumentAlert &other, const DocumentAlert &other2)
    {
        return other.getId() == other2.getId();
    }

    /**
     * @brief actionLabels
     * @return
     */
    QStringList actionLabels() const
    {
        return std::accumulate(this->m_actions.constBegin(), this->m_actions.constEnd(), QStringList(), [](QStringList &labels, const AlertAction &action) -> QStringList {
            labels << action.label;
            return labels;
        });
    }

private:
    QString m_title;
    QString m_body;
    uint m_level;
    int m_index = -1;
    int m_id = -1;

    QVector<AlertAction> m_actions;

public slots:
    /**
     * @brief triggerAction
     * @param actionIndex
     * @param alertIndex
     */
    void triggerAction(const int &actionIndex, const int &alertIndex)
    {
        qDebug() << "TRIGGERING DOCUMENT ACTION AT INDEX << " << actionIndex << alertIndex;
        this->m_actions.at(actionIndex).action();
        emit this->done(alertIndex);
    }

signals:
    /**
     * @brief done
     * @param index
     */
    void done(int index);
};

/**
 * @brief The Alerts class
 */
class Alerts : public QAbstractListModel
{
    Q_OBJECT

public:
    enum ROLES : uint { ALERT = Qt::DisplayRole + 1 };

    enum ALERT_TYPES : uint { MISSING, UNSAVED, MODIFIED, SAVE_ERROR };

    explicit Alerts(QObject *parent = nullptr);
    ~Alerts();
    QVariant data(const QModelIndex &index, int role) const override final;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override final;
    QHash<int, QByteArray> roleNames() const override;

    void append(DocumentAlert *alert);

private:
    QVector<DocumentAlert *> m_alerts;
    bool contains(DocumentAlert *const alert);
};

/**
 * @brief The FileLoader class
 */
class FileLoader : public QObject
{
    Q_OBJECT

public slots:
    /**
     * @brief loadFile
     * @param url
     */
    void loadFile(const QUrl &url);

signals:
    /**
     * @brief fileReady
     * @param array
     * @param url
     */
    void fileReady(QString array, QUrl url);
};

/**
 * @brief The DocumentHandler class
 */
class DocumentHandler : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQuickTextDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(int cursorPosition READ cursorPosition WRITE setCursorPosition NOTIFY cursorPositionChanged)
    Q_PROPERTY(int selectionStart READ selectionStart WRITE setSelectionStart NOTIFY selectionStartChanged)
    Q_PROPERTY(int selectionEnd READ selectionEnd WRITE setSelectionEnd NOTIFY selectionEndChanged)

    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY fontFamilyChanged)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment NOTIFY alignmentChanged)

    Q_PROPERTY(bool bold READ bold WRITE setBold NOTIFY boldChanged)
    Q_PROPERTY(bool uppercase READ uppercase WRITE setUppercase NOTIFY uppercaseChanged)
    Q_PROPERTY(bool italic READ italic WRITE setItalic NOTIFY italicChanged)
    Q_PROPERTY(bool underline READ underline WRITE setUnderline NOTIFY underlineChanged)
    Q_PROPERTY(bool isRich READ getIsRich NOTIFY isRichChanged)

    Q_PROPERTY(int lineCount READ lineCount NOTIFY lineCountChanged)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)
    Q_PROPERTY(qreal tabSpace READ tabSpace WRITE setTabSpace NOTIFY tabSpaceChanged)

    Q_PROPERTY(QString fileName READ fileName NOTIFY fileUrlChanged)
    Q_PROPERTY(QString fileType READ fileType NOTIFY fileUrlChanged)

    Q_PROPERTY(QVariantMap fileInfo READ fileInfo NOTIFY fileInfoChanged)
    Q_PROPERTY(QUrl fileUrl READ fileUrl WRITE setFileUrl NOTIFY fileUrlChanged)

    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)

    Q_PROPERTY(bool externallyModified READ getExternallyModified WRITE setExternallyModified NOTIFY externallyModifiedChanged)

    Q_PROPERTY(bool modified READ getModified NOTIFY modifiedChanged)

    Q_PROPERTY(bool autoReload READ getAutoReload WRITE setAutoReload NOTIFY autoReloadChanged)
    Q_PROPERTY(bool autoSave READ autoSave WRITE setAutoSave NOTIFY autoSaveChanged)

    Q_PROPERTY(QString formatName READ formatName WRITE setFormatName NOTIFY formatNameChanged)

    Q_PROPERTY(int currentLineIndex READ getCurrentLineIndex NOTIFY currentLineIndexChanged)

    Q_PROPERTY(Alerts *alerts READ getAlerts CONSTANT FINAL)

    Q_PROPERTY(QColor backgroundColor READ getBackgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)

    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)

    Q_PROPERTY(bool enableSyntaxHighlighting READ enableSyntaxHighlighting WRITE setEnableSyntaxHighlighting NOTIFY enableSyntaxHighlightingChanged)

    Q_PROPERTY(bool findWholeWords MEMBER m_findWholeWords NOTIFY findWholeWordsChanged)

    Q_PROPERTY(bool findCaseSensitively MEMBER m_findCaseSensitively NOTIFY findCaseSensitivelyChanged)

    Q_PROPERTY(QString fileName MEMBER m_fileName NOTIFY fileNameChanged)

public:
    explicit DocumentHandler(QObject *parent = nullptr);
    ~DocumentHandler();

    /**
     * @brief document
     * @return
     */
    QQuickTextDocument *document() const;

    /**
     * @brief setDocument
     * @param document
     */
    void setDocument(QQuickTextDocument *document);

    /**
     * @brief cursorPosition
     * @return
     */
    int cursorPosition() const;

    /**
     * @brief setCursorPosition
     * @param position
     */
    void setCursorPosition(int position);

    /**
     * @brief selectionStart
     * @return
     */
    int selectionStart() const;

    /**
     * @brief setSelectionStart
     * @param position
     */
    void setSelectionStart(int position);

    /**
     * @brief selectionEnd
     * @return
     */
    int selectionEnd() const;

    /**
     * @brief setSelectionEnd
     * @param position
     */
    void setSelectionEnd(int position);

    /**
     * @brief fontFamily
     * @return
     */
    QString fontFamily() const;

    /**
     * @brief setFontFamily
     * @param family
     */
    void setFontFamily(const QString &family);

    /**
     * @brief textColor
     * @return
     */
    QColor textColor() const;

    /**
     * @brief setTextColor
     * @param color
     */
    void setTextColor(const QColor &color);

    /**
     * @brief alignment
     * @return
     */
    Qt::Alignment alignment() const;

    /**
     * @brief setAlignment
     * @param alignment
     */
    void setAlignment(Qt::Alignment alignment);

    /**
     * @brief bold
     * @return
     */
    bool bold() const;

    /**
     * @brief setBold
     * @param bold
     */
    void setBold(bool bold);

    /**
     * @brief uppercase
     * @return
     */
    bool uppercase() const;

    /**
     * @brief setUppercase
     * @param uppercase
     */
    void setUppercase(bool uppercase);

    /**
     * @brief italic
     * @return
     */
    bool italic() const;

    /**
     * @brief setItalic
     * @param italic
     */
    void setItalic(bool italic);

    /**
     * @brief underline
     * @return
     */
    bool underline() const;

    /**
     * @brief setUnderline
     * @param underline
     */
    void setUnderline(bool underline);

    /**
     * @brief getIsRich
     * @return
     */
    bool getIsRich() const;

    /**
     * @brief fontSize
     * @return
     */
    int fontSize() const;

    /**
     * @brief setFontSize
     * @param size
     */
    void setFontSize(int size);

    qreal tabSpace() const;
    void setTabSpace(qreal value);

    /**
     * @brief fileName
     * @return
     */
    QString fileName() const;

    /**
     * @brief fileType
     * @return
     */
    QString fileType() const;

    /**
     * @brief fileUrl
     * @return
     */
    QUrl fileUrl() const;

    /**
     * @brief setFileUrl
     * @param url
     */
    void setFileUrl(const QUrl &url);

    /**
     * @brief fileInfo
     * @return
     */
    QVariantMap fileInfo() const;

    /**
     * @brief text
     * @return
     */
    inline QString text() const
    {
        return m_text;
    }

    /**
     * @brief setText
     * @param text
     */
    void setText(const QString &text);

    /**
     * @brief getAutoReload
     * @return
     */
    bool getAutoReload() const;

    /**
     * @brief setAutoReload
     * @param value
     */
    void setAutoReload(const bool &value);

    /**
     * @brief autoSave
     * @return
     */
    bool autoSave() const;

    /**
     * @brief setAutoSave
     * @param value
     */
    void setAutoSave(const bool &value);

    /**
     * @brief getModified
     * @return
     */
    bool getModified() const;

    /**
     * @brief getExternallyModified
     * @return
     */
    bool getExternallyModified() const;

    /**
     * @brief setExternallyModified
     * @param value
     */
    void setExternallyModified(const bool &value);

    /**
     * @brief formatName
     * @return
     */
    QString formatName() const;

    /**
     * @brief setFormatName
     * @param formatName
     */
    void setFormatName(const QString &formatName);

    /**
     * @brief getBackgroundColor
     * @return
     */
    QColor getBackgroundColor() const;

    /**
     * @brief setBackgroundColor
     * @param color
     */
    void setBackgroundColor(const QColor &color);

    /**
     * @brief getAlerts
     * @return
     */
    Alerts *getAlerts() const;

    /**
     * @brief theme
     * @return
     */
    QString theme() const;

    /**
     * @brief setTheme
     * @param theme
     */
    void setTheme(const QString &theme);

    /**
     * @brief enableSyntaxHighlighting
     * @return
     */
    bool enableSyntaxHighlighting() const;

    /**
     * @brief setEnableSyntaxHighlighting
     * @param value
     */
    void setEnableSyntaxHighlighting(const bool &value);

    /**
     * @brief isDark
     * @param color
     * @return
     */
    static bool isDark(const QColor &color)
    {
        const double darkness = 1 - (0.299 * color.red() + 0.587 * color.green() + 0.114 * color.blue()) / 255;
        return (darkness > 0.5);
    }

    QString fileName();

public slots:
    /**
     * @brief load
     * @param url
     */
    void load(const QUrl &url);

    /**
     * @brief saveAs
     * @param url
     */
    void saveAs(const QUrl &url);

    /**
     * @brief find
     * @param query
     */
    void find(const QString &query, const bool &forward = true);

    void replace(const QString &query, const QString &value);

    void replaceAll(const QString &query, const QString &value);

    bool isFoldable(const int &line) const;
    bool isFolded(const int &line) const;
    void toggleFold(const int &line);

    /**
     * @brief lineHeight
     * @param line
     * @return
     */
    int lineHeight(const int &line);

    int lineCount();

    /**
     * @brief getCurrentLineIndex
     * @return
     */
    int getCurrentLineIndex();

    int goToLine(const int &line);

    /**
     * @brief getLanguageNameList
     * @return
     */
    static const QStringList getLanguageNameList();

    /**
     * @brief getLanguageNameFromFileName
     * @param fileName
     * @return
     */
    static const QString getLanguageNameFromFileName(const QUrl &fileName);

    /**
     * @brief getThemes
     * @return
     */
    static const QStringList getThemes();

signals:
    void documentChanged();
    void fileSaved();

    void cursorPositionChanged();
    void selectionStartChanged();
    void selectionEndChanged();

    void fontFamilyChanged();
    void textColorChanged();
    void alignmentChanged();

    void boldChanged();
    void uppercaseChanged();
    void italicChanged();
    void underlineChanged();
    void isRichChanged();

    void lineCountChanged();
    void fontSizeChanged();
    void tabSpaceChanged();

    void textChanged();
    void fileUrlChanged();
    void fileInfoChanged();

    void loaded(const QUrl &url);
    void error(const QString &message);
    void loadFile(QUrl url);

    void autoReloadChanged();
    void autoSaveChanged();

    void externallyModifiedChanged();

    void backgroundColorChanged();

    void formatNameChanged();

    void modifiedChanged();

    void currentLineIndexChanged();

    void enableSyntaxHighlightingChanged();
    void themeChanged();

    void searchFound(int start, int end);
    void findCaseSensitivelyChanged();
    void findWholeWordsChanged();
//     void cursorYPosChanged();

    void fileNameChanged();

private:
    void reset();
    void setStyle();

    QTextCursor textCursor() const;
    QTextDocument *textDocument() const;
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);

    QQuickTextDocument *m_document;
    QFileSystemWatcher *m_watcher;

    int m_cursorPosition;
    int m_selectionStart;
    int m_selectionEnd;

    bool isRich = false;

    QFont m_font;
    int m_fontSize = 12;
    qreal m_tabSpace = 8;

    QUrl m_fileUrl;

    QThread m_worker;
    QString m_text;

    bool m_autoReload = false;
    bool m_autoSave = false;

    bool m_externallyModified = false;
    bool m_internallyModified = false;

    bool m_findCaseSensitively = false;
    bool m_findWholeWords = false;

    QColor m_backgroundColor;

    static int m_instanceCount;
    QString m_formatName = "None";
    static KSyntaxHighlighting::Repository *m_repository;
    KSyntaxHighlighting::SyntaxHighlighter *m_highlighter;

    bool m_enableSyntaxHighlighting = false;
    QString m_theme;

    QString m_searchQuery;
    QString m_replaceText;

    Alerts *m_alerts;
    DocumentAlert *missingAlert();
    DocumentAlert *externallyModifiedAlert();
    DocumentAlert *canNotSaveAlert(const QString &details);

    QTimer m_autoSaveTimer;

    QString m_fileName;

    void refreshAllBlocks();
};

#endif // DOCUMENTHANDLER_H
