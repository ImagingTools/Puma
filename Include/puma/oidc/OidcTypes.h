// SPDX-License-Identifier: LicenseRef-Puma-Commercial
#pragma once

/**
* @file OidcTypes.h
* @brief OpenID Connect SDK configuration types.
*
* This header defines the SDK-level configuration structures for
* OIDC client registration and server configuration. These types
* are used by the AuthServerSdk to configure the underlying ImtCore
* OIDC provider.
*
* @note Internal OIDC types (token claims, authorization codes,
*       token responses, etc.) are managed by ImtCore's OIDC
*       component framework (imtauth/imtoidc).
*
* @ingroup OidcProvider
*/

// Qt includes
#include <QtCore/QString>
#include <QtCore/QStringList>


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
* @see CAuthorizableServer::RegisterOidcClient()
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
* @brief OIDC provider server configuration.
*
* Configuration parameters for the OIDC provider, including
* issuer URL, token lifetimes, signing key paths, and
* supported authentication flows.
*
* @see CAuthorizableServer::ConfigureOidc()
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


} // namespace Oidc
