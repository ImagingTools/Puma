# OIDC Support in Puma

## Table of Contents / Inhaltsverzeichnis / Оглавление

1. [English](#english)
2. [Deutsch](#deutsch)
3. [Русский](#русский)

---

<a name="english"></a>
# English

## Overview

Puma provides OpenID Connect (OIDC) provider functionality through the AuthServerSdk, enabling applications to act as identity providers. The implementation supports the Authorization Code Flow with PKCE (Proof Key for Code Exchange), which is the recommended flow for both public and confidential clients.

## Architecture

The OIDC implementation spans two repositories:
- **ImtCore** — Core logic: JWT token provider, OAuth REST endpoints, scope-to-claims mapping, DB persistence
- **Puma** — SDK wrapper: public API types, PIMPL wrappers, ACF component wiring, DB migrations

### Components

| Component | Description |
|-----------|-------------|
| `CJwtTokenProvider` | RS256 JWT token signing and verification using OpenSSL EVP API |
| `OidcTypes` | Data structures: `ClientRegistration`, `TokenClaims`, `AuthorizationRequest`, `TokenRequest`, `TokenResponse`, `AuthorizationCode`, `ServerConfig`, `IntrospectionResponse` |
| `COidcServletComp` | REST servlet handling all OAuth2/OIDC endpoints |
| `COidcScopeMapperComp` | Maps OIDC scopes to user claims from the Puma RBAC model |

### REST Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/.well-known/openid-configuration` | GET | Discovery document |
| `/oauth/authorize` | GET | Authorization endpoint |
| `/oauth/token` | POST | Token endpoint |
| `/oauth/userinfo` | GET | UserInfo endpoint |
| `/oauth/jwks` | GET | JSON Web Key Set |
| `/oauth/revoke` | POST | Token revocation (RFC 7009) |
| `/oauth/introspect` | POST | Token introspection (RFC 7662) |

## Getting Started

### 1. Configure OIDC Provider

```cpp
#include <AuthServerSdk/AuthServerSdk.h>
#include <puma/oidc/OidcTypes.h>

AuthServerSdk::CAuthorizableServer server;

// Configure OIDC
Oidc::ServerConfig oidcConfig;
oidcConfig.issuer = "https://auth.example.com";
oidcConfig.signingKeyPath = "/etc/puma/oidc/private_key.pem";
oidcConfig.verificationKeyPath = "/etc/puma/oidc/public_key.pem";
oidcConfig.accessTokenExpirySeconds = 3600;       // 1 hour
oidcConfig.idTokenExpirySeconds = 3600;            // 1 hour
oidcConfig.refreshTokenExpirySeconds = 86400;      // 24 hours
oidcConfig.authorizationCodeExpirySeconds = 600;   // 10 minutes
oidcConfig.requirePkce = true;

if (!server.ConfigureOidc(oidcConfig)) {
    qCritical() << "Failed to configure OIDC";
    return -1;
}
```

### 2. Register OIDC Clients

```cpp
Oidc::ClientRegistration client;
client.clientId = "my-web-app";
client.clientSecret = "hashed-secret";
client.clientName = "My Web Application";
client.redirectUris = {"https://app.example.com/callback"};
client.grantTypes = {"authorization_code", "refresh_token"};
client.responseTypes = {"code"};
client.scopes = {"openid", "profile", "email", "roles"};
client.tokenEndpointAuthMethod = "client_secret_basic";

if (!server.RegisterOidcClient(client)) {
    qCritical() << "Failed to register OIDC client";
}
```

### 3. Generate RSA Key Pair

```bash
# Generate private key
openssl genrsa -out private_key.pem 2048

# Extract public key
openssl rsa -in private_key.pem -pubout -out public_key.pem
```

### 4. Using the JWT Token Provider Directly

```cpp
#include <puma/oidc/CJwtTokenProvider.h>
#include <puma/oidc/OidcTypes.h>

Oidc::CJwtTokenProvider provider;
provider.LoadSigningKey("/path/to/private_key.pem");
provider.LoadVerificationKey("/path/to/public_key.pem");

// Generate token
Oidc::TokenClaims claims;
claims.iss = "https://auth.example.com";
claims.sub = "user-123";
claims.aud = "my-client";
claims.iat = QDateTime::currentSecsSinceEpoch();
claims.exp = claims.iat + 3600;
claims.name = "John Doe";
claims.email = "john@example.com";

QString token = provider.GenerateToken(claims);

// Verify token
Oidc::TokenClaims decoded;
if (provider.VerifyToken(token, decoded)) {
    qDebug() << "Token valid, subject:" << decoded.sub;
}

// Get JWKS for /.well-known/jwks.json
QJsonObject jwks = provider.GetJwks();
```

## Supported Scopes

| Scope | Claims |
|-------|--------|
| `openid` | `sub` |
| `profile` | `name`, `preferred_username` |
| `email` | `email` |
| `roles` | `roles` (Puma role assignments) |
| `groups` | `groups` (Puma group memberships) |
| `permissions` | `permissions` (Puma permissions) |

## Database

OIDC data is persisted in three tables (migration step 7):

- **`OidcClients`** — Registered client applications
- **`OidcAuthorizationCodes`** — Issued authorization codes (short-lived)
- **`OidcTokens`** — Issued access/refresh tokens

## Security Considerations

- Always use HTTPS in production (`requireSsl = true`)
- Require PKCE for all clients (`requirePkce = true`)
- Use RSA 2048-bit or larger keys
- Protect private key files with appropriate file permissions
- Set short authorization code lifetimes (≤ 10 minutes)
- Rotate signing keys periodically

---

<a name="deutsch"></a>
# Deutsch

## Überblick

Puma bietet OpenID Connect (OIDC)-Provider-Funktionalität über das AuthServerSdk, die es Anwendungen ermöglicht, als Identity Provider zu agieren. Die Implementierung unterstützt den Authorization Code Flow mit PKCE (Proof Key for Code Exchange).

## Architektur

Die OIDC-Implementierung umfasst zwei Repositories:
- **ImtCore** — Kernlogik: JWT-Token-Provider, OAuth-REST-Endpunkte, Scope-zu-Claims-Mapping, DB-Persistenz
- **Puma** — SDK-Wrapper: öffentliche API-Typen, PIMPL-Wrapper, ACF-Komponentenverdrahtung, DB-Migrationen

### REST-Endpunkte

| Endpunkt | Methode | Beschreibung |
|----------|---------|-------------|
| `/.well-known/openid-configuration` | GET | Discovery-Dokument |
| `/oauth/authorize` | GET | Autorisierungsendpunkt |
| `/oauth/token` | POST | Token-Endpunkt |
| `/oauth/userinfo` | GET | UserInfo-Endpunkt |
| `/oauth/jwks` | GET | JSON Web Key Set |
| `/oauth/revoke` | POST | Token-Widerruf (RFC 7009) |
| `/oauth/introspect` | POST | Token-Introspektion (RFC 7662) |

## Erste Schritte

### 1. OIDC-Provider konfigurieren

```cpp
Oidc::ServerConfig oidcConfig;
oidcConfig.issuer = "https://auth.example.com";
oidcConfig.signingKeyPath = "/etc/puma/oidc/private_key.pem";
oidcConfig.verificationKeyPath = "/etc/puma/oidc/public_key.pem";

server.ConfigureOidc(oidcConfig);
```

### 2. RSA-Schlüsselpaar generieren

```bash
openssl genrsa -out private_key.pem 2048
openssl rsa -in private_key.pem -pubout -out public_key.pem
```

### 3. OIDC-Client registrieren

```cpp
Oidc::ClientRegistration client;
client.clientId = "meine-web-app";
client.clientName = "Meine Webanwendung";
client.redirectUris = {"https://app.example.com/callback"};
client.grantTypes = {"authorization_code", "refresh_token"};
client.scopes = {"openid", "profile", "email", "roles"};

server.RegisterOidcClient(client);
```

## Unterstützte Scopes

| Scope | Claims |
|-------|--------|
| `openid` | `sub` |
| `profile` | `name`, `preferred_username` |
| `email` | `email` |
| `roles` | `roles` (Puma-Rollenzuweisungen) |
| `groups` | `groups` (Puma-Gruppenmitgliedschaften) |
| `permissions` | `permissions` (Puma-Berechtigungen) |

## Sicherheitshinweise

- Verwenden Sie immer HTTPS in der Produktion
- Aktivieren Sie PKCE für alle Clients
- Verwenden Sie RSA-Schlüssel mit mindestens 2048 Bit
- Schützen Sie private Schlüsseldateien mit entsprechenden Dateiberechtigungen
- Setzen Sie kurze Autorisierungscode-Lebensdauern (≤ 10 Minuten)

---

<a name="русский"></a>
# Русский

## Обзор

Puma предоставляет функциональность провайдера OpenID Connect (OIDC) через AuthServerSdk, позволяя приложениям выступать в качестве поставщиков идентификации. Реализация поддерживает Authorization Code Flow с PKCE (Proof Key for Code Exchange).

## Архитектура

Реализация OIDC охватывает два репозитория:
- **ImtCore** — основная логика: провайдер JWT-токенов, REST-эндпоинты OAuth, маппинг scope-в-claims, DB-персистенция
- **Puma** — SDK-обёртка: публичные типы API, PIMPL-обёртки, связывание ACF-компонентов, миграции БД

### REST-эндпоинты

| Эндпоинт | Метод | Описание |
|----------|-------|---------|
| `/.well-known/openid-configuration` | GET | Документ обнаружения |
| `/oauth/authorize` | GET | Эндпоинт авторизации |
| `/oauth/token` | POST | Эндпоинт токенов |
| `/oauth/userinfo` | GET | Эндпоинт информации о пользователе |
| `/oauth/jwks` | GET | Набор ключей JSON Web Key Set |
| `/oauth/revoke` | POST | Отзыв токена (RFC 7009) |
| `/oauth/introspect` | POST | Интроспекция токена (RFC 7662) |

## Начало работы

### 1. Настройка провайдера OIDC

```cpp
Oidc::ServerConfig oidcConfig;
oidcConfig.issuer = "https://auth.example.com";
oidcConfig.signingKeyPath = "/etc/puma/oidc/private_key.pem";
oidcConfig.verificationKeyPath = "/etc/puma/oidc/public_key.pem";

server.ConfigureOidc(oidcConfig);
```

### 2. Генерация пары ключей RSA

```bash
openssl genrsa -out private_key.pem 2048
openssl rsa -in private_key.pem -pubout -out public_key.pem
```

### 3. Регистрация клиента OIDC

```cpp
Oidc::ClientRegistration client;
client.clientId = "мое-веб-приложение";
client.clientName = "Моё веб-приложение";
client.redirectUris = {"https://app.example.com/callback"};
client.grantTypes = {"authorization_code", "refresh_token"};
client.scopes = {"openid", "profile", "email", "roles"};

server.RegisterOidcClient(client);
```

## Поддерживаемые scope

| Scope | Claims |
|-------|--------|
| `openid` | `sub` |
| `profile` | `name`, `preferred_username` |
| `email` | `email` |
| `roles` | `roles` (назначения ролей Puma) |
| `groups` | `groups` (членство в группах Puma) |
| `permissions` | `permissions` (разрешения Puma) |

## Рекомендации по безопасности

- Всегда используйте HTTPS в продакшене
- Включите PKCE для всех клиентов
- Используйте ключи RSA не менее 2048 бит
- Защитите файлы закрытых ключей соответствующими правами доступа
- Устанавливайте короткие сроки действия кодов авторизации (≤ 10 минут)
- Периодически ротируйте ключи подписи
