/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Chusslove Illich <caslav.ilic@gmx.net>
    SPDX-FileCopyrightText: 2014 Kevin Krammer <krammer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <common_helpers_p.h>
#include <ktranscript_p.h>

#include <ktranscript_export.h>

//#include <unistd.h>

#include <QJSEngine>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QIODevice>
#include <QJSValueIterator>
#include <QList>
#include <QSet>
#include <QStandardPaths>
#include <QStringList>
#include <QTextStream>
#include <QVariant>
#include <qendian.h>

class KTranscriptImp;
class Scriptface;

typedef QHash<QString, QString> TsConfigGroup;
typedef QHash<QString, TsConfigGroup> TsConfig;

// Transcript implementation (used as singleton).
class KTranscriptImp : public KTranscript
{
public:
    KTranscriptImp();
    ~KTranscriptImp() override;

    QString eval(const QList<QVariant> &argv,
                 const QString &lang,
                 const QString &ctry,
                 const QString &msgctxt,
                 const QHash<QString, QString> &dynctxt,
                 const QString &msgid,
                 const QStringList &subs,
                 const QList<QVariant> &vals,
                 const QString &ftrans,
                 QList<QStringList> &mods,
                 QString &error,
                 bool &fallback) override;

    QStringList postCalls(const QString &lang) override;

    // Lexical path of the module for the executing code.
    QString currentModulePath;

private:
    void loadModules(const QList<QStringList> &mods, QString &error);
    void setupInterpreter(const QString &lang);

    TsConfig config;

    QHash<QString, Scriptface *> m_sface;
};

// Script-side transcript interface.
class Scriptface : public QObject
{
    Q_OBJECT
public:
    explicit Scriptface(const TsConfigGroup &config, QObject *parent = nullptr);
    ~Scriptface();

    // Interface functions.
    Q_INVOKABLE QJSValue load(const QString &name);
    Q_INVOKABLE QJSValue setcall(const QJSValue &name, const QJSValue &func, const QJSValue &fval = QJSValue::NullValue);
    Q_INVOKABLE QJSValue hascall(const QString &name);
    Q_INVOKABLE QJSValue acallInternal(const QJSValue &args);
    Q_INVOKABLE QJSValue setcallForall(const QJSValue &name, const QJSValue &func, const QJSValue &fval = QJSValue::NullValue);
    Q_INVOKABLE QJSValue fallback();
    Q_INVOKABLE QJSValue nsubs();
    Q_INVOKABLE QJSValue subs(const QJSValue &index);
    Q_INVOKABLE QJSValue vals(const QJSValue &index);
    Q_INVOKABLE QJSValue msgctxt();
    Q_INVOKABLE QJSValue dynctxt(const QString &key);
    Q_INVOKABLE QJSValue msgid();
    Q_INVOKABLE QJSValue msgkey();
    Q_INVOKABLE QJSValue msgstrf();
    Q_INVOKABLE void dbgputs(const QString &str);
    Q_INVOKABLE void warnputs(const QString &str);
    Q_INVOKABLE QJSValue localeCountry();
    Q_INVOKABLE QJSValue normKey(const QJSValue &phrase);
    Q_INVOKABLE QJSValue loadProps(const QString &name);
    Q_INVOKABLE QJSValue getProp(const QJSValue &phrase, const QJSValue &prop);
    Q_INVOKABLE QJSValue setProp(const QJSValue &phrase, const QJSValue &prop, const QJSValue &value);
    Q_INVOKABLE QJSValue toUpperFirst(const QJSValue &str, const QJSValue &nalt = QJSValue::NullValue);
    Q_INVOKABLE QJSValue toLowerFirst(const QJSValue &str, const QJSValue &nalt = QJSValue::NullValue);
    Q_INVOKABLE QJSValue getConfString(const QJSValue &key, const QJSValue &dval = QJSValue::NullValue);
    Q_INVOKABLE QJSValue getConfBool(const QJSValue &key, const QJSValue &dval = QJSValue::NullValue);
    Q_INVOKABLE QJSValue getConfNumber(const QJSValue &key, const QJSValue &dval = QJSValue::NullValue);

    // Helper methods to interface functions.
    QJSValue load(const QJSValueList &names);
    QJSValue loadProps(const QJSValueList &names);
    QString loadProps_text(const QString &fpath);
    QString loadProps_bin(const QString &fpath);
    QString loadProps_bin_00(const QString &fpath);
    QString loadProps_bin_01(const QString &fpath);

    void put(const QString &propertyName, const QJSValue &value);

    // Link to its script engine
    QJSEngine *const scriptEngine;

    // Current message data.
    const QString *msgcontext;
    const QHash<QString, QString> *dyncontext;
    const QString *msgId;
    const QStringList *subList;
    const QList<QVariant> *valList;
    const QString *ftrans;
    const QString *ctry;

    // Fallback request handle.
    bool *fallbackRequest;

    // Function register.
    QHash<QString, QJSValue> funcs;
    QHash<QString, QJSValue> fvals;
    QHash<QString, QString> fpaths;

    // Ordering of those functions which execute for all messages.
    QList<QString> nameForalls;

    // Property values per phrase (used by *Prop interface calls).
    // Not QStrings, in order to avoid conversion from UTF-8 when
    // loading compiled maps (less latency on startup).
    QHash<QByteArray, QHash<QByteArray, QByteArray>> phraseProps;
    // Unresolved property values per phrase,
    // containing the pointer to compiled pmap file handle and offset in it.
    struct UnparsedPropInfo {
        QFile *pmapFile = nullptr;
        quint64 offset = -1;
    };
    QHash<QByteArray, UnparsedPropInfo> phraseUnparsedProps;
    QHash<QByteArray, QByteArray> resolveUnparsedProps(const QByteArray &phrase);
    // Set of loaded pmap files by paths and file handle pointers.
    QSet<QString> loadedPmapPaths;
    QSet<QFile *> loadedPmapHandles;

    // User config.
    TsConfigGroup config;
};

// ----------------------------------------------------------------------
// Custom debug and warning output (kdebug not available)
#define DBGP "KTranscript: "
void dbgout(const char *str)
{
#ifndef NDEBUG
    fprintf(stderr, DBGP "%s\n", str);
#else
    Q_UNUSED(str);
#endif
}
template<typename T1>
void dbgout(const char *str, const T1 &a1)
{
#ifndef NDEBUG
    fprintf(stderr, DBGP "%s\n", QString::fromUtf8(str).arg(a1).toLocal8Bit().data());
#else
    Q_UNUSED(str);
    Q_UNUSED(a1);
#endif
}
template<typename T1, typename T2>
void dbgout(const char *str, const T1 &a1, const T2 &a2)
{
#ifndef NDEBUG
    fprintf(stderr, DBGP "%s\n", QString::fromUtf8(str).arg(a1).arg(a2).toLocal8Bit().data());
#else
    Q_UNUSED(str);
    Q_UNUSED(a1);
    Q_UNUSED(a2);
#endif
}
template<typename T1, typename T2, typename T3>
void dbgout(const char *str, const T1 &a1, const T2 &a2, const T3 &a3)
{
#ifndef NDEBUG
    fprintf(stderr, DBGP "%s\n", QString::fromUtf8(str).arg(a1).arg(a2).arg(a3).toLocal8Bit().data());
#else
    Q_UNUSED(str);
    Q_UNUSED(a1);
    Q_UNUSED(a2);
    Q_UNUSED(a3);
#endif
}

#define WARNP "KTranscript: "
void warnout(const char *str)
{
    fprintf(stderr, WARNP "%s\n", str);
}
template<typename T1>
void warnout(const char *str, const T1 &a1)
{
    fprintf(stderr, WARNP "%s\n", QString::fromUtf8(str).arg(a1).toLocal8Bit().data());
}

// ----------------------------------------------------------------------
// Produces a string out of a script exception.

QString expt2str(const QJSValue &expt)
{
    if (expt.isError()) {
        const QJSValue message = expt.property(QStringLiteral("message"));
        if (!message.isUndefined()) {
            return QStringLiteral("Error: %1").arg(message.toString());
        }
    }

    QString strexpt = expt.toString();
    return QStringLiteral("Caught exception: %1").arg(strexpt);
}

// ----------------------------------------------------------------------
// Count number of lines in the string,
// up to and excluding the requested position.
int countLines(const QString &s, int p)
{
    int n = 1;
    int len = s.length();
    for (int i = 0; i < p && i < len; ++i) {
        if (s[i] == QLatin1Char('\n')) {
            ++n;
        }
    }
    return n;
}

// ----------------------------------------------------------------------
// Normalize string key for hash lookups,
QByteArray normKeystr(const QString &raw, bool mayHaveAcc = true)
{
    // NOTE: Regexes should not be used here for performance reasons.
    // This function may potentially be called thousands of times
    // on application startup.

    QString key = raw;

    // Strip all whitespace.
    int len = key.length();
    QString nkey;
    for (int i = 0; i < len; ++i) {
        QChar c = key[i];
        if (!c.isSpace()) {
            nkey.append(c);
        }
    }
    key = nkey;

    // Strip accelerator marker.
    if (mayHaveAcc) {
        key = removeAcceleratorMarker(key);
    }

    // Convert to lower case.
    key = key.toLower();

    return key.toUtf8();
}

// ----------------------------------------------------------------------
// Trim multiline string in a "smart" way:
// Remove leading and trailing whitespace up to and including first
// newline from that side, if there is one; otherwise, don't touch.
QString trimSmart(const QString &raw)
{
    // NOTE: This could be done by a single regex, but is not due to
    // performance reasons.
    // This function may potentially be called thousands of times
    // on application startup.

    int len = raw.length();

    int is = 0;
    while (is < len && raw[is].isSpace() && raw[is] != QLatin1Char('\n')) {
        ++is;
    }
    if (is >= len || raw[is] != QLatin1Char('\n')) {
        is = -1;
    }

    int ie = len - 1;
    while (ie >= 0 && raw[ie].isSpace() && raw[ie] != QLatin1Char('\n')) {
        --ie;
    }
    if (ie < 0 || raw[ie] != QLatin1Char('\n')) {
        ie = len;
    }

    return raw.mid(is + 1, ie - is - 1);
}

// ----------------------------------------------------------------------
// Produce a JavaScript object out of Qt variant.

QJSValue variantToJsValue(const QVariant &val)
{
    const auto vtype = val.userType();
    if (vtype == QMetaType::QString) {
        return QJSValue(val.toString());
    } else if (vtype == QMetaType::Bool) {
        return QJSValue(val.toBool());
    } else if (vtype == QMetaType::Double //
               || vtype == QMetaType::Int //
               || vtype == QMetaType::UInt //
               || vtype == QMetaType::LongLong //
               || vtype == QMetaType::ULongLong) {
        return QJSValue(val.toDouble());
    } else {
        return QJSValue::UndefinedValue;
    }
}

// ----------------------------------------------------------------------
// Parse ini-style config file,
// returning content as hash of hashes by group and key.
// Parsing is not fussy, it will read what it can.
TsConfig readConfig(const QString &fname)
{
    TsConfig config;
    // Add empty group.
    TsConfig::iterator configGroup;
    configGroup = config.insert(QString(), TsConfigGroup());

    QFile file(fname);
    if (!file.open(QIODevice::ReadOnly)) {
        return config;
    }
    QTextStream stream(&file);
    while (!stream.atEnd()) {
        QString line = stream.readLine();
        int p1;
        int p2;

        // Remove comment from the line.
        p1 = line.indexOf(QLatin1Char('#'));
        if (p1 >= 0) {
            line.truncate(p1);
        }
        line = line.trimmed();
        if (line.isEmpty()) {
            continue;
        }

        if (line[0] == QLatin1Char('[')) {
            // Group switch.
            p1 = 0;
            p2 = line.indexOf(QLatin1Char(']'), p1 + 1);
            if (p2 < 0) {
                continue;
            }
            QString group = line.mid(p1 + 1, p2 - p1 - 1).trimmed();
            configGroup = config.find(group);
            if (configGroup == config.end()) {
                // Add new group.
                configGroup = config.insert(group, TsConfigGroup());
            }
        } else {
            // Field.
            p1 = line.indexOf(QLatin1Char('='));
            if (p1 < 0) {
                continue;
            }

            const QStringView lineView(line);
            const QStringView field = lineView.left(p1).trimmed();
            if (!field.isEmpty()) {
                const QStringView value = lineView.mid(p1 + 1).trimmed();
                (*configGroup)[field.toString()] = value.toString();
            }
        }
    }
    file.close();

    return config;
}

