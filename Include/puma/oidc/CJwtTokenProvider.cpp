// SPDX-License-Identifier: LicenseRef-Puma-Commercial
/**
 * @file CJwtTokenProvider.cpp
 * @brief Implementation of the CJwtTokenProvider PIMPL wrapper.
 *
 * This file implements the public CJwtTokenProvider class using
 * OpenSSL EVP API for RS256 JWT token operations. It delegates
 * to an internal implementation class that manages the OpenSSL
 * key context and performs cryptographic operations.
 *
 * @ingroup OidcProvider
 */

#include <puma/oidc/CJwtTokenProvider.h>

// Qt includes
#include <QtCore/QCryptographicHash>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QUuid>

// OpenSSL includes
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/err.h>


namespace Oidc
{


/**
 * @brief Internal implementation class for CJwtTokenProvider.
 *
 * Manages OpenSSL EVP_PKEY context for RSA signing and verification.
 */
class CJwtTokenProviderImpl
{
public:
	CJwtTokenProviderImpl()
		: m_signingKey(nullptr)
		, m_verificationKey(nullptr)
		, m_keyId(QUuid::createUuid().toString(QUuid::WithoutBraces))
	{
	}

	~CJwtTokenProviderImpl()
	{
		if (m_signingKey != nullptr){
			EVP_PKEY_free(m_signingKey);
		}
		if (m_verificationKey != nullptr){
			EVP_PKEY_free(m_verificationKey);
		}
	}

	bool LoadSigningKey(const QString& pemFilePath)
	{
		QFile file(pemFilePath);
		if (!file.open(QIODevice::ReadOnly)){
			qWarning() << "CJwtTokenProvider: Cannot open signing key file:" << pemFilePath;
			return false;
		}

		QByteArray keyData = file.readAll();
		file.close();

		BIO* bio = BIO_new_mem_buf(keyData.constData(), keyData.size());
		if (bio == nullptr){
			return false;
		}

		EVP_PKEY* key = PEM_read_bio_PrivateKey(bio, nullptr, nullptr, nullptr);
		BIO_free(bio);

		if (key == nullptr){
			qWarning() << "CJwtTokenProvider: Failed to parse signing key";
			return false;
		}

		if (m_signingKey != nullptr){
			EVP_PKEY_free(m_signingKey);
		}
		m_signingKey = key;

		return true;
	}

	bool LoadVerificationKey(const QString& pemFilePath)
	{
		QFile file(pemFilePath);
		if (!file.open(QIODevice::ReadOnly)){
			qWarning() << "CJwtTokenProvider: Cannot open verification key file:" << pemFilePath;
			return false;
		}

		QByteArray keyData = file.readAll();
		file.close();

		BIO* bio = BIO_new_mem_buf(keyData.constData(), keyData.size());
		if (bio == nullptr){
			return false;
		}

		EVP_PKEY* key = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
		BIO_free(bio);

		if (key == nullptr){
			qWarning() << "CJwtTokenProvider: Failed to parse verification key";
			return false;
		}

		if (m_verificationKey != nullptr){
			EVP_PKEY_free(m_verificationKey);
		}
		m_verificationKey = key;

		return true;
	}

	QString GenerateToken(const TokenClaims& claims) const
	{
		if (m_signingKey == nullptr){
			qWarning() << "CJwtTokenProvider: No signing key loaded";
			return {};
		}

		// Header
		QJsonObject header;
		header["alg"] = "RS256";
		header["typ"] = "JWT";
		header["kid"] = m_keyId;

		QByteArray headerJson = QJsonDocument(header).toJson(QJsonDocument::Compact);
		QByteArray payloadJson = QJsonDocument(claims.ToJson()).toJson(QJsonDocument::Compact);

		QByteArray headerB64 = headerJson.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
		QByteArray payloadB64 = payloadJson.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

		QByteArray signingInput = headerB64 + "." + payloadB64;

		// Sign
		EVP_MD_CTX* ctx = EVP_MD_CTX_new();
		if (ctx == nullptr){
			return {};
		}

		if (EVP_DigestSignInit(ctx, nullptr, EVP_sha256(), nullptr, m_signingKey) != 1){
			EVP_MD_CTX_free(ctx);
			return {};
		}

		if (EVP_DigestSignUpdate(ctx, signingInput.constData(), static_cast<size_t>(signingInput.size())) != 1){
			EVP_MD_CTX_free(ctx);
			return {};
		}

		size_t signatureLen = 0;
		if (EVP_DigestSignFinal(ctx, nullptr, &signatureLen) != 1){
			EVP_MD_CTX_free(ctx);
			return {};
		}

		QByteArray signature(static_cast<int>(signatureLen), '\0');
		if (EVP_DigestSignFinal(ctx, reinterpret_cast<unsigned char*>(signature.data()), &signatureLen) != 1){
			EVP_MD_CTX_free(ctx);
			return {};
		}
		signature.resize(static_cast<int>(signatureLen));

		EVP_MD_CTX_free(ctx);

		QByteArray signatureB64 = signature.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

		return QString::fromLatin1(signingInput + "." + signatureB64);
	}

