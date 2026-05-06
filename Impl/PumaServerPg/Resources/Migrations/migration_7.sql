-- OIDC tables migration
-- Creates tables for OIDC client registrations, authorization codes, and tokens

CREATE TABLE IF NOT EXISTS public."OidcClients" (
    "Id" UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    "ClientId" VARCHAR(255) NOT NULL UNIQUE,
    "ClientSecret" VARCHAR(512),
    "ClientName" VARCHAR(255) NOT NULL,
    "RedirectUris" JSONB NOT NULL DEFAULT '[]',
    "GrantTypes" JSONB NOT NULL DEFAULT '["authorization_code"]',
    "ResponseTypes" JSONB NOT NULL DEFAULT '["code"]',
    "Scopes" JSONB NOT NULL DEFAULT '["openid"]',
    "TokenEndpointAuthMethod" VARCHAR(64) NOT NULL DEFAULT 'client_secret_basic',
    "IsActive" BOOLEAN NOT NULL DEFAULT TRUE,
    "CreatedAt" TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now(),
    "UpdatedAt" TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now()
);

CREATE TABLE IF NOT EXISTS public."OidcAuthorizationCodes" (
    "Id" UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    "Code" VARCHAR(512) NOT NULL UNIQUE,
    "ClientId" VARCHAR(255) NOT NULL,
    "UserId" VARCHAR(255) NOT NULL,
    "RedirectUri" TEXT NOT NULL,
    "Scopes" JSONB NOT NULL DEFAULT '[]',
    "State" VARCHAR(512),
    "Nonce" VARCHAR(512),
    "CodeChallenge" VARCHAR(512),
    "CodeChallengeMethod" VARCHAR(16),
    "CreatedAt" BIGINT NOT NULL,
    "ExpiresAt" BIGINT NOT NULL,
    "IsUsed" BOOLEAN NOT NULL DEFAULT FALSE
);

CREATE TABLE IF NOT EXISTS public."OidcTokens" (
    "Id" UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    "TokenHash" VARCHAR(512) NOT NULL,
    "TokenType" VARCHAR(32) NOT NULL,
    "ClientId" VARCHAR(255) NOT NULL,
    "UserId" VARCHAR(255) NOT NULL,
    "Scopes" JSONB NOT NULL DEFAULT '[]',
    "IssuedAt" BIGINT NOT NULL,
    "ExpiresAt" BIGINT NOT NULL,
    "IsRevoked" BOOLEAN NOT NULL DEFAULT FALSE,
    "RefreshTokenHash" VARCHAR(512),
    "CreatedAt" TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now()
);

CREATE INDEX IF NOT EXISTS idx_oidc_auth_codes_code ON public."OidcAuthorizationCodes" ("Code");
CREATE INDEX IF NOT EXISTS idx_oidc_auth_codes_client ON public."OidcAuthorizationCodes" ("ClientId");
CREATE INDEX IF NOT EXISTS idx_oidc_tokens_hash ON public."OidcTokens" ("TokenHash");
CREATE INDEX IF NOT EXISTS idx_oidc_tokens_client ON public."OidcTokens" ("ClientId");
CREATE INDEX IF NOT EXISTS idx_oidc_tokens_user ON public."OidcTokens" ("UserId");
CREATE INDEX IF NOT EXISTS idx_oidc_tokens_refresh ON public."OidcTokens" ("RefreshTokenHash");
