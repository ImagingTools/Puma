CREATE TABLE IF NOT EXISTS "Users"
(
    "Id" INTEGER PRIMARY KEY AUTOINCREMENT,
    "DocumentId" TEXT NOT NULL,
    "Document" TEXT,
    "RevisionNumber" INTEGER,
    "LastModified" TEXT,
    "Checksum" INTEGER,
    "IsActive" INTEGER
);

CREATE TABLE IF NOT EXISTS "Roles"
(
    "Id" INTEGER PRIMARY KEY AUTOINCREMENT,
    "DocumentId" TEXT NOT NULL,
    "Document" TEXT,
    "RevisionNumber" INTEGER,
    "LastModified" TEXT,
    "Checksum" INTEGER,
    "IsActive" INTEGER
);

CREATE TABLE IF NOT EXISTS "UserGroups"
(
    "Id" INTEGER PRIMARY KEY AUTOINCREMENT,
    "DocumentId" TEXT NOT NULL,
    "Document" TEXT,
    "RevisionNumber" INTEGER,
    "LastModified" TEXT,
    "Checksum" INTEGER,
    "IsActive" INTEGER
);

CREATE TABLE IF NOT EXISTS "UserSessions"(
    "AccessToken" TEXT NOT NULL,
    "UserId" TEXT NOT NULL,
    "LastActivity" TEXT NOT NULL,
    PRIMARY KEY ("AccessToken")
);