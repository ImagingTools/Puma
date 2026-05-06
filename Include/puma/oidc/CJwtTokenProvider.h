// SPDX-License-Identifier: LicenseRef-Puma-Commercial
#pragma once

/**
* @file CJwtTokenProvider.h
* @brief JWT token generation and verification using OpenSSL.
*
* This header declares the CJwtTokenProvider class, which handles
* JWT (JSON Web Token) creation, signing, and verification for
* OIDC ID tokens and OAuth 2.0 access tokens.
*
* The implementation uses OpenSSL for RSA key operations and
* SHA-256 hashing, with no additional third-party JWT libraries.
*
* @section supported_algorithms Supported Algorithms
* - RS256 (RSA with SHA-256) — recommended for production
*
* @section token_format Token Format
* Tokens follow the JWT specification (RFC 7519):
* - Header: {"alg": "RS256", "typ": "JWT", "kid": "<key-id>"}
* - Payload: OIDC/OAuth claims
* - Signature: RSA-SHA256 signature of header.payload
*
* @ingroup OidcProvider
*/

// Qt includes
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QJsonObject>

// Local includes
#include "OidcTypes.h"


namespace Oidc
{


class CJwtTokenProviderImpl;


/**
* @brief JWT token provider for OIDC token generation and verification.
*
* This class provides methods to:
* - Load RSA key pairs from PEM files
* - Generate signed JWT tokens (ID tokens and access tokens)
* - Verify and decode JWT tokens
* - Export public keys in JWKS (JSON Web Key Set) format
*
* @section usage_example Usage Example
* @code
* Oidc::CJwtTokenProvider provider;
*
* // Load signing keys
* if (!provider.LoadSigningKey("/path/to/private_key.pem")) {
*     qCritical() << "Failed to load signing key";
*     return;
* }
* provider.LoadVerificationKey("/path/to/public_key.pem");
*
* // Generate an ID token
* Oidc::TokenClaims claims;
* claims.iss = "https://auth.example.com";
* claims.sub = "user-123";
* claims.aud = "my-client-app";
* claims.iat = QDateTime::currentSecsSinceEpoch();
* claims.exp = claims.iat + 3600;
* claims.name = "John Doe";
* claims.email = "john@example.com";
*
* QString idToken = provider.GenerateToken(claims);
*
* // Verify a token
* Oidc::TokenClaims decoded;
* if (provider.VerifyToken(idToken, decoded)) {
*     qDebug() << "Token valid, subject:" << decoded.sub;
* }
* @endcode
*
* @section thread_safety Thread Safety
* The class is thread-safe for concurrent token generation and
* verification after key loading is complete. Key loading methods
* should not be called concurrently with other operations.
*/
class CJwtTokenProvider
{
public:
	CJwtTokenProvider();
	~CJwtTokenProvider();

	// Non-copyable
	CJwtTokenProvider(const CJwtTokenProvider&) = delete;
	CJwtTokenProvider& operator=(const CJwtTokenProvider&) = delete;

	/**
	* @brief Loads the RSA private key for token signing.
	*
	* @param pemFilePath Path to a PEM-encoded RSA private key file.
	* @return true if the key was loaded successfully.
	* @return false if the file cannot be read or the key is invalid.
	*
	* @note Call this before generating tokens.
	*/
	bool LoadSigningKey(const QString& pemFilePath);

	/**
	* @brief Loads the RSA public key for token verification.
	*
	* @param pemFilePath Path to a PEM-encoded RSA public key file.
	* @return true if the key was loaded successfully.
	* @return false if the file cannot be read or the key is invalid.
	*
	* @note Call this before verifying tokens.
	*/
	bool LoadVerificationKey(const QString& pemFilePath);

	/**
	* @brief Generates a signed JWT token from claims.
	*
	* Creates a JWT with the RS256 algorithm, encoding the provided
	* claims as the payload and signing with the loaded private key.
	*
	* @param claims Token claims to encode.
	* @return Signed JWT string (header.payload.signature), or empty
	*         string if signing fails.
	*
	* @pre LoadSigningKey() must have been called successfully.
	*
	* @see VerifyToken()
	*/
	QString GenerateToken(const TokenClaims& claims) const;

	/**
	* @brief Verifies a JWT token and extracts its claims.
	*
	* Validates the token signature using the loaded public key,
	* checks the expiration time, and decodes the claims.
	*
	* @param token JWT string to verify.
	* @param[out] claims Decoded claims (populated on success).
	* @return true if the token signature is valid and not expired.
	* @return false if verification fails (invalid signature, expired,
	*         malformed token).
	*
	* @pre LoadVerificationKey() must have been called successfully.
	*
	* @see GenerateToken()
	*/
	bool VerifyToken(const QString& token, TokenClaims& claims) const;

	/**
	* @brief Returns the public key in JWKS (JSON Web Key Set) format.
	*
	* Generates a JSON object containing the public key parameters
	* suitable for the `/.well-known/jwks.json` endpoint.
	*
	* @return QJsonObject with "keys" array containing the public key,
	*         or empty object if no verification key is loaded.
	*
	* @section jwks_format JWKS Format
	* @code
	* {
	*   "keys": [{
	*     "kty": "RSA",
	*     "use": "sig",
	*     "alg": "RS256",
	*     "kid": "<key-id>",
	*     "n": "<modulus>",
	*     "e": "<exponent>"
	*   }]
	* }
	* @endcode
	*
	* @pre LoadVerificationKey() must have been called successfully.
	*/
	QJsonObject GetJwks() const;

	/**
	* @brief Returns the key ID used in token headers.
	* @return Key identifier string.
	*/
	QString GetKeyId() const;

	/**
	* @brief Generates a cryptographically secure random string.
	*
	* Used for generating authorization codes, state parameters,
	* and other security tokens.
	*
	* @param length Number of random bytes (output will be Base64url-encoded).
	* @return Base64url-encoded random string.
	*/
	static QString GenerateRandomString(int length = 32);

	/**
	* @brief Computes SHA-256 hash and returns Base64url-encoded result.
	*
	* Used for PKCE code challenge verification (S256 method).
	*
	* @param input String to hash.
	* @return Base64url-encoded SHA-256 hash.
	*/
	static QString ComputeS256Challenge(const QString& input);

private:
	CJwtTokenProviderImpl* m_implPtr;
};


} // namespace Oidc