// ----------------------------------------------------------------------
// throw or log error, depending on context availability
static QJSValue throwError(QJSEngine *engine, const QString &message)
{
    if (engine) {
        return engine->evaluate(QStringLiteral("new Error(%1)").arg(message));
    }

    qCritical() << "Script error" << message;
    return QJSValue::UndefinedValue;
}

#ifdef KTRANSCRIPT_TESTBUILD

// ----------------------------------------------------------------------
// Test build creation/destruction hooks
static KTranscriptImp *s_transcriptInstance = nullptr;

KTranscriptImp *globalKTI()
{
    return s_transcriptInstance;
}

KTranscript *autotestCreateKTranscriptImp()
{
    Q_ASSERT(s_transcriptInstance == nullptr);
    s_transcriptInstance = new KTranscriptImp;
    return s_transcriptInstance;
}

void autotestDestroyKTranscriptImp()
{
    Q_ASSERT(s_transcriptInstance != nullptr);
    delete s_transcriptInstance;
    s_transcriptInstance = nullptr;
}

#else

// ----------------------------------------------------------------------
// Dynamic loading.
Q_GLOBAL_STATIC(KTranscriptImp, globalKTI)
extern "C" {
KTRANSCRIPT_EXPORT KTranscript *load_transcript()
{
    return globalKTI();
}
}
#endif

// ----------------------------------------------------------------------
// KTranscript definitions.

KTranscriptImp::KTranscriptImp()
{
    // Load user configuration.

    QString tsConfigPath = QStandardPaths::locate(QStandardPaths::ConfigLocation, QStringLiteral("ktranscript.ini"));
    if (tsConfigPath.isEmpty()) {
        tsConfigPath = QDir::homePath() + QLatin1Char('/') + QLatin1String(".transcriptrc");
    }
    config = readConfig(tsConfigPath);
}

KTranscriptImp::~KTranscriptImp()
{
    qDeleteAll(m_sface);
}

QString KTranscriptImp::eval(const QList<QVariant> &argv,
                             const QString &lang,
                             const QString &ctry,
                             const QString &msgctxt,
                             const QHash<QString, QString> &dynctxt,
                             const QString &msgid,
                             const QStringList &subs,
                             const QList<QVariant> &vals,
                             const QString &ftrans,
                             QList<QStringList> &mods,
                             QString &error,
                             bool &fallback)
{
    // error = "debug"; return QString();

    error.clear(); // empty error message means successful evaluation
    fallback = false; // fallback not requested

#if 0
    // FIXME: Maybe not needed, as QJSEngine has no native outside access?
    // Unportable (needs unistd.h)?

    // If effective user id is root and real user id is not root.
    if (geteuid() == 0 && getuid() != 0) {
        // Since scripts are user input, and the program is running with
        // root permissions while real user is not root, do not invoke
        // scripting at all, to prevent exploits.
        error = "Security block: trying to execute a script in suid environment.";
        return QString();
    }
#endif

    // Load any new modules and clear the list.
    if (!mods.isEmpty()) {
        loadModules(mods, error);
        mods.clear();
        if (!error.isEmpty()) {
            return QString();
        }
    }

    // Add interpreters for new languages.
    // (though it should never happen here, but earlier when loading modules;
    // this also means there are no calls set, so the unregistered call error
    // below will be reported).
    if (!m_sface.contains(lang)) {
        setupInterpreter(lang);
    }

    // Shortcuts.
    Scriptface *sface = m_sface[lang];

    QJSEngine *engine = sface->scriptEngine;
    QJSValue gobj = engine->globalObject();

    // Link current message data for script-side interface.
    sface->msgcontext = &msgctxt;
    sface->dyncontext = &dynctxt;
    sface->msgId = &msgid;
    sface->subList = &subs;
    sface->valList = &vals;
    sface->ftrans = &ftrans;
    sface->fallbackRequest = &fallback;
    sface->ctry = &ctry;

    // Find corresponding JS function.
    int argc = argv.size();
    if (argc < 1) {
        // error = "At least the call name must be supplied.";
        // Empty interpolation is OK, possibly used just to initialize
        // at a given point (e.g. for Ts.setForall() to start having effect).
        return QString();
    }
    QString funcName = argv[0].toString();
    if (!sface->funcs.contains(funcName)) {
        error = QStringLiteral("Unregistered call to '%1'.").arg(funcName);
        return QString();
    }

    QJSValue func = sface->funcs[funcName];
    QJSValue fval = sface->fvals[funcName];

    // Recover module path from the time of definition of this call,
    // for possible load calls.
    currentModulePath = sface->fpaths[funcName];

    // Execute function.
    QJSValueList arglist;
    arglist.reserve(argc - 1);
    for (int i = 1; i < argc; ++i) {
        arglist.append(engine->toScriptValue(argv[i]));
    }

    QJSValue val;
    if (fval.isObject()) {
        val = func.callWithInstance(fval, arglist);
    } else { // no object associated to this function, use global
        val = func.callWithInstance(gobj, arglist);
    }

    if (fallback) {
        // Fallback to ordinary translation requested.
        return QString();
    } else if (!val.isError()) {
        // Evaluation successful.

        if (val.isString()) {
            // Good to go.

            return val.toString();
        } else {
            // Accept only strings.

            QString strval = val.toString();
            error = QStringLiteral("Non-string return value: %1").arg(strval);
            return QString();
        }
    } else {
        // Exception raised.

        error = expt2str(val);

        return QString();
    }
}

QStringList KTranscriptImp::postCalls(const QString &lang)
{
    // Return no calls if scripting was not already set up for this language.
    // NOTE: This shouldn't happen, as postCalls cannot be called in such case.
    if (!m_sface.contains(lang)) {
        return QStringList();
    }

    // Shortcuts.
    Scriptface *sface = m_sface[lang];

    return sface->nameForalls;
}

