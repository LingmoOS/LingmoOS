#include "common.h"
#include "../kgpginterface.h"
#include "../kgpgsettings.h"
#include "../transactions/kgpgtransaction.h"

#include <gpgme.h>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QProcess>
#include <QTemporaryDir>
#include <QDebug>

bool resetGpgConf(QTemporaryDir &basedir)
{
	if (!basedir.isValid())
		return false;

	// export path from which kgpgsettings will pick up the kgpgrc
	qputenv("XDG_CONFIG_HOME", basedir.path().toUtf8());

	QFile kgpgconf(basedir.filePath(QLatin1String("kgpgrc")));
	if (!kgpgconf.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
		return false;

	QDir dir(basedir.filePath(QLatin1String(".gnupg")));
	QString confPath = dir.filePath(QLatin1String("gpg.conf"));

	kgpgconf.write("[GPG Settings]\n"
			"gpg_config_path[$e]=" + confPath.toUtf8() + "\n"
			"[General Options]\n"
			"first run=false\n"
			);
	kgpgconf.close();

	// (re)create the home directory for GnuPG
	dir.removeRecursively();
	if (!dir.mkpath(dir.path()))
		return false;
	if (!QFile::setPermissions(dir.path(),
				   QFileDevice::ReadOwner | QFileDevice::WriteOwner |
					   QFileDevice::ExeOwner))
		return false;

	QFile conf(confPath);
	if (!conf.open(QIODevice::WriteOnly))
		return false;

	conf.write("keyserver  hkp://pool.sks-keyservers.net\n");

	return true;
}

QString readFile(const QString &filename)
{
	QFile file(filename);
	if (file.open(QIODevice::ReadOnly))
		return QLatin1String(file.readAll());
	else
		return QString();
}

static QStringList configArguments(const QTemporaryDir &dir)
{
	const QString conf = dir.filePath(QLatin1String(".gnupg/gpg.conf"));
	const QString gpgHome = dir.filePath(QLatin1String(".gnupg"));
	return { QLatin1String("--options"), conf, QLatin1String("--homedir"), gpgHome };
}

void addGpgKey(QTemporaryDir &dir, const QString &file, const QString &password)
{
	QString command = QLatin1String("gpg");
	QStringList args;
	args.push_back(QLatin1String("--no-secmem-warning"));
	args.push_back(QLatin1String("--no-tty"));
	args.push_back(QLatin1String("--batch"));
	if (!password.isEmpty()) {
		args.push_back(QLatin1String("--passphrase"));
		args.push_back(password);
	}
	args << configArguments(dir);
	args.push_back(QLatin1String("--debug-level"));
	args.push_back(QLatin1String("none"));
	args.push_back(QLatin1String("--status-fd=1"));
	args.push_back(QLatin1String("--import"));
	args.push_back(QLatin1String("--allow-secret-key-import"));
	args.push_back(QLatin1String("--command-fd=0"));
	args.push_back(file);
	QProcess process;
	process.execute(command, args);
	qDebug() << "Added Gpg key: " << file;
}

void addPasswordArguments(KGpgTransaction *transaction, const QString &passphrase)
{
	QStringList args;
	args.push_back(QLatin1String("--batch"));
	args.push_back(QLatin1String("--passphrase"));
	args.push_back(passphrase);
	args.push_back(QLatin1String("--pinentry-mode"));
	args.push_back(QLatin1String("loopback"));
	transaction->insertArguments(1, args);
}

bool hasPhoto(QTemporaryDir &dir, const QString &id)
{
	QStringList args{ QLatin1String("--list-keys"), id };
	QString command = QLatin1String("gpg");
	QProcess process;
	process.start(command, configArguments(dir) << args);
	process.waitForFinished();
	QString output = QLatin1String(process.readAllStandardOutput());
	qDebug()<< output;
	return output.contains(QLatin1String("image"));
}
