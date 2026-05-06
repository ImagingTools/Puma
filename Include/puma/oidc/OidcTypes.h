// SPDX-License-Identifier: LicenseRef-Puma-Commercial
#pragma once

/**
* @file OidcTypes.h
* @brief OpenID Connect data structures and type definitions.
*
* This header defines the core data structures used throughout
* the OIDC provider implementation, including client registrations,
* token claims, authorization requests, and server configuration.
*
* @section oidc_overview OpenID Connect Overview
*
* OpenID Connect (OIDC) is an identity layer built on top of OAuth 2.0.
* It allows clients to verify the identity of end-users and obtain
* basic profile information in an interoperable and REST-like manner.
*
* @section supported_flows Supported Flows
*
* Currently supported:
* - Authorization Code Flow with PKCE (recommended)
* - Refresh Token Flow
*
* @ingroup OidcProvider
*/

// Qt includes
#include <QtCore/QByteArray>
#include <QtCore/QByteArrayList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QJsonObject>

// STL includes
#include <optional>


namespace Oidc
{


/**
* @brief Registered OIDC client information.
*
* Represents a client application registered with the OIDC provider.
* Clients must be registered before they can initiate authentication
* flows. Registration includes specifying allowed redirect URIs,
* grant types, and scopes.
*
* @see COidcAuthorizationEndpoint, COidcTokenEndpoint
*/
struct ClientRegistration
{
	/** @brief Unique client identifier issued during registration. */
	QString clientId;

	/**
	* @brief Hashed client secret for confidential clients.
	*
	* Public clients (native/SPA apps using PKCE) may have an empty
	* secret. Confidential clients must authenticate with this secret
	* at the token endpoint.
	*/
	QString clientSecret;

	/** @brief Human-readable client application name. */
	QString clientName;

	/**
	* @brief Allowed redirect URIs for authorization responses.
	*
	* The authorization endpoint will only redirect to URIs that
	* exactly match one of these registered values. This prevents
	* open redirect vulnerabilities.
	*
	* @warning Exact string matching is used — no wildcards or patterns.
	*/
	QStringList redirectUris;

	/**
	* @brief Allowed OAuth 2.0 grant types.
	*
	* Supported values:
	* - "authorization_code" — Standard authorization code flow
	* - "refresh_token" — Token refresh
	*
	* @note "implicit" and "client_credentials" are not currently supported.
	*/
	QStringList grantTypes;

	/**
	* @brief Allowed response types for authorization requests.
	*
	* Supported values:
	* - "code" — Authorization code flow
	*/
	QStringList responseTypes;

	/**
	* @brief Default scopes requested when none are specified.
	*
	* Standard OIDC scopes:
	* - "openid" — Required for OIDC (returns sub claim)
	* - "profile" — Name, preferred_username
	* - "email" — Email address
	* - "roles" — Puma role assignments
	* - "groups" — Puma group memberships
	* - "permissions" — Puma permissions
	*/
	QStringList scopes;

	/**
	* @brief Token endpoint authentication method.
	*
	* Supported values:
	* - "client_secret_basic" — HTTP Basic authentication (default)
	* - "client_secret_post" — Client credentials in POST body
	* - "none" — Public clients (must use PKCE)
	*/
	QString tokenEndpointAuthMethod = "client_secret_basic";

	/** @brief Whether the client is currently active. */
	bool isActive = true;
};


/**
* @brief JWT token claims for ID tokens and access tokens.
*
* Contains both standard OIDC claims and Puma-specific claims
* for roles, groups, and permissions.
*
* @section standard_claims Standard OIDC Claims
* - iss, sub, aud, iat, exp, auth_time, nonce
*
* @section puma_claims Puma-Specific Claims
* - name, email, preferred_username
* - roles, groups, permissions
*
* @see CJwtTokenProvider
*/
struct TokenClaims
{
	/** @brief Issuer identifier (authorization server URL). */
	QString iss;

	/** @brief Subject identifier (unique user ID). */
	QString sub;

	/** @brief Audience (client_id the token is intended for). */
	QString aud;

	/** @brief Issued-at time (Unix timestamp). */
	qint64 iat = 0;

	/** @brief Expiration time (Unix timestamp). */
	qint64 exp = 0;

	/** @brief Time of user authentication (Unix timestamp). */
	qint64 auth_time = 0;