	bool VerifyToken(const QString& token, TokenClaims& claims) const
	{
		if (m_verificationKey == nullptr){
			qWarning() << "CJwtTokenProvider: No verification key loaded";
			return false;
		}

		QByteArray tokenBytes = token.toLatin1();
		QList<QByteArray> parts = tokenBytes.split('.');
		if (parts.size() != 3){
			return false;
		}

		QByteArray signingInput = parts[0] + "." + parts[1];
		QByteArray signature = QByteArray::fromBase64(parts[2], QByteArray::Base64UrlEncoding);

		// Verify signature
		EVP_MD_CTX* ctx = EVP_MD_CTX_new();
		if (ctx == nullptr){
			return false;
		}

		if (EVP_DigestVerifyInit(ctx, nullptr, EVP_sha256(), nullptr, m_verificationKey) != 1){
			EVP_MD_CTX_free(ctx);
			return false;
		}

		if (EVP_DigestVerifyUpdate(ctx, signingInput.constData(), static_cast<size_t>(signingInput.size())) != 1){
			EVP_MD_CTX_free(ctx);
			return false;
		}

		int result = EVP_DigestVerifyFinal(ctx, reinterpret_cast<const unsigned char*>(signature.constData()), static_cast<size_t>(signature.size()));
		EVP_MD_CTX_free(ctx);

		if (result != 1){
			return false;
		}

		// Decode payload
		QByteArray payloadJson = QByteArray::fromBase64(parts[1], QByteArray::Base64UrlEncoding);
		QJsonDocument doc = QJsonDocument::fromJson(payloadJson);
		if (doc.isNull()){
			return false;
		}

		claims.FromJson(doc.object());

		// Check expiration
		qint64 now = QDateTime::currentSecsSinceEpoch();
		if (claims.exp != 0 && claims.exp < now){
			return false;
		}

		return true;
	}

	QJsonObject GetJwks() const
	{
		if (m_verificationKey == nullptr){
			return {};
		}

		QJsonObject jwk;
		jwk["kty"] = "RSA";
		jwk["use"] = "sig";
		jwk["alg"] = "RS256";
		jwk["kid"] = m_keyId;

#if OPENSSL_VERSION_NUMBER >= 0x30000000L
		BIGNUM* n = nullptr;
		BIGNUM* e = nullptr;
		if (EVP_PKEY_get_bn_param(m_verificationKey, "n", &n) == 1 &&
			EVP_PKEY_get_bn_param(m_verificationKey, "e", &e) == 1){
			QByteArray nBytes(BN_num_bytes(n), '\0');
			BN_bn2bin(n, reinterpret_cast<unsigned char*>(nBytes.data()));
			jwk["n"] = QString::fromLatin1(nBytes.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));

			QByteArray eBytes(BN_num_bytes(e), '\0');
			BN_bn2bin(e, reinterpret_cast<unsigned char*>(eBytes.data()));
			jwk["e"] = QString::fromLatin1(eBytes.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
		}
		if (n != nullptr) BN_free(n);
		if (e != nullptr) BN_free(e);
#else
		const RSA* rsa = EVP_PKEY_get0_RSA(m_verificationKey);
		if (rsa != nullptr){
			const BIGNUM* n = nullptr;
			const BIGNUM* e = nullptr;
			RSA_get0_key(rsa, &n, &e, nullptr);

			if (n != nullptr && e != nullptr){
				QByteArray nBytes(BN_num_bytes(n), '\0');
				BN_bn2bin(n, reinterpret_cast<unsigned char*>(nBytes.data()));
				jwk["n"] = QString::fromLatin1(nBytes.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));

				QByteArray eBytes(BN_num_bytes(e), '\0');
				BN_bn2bin(e, reinterpret_cast<unsigned char*>(eBytes.data()));
				jwk["e"] = QString::fromLatin1(eBytes.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
			}
		}
#endif

		QJsonArray keys;
		keys.append(jwk);

		QJsonObject jwks;
		jwks["keys"] = keys;

		return jwks;
	}

	QString GetKeyId() const
	{
		return m_keyId;
	}

private:
	EVP_PKEY* m_signingKey;
	EVP_PKEY* m_verificationKey;
	QString m_keyId;
};


// Public API implementation

CJwtTokenProvider::CJwtTokenProvider()
	: m_implPtr(new CJwtTokenProviderImpl)
{
}

CJwtTokenProvider::~CJwtTokenProvider()
{
	delete m_implPtr;
}

bool CJwtTokenProvider::LoadSigningKey(const QString& pemFilePath)
{
	return m_implPtr->LoadSigningKey(pemFilePath);
}

bool CJwtTokenProvider::LoadVerificationKey(const QString& pemFilePath)
{
	return m_implPtr->LoadVerificationKey(pemFilePath);
}

QString CJwtTokenProvider::GenerateToken(const TokenClaims& claims) const
{
	return m_implPtr->GenerateToken(claims);
}

bool CJwtTokenProvider::VerifyToken(const QString& token, TokenClaims& claims) const
{
	return m_implPtr->VerifyToken(token, claims);
}

QJsonObject CJwtTokenProvider::GetJwks() const
{
	return m_implPtr->GetJwks();
}

QString CJwtTokenProvider::GetKeyId() const
{
	return m_implPtr->GetKeyId();
}

QString CJwtTokenProvider::GenerateRandomString(int length)
{
	QByteArray randomData(length, '\0');
	QFile urandom("/dev/urandom");
	if (urandom.open(QIODevice::ReadOnly)){
		urandom.read(randomData.data(), length);
		urandom.close();
	}
	else{
		// Fallback: use QUuid-based generation
		for (int i = 0; i < length; ++i){
			randomData[i] = static_cast<char>(QUuid::createUuid().toByteArray().at(i % 38));
		}
	}

	return QString::fromLatin1(randomData.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
}

QString CJwtTokenProvider::ComputeS256Challenge(const QString& input)
{
	QByteArray hash = QCryptographicHash::hash(input.toUtf8(), QCryptographicHash::Sha256);
	return QString::fromLatin1(hash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
}


} // namespace Oidc
