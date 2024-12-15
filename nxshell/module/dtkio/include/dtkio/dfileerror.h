// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DFILEERROR_H
#define DFILEERROR_H

#include <QString>

#include "dtkio_global.h"

DIO_BEGIN_NAMESPACE

// TODO: refactor it
enum IOErrorCode {
    NoError = -1,   //!<@~english No error

    Failed = 0,   /// !<@~english Generic error condition for when an operation fails and no more specific DFMIOErrorEnum value is defined
    FileNotFound,   //!<@~english File not found
    FileExists,   //!<@~english//!<@~english File already exists
    FileIsDirectory,   //!<@~english File is a directory
    FileNotDirectory,   //!<@~english File is not a directory
    DirectoryNotEmpty,   //!<@~english File is a directory that isn't empty
    FileNotRegular,   //!<@~english File is not a regular file
    FileNotSymbolicLink,   //!<@~english File is not a symbolic link
    FileNotMountable,   //!<@~english File cannot be mounted
    FileNameTooLong,   //!<@~english Filename is too many characters
    FileNameInvalid,   //!<@~english Filename is invalid or contains invalid characters
    FileTooManyLinks,   //!<@~english File contains too many symbolic links
    DriveNoSpace,   //!<@~english No space left on drive
    InvalidArgument,   //!<@~english Invalid argument
    PermissionDenied,   //!<@~english Permission denied
    NotSupported,   //!<@~english Operation (or one of its parameters) not supported
    FileNotMounted,   //!<@~english File isn't mounted
    FileAlreadyMounted,   //!<@~english File is already mounted
    FileClosed,   //!<@~english File was closed
    OperationCancelled,   //!<@~english Operation was cancelled
    OperationPending,   //!<@~english Operations are still pending
    FileReadOnly,   //!<@~english File is read only
    CantCreateBackup,   //!<@~english Backup couldn't be created
    FileWrongEtag,   //!<@~english File's Entity Tag was incorrect
    TimedOut,   //!<@~english Operation timed out
    WouldRecurse,   //!<@~english Operation would be recursive
    FileBusy,   //!<@~english File is busy
    WouldBlock,   //!<@~english Operation would block
    HostNotFound,   //!<@~english Host couldn't be found (remote operations)
    WouldMerge,   //!<@~english Operation would merge files
    FailedHandled,   //!<@~english Operation failed and a helper program has already interacted with the user
    //!<@~english Do not display any error dialog
    TooManyOpenFiles,   //!<@~english The current process has too many files open and can't open any more
    //!<@~english Duplicate descriptors do count toward this limit
    NotInitialized,   //!<@~english The object has not been initialized
    AddressInUse,   //!<@~english The requested address is already in use
    PartialInput,   //!<@~english Need more input to finish operation
    InvalidData,   //!<@~english The input data was invalid
    DbusError,   //!<@~english A remote object generated an error(dbus)
    HostUnreachable,   //!<@~english Host unreachable
    NetworkUnreachable,   //!<@~english Network unreachable
    ConnectionRefused,   //!<@~english Connection refused
    ProxyFailed,   //!<@~english Connection to proxy server failed
    ProxyAuthFailed,   //!<@~english Proxy authentication failed
    ProxyNeedAuth,   //!<@~english Proxy server needs authentication
    ProxyNotAllowed,   //!<@~english Proxy connection is not allowed by ruleset
    BrokenPipe,   //!<@~english Broken pipe
    ConnectionClosed,   //!<@~english Connection closed by peer
    NotConnected,   //!<@~english Transport endpoint is not connected
    MessageTooLarge,   //!<@~english Message too large

    CustomFailed = 1000,   //!<@~english custom failed
    OpenFailed,   //!<@~english File open failed
    OpenFlagError,   //!<@~english File open flag is error
    FileAlreadyOpened,   //!<@~english File is already opened
    InfoNoAttribute,   //!<@~english File info has no attribute
};

