/*
    SPDX-FileCopyrightText: 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "gpgproc.h"

#include "kgpgsettings.h"
#include "kgpg_general_debug.h"

#include <KProcess>


#include <QDir>
#include <QFileInfo>
#include <QTextCodec>

#ifndef Q_OS_WIN
  #include <sys/stat.h>
#endif

class GnupgBinary {
public:
	GnupgBinary();

	const QString &binary() const;
	void setBinary(const QString &executable);
	const QStringList &standardArguments() const;
	unsigned int version() const;
	bool supportsDebugLevel() const;

private:
	QString m_binary;
	QStringList m_standardArguments;
	unsigned int m_version;
	bool m_useDebugLevel;
};

GnupgBinary::GnupgBinary()
	: m_version(0),
	m_useDebugLevel(false)
{
}

const QString &GnupgBinary::binary() const
{
	return m_binary;
}

/**
 * @brief check if GnuPG returns an error for this arguments
 * @param executable the GnuPG executable to call
 * @param arguments the arguments to pass to executable
 *
 * The arguments will be used together with "--version", so they should not
 * be any commands.
 */
static bool checkGnupgArguments(const QString &executable, const QStringList &arguments)
{
	KProcess gpg;

	// We ignore the output anyway, just make sure it doesn't clutter the output of
	// the parent process. Simplify the handling by putting all trash in one can.
	gpg.setOutputChannelMode(KProcess::MergedChannels);

	QStringList allArguments = arguments;
	allArguments << QLatin1String("--version");
	gpg.setProgram(executable, allArguments);

	return (gpg.execute() == 0);
}

static QString
getGpgStatusLine(const QString &binary, const QString &key)
{
    GPGProc process(nullptr, binary);
	process << QLatin1String( "--version" );

	QProcessEnvironment env = process.processEnvironment();
    env.insert(QStringLiteral("LANG"), QStringLiteral("C"));
	process.setProcessEnvironment(env);

	process.start();
	process.waitForFinished(-1);

	if (process.exitCode() == 255) {
		return QString();
	}

	QString line;
	while (process.readln(line) != -1) {
		if (line.startsWith(key)) {
			line.remove(0, key.length());
			return line.trimmed();
		}
	}

	return QString();
}

static QString
getGpgProcessHome(const QString &binary)
{
	return getGpgStatusLine(binary, QLatin1String("Home: "));
}

void GnupgBinary::setBinary(const QString &executable)
{
	qCDebug(KGPG_LOG_GENERAL) << "checking version of GnuPG executable" << executable;
	// must be set first as gpgVersionString() uses GPGProc to parse the output
	m_binary = executable;
	const QString verstr = GPGProc::gpgVersionString(executable);
	m_version = GPGProc::gpgVersion(verstr);
	qCDebug(KGPG_LOG_GENERAL) << "version is" << verstr << m_version;

	m_useDebugLevel = (m_version > 0x20000);

	m_standardArguments.clear();
	m_standardArguments << QLatin1String( "--no-secmem-warning" )
			<< QLatin1String( "--no-tty" )
			<< QLatin1String("--no-greeting");

	m_standardArguments << GPGProc::getGpgHomeArguments(executable);

	QStringList debugLevelArguments(QLatin1String("--debug-level"));
	debugLevelArguments << QLatin1String("none");
	if (checkGnupgArguments(executable, debugLevelArguments))
		m_standardArguments << debugLevelArguments;
}

const QStringList& GnupgBinary::standardArguments() const
{
	return m_standardArguments;
}

unsigned int GnupgBinary::version() const
{
	return m_version;
}

bool GnupgBinary::supportsDebugLevel() const
{
	return m_useDebugLevel;
}

Q_GLOBAL_STATIC(GnupgBinary, lastBinary)

GPGProc::GPGProc(QObject *parent, const QString &binary)
       : KLineBufferedProcess(parent)
{
	resetProcess(binary);
}

GPGProc::~GPGProc()
{
}

void
GPGProc::resetProcess(const QString &binary)
{
	GnupgBinary *bin = lastBinary;
	QString executable;
	if (binary.isEmpty())
		executable = KGpgSettings::gpgBinaryPath();
	else
		executable = binary;

	if (bin->binary() != executable)
		bin->setBinary(executable);

	setProgram(executable, bin->standardArguments());

	setOutputChannelMode(OnlyStdoutChannel);

	disconnect(this, QOverload<int, QProcess::ExitStatus>::of(&GPGProc::finished), this, &GPGProc::processExited);
	disconnect(this, &GPGProc::lineReadyStandardOutput, this, &GPGProc::readReady);
}

void GPGProc::start()
{
	// make sure there is exactly one connection from us to that signal
	connect(this, QOverload<int, QProcess::ExitStatus>::of(&GPGProc::finished), this, &GPGProc::processExited, Qt::UniqueConnection);
	connect(this, &GPGProc::lineReadyStandardOutput, this, &GPGProc::readReady, Qt::UniqueConnection);
	KProcess::start();
}

int GPGProc::readln(QString &line, const bool colons)
{
	QByteArray a;
	if (!readLineStandardOutput(&a))
		return -1;

	line = recode(a, colons, m_codec);

	return line.length();
}

int GPGProc::readln(QStringList &l)
{
    QString s;

    int len = readln(s);
    if (len < 0)
        return len;

    l = s.split(QLatin1Char( ':' ));

    for (int i = 0; i < l.count(); ++i)
    {
        int j = 0;
        while ((j = l[i].indexOf(QLatin1String( "\\x3a" ), j, Qt::CaseInsensitive)) >= 0)
        {
            l[i].replace(j, 4, QLatin1Char( ':' ));
            j++;
        }
    }

    return l.count();
}