void KTranscriptImp::loadModules(const QList<QStringList> &mods, QString &error)
{
    QList<QString> modErrors;

    for (const QStringList &mod : mods) {
        QString mpath = mod[0];
        QString mlang = mod[1];

        // Add interpreters for new languages.
        if (!m_sface.contains(mlang)) {
            setupInterpreter(mlang);
        }

        // Setup current module path for loading submodules.
        // (sort of closure over invocations of loadf)
        int posls = mpath.lastIndexOf(QLatin1Char('/'));
        if (posls < 1) {
            modErrors.append(QStringLiteral("Funny module path '%1', skipping.").arg(mpath));
            continue;
        }
        currentModulePath = mpath.left(posls);
        QString fname = mpath.mid(posls + 1);
        // Scriptface::loadf() wants no extension on the filename
        fname = fname.left(fname.lastIndexOf(QLatin1Char('.')));

        // Load the module.
        QJSValueList alist;
        alist.append(QJSValue(fname));

        m_sface[mlang]->load(alist);
    }

    // Unset module path.
    currentModulePath.clear();

    for (const QString &merr : std::as_const(modErrors)) {
        error.append(merr + QLatin1Char('\n'));
    }
}

#define SFNAME "Ts"
void KTranscriptImp::setupInterpreter(const QString &lang)
{
    // Add scripting interface
    // Creates its own script engine and registers with it
    // NOTE: Config may not contain an entry for the language, in which case
    // it is automatically constructed as an empty hash. This is intended.
    Scriptface *sface = new Scriptface(config[lang]);

    // Store scriptface
    m_sface[lang] = sface;

    // dbgout("=====> Created interpreter for '%1'", lang);
}

Scriptface::Scriptface(const TsConfigGroup &config_, QObject *parent)
    : QObject(parent)
    , scriptEngine(new QJSEngine)
    , fallbackRequest(nullptr)
    , config(config_)
{
    QJSValue object = scriptEngine->newQObject(this);
    scriptEngine->globalObject().setProperty(QStringLiteral(SFNAME), object);
    scriptEngine->evaluate(QStringLiteral("Ts.acall = function() { return Ts.acallInternal(Array.prototype.slice.call(arguments)); };"));
}

Scriptface::~Scriptface()
{
    qDeleteAll(loadedPmapHandles);
    scriptEngine->deleteLater();
}

void Scriptface::put(const QString &propertyName, const QJSValue &value)
{
    QJSValue internalObject = scriptEngine->globalObject().property(QStringLiteral("ScriptfaceInternal"));
    if (internalObject.isUndefined()) {
        internalObject = scriptEngine->newObject();
        scriptEngine->globalObject().setProperty(QStringLiteral("ScriptfaceInternal"), internalObject);
    }

    internalObject.setProperty(propertyName, value);
}

// ----------------------------------------------------------------------
// Scriptface interface functions.

#ifdef _MSC_VER
// Work around bizarre MSVC (2013) bug preventing use of QStringLiteral for concatenated string literals
#define SPREF(X) QString::fromLatin1(SFNAME "." X)
#else
#define SPREF(X) QStringLiteral(SFNAME "." X)
#endif

QJSValue Scriptface::load(const QString &name)
{
    QJSValueList fnames;
    fnames << name;
    return load(fnames);
}

QJSValue Scriptface::setcall(const QJSValue &name, const QJSValue &func, const QJSValue &fval)
{
    if (!name.isString()) {
        return throwError(scriptEngine, SPREF("setcall: expected string as first argument"));
    }
    if (!func.isCallable()) {
        return throwError(scriptEngine, SPREF("setcall: expected function as second argument"));
    }
    if (!(fval.isObject() || fval.isNull())) {
        return throwError(scriptEngine, SPREF("setcall: expected object or null as third argument"));
    }

    QString qname = name.toString();
    funcs[qname] = func;
    fvals[qname] = fval;

    // Register values to keep GC from collecting them. Is this needed?
    put(QStringLiteral("#:f<%1>").arg(qname), func);
    put(QStringLiteral("#:o<%1>").arg(qname), fval);

    // Set current module path as module path for this call,
    // in case it contains load subcalls.
    fpaths[qname] = globalKTI()->currentModulePath;

    return QJSValue::UndefinedValue;
}

QJSValue Scriptface::hascall(const QString &qname)
{
    return QJSValue(funcs.contains(qname));
}

QJSValue Scriptface::acallInternal(const QJSValue &args)
{
    QJSValueIterator it(args);

    if (!it.next()) {
        return throwError(scriptEngine, SPREF("acall: expected at least one argument (call name)"));
    }
    if (!it.value().isString()) {
        return throwError(scriptEngine, SPREF("acall: expected string as first argument (call name)"));
    }
    // Get the function and its context object.
    QString callname = it.value().toString();
    if (!funcs.contains(callname)) {
        return throwError(scriptEngine, SPREF("acall: unregistered call to '%1'").arg(callname));
    }
    QJSValue func = funcs[callname];
    QJSValue fval = fvals[callname];

    // Recover module path from the time of definition of this call,
    // for possible load calls.
    globalKTI()->currentModulePath = fpaths[callname];

    // Execute function.
    QJSValueList arglist;
    while (it.next()) {
        arglist.append(it.value());
    }

    QJSValue val;
    if (fval.isObject()) {
        // Call function with the context object.
        val = func.callWithInstance(fval, arglist);
    } else {
        // No context object associated to this function, use global.
        val = func.callWithInstance(scriptEngine->globalObject(), arglist);
    }
    return val;
}