	/** @brief Nonce value from the authorization request. */
	QString nonce;

	// Profile claims

	/** @brief User's display name. */
	QString name;

	/** @brief User's email address. */
	QString email;

	/** @brief User's preferred login identifier. */
	QString preferred_username;

	// Puma-specific claims

	/** @brief User's assigned role names. */
	QStringList roles;

	/** @brief User's group memberships. */
	QStringList groups;

	/** @brief User's aggregated permission identifiers. */
	QByteArrayList permissions;

	/**
	* @brief Converts claims to a JSON object for JWT payload.
	* @return QJsonObject containing all non-empty claims.
	*/
	QJsonObject ToJson() const;

	/**
	* @brief Populates claims from a JSON object.
	* @param json JSON object containing JWT payload.
	*/
	void FromJson(const QJsonObject& json);
};


/**
* @brief OIDC authorization request parameters.
*
* Represents the parameters sent by a client to the authorization
* endpoint. Includes PKCE parameters for public client security.
*
* @see COidcAuthorizationEndpoint
*/
struct AuthorizationRequest
{
	/** @brief Client identifier. */
	QString clientId;

	/**
	* @brief Requested response type.
	*
	* Only "code" is supported (Authorization Code Flow).
	*/
	QString responseType;

	/**
	* @brief Requested scopes (space-separated in the original request).
	*
	* Must include "openid" for OIDC requests.
	*/
	QStringList scope;

	/** @brief URI to redirect the user-agent after authorization. */
	QString redirectUri;

	/**
	* @brief Opaque state value for CSRF protection.
	*
	* The authorization server returns this value unchanged in the
	* redirect response. Clients should verify it matches their
	* original value.
	*/
	QString state;

	/**
	* @brief Nonce for replay attack prevention.
	*
	* Included in the ID token so the client can verify the token
	* was issued for this specific request.
	*/
	QString nonce;

	// PKCE parameters

	/**
	* @brief PKCE code challenge.
	*
	* Base64url-encoded SHA-256 hash of the code_verifier, or the
	* plain code_verifier value when code_challenge_method is "plain".
	*/
	QString codeChallenge;

	/**
	* @brief PKCE code challenge method.
	*
	* Supported values:
	* - "S256" — SHA-256 hash (recommended)
	* - "plain" — Plain text (less secure, not recommended)
	*/
	QString codeChallengeMethod;

	/**
	* @brief Optional prompt parameter.
	*
	* Supported values:
	* - "none" — No user interaction
	* - "login" — Force re-authentication
	* - "consent" — Force consent screen
	*/
	QString prompt;

	/** @brief Optional login hint to pre-fill the username. */
	QString loginHint;
};


/**
* @brief Token endpoint request parameters.
*
* Represents a request to exchange an authorization code for tokens,
* or to refresh an existing token.
*
* @see COidcTokenEndpoint
*/
struct TokenRequest
{
	/**
	* @brief Grant type being requested.
	*
	* Supported values:
	* - "authorization_code" — Exchange code for tokens
	* - "refresh_token" — Refresh access/ID tokens
	*/
	QString grantType;

	/** @brief Authorization code (for authorization_code grant). */
	QString code;

	/** @brief Redirect URI used in the original authorization request. */
	QString redirectUri;

	/** @brief Client identifier. */
	QString clientId;

	/** @brief Client secret (for confidential clients). */
	QString clientSecret;

	/** @brief PKCE code verifier (for public clients). */
	QString codeVerifier;

	/** @brief Refresh token (for refresh_token grant). */
	QString refreshToken;
};


/**
* @brief Token endpoint response.
*
* Contains the tokens issued by the token endpoint after a
* successful authorization code exchange or token refresh.
*/
struct TokenResponse
{
	/** @brief OAuth 2.0 access token (JWT). */
	QString accessToken;

	/** @brief Token type (always "Bearer"). */
	QString tokenType = "Bearer";

	/** @brief Access token lifetime in seconds. */
	int expiresIn = 3600;

	/** @brief Refresh token for obtaining new access tokens. */
	QString refreshToken;

	/** @brief OIDC ID token (JWT containing user identity claims). */
	QString idToken;

	/** @brief Granted scopes (space-separated). */
	QString scope;

