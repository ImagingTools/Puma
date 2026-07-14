CREATE TABLE IF NOT EXISTS "PersonalAccessTokens"
(
    "Id"          TEXT PRIMARY KEY,
    "Name"        TEXT NOT NULL,
    "Description" TEXT NOT NULL DEFAULT '',
    "UserId"      TEXT NOT NULL,
    "ProductId"   TEXT,
    "TokenHash"   BLOB NOT NULL,
    "Scopes"      TEXT NOT NULL DEFAULT '',
    "CreatedAt"   TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP,
    "LastUsedAt"  TEXT,
    "ExpiresAt"   TEXT,
    "Revoked"     INTEGER NOT NULL DEFAULT 0
);