QJSValue Scriptface::setcallForall(const QJSValue &name, const QJSValue &func, const QJSValue &fval)
{
    if (!name.isString()) {
        return throwError(scriptEngine, SPREF("setcallForall: expected string as first argument"));
    }
    if (!func.isCallable()) {
        return throwError(scriptEngine, SPREF("setcallForall: expected function as second argument"));
    }
    if (!(fval.isObject() || fval.isNull())) {
        return throwError(scriptEngine, SPREF("setcallForall: expected object or null as third argument"));
    }

    QString qname = name.toString();
    funcs[qname] = func;
    fvals[qname] = fval;

    // Register values to keep GC from collecting them. Is this needed?
    put(QStringLiteral("#:fall<%1>").arg(qname), func);
    put(QStringLiteral("#:oall<%1>").arg(qname), fval);

    // Set current module path as module path for this call,
    // in case it contains load subcalls.
    fpaths[qname] = globalKTI()->currentModulePath;

    // Put in the queue order for execution on all messages.
    nameForalls.append(qname);

    return QJSValue::UndefinedValue;
}

QJSValue Scriptface::fallback()
{
    if (fallbackRequest) {
        *fallbackRequest = true;
    }
    return QJSValue::UndefinedValue;
}

QJSValue Scriptface::nsubs()
{
    return QJSValue(static_cast<int>(subList->size()));
}

QJSValue Scriptface::subs(const QJSValue &index)
{
    if (!index.isNumber()) {
        return throwError(scriptEngine, SPREF("subs: expected number as first argument"));
    }

    int i = qRound(index.toNumber());
    if (i < 0 || i >= subList->size()) {
        return throwError(scriptEngine, SPREF("subs: index out of range"));
    }

    return QJSValue(subList->at(i));
}

QJSValue Scriptface::vals(const QJSValue &index)
{
    if (!index.isNumber()) {
        return throwError(scriptEngine, SPREF("vals: expected number as first argument"));
    }

    int i = qRound(index.toNumber());
    if (i < 0 || i >= valList->size()) {
        return throwError(scriptEngine, SPREF("vals: index out of range"));
    }

    return scriptEngine->toScriptValue(valList->at(i));
    //     return variantToJsValue(valList->at(i));
}

QJSValue Scriptface::msgctxt()
{
    return QJSValue(*msgcontext);
}

QJSValue Scriptface::dynctxt(const QString &qkey)
{
    auto valIt = dyncontext->constFind(qkey);
    if (valIt != dyncontext->constEnd()) {
        return QJSValue(*valIt);
    }
    return QJSValue::UndefinedValue;
}

QJSValue Scriptface::msgid()
{
    return QJSValue(*msgId);
}

QJSValue Scriptface::msgkey()
{
    return QJSValue(QString(*msgcontext + QLatin1Char('|') + *msgId));
}

QJSValue Scriptface::msgstrf()
{
    return QJSValue(*ftrans);
}

void Scriptface::dbgputs(const QString &qstr)
{
    dbgout("[JS-debug] %1", qstr);
}

void Scriptface::warnputs(const QString &qstr)
{
    warnout("[JS-warning] %1", qstr);
}

QJSValue Scriptface::localeCountry()
{
    return QJSValue(*ctry);
}

QJSValue Scriptface::normKey(const QJSValue &phrase)
{
    if (!phrase.isString()) {
        return throwError(scriptEngine, SPREF("normKey: expected string as argument"));
    }

    QByteArray nqphrase = normKeystr(phrase.toString());
    return QJSValue(QString::fromUtf8(nqphrase));
}

QJSValue Scriptface::loadProps(const QString &name)
{
    QJSValueList fnames;
    fnames << name;
    return loadProps(fnames);
}

QJSValue Scriptface::loadProps(const QJSValueList &fnames)
{
    if (globalKTI()->currentModulePath.isEmpty()) {
        return throwError(scriptEngine, SPREF("loadProps: no current module path, aiiie..."));
    }

    for (int i = 0; i < fnames.size(); ++i) {
        if (!fnames[i].isString()) {
            return throwError(scriptEngine, SPREF("loadProps: expected string as file name"));
        }
    }

    for (int i = 0; i < fnames.size(); ++i) {
        QString qfname = fnames[i].toString();
        QString qfpath_base = globalKTI()->currentModulePath + QLatin1Char('/') + qfname;

        // Determine which kind of map is available.
        // Give preference to compiled map.
        QString qfpath = qfpath_base + QLatin1String(".pmapc");
        bool haveCompiled = true;
        QFile file_check(qfpath);
        if (!file_check.open(QIODevice::ReadOnly)) {
            haveCompiled = false;
            qfpath = qfpath_base + QLatin1String(".pmap");
            QFile file_check(qfpath);
            if (!file_check.open(QIODevice::ReadOnly)) {
                return throwError(scriptEngine, SPREF("loadProps: cannot read map '%1'").arg(qfpath));
            }
        }
        file_check.close();

        // Load from appropriate type of map.
        if (!loadedPmapPaths.contains(qfpath)) {
            QString errorString;
            if (haveCompiled) {
                errorString = loadProps_bin(qfpath);
            } else {
                errorString = loadProps_text(qfpath);
            }
            if (!errorString.isEmpty()) {
                return throwError(scriptEngine, errorString);
            }
            dbgout("Loaded property map: %1", qfpath);
            loadedPmapPaths.insert(qfpath);
        }
    }

    return QJSValue::UndefinedValue;
}

QJSValue Scriptface::getProp(const QJSValue &phrase, const QJSValue &prop)
{
    if (!phrase.isString()) {
        return throwError(scriptEngine, SPREF("getProp: expected string as first argument"));
    }
    if (!prop.isString()) {
        return throwError(scriptEngine, SPREF("getProp: expected string as second argument"));
    }

    QByteArray qphrase = normKeystr(phrase.toString());
    QHash<QByteArray, QByteArray> props = phraseProps.value(qphrase);
    if (props.isEmpty()) {
        props = resolveUnparsedProps(qphrase);
    }
    if (!props.isEmpty()) {
        QByteArray qprop = normKeystr(prop.toString());
        QByteArray qval = props.value(qprop);
        if (!qval.isEmpty()) {
            return QJSValue(QString::fromUtf8(qval));
        }
    }
    return QJSValue::UndefinedValue;
}