	/**
	* @brief Converts the response to a JSON object.
	* @return QJsonObject suitable for HTTP response body.
	*/
	QJsonObject ToJson() const;
};


/**
* @brief Stored authorization code data.
*
* Internal representation of an issued authorization code,
* including all metadata needed to validate it during the
* token exchange.
*/
struct AuthorizationCode
{
	/** @brief The authorization code value. */
	QString code;

	/** @brief Client that requested the code. */
	QString clientId;

	/** @brief Authenticated user's ID. */
	QString userId;

	/** @brief Redirect URI used in the request. */
	QString redirectUri;

	/** @brief Granted scopes. */
	QStringList scopes;

	/** @brief State parameter from the request. */
	QString state;

	/** @brief Nonce parameter from the request. */
	QString nonce;

	/** @brief PKCE code challenge for verification. */
	QString codeChallenge;

	/** @brief PKCE code challenge method. */
	QString codeChallengeMethod;

	/** @brief Unix timestamp when the code was created. */
	qint64 createdAt = 0;

	/** @brief Unix timestamp when the code expires. */
	qint64 expiresAt = 0;

	/** @brief Whether the code has already been used. */
	bool isUsed = false;
};


/**
* @brief OIDC provider server configuration.
*
* Configuration parameters for the OIDC provider, including
* issuer URL, token lifetimes, signing key paths, and
* supported authentication flows.
*
* @see CAuthorizableServer::StartOidcProvider()
*/
struct ServerConfig
{
	/**
	* @brief Issuer identifier URL.
	*
	* The base URL of the authorization server, used as the "iss"
	* claim in tokens and in the discovery document.
	*
	* @note Must be an HTTPS URL in production.
	*
	* Example: "https://auth.example.com"
	*/
	QString issuer;

	/**
	* @brief Path to the RSA private key for signing tokens.
	*
	* The key must be in PEM format. Used for RS256 signing.
	*
	* @warning Protect this file with appropriate permissions.
	*/
	QString signingKeyPath;

	/**
	* @brief Path to the corresponding RSA public key.
	*
	* Used for the JWKS endpoint and token verification.
	*/
	QString verificationKeyPath;

	/**
	* @brief Access token lifetime in seconds.
	* Default: 3600 (1 hour).
	*/
	int accessTokenExpirySeconds = 3600;

	/**
	* @brief ID token lifetime in seconds.
	* Default: 3600 (1 hour).
	*/
	int idTokenExpirySeconds = 3600;

	/**
	* @brief Refresh token lifetime in seconds.
	* Default: 86400 (24 hours).
	*/
	int refreshTokenExpirySeconds = 86400;

	/**
	* @brief Authorization code lifetime in seconds.
	* Default: 600 (10 minutes).
	*/
	int authorizationCodeExpirySeconds = 600;

	/**
	* @brief JWT signing algorithm.
	*
	* Supported values:
	* - "RS256" — RSA with SHA-256 (default, recommended)
	*/
	QString signingAlgorithm = "RS256";

	/**
	* @brief Whether PKCE is required for all clients.
	*
	* When true, all authorization requests must include PKCE
	* parameters. Recommended for improved security.
	*
	* Default: true
	*/
	bool requirePkce = true;

	/**
	* @brief Whether SSL is required for OIDC endpoints.
	*
	* Should always be true in production.
	*
	* Default: true
	*/
	bool requireSsl = true;
};


/**
* @brief Token introspection response.
*
* Response from the token introspection endpoint containing
* information about the validity and claims of a token.
*
* @see COidcIntrospectionEndpoint
*/
struct IntrospectionResponse
{
	/** @brief Whether the token is currently active/valid. */
	bool active = false;

	/** @brief Scopes associated with the token. */
	QString scope;

	/** @brief Client that the token was issued to. */
	QString clientId;

	/** @brief Username of the resource owner. */
	QString username;

	/** @brief Token type (e.g., "Bearer"). */
	QString tokenType;

	/** @brief Expiration time (Unix timestamp). */
	qint64 exp = 0;

	/** @brief Issued-at time (Unix timestamp). */
	qint64 iat = 0;

	/** @brief Subject identifier. */
	QString sub;

	/** @brief Issuer identifier. */
	QString iss;

	/**
	* @brief Converts the response to a JSON object.
	* @return QJsonObject suitable for HTTP response body.
	*/
	QJsonObject ToJson() const;
};


} // namespace Oidc
