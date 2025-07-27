/*
    SPDX-FileCopyrightText: 2008 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KLINEBUFFEREDPROCESS_H
#define KLINEBUFFEREDPROCESS_H

#include <KProcess>

#include "klinebufferedprocessprivate.h"

class QByteArray;

/**
 * Read output of a process split into lines
 *
 * This class reads the output of a process and splits it up into lines. This
 * is especially useful if you try to parse the output of a command line tool.
 *
 * \b Usage \n
 *
 * The class is created and set up like a KProcess. After this you can do
 * something like this:
 *
 * \code
 * connect(m_linebufprocess, &KLineBufferedProcess::lineReadyStandardOutput, this, &Class::dataStdout);
 * ...
 * void myobj::dataStdout()
 * {
 *   while (m_linebufprocess->hasLineStandardOutput()) {
 *     QByteArray line;
 *     m_linebufprocess->readLineStandardOutput(line);
 *     ...
 *   }
 * }
 * \endcode
 *
 * Never use the read functionality of KProcess with this class. This class
 * needs to read all data from the process into an internal buffer first. If
 * you try to use the read functions of the parent classes you would normally
 * get no output at all.
 *
 * The write functions of the parent classes are not effected. You can use
 * them exactly the same way as in KProcess.
 *
 * @author Rolf Eike Beer
 */
class KLineBufferedProcess : public KProcess
{
    Q_OBJECT
    friend class KLineBufferedProcessPrivate;

public:
    /**
     * Constructor
     */
    explicit KLineBufferedProcess(QObject *parent = nullptr);

    /**
     * Destructor
     */
    ~KLineBufferedProcess() override;

    /**
     * Reads a line of text (excluding '\\n') from stdout.
     *
     * Use readLineStdout() in response to a lineReadyStdout() signal or
     * when hasLineStdout() returns true. You may use it multiple times if
     * more than one line of data is available. If no complete line is
     * available the content of line is undefined and the function returns
     * false.
     *
     * @param line is used to store the line that was read.
     * @return if data was read or not
     */
    bool readLineStandardOutput(QByteArray *line);

    /**
     * Reads a line of text (excluding '\\n') from stderr.
     *
     * Use readLineStderr() in response to a lineReadyStderr() signal or
     * when hasLineStderr() returns true. You may use it multiple times if
     * more than one line of data is available. If no complete line is
     * available the content of line is undefined and the function returns
     * false.
     *
     * @param line is used to store the line that was read.
     * @return if data was read or not
     */
    bool readLineStandardError(QByteArray *line);

    /**
     * Checks if a line is ready on stdout
     *
     * @return true if a complete line can be read
     */
    bool hasLineStandardOutput() const;

    /**
     * Checks if a line is ready on stdout
     *
     * @return true if a complete line can be read
     */
    bool hasLineStandardError() const;

Q_SIGNALS:
    /**
     * Emitted when there is a line of data available from stdout when there was
     * previously none.
     * There may or may not be more than one line available for reading when this
     * signal is emitted.
     */
    void lineReadyStandardOutput();

    /**
     * Emitted when there is a line of data available from stderr when there was
     * previously none.
     * There may or may not be more than one line available for reading when this
     * signal is emitted.
     */
    void lineReadyStandardError();

private:
    KLineBufferedProcessPrivate* const d;
};

#endif // KLINEBUFFEREDPROCESS_H
