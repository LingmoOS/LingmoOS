/*
 * Compilation Copyright (c) 1995-2019 by Wei Dai.  All rights reserved.
 *	This copyright applies only to this software distribution package
 *	as a compilation, and does not imply a copyright on any particular
 *	file in the package.
 * 
 *	Boost Software License - Version 1.0 - August 17th, 2003
 *
 *	Permission is hereby granted, free of charge, to any person or organization
 *	obtaining a copy of the software and accompanying documentation covered by
 *	this license (the "Software") to use, reproduce, display, distribute,
 *	execute, and transmit the Software, and to prepare derivative works of the
 *	Software, and to permit third-parties to whom the Software is furnished to
 *	do so, all subject to the following:
 * 
 *	The copyright notices in the Software and this entire statement, including
 *	the above license grant, this restriction and the following disclaimer,
 *	must be included in all copies of the Software, in whole or in part, and
 *	all derivative works of the Software, unless such copies or derivative
 *	works are solely in the form of machine-executable object code generated by
 *	a source language processor.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 *	SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 *	FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 *	ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *	DEALINGS IN THE SOFTWARE.
 */

// dh2.h - originally written and placed in the public domain by Wei Dai

/// \file dh2.h
/// \brief Classes for Unified Diffie-Hellman key exchange
/// \since Crypto++ 3.0

#ifndef CRYPTOPP_DH2_H
#define CRYPTOPP_DH2_H

#include "cryptlib.h"

NAMESPACE_BEGIN(CryptoPP)

/// \brief Unified Diffie-Hellman in GF(p)
/// \details A Diffie-Hellman domain is a set of parameters that must be shared
///   by two parties in a key agreement protocol, along with the algorithms
///   for generating key pairs and deriving agreed values.
/// \sa AuthenticatedKeyAgreementDomain, <a href="http://www.weidai.com/scan-mirror/ka.html#DH2">Unified Diffie-Hellman</a>
/// \since Crypto++ 3.0
class DH2 : public AuthenticatedKeyAgreementDomain
{
public:
	virtual ~DH2() {}

	/// \brief Construct a DH2
	DH2(SimpleKeyAgreementDomain &domain)
		: d1(domain), d2(domain) {}
	/// \brief Construct a DH2
	DH2(SimpleKeyAgreementDomain &staticDomain, SimpleKeyAgreementDomain &ephemeralDomain)
		: d1(staticDomain), d2(ephemeralDomain) {}

	CryptoParameters & AccessCryptoParameters() {return d1.AccessCryptoParameters();}

	unsigned int AgreedValueLength() const
		{return d1.AgreedValueLength() + d2.AgreedValueLength();}

	unsigned int StaticPrivateKeyLength() const
		{return d1.PrivateKeyLength();}
	unsigned int StaticPublicKeyLength() const
		{return d1.PublicKeyLength();}
	void GenerateStaticPrivateKey(RandomNumberGenerator &rng, byte *privateKey) const
		{d1.GeneratePrivateKey(rng, privateKey);}
	void GenerateStaticPublicKey(RandomNumberGenerator &rng, const byte *privateKey, byte *publicKey) const
		{d1.GeneratePublicKey(rng, privateKey, publicKey);}
	void GenerateStaticKeyPair(RandomNumberGenerator &rng, byte *privateKey, byte *publicKey) const
		{d1.GenerateKeyPair(rng, privateKey, publicKey);}

	unsigned int EphemeralPrivateKeyLength() const
		{return d2.PrivateKeyLength();}
	unsigned int EphemeralPublicKeyLength() const
		{return d2.PublicKeyLength();}
	void GenerateEphemeralPrivateKey(RandomNumberGenerator &rng, byte *privateKey) const
		{d2.GeneratePrivateKey(rng, privateKey);}
	void GenerateEphemeralPublicKey(RandomNumberGenerator &rng, const byte *privateKey, byte *publicKey) const
		{d2.GeneratePublicKey(rng, privateKey, publicKey);}
	void GenerateEphemeralKeyPair(RandomNumberGenerator &rng, byte *privateKey, byte *publicKey) const
		{d2.GenerateKeyPair(rng, privateKey, publicKey);}

	bool Agree(byte *agreedValue,
		const byte *staticPrivateKey, const byte *ephemeralPrivateKey,
		const byte *staticOtherPublicKey, const byte *ephemeralOtherPublicKey,
		bool validateStaticOtherPublicKey=true) const;

protected:
	SimpleKeyAgreementDomain &d1, &d2;
};

NAMESPACE_END

#endif
