/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Valentin Rusu <kde@rusu.info>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef BACKENDPERSISTHANDLER_H
#define BACKENDPERSISTHANDLER_H

#define KWMAGIC_LEN 12

#include <qwindowdefs.h>

class QFile;
class QSaveFile;
namespace KWallet
{
class Backend;

enum BackendCipherType {
    BACKEND_CIPHER_UNKNOWN, /// this is used by freshly allocated wallets
    BACKEND_CIPHER_BLOWFISH, /// use the legacy blowfish cipher type
#ifdef HAVE_GPGMEPP
    BACKEND_CIPHER_GPG, /// use GPG backend to encrypt wallet contents
#endif // HAVE_GPGMEPP
};

class BackendPersistHandler
{
protected:
    BackendPersistHandler()
    {
    }

public:
    virtual ~BackendPersistHandler()
    {
    }
    /**
     * This is a factory method used to get an instance of the backend suitable
     * for reading/writing using the given cipher type
     *
     * @param cipherType indication of the backend that should be returned
     * @return a pointer to an instance of the requested handler type. No need to delete this pointer, it's lifetime is taken care of by this factory
     */
    static BackendPersistHandler *getPersistHandler(BackendCipherType cipherType);
    static BackendPersistHandler *getPersistHandler(char magicBuf[KWMAGIC_LEN]);

    virtual int write(Backend *wb, QSaveFile &sf, QByteArray &version, WId w) = 0;
    virtual int read(Backend *wb, QFile &sf, WId w) = 0;
};

class BlowfishPersistHandler : public BackendPersistHandler
{
public:
    explicit BlowfishPersistHandler(bool useECBforReading = false)
        : _useECBforReading(useECBforReading)
    {
    }
    ~BlowfishPersistHandler() override
    {
    }

    int write(Backend *wb, QSaveFile &sf, QByteArray &version, WId w) override;
    int read(Backend *wb, QFile &sf, WId w) override;

private:
    bool _useECBforReading;
};

#ifdef HAVE_GPGMEPP
class GpgPersistHandler : public BackendPersistHandler
{
public:
    GpgPersistHandler()
    {
    }
    ~GpgPersistHandler() override
    {
    }

    int write(Backend *wb, QSaveFile &sf, QByteArray &version, WId w) override;
    int read(Backend *wb, QFile &sf, WId w) override;
};
#endif // HAVE_GPGMEPP

} // namespace

#endif // BACKENDPERSISTHANDLER_H
