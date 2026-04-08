CREATE TABLE IF NOT EXISTS "Roles"
(
    "Id" TEXT PRIMARY KEY DEFAULT (lower(hex(randomblob(16)))),
    "DocumentId" TEXT NOT NULL,
    "TypeId" TEXT,
    "Document" TEXT NOT NULL,
    "Name" TEXT,
    "Description" TEXT,
    "RevisionInfo" TEXT,
    "DataMetaInfo" TEXT,
    "Derivates" TEXT,
    "TimeStamp" TEXT NOT NULL,
    "State" TEXT
);

CREATE TABLE IF NOT EXISTS "Users"
(
    "Id" TEXT PRIMARY KEY DEFAULT (lower(hex(randomblob(16)))),
    "DocumentId" TEXT NOT NULL,
    "TypeId" TEXT,
    "Document" TEXT NOT NULL,
    "Name" TEXT,
    "Description" TEXT,
    "RevisionInfo" TEXT,
    "DataMetaInfo" TEXT,
    "Derivates" TEXT,
    "TimeStamp" TEXT NOT NULL,
    "State" TEXT
);

CREATE TABLE IF NOT EXISTS "UserGroups"
(
    "Id" TEXT PRIMARY KEY DEFAULT (lower(hex(randomblob(16)))),
    "DocumentId" TEXT NOT NULL,
    "TypeId" TEXT,
    "Document" TEXT NOT NULL,
    "Name" TEXT,
    "Description" TEXT,
    "RevisionInfo" TEXT,
    "DataMetaInfo" TEXT,
    "Derivates" TEXT,
    "TimeStamp" TEXT NOT NULL,
    "State" TEXT
);