QString
GPGProc::recode(QByteArray a, const bool colons, const QByteArray &codec)
{
	const char *textcodec = codec.isEmpty() ? "utf8" : codec.constData();
	int pos = 0;

	while ((pos = a.indexOf("\\x", pos)) >= 0) {
		if (pos > a.length() - 4)
			break;

		const QByteArray pattern(a.mid(pos, 4));
		const QByteArray hexnum(pattern.right(2));
		bool ok;
		char n[2];
		n[0] = hexnum.toUShort(&ok, 16);
		n[1] = '\0';	// to use n as a 0-terminated string
		if (!ok) {
			// skip this occurrence
			pos += 2;
			continue;
		}

		// QLatin1Char( ':' ) must be skipped, it is used as column delimiter
		// since it is pure ascii it can be replaced in QString.
		if (!colons && (n[0] == ':' )) {
			pos += 3;
			continue;
		}

		// it is likely to find the same byte sequence more than once
		int npos = pos;
		do {
			a.replace(npos, 4, n);
		} while ((npos = a.indexOf(pattern, npos)) >= 0);
	}

	return QTextCodec::codecForName(textcodec)->toUnicode(a);
}

bool
GPGProc::setCodec(const QByteArray &codec)
{
	const QList<QByteArray> codecs = QTextCodec::availableCodecs();
	if (!codecs.contains(codec))
		return false;

	m_codec = codec;

	return true;
}

int GPGProc::gpgVersion(const QString &vstr)
{
	if (vstr.isEmpty())
		return -1;

	QStringList values(vstr.split(QLatin1Char( '.' )));
	if (values.count() < 3)
		return -2;

	return (0x10000 * values[0].toInt() + 0x100 * values[1].toInt() + values[2].toInt());
}

QString GPGProc::gpgVersionString(const QString &binary)
{
	const QStringList vlist = getGgpParsedConfig(binary, "version");

	if (vlist.empty())
		return QString();

	return vlist.first().split(QLatin1Char(':')).first();
}

QStringList
GPGProc::getGpgPubkeyAlgorithms(const QString &binary)
{
	QStringList ret = getGgpParsedConfig(binary, "pubkeyname");

	if (ret.isEmpty())
		return ret;

	return ret.first().split(QLatin1Char(':')).first().split(QLatin1Char(';'));
}

QString GPGProc::getGpgStartupError(const QString &binary)
{
    GPGProc process(nullptr, binary);
	process << QLatin1String( "--version" );
	process.start();
	process.waitForFinished(-1);

	QString result;

	while (process.hasLineStandardError()) {
		QByteArray tmp;
		process.readLineStandardError(&tmp);
		tmp += '\n';
		result += QString::fromUtf8(tmp);
	}

	return result;
}

QStringList GPGProc::getGgpParsedConfig(const QString &binary, const QByteArray &key)
{
    GPGProc process(nullptr, binary);
	process << QLatin1String("--list-config") << QLatin1String("--with-colons");
	process.start();
	process.waitForFinished(-1);

	QStringList result;
	QByteArray filter = "cfg:";
	if (!key.isEmpty())
		filter += key + ':';

	while (process.hasLineStandardOutput()) {
		QByteArray tmp;
		process.readLineStandardOutput(&tmp);

		if (tmp.startsWith(filter))
			result << QString::fromUtf8(tmp.mid(filter.length()));
	}

	return result;
}

QString GPGProc::getGpgHome(const QString &binary)
{
	// First try: if environment is set GnuPG will use that directory
	// We can use this directly without starting a new process
	QByteArray env(qgetenv("GNUPGHOME"));
	QString gpgHome;
	if (!env.isEmpty()) {
		gpgHome = QLatin1String( env );
	} else if (!binary.isEmpty()) {
		// Second try: start GnuPG and ask what it is
		gpgHome = getGpgProcessHome(binary);
	}

	// Third try: guess what it is.
	if (gpgHome.isEmpty()) {
#ifdef Q_OS_WIN 	//krazy:exclude=cpp
		gpgHome = qgetenv("APPDATA") + QLatin1String( "/gnupg/" );
		gpgHome.replace(QLatin1Char( '\\' ), QLatin1Char( '/' ));
#else
		gpgHome = QDir::homePath() + QLatin1String( "/.gnupg/" );
#endif
	}

	gpgHome.replace(QLatin1String( "//" ), QLatin1String( "/" ));

	if (!gpgHome.endsWith(QLatin1Char( '/' )))
		gpgHome.append(QLatin1Char( '/' ));

	if (gpgHome.startsWith(QLatin1String("~/")))
		gpgHome.replace(0, 1, QDir::homePath());

#ifdef Q_OS_WIN
	QDir().mkpath(gpgHome);
#else
	uint mask = umask(077);
	QDir().mkpath(gpgHome);
	umask(mask);
#endif
	return gpgHome;
}

QStringList
GPGProc::getGpgHomeArguments(const QString &binary)
{
	const QString gpgConfigFile = KGpgSettings::gpgConfigPath();

	if (gpgConfigFile.isEmpty())
		return {};

	QStringList options{ QLatin1String("--options"), gpgConfigFile };

	// Check if the config file is in the default home directory
	// of the binary. If it isn't add --homedir to command line also.
	QString gpgdir = GPGProc::getGpgHome(binary);
	gpgdir.chop(1);	// remove trailing '/' as QFileInfo returns string without it
	QFileInfo confFile(gpgConfigFile);
	if (confFile.absolutePath() != gpgdir)
		options << QLatin1String("--homedir") << confFile.absolutePath();

	return options;
}