QJSValue Scriptface::setProp(const QJSValue &phrase, const QJSValue &prop, const QJSValue &value)
{
    if (!phrase.isString()) {
        return throwError(scriptEngine, SPREF("setProp: expected string as first argument"));
    }
    if (!prop.isString()) {
        return throwError(scriptEngine, SPREF("setProp: expected string as second argument"));
    }
    if (!value.isString()) {
        return throwError(scriptEngine, SPREF("setProp: expected string as third argument"));
    }

    QByteArray qphrase = normKeystr(phrase.toString());
    QByteArray qprop = normKeystr(prop.toString());
    QByteArray qvalue = value.toString().toUtf8();
    // Any non-existent key in first or second-level hash will be created.
    phraseProps[qphrase][qprop] = qvalue;
    return QJSValue::UndefinedValue;
}

static QString toCaseFirst(const QString &qstr, int qnalt, bool toupper)
{
    static const QLatin1String head("~@");
    static const int hlen = 2; // head.length()

    // If the first letter is found within an alternatives directive,
    // change case of the first letter in each of the alternatives.
    QString qstrcc = qstr;
    const int len = qstr.length();
    QChar altSep;
    int remainingAlts = 0;
    bool checkCase = true;
    int numChcased = 0;
    int i = 0;
    while (i < len) {
        QChar c = qstr[i];

        if (qnalt && !remainingAlts && QStringView(qstr).mid(i, hlen) == head) {
            // An alternatives directive is just starting.
            i += 2;
            if (i >= len) {
                break; // malformed directive, bail out
            }
            // Record alternatives separator, set number of remaining
            // alternatives, reactivate case checking.
            altSep = qstrcc[i];
            remainingAlts = qnalt;
            checkCase = true;
        } else if (remainingAlts && c == altSep) {
            // Alternative separator found, reduce number of remaining
            // alternatives and reactivate case checking.
            --remainingAlts;
            checkCase = true;
        } else if (checkCase && c.isLetter()) {
            // Case check is active and the character is a letter; change case.
            if (toupper) {
                qstrcc[i] = c.toUpper();
            } else {
                qstrcc[i] = c.toLower();
            }
            ++numChcased;
            // No more case checks until next alternatives separator.
            checkCase = false;
        }

        // If any letter has been changed, and there are no more alternatives
        // to be processed, we're done.
        if (numChcased > 0 && remainingAlts == 0) {
            break;
        }

        // Go to next character.
        ++i;
    }

    return qstrcc;
}

QJSValue Scriptface::toUpperFirst(const QJSValue &str, const QJSValue &nalt)
{
    if (!str.isString()) {
        return throwError(scriptEngine, SPREF("toUpperFirst: expected string as first argument"));
    }
    if (!(nalt.isNumber() || nalt.isNull())) {
        return throwError(scriptEngine, SPREF("toUpperFirst: expected number as second argument"));
    }

    QString qstr = str.toString();
    int qnalt = nalt.isNull() ? 0 : nalt.toInt();

    QString qstruc = toCaseFirst(qstr, qnalt, true);

    return QJSValue(qstruc);
}

QJSValue Scriptface::toLowerFirst(const QJSValue &str, const QJSValue &nalt)
{
    if (!str.isString()) {
        return throwError(scriptEngine, SPREF("toLowerFirst: expected string as first argument"));
    }
    if (!(nalt.isNumber() || nalt.isNull())) {
        return throwError(scriptEngine, SPREF("toLowerFirst: expected number as second argument"));
    }

    QString qstr = str.toString();
    int qnalt = nalt.isNull() ? 0 : nalt.toInt();

    QString qstrlc = toCaseFirst(qstr, qnalt, false);

    return QJSValue(qstrlc);
}

QJSValue Scriptface::getConfString(const QJSValue &key, const QJSValue &dval)
{
    if (!key.isString()) {
        return throwError(scriptEngine, QStringLiteral("getConfString: expected string as first argument"));
    }
    if (!(dval.isString() || dval.isNull())) {
        return throwError(scriptEngine, SPREF("getConfString: expected string as second argument (when given)"));
    }

    QString qkey = key.toString();
    auto valIt = config.constFind(qkey);
    if (valIt != config.constEnd()) {
        return QJSValue(*valIt);
    }

    return dval.isNull() ? QJSValue::UndefinedValue : dval;
}

QJSValue Scriptface::getConfBool(const QJSValue &key, const QJSValue &dval)
{
    if (!key.isString()) {
        return throwError(scriptEngine, SPREF("getConfBool: expected string as first argument"));
    }
    if (!(dval.isBool() || dval.isNull())) {
        return throwError(scriptEngine, SPREF("getConfBool: expected boolean as second argument (when given)"));
    }

    static QStringList falsities;
    if (falsities.isEmpty()) {
        falsities.append(QString(QLatin1Char('0')));
        falsities.append(QStringLiteral("no"));
        falsities.append(QStringLiteral("false"));
    }

    QString qkey = key.toString();
    auto valIt = config.constFind(qkey);
    if (valIt != config.constEnd()) {
        QString qval = valIt->toLower();
        return QJSValue(!falsities.contains(qval));
    }

    return dval.isNull() ? QJSValue::UndefinedValue : dval;
}

QJSValue Scriptface::getConfNumber(const QJSValue &key, const QJSValue &dval)
{
    if (!key.isString()) {
        return throwError(scriptEngine,
                          SPREF("getConfNumber: expected string "
                                "as first argument"));
    }
    if (!(dval.isNumber() || dval.isNull())) {
        return throwError(scriptEngine,
                          SPREF("getConfNumber: expected number "
                                "as second argument (when given)"));
    }

    QString qkey = key.toString();
    auto valIt = config.constFind(qkey);
    if (valIt != config.constEnd()) {
        const QString &qval = *valIt;
        bool convOk;
        double qnum = qval.toDouble(&convOk);
        if (convOk) {
            return QJSValue(qnum);
        }
    }

    return dval.isNull() ? QJSValue::UndefinedValue : dval;
}

// ----------------------------------------------------------------------
// Scriptface helpers to interface functions.

