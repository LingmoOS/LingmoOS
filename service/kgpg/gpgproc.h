/*
    SPDX-FileCopyrightText: 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef GPGPROC_H
#define GPGPROC_H

#include "klinebufferedprocess.h"

#include <QByteArray>
#include <QString>
#include <QStringList>


/**
 * @brief A interface to GnuPG handling UTF8 recoding correctly
 *
 * This class handles the GnuPG formatted UTF8 output correctly.
 * GnuPG recodes some characters as \\xnn where nn is the hex representation
 * of the character. This can't be fixed up simply when using QString as
 * QString already did it's own UTF8 conversion. Therefore we replace this
 * sequences by their corresponding character so QString will work just fine.
 *
 * As we know that GnuPG limits it's columns by QLatin1Char( ':' ) we skip \\x3a. Since this
 * is an ascii character (single byte) the replacement can be done later without
 * problems after the line has been split into pieces.
 *
 * @author Rolf Eike Beer
 */
class GPGProc : public KLineBufferedProcess
{
    Q_OBJECT

public:
    /**
     * Constructor
     * @param parent parent object
     * @param binary path to GnuPG binary or QString() to use the configured
     */
    explicit GPGProc(QObject *parent = nullptr, const QString &binary = QString());

    /**
     * Destructor
     */
    ~GPGProc() override;

    /**
     *  Starts the process
     */
    void start();

    /**
     * Reads a line of text (excluding '\\n').
     *
     * Use readln() in response to a readReady() signal.
     * You may use it multiple times if more than one line of data is
     * available.
     *
     * readln() never blocks.
     *
     * @param line is used to store the line that was read.
     * @param colons recode also colons
     * @return the number of characters read, or -1 if no data is available.
     */
    int readln(QString &line, const bool colons = false);

    /**
     * Reads a line of text and splits it into parts.
     *
     * Use readln() in response to a readReady() signal.
     * You may use it multiple times if more than one line of data is
     * available.
     *
     * readln() never blocks.
     *
     * @param l is used to store the parts of the line that was read.
     * @return the number of characters read, or -1 if no data is available.
     */
    int readln(QStringList &l);

    /**
     * Recode a line from GnuPG encoding
     *
     * @param a data to recode
     * @param colons recode also colons
     * @param codec the name of the new encoding. The default encoding is utf8.
     * @return recoded string
     */
    static QString recode(QByteArray a, const bool colons = true, const QByteArray &codec = QByteArray());

    /**
     * @brief sets the codec used to translate the incoming data
     * @param codec the name of the new codec
     * @return if the new codec has been accepted
     *
     * The default codec is utf8. If the given codec is not known to
     * QTextCodec the method will return false.
     */
    bool setCodec(const QByteArray &codec);

    /**
     * Reset the class to the state it had right after creation
     * @param binary path to GnuPG binary or empty string to use the configured one
     */
    void resetProcess(const QString &binary = QString());

    /**
     * @brief parse GnuPG version string and return version as number
     * @param vstr version string
     * @return -1 if vstr is empty, -2 on parse error, parsed number on success
     *
     * The version string must be in format A.B.C with A, B, and C numbers. The
     * returned number is A * 65536 + B * 256 + C.
     */
    static int gpgVersion(const QString &vstr);
    /**
     * @brief get the GnuPG version string of the given binary
     * @param binary name or path to GnuPG binary
     * @return version string or empty string on error
     *
     * This starts a GnuPG process and asks the binary for version information.
     * The returned string is the version information without any leading text.
     */
    static QString gpgVersionString(const QString &binary);

    /**
     * @brief find users GnuPG directory
     * @param binary name or path to GnuPG binary
     * @return path to directory
     *
     * Use this function to find out where GnuPG would store it's configuration
     * and data files. The returned path always ends with a '/'.
     */
    static QString getGpgHome(const QString &binary);

    /**
     * @brief get arguments to set the configuration file and home directory
     * @param binary name or path to GnuPG binary
     * @return GnuPG argument list
     */
    static QStringList getGpgHomeArguments(const QString &binary);

    /**
     * @brief return a list of the public key algorithms GnuPG announces support for
     * @param binary name or path to GnuPG binary
     * @return list of algorithm names
     *
     * All '?' entries are removed.
     */
    static QStringList getGpgPubkeyAlgorithms(const QString &binary);

    /**
     * @brief run GnuPG and check if it complains about anything
     * @param binary the GnuPG binary to run
     * @return the error message GnuPG gave out (if any)
     */
    static QString getGpgStartupError(const QString &binary);

    /**
     * @brief run GnuPG and let it return it's config output
     * @param binary the GnuPG binary to run
     * @param key if only fields of a given type should be returned
     * @return all matching fields
     *
     * In case a key is given the key is already removed from the
     * returned lines.
     */
    static QStringList getGgpParsedConfig(const QString &binary, const QByteArray &key = QByteArray());
Q_SIGNALS:
    /**
     * Emitted when the process is ready for reading.
     * The signal is only emitted if at least one complete line of data is ready.
     */
    void readReady();

    /**
     * Emitted when the process has finished
     */
    void processExited();

private:
    QByteArray m_codec;
};

#endif // GPGPROC_H
