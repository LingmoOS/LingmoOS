/*
    SPDX-FileCopyrightText: 2008 Paul Giannaros <paul@giannaros.org>
    SPDX-FileCopyrightText: 2009-2018 Dominik Haumann <dhaumann@kde.org>
    SPDX-FileCopyrightText: 2010 Joseph Wenninger <jowenn@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "katescript.h"

#include "katepartdebug.h"
#include "katescriptdocument.h"
#include "katescripteditor.h"
#include "katescripthelpers.h"
#include "katescriptview.h"
#include "kateview.h"

#include <KLocalizedString>
#include <iostream>

#include <QFile>
#include <QFileInfo>
#include <QJSEngine>
#include <QQmlEngine>

KateScript::KateScript(const QString &urlOrScript, enum InputType inputType)
    : m_url(inputType == InputURL ? urlOrScript : QString())
    , m_inputType(inputType)
    , m_script(inputType == InputSCRIPT ? urlOrScript : QString())
{
}

KateScript::~KateScript()
{
    if (m_loadSuccessful) {
        // remove data...
        delete m_editor;
        delete m_document;
        delete m_view;
        delete m_engine;
    }
}

QString KateScript::backtrace(const QJSValue &error, const QString &header)
{
    QString bt;
    if (!header.isNull()) {
        bt += header + QLatin1String(":\n");
    }
    if (error.isError()) {
        bt += error.toString() + QLatin1String("\nStrack trace:\n") + error.property(QStringLiteral("stack")).toString();
    }

    return bt;
}

void KateScript::displayBacktrace(const QJSValue &error, const QString &header)
{
    if (!m_engine) {
        std::cerr << "KateScript::displayBacktrace: no engine, cannot display error\n";
        return;
    }
    std::cerr << "\033[31m" << qPrintable(backtrace(error, header)) << "\033[0m" << '\n';
}

void KateScript::clearExceptions()
{
    if (!load()) {
        return;
    }
}

QJSValue KateScript::global(const QString &name)
{
    // load the script if necessary
    if (!load()) {
        return QJSValue::UndefinedValue;
    }
    return m_engine->globalObject().property(name);
}

QJSValue KateScript::function(const QString &name)
{
    QJSValue value = global(name);
    if (!value.isCallable()) {
        return QJSValue::UndefinedValue;
    }
    return value;
}

bool KateScript::load()
{
    if (m_loaded) {
        return m_loadSuccessful;
    }

    m_loaded = true;
    m_loadSuccessful = false; // here set to false, and at end of function to true

    // read the script file into memory
    QString source;
    if (m_inputType == InputURL) {
        if (!Kate::Script::readFile(m_url, source)) {
            return false;
        }
    } else {
        source = m_script;
    }

    // create script engine, register meta types
    m_engine = new QJSEngine();

    // export read & require function and add the require guard object
    auto scriptHelper = new Kate::ScriptHelper(m_engine);
    QJSValue functions = m_engine->newQObject(scriptHelper);
    m_engine->globalObject().setProperty(QStringLiteral("functions"), functions);
    m_engine->globalObject().setProperty(QStringLiteral("read"), functions.property(QStringLiteral("read")));
    m_engine->globalObject().setProperty(QStringLiteral("require"), functions.property(QStringLiteral("require")));
    m_engine->globalObject().setProperty(QStringLiteral("require_guard"), m_engine->newObject());

    // View and Document expose JS Range objects in the API, which will fail to work
    // if Range is not included. range.js includes cursor.js
    scriptHelper->require(QStringLiteral("range.js"));

    // export debug function
    m_engine->globalObject().setProperty(QStringLiteral("debug"), functions.property(QStringLiteral("debug")));

    // export translation functions
    m_engine->globalObject().setProperty(QStringLiteral("i18n"), functions.property(QStringLiteral("_i18n")));
    m_engine->globalObject().setProperty(QStringLiteral("i18nc"), functions.property(QStringLiteral("_i18nc")));
    m_engine->globalObject().setProperty(QStringLiteral("i18np"), functions.property(QStringLiteral("_i18np")));
    m_engine->globalObject().setProperty(QStringLiteral("i18ncp"), functions.property(QStringLiteral("_i18ncp")));

    // register default styles as ds* global properties
    m_engine->globalObject().setProperty(QStringLiteral("dsNormal"), KSyntaxHighlighting::Theme::TextStyle::Normal);
    m_engine->globalObject().setProperty(QStringLiteral("dsKeyword"), KSyntaxHighlighting::Theme::TextStyle::Keyword);
    m_engine->globalObject().setProperty(QStringLiteral("dsFunction"), KSyntaxHighlighting::Theme::TextStyle::Function);
    m_engine->globalObject().setProperty(QStringLiteral("dsVariable"), KSyntaxHighlighting::Theme::TextStyle::Variable);
    m_engine->globalObject().setProperty(QStringLiteral("dsControlFlow"), KSyntaxHighlighting::Theme::TextStyle::ControlFlow);
    m_engine->globalObject().setProperty(QStringLiteral("dsOperator"), KSyntaxHighlighting::Theme::TextStyle::Operator);
    m_engine->globalObject().setProperty(QStringLiteral("dsBuiltIn"), KSyntaxHighlighting::Theme::TextStyle::BuiltIn);
    m_engine->globalObject().setProperty(QStringLiteral("dsExtension"), KSyntaxHighlighting::Theme::TextStyle::Extension);
    m_engine->globalObject().setProperty(QStringLiteral("dsPreprocessor"), KSyntaxHighlighting::Theme::TextStyle::Preprocessor);
    m_engine->globalObject().setProperty(QStringLiteral("dsAttribute"), KSyntaxHighlighting::Theme::TextStyle::Attribute);
    m_engine->globalObject().setProperty(QStringLiteral("dsChar"), KSyntaxHighlighting::Theme::TextStyle::Char);
    m_engine->globalObject().setProperty(QStringLiteral("dsSpecialChar"), KSyntaxHighlighting::Theme::TextStyle::SpecialChar);
    m_engine->globalObject().setProperty(QStringLiteral("dsString"), KSyntaxHighlighting::Theme::TextStyle::String);
    m_engine->globalObject().setProperty(QStringLiteral("dsVerbatimString"), KSyntaxHighlighting::Theme::TextStyle::VerbatimString);
    m_engine->globalObject().setProperty(QStringLiteral("dsSpecialString"), KSyntaxHighlighting::Theme::TextStyle::SpecialString);
    m_engine->globalObject().setProperty(QStringLiteral("dsImport"), KSyntaxHighlighting::Theme::TextStyle::Import);
    m_engine->globalObject().setProperty(QStringLiteral("dsDataType"), KSyntaxHighlighting::Theme::TextStyle::DataType);
    m_engine->globalObject().setProperty(QStringLiteral("dsDecVal"), KSyntaxHighlighting::Theme::TextStyle::DecVal);
    m_engine->globalObject().setProperty(QStringLiteral("dsBaseN"), KSyntaxHighlighting::Theme::TextStyle::BaseN);
    m_engine->globalObject().setProperty(QStringLiteral("dsFloat"), KSyntaxHighlighting::Theme::TextStyle::Float);
    m_engine->globalObject().setProperty(QStringLiteral("dsConstant"), KSyntaxHighlighting::Theme::TextStyle::Constant);
    m_engine->globalObject().setProperty(QStringLiteral("dsComment"), KSyntaxHighlighting::Theme::TextStyle::Comment);
    m_engine->globalObject().setProperty(QStringLiteral("dsDocumentation"), KSyntaxHighlighting::Theme::TextStyle::Documentation);
    m_engine->globalObject().setProperty(QStringLiteral("dsAnnotation"), KSyntaxHighlighting::Theme::TextStyle::Annotation);
    m_engine->globalObject().setProperty(QStringLiteral("dsCommentVar"), KSyntaxHighlighting::Theme::TextStyle::CommentVar);
    m_engine->globalObject().setProperty(QStringLiteral("dsRegionMarker"), KSyntaxHighlighting::Theme::TextStyle::RegionMarker);
    m_engine->globalObject().setProperty(QStringLiteral("dsInformation"), KSyntaxHighlighting::Theme::TextStyle::Information);
    m_engine->globalObject().setProperty(QStringLiteral("dsWarning"), KSyntaxHighlighting::Theme::TextStyle::Warning);
    m_engine->globalObject().setProperty(QStringLiteral("dsAlert"), KSyntaxHighlighting::Theme::TextStyle::Alert);
    m_engine->globalObject().setProperty(QStringLiteral("dsOthers"), KSyntaxHighlighting::Theme::TextStyle::Others);
    m_engine->globalObject().setProperty(QStringLiteral("dsError"), KSyntaxHighlighting::Theme::TextStyle::Error);

    // register scripts itself
    QJSValue result = m_engine->evaluate(source, m_url);
    if (hasException(result, m_url)) {
        return false;
    }

    // AFTER SCRIPT: set the view/document objects as necessary
    m_engine->globalObject().setProperty(QStringLiteral("editor"), m_engine->newQObject(m_editor = new KateScriptEditor()));
    m_engine->globalObject().setProperty(QStringLiteral("document"), m_engine->newQObject(m_document = new KateScriptDocument(m_engine)));
    m_engine->globalObject().setProperty(QStringLiteral("view"), m_engine->newQObject(m_view = new KateScriptView(m_engine)));

    // yip yip!
    m_loadSuccessful = true;

    return true;
}

QJSValue KateScript::evaluate(const QString &program, const FieldMap &env)
{
    if (!load()) {
        qCWarning(LOG_KTE) << "load of script failed:" << program;
        return QJSValue();
    }

    // Wrap the arguments in a function to avoid polluting the global object
    QString programWithContext =
        QLatin1String("(function(") + QStringList(env.keys()).join(QLatin1Char(',')) + QLatin1String(") { return ") + program + QLatin1String("})");
    QJSValue programFunction = m_engine->evaluate(programWithContext);
    Q_ASSERT(programFunction.isCallable());

    QJSValueList args;
    args.reserve(env.size());
    for (auto it = env.begin(); it != env.end(); it++) {
        args << it.value();
    }

    QJSValue result = programFunction.call(args);
    if (result.isError()) {
        qCWarning(LOG_KTE) << "Error evaluating script: " << result.toString();
    }

    return result;
}

bool KateScript::hasException(const QJSValue &object, const QString &file)
{
    if (object.isError()) {
        m_errorMessage = i18n("Error loading script %1", file);
        displayBacktrace(object, m_errorMessage);
        delete m_engine;
        m_engine = nullptr;
        m_loadSuccessful = false;
        return true;
    }
    return false;
}

bool KateScript::setView(KTextEditor::ViewPrivate *view)
{
    if (!load()) {
        return false;
    }
    // setup the stuff
    m_document->setDocument(view->doc());
    m_view->setView(view);
    return true;
}

void KateScript::setGeneralHeader(const KateScriptHeader &generalHeader)
{
    m_generalHeader = generalHeader;
}

KateScriptHeader &KateScript::generalHeader()
{
    return m_generalHeader;
}