QJSValue Scriptface::load(const QJSValueList &fnames)
{
    if (globalKTI()->currentModulePath.isEmpty()) {
        return throwError(scriptEngine, SPREF("load: no current module path, aiiie..."));
    }

    for (int i = 0; i < fnames.size(); ++i) {
        if (!fnames[i].isString()) {
            return throwError(scriptEngine, SPREF("load: expected string as file name"));
        }
    }

    for (int i = 0; i < fnames.size(); ++i) {
        QString qfname = fnames[i].toString();
        QString qfpath = globalKTI()->currentModulePath + QLatin1Char('/') + qfname + QLatin1String(".js");

        QFile file(qfpath);
        if (!file.open(QIODevice::ReadOnly)) {
            return throwError(scriptEngine, SPREF("load: cannot read file '%1'").arg(qfpath));
        }

        QTextStream stream(&file);
        QString source = stream.readAll();
        file.close();

        QJSValue comp = scriptEngine->evaluate(source, qfpath, 0);

        if (comp.isError()) {
            QString msg = comp.toString();

            QString line;
            if (comp.isObject()) {
                QJSValue lval = comp.property(QStringLiteral("line"));
                if (lval.isNumber()) {
                    line = QString::number(lval.toInt());
                }
            }

            return throwError(scriptEngine, QStringLiteral("at %1:%2: %3").arg(qfpath, line, msg));
        }
        dbgout("Loaded module: %1", qfpath);
    }
    return QJSValue::UndefinedValue;
}

QString Scriptface::loadProps_text(const QString &fpath)
{
    QFile file(fpath);
    if (!file.open(QIODevice::ReadOnly)) {
        return SPREF("loadProps_text: cannot read file '%1'").arg(fpath);
    }
    QTextStream stream(&file);
    QString s = stream.readAll();
    file.close();

    // Parse the map.
    // Should care about performance: possibly executed on each KDE
    // app startup and reading houndreds of thousands of characters.
    enum { s_nextEntry, s_nextKey, s_nextValue };
    QList<QByteArray> ekeys; // holds keys for current entry
    QHash<QByteArray, QByteArray> props; // holds properties for current entry
    int slen = s.length();
    int state = s_nextEntry;
    QByteArray pkey;
    QChar prop_sep;
    QChar key_sep;
    int i = 0;
    while (1) {
        int i_checkpoint = i;

        if (state == s_nextEntry) {
            while (s[i].isSpace()) {
                ++i;
                if (i >= slen) {
                    goto END_PROP_PARSE;
                }
            }
            if (i + 1 >= slen) {
                return SPREF("loadProps_text: unexpected end of file in %1").arg(fpath);
            }
            if (s[i] != QLatin1Char('#')) {
                // Separator characters for this entry.
                key_sep = s[i];
                prop_sep = s[i + 1];
                if (key_sep.isLetter() || prop_sep.isLetter()) {
                    return SPREF("loadProps_text: separator characters must not be letters at %1:%2").arg(fpath).arg(countLines(s, i));
                }

                // Reset all data for current entry.
                ekeys.clear();
                props.clear();
                pkey.clear();

                i += 2;
                state = s_nextKey;
            } else {
                // This is a comment, skip to EOL, don't change state.
                while (s[i] != QLatin1Char('\n')) {
                    ++i;
                    if (i >= slen) {
                        goto END_PROP_PARSE;
                    }
                }
            }
        } else if (state == s_nextKey) {
            int ip = i;
            // Proceed up to next key or property separator.
            while (s[i] != key_sep && s[i] != prop_sep) {
                ++i;
                if (i >= slen) {
                    goto END_PROP_PARSE;
                }
            }
            if (s[i] == key_sep) {
                // This is a property key,
                // record for when the value gets parsed.
                pkey = normKeystr(s.mid(ip, i - ip), false);

                i += 1;
                state = s_nextValue;
            } else { // if (s[i] == prop_sep) {
                // This is an entry key, or end of entry.
                QByteArray ekey = normKeystr(s.mid(ip, i - ip), false);
                if (!ekey.isEmpty()) {
                    // An entry key.
                    ekeys.append(ekey);

                    i += 1;
                    state = s_nextKey;
                } else {
                    // End of entry.
                    if (ekeys.size() < 1) {
                        return SPREF("loadProps_text: no entry key for entry ending at %1:%2").arg(fpath).arg(countLines(s, i));
                    }

                    // Add collected entry into global store,
                    // once for each entry key (QHash implicitly shared).
                    for (const QByteArray &ekey : std::as_const(ekeys)) {
                        phraseProps[ekey] = props;
                    }

                    i += 1;
                    state = s_nextEntry;
                }
            }
        } else if (state == s_nextValue) {
            int ip = i;
            // Proceed up to next property separator.
            while (s[i] != prop_sep) {
                ++i;
                if (i >= slen) {
                    goto END_PROP_PARSE;
                }
                if (s[i] == key_sep) {
                    return SPREF("loadProps_text: property separator inside property value at %1:%2").arg(fpath).arg(countLines(s, i));
                }
            }
            // Extract the property value and store the property.
            QByteArray pval = trimSmart(s.mid(ip, i - ip)).toUtf8();
            props[pkey] = pval;

            i += 1;
            state = s_nextKey;
        } else {
            return SPREF("loadProps: internal error 10 at %1:%2").arg(fpath).arg(countLines(s, i));
        }

        // To avoid infinite looping and stepping out.
        if (i == i_checkpoint || i >= slen) {
            return SPREF("loadProps: internal error 20 at %1:%2").arg(fpath).arg(countLines(s, i));
        }
    }

END_PROP_PARSE:

    if (state != s_nextEntry) {
        return SPREF("loadProps: unexpected end of file in %1").arg(fpath);
    }

    return QString();
}

// Read big-endian integer of nbytes length at position pos
// in character array fc of length len.
// Update position to point after the number.
// In case of error, pos is set to -1.
template<typename T>
static int bin_read_int_nbytes(const char *fc, qlonglong len, qlonglong &pos, int nbytes)
{
    if (pos + nbytes > len) {
        pos = -1;
        return 0;
    }
    T num = qFromBigEndian<T>((uchar *)fc + pos);
    pos += nbytes;
    return num;
}

// Read 64-bit big-endian integer.
static quint64 bin_read_int64(const char *fc, qlonglong len, qlonglong &pos)
{
    return bin_read_int_nbytes<quint64>(fc, len, pos, 8);
}

// Read 32-bit big-endian integer.
static quint32 bin_read_int(const char *fc, qlonglong len, qlonglong &pos)
{
    return bin_read_int_nbytes<quint32>(fc, len, pos, 4);
}

// Read string at position pos of character array fc of length n.
// String is represented as 32-bit big-endian byte length followed by bytes.
// Update position to point after the string.
// In case of error, pos is set to -1.
static QByteArray bin_read_string(const char *fc, qlonglong len, qlonglong &pos)
{
    // Binary format stores strings as length followed by byte sequence.
    // No null-termination.
    int nbytes = bin_read_int(fc, len, pos);
    if (pos < 0) {
        return QByteArray();
    }
    if (nbytes < 0 || pos + nbytes > len) {
        pos = -1;
        return QByteArray();
    }
    QByteArray s(fc + pos, nbytes);
    pos += nbytes;
    return s;
}

QString Scriptface::loadProps_bin(const QString &fpath)
{
    QFile file(fpath);
    if (!file.open(QIODevice::ReadOnly)) {
        return SPREF("loadProps: cannot read file '%1'").arg(fpath);
    }
    // Collect header.
    QByteArray head(8, '0');
    file.read(head.data(), head.size());
    file.close();

    // Choose pmap loader based on header.
    if (head == "TSPMAP00") {
        return loadProps_bin_00(fpath);
    } else if (head == "TSPMAP01") {
        return loadProps_bin_01(fpath);
    } else {
        return SPREF("loadProps: unknown version of compiled map '%1'").arg(fpath);
    }
}

QString Scriptface::loadProps_bin_00(const QString &fpath)
{
    QFile file(fpath);
    if (!file.open(QIODevice::ReadOnly)) {
        return SPREF("loadProps: cannot read file '%1'").arg(fpath);
    }
    QByteArray fctmp = file.readAll();
    file.close();
    const char *fc = fctmp.data();
    const int fclen = fctmp.size();

    // Indicates stream state.
    qlonglong pos = 0;

    // Match header.
    QByteArray head(fc, 8);
    pos += 8;
    if (head != "TSPMAP00") {
        goto END_PROP_PARSE;
    }

    // Read total number of entries.
    int nentries;
    nentries = bin_read_int(fc, fclen, pos);
    if (pos < 0) {
        goto END_PROP_PARSE;
    }

    // Read all entries.
    for (int i = 0; i < nentries; ++i) {
        // Read number of entry keys and all entry keys.
        QList<QByteArray> ekeys;
        int nekeys = bin_read_int(fc, fclen, pos);
        if (pos < 0) {
            goto END_PROP_PARSE;
        }
        ekeys.reserve(nekeys); // nekeys are appended if data is not corrupted
        for (int j = 0; j < nekeys; ++j) {
            QByteArray ekey = bin_read_string(fc, fclen, pos);
            if (pos < 0) {
                goto END_PROP_PARSE;
            }
            ekeys.append(ekey);
        }
        // dbgout("--------> ekey[0]={%1}", QString::fromUtf8(ekeys[0]));

        // Read number of properties and all properties.
        QHash<QByteArray, QByteArray> props;
        int nprops = bin_read_int(fc, fclen, pos);
        if (pos < 0) {
            goto END_PROP_PARSE;
        }
        for (int j = 0; j < nprops; ++j) {
            QByteArray pkey = bin_read_string(fc, fclen, pos);
            if (pos < 0) {
                goto END_PROP_PARSE;
            }
            QByteArray pval = bin_read_string(fc, fclen, pos);
            if (pos < 0) {
                goto END_PROP_PARSE;
            }
            props[pkey] = pval;
        }

        // Add collected entry into global store,
        // once for each entry key (QHash implicitly shared).
        for (const QByteArray &ekey : std::as_const(ekeys)) {
            phraseProps[ekey] = props;
        }
    }

END_PROP_PARSE:

    if (pos < 0) {
        return SPREF("loadProps: corrupt compiled map '%1'").arg(fpath);
    }

    return QString();
}

QString Scriptface::loadProps_bin_01(const QString &fpath)
{
    QFile *file = new QFile(fpath);
    if (!file->open(QIODevice::ReadOnly)) {
        return SPREF("loadProps: cannot read file '%1'").arg(fpath);
    }

    QByteArray fstr;
    qlonglong pos;

    // Read the header and number and length of entry keys.
    fstr = file->read(8 + 4 + 8);
    pos = 0;
    QByteArray head = fstr.left(8);
    pos += 8;
    if (head != "TSPMAP01") {
        return SPREF("loadProps: corrupt compiled map '%1'").arg(fpath);
    }
    quint32 numekeys = bin_read_int(fstr, fstr.size(), pos);
    quint64 lenekeys = bin_read_int64(fstr, fstr.size(), pos);

    // Read entry keys.
    fstr = file->read(lenekeys);
    pos = 0;
    for (quint32 i = 0; i < numekeys; ++i) {
        QByteArray ekey = bin_read_string(fstr, lenekeys, pos);
        quint64 offset = bin_read_int64(fstr, lenekeys, pos);
        phraseUnparsedProps[ekey] = {file, offset};
    }

    // // Read property keys.
    // ...when it becomes necessary

    loadedPmapHandles.insert(file);
    return QString();
}

QHash<QByteArray, QByteArray> Scriptface::resolveUnparsedProps(const QByteArray &phrase)
{
    auto [file, offset] = phraseUnparsedProps.value(phrase);
    QHash<QByteArray, QByteArray> props;
    if (file && file->seek(offset)) {
        QByteArray fstr = file->read(4 + 4);
        qlonglong pos = 0;
        quint32 numpkeys = bin_read_int(fstr, fstr.size(), pos);
        quint32 lenpkeys = bin_read_int(fstr, fstr.size(), pos);
        fstr = file->read(lenpkeys);
        pos = 0;
        for (quint32 i = 0; i < numpkeys; ++i) {
            QByteArray pkey = bin_read_string(fstr, lenpkeys, pos);
            QByteArray pval = bin_read_string(fstr, lenpkeys, pos);
            props[pkey] = pval;
        }
        phraseProps[phrase] = props;
        phraseUnparsedProps.remove(phrase);
    }
    return props;
}

#include "ktranscript.moc"