inline const QString IOErrorMessage(IOErrorCode errorCode)
{
    switch (errorCode) {
    case Failed:
        return QString("Generic error condition for when an operation fails and no more specific DFMIOErrorEnum value is defined");
    case NoError:
        return QString("No error");
    case FileNotFound:
        return QString("File not found");
    case FileExists:
        return QString("File already exists");
    case FileIsDirectory:
        return QString("File is a directory");
    case FileNotDirectory:
        return QString("File is not a directory");
    case DirectoryNotEmpty:
        return QString("File is a directory that isn't empty");
    case FileNotRegular:
        return QString("File is not a regular file");
    case FileNotSymbolicLink:
        return QString("File is not a symbolic link");
    case FileNotMountable:
        return QString("File cannot be mounted");
    case FileNameTooLong:
        return QString("Filename is too many characters");
    case FileNameInvalid:
        return QString("Filename is invalid or contains invalid characters");
    case FileTooManyLinks:
        return QString("File contains too many symbolic links");
    case DriveNoSpace:
        return QString("No space left on drive");
    case InvalidArgument:
        return QString("Invalid argument");
    case PermissionDenied:
        return QString("Permission denied");
    case NotSupported:
        return QString("Operation (or one of its parameters) not supported");
    case FileNotMounted:
        return QString("File isn't mounted");
    case FileAlreadyMounted:
        return QString("File is already mounted");
    case FileClosed:
        return QString("File was closed");
    case OperationCancelled:
        return QString("Operation was cancelled");
    case OperationPending:
        return QString("Operations are still pending");
    case FileReadOnly:
        return QString("File is read only");
    case CantCreateBackup:
        return QString("Backup couldn't be created");
    case FileWrongEtag:
        return QString("File's Entity Tag was incorrect");
    case TimedOut:
        return QString("Operation timed out");
    case WouldRecurse:
        return QString("Operation would be recursive");
    case FileBusy:
        return QString("File is busy");
    case WouldBlock:
        return QString("Operation would block");
    case HostNotFound:
        return QString("Host couldn't be found (remote operations)");
    case WouldMerge:
        return QString("Operation would merge files");
    case FailedHandled:
        return QString("Operation failed and a helper program has already interacted with the user. Do not display any error dialog");
    case TooManyOpenFiles:
        return QString("The current process has too many files open and can't open any more. Duplicate descriptors do count toward this limit");
    case NotInitialized:
        return QString("The object has not been initialized");
    case AddressInUse:
        return QString("The requested address is already in use");
    case PartialInput:
        return QString("Need more input to finish operation");
    case InvalidData:
        return QString("The input data was invalid");
    case DbusError:
        return QString("A remote object generated an error(dbus)");
    case HostUnreachable:
        return QString("Host unreachable");
    case NetworkUnreachable:
        return QString("Network unreachable");
    case ConnectionRefused:
        return QString("Connection refused");
    case ProxyFailed:
        return QString("Connection to proxy server failed");
    case ProxyAuthFailed:
        return QString("Proxy authentication failed");
    case ProxyNeedAuth:
        return QString("Proxy server needs authentication");
    case ProxyNotAllowed:
        return QString("Proxy connection is not allowed by ruleset");
    case BrokenPipe:
        return QString("Broken pipe");
    case ConnectionClosed:
        return QString("Connection closed by peer");
    case NotConnected:
        return QString("Transport endpoint is not connected");
    case MessageTooLarge:
        return QString("Message too large");
    case CustomFailed:
        return QString("Custom failed");
    case OpenFailed:
        return QString("File open failed");
    case OpenFlagError:
        return QString("File open flag error");
    case FileAlreadyOpened:
        return QString("File is already opened");
    case InfoNoAttribute:
        return QString("File info has no attribute");
    }

    return QString("Unknown error");
}

DIO_END_NAMESPACE
#endif   // DFILEERROR_H
