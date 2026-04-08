DROP TABLE IF EXISTS "UserSessions";

CREATE TABLE IF NOT EXISTS "UserSessions"(
    "Id" TEXT NOT NULL,
    "RefreshToken" TEXT NOT NULL,
    "UserId" TEXT NOT NULL,
    "CreationDate" TEXT NOT NULL,
    "ExpirationDate" TEXT NOT NULL,
    PRIMARY KEY ("Id")
);
