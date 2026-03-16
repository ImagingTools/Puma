CREATE TABLE IF NOT EXISTS "Users"
(
    "Id" SERIAL,
    "DocumentId" character varying(1000) COLLATE pg_catalog."default" NOT NULL,
    "Document" jsonb,
    "RevisionNumber" bigint,
    "LastModified" timestamp without time zone,
    "Checksum" bigint,
    "IsActive" boolean,
     PRIMARY KEY ("Id")
);

CREATE TABLE IF NOT EXISTS "Roles"
(
    "Id" SERIAL,
    "DocumentId" character varying(1000) COLLATE pg_catalog."default" NOT NULL,
    "Document" jsonb,
    "RevisionNumber" bigint,
    "LastModified" timestamp without time zone,
    "Checksum" bigint,
    "IsActive" boolean,
     PRIMARY KEY ("Id")
);

CREATE TABLE IF NOT EXISTS "UserGroups"
(
    "Id" SERIAL,
    "DocumentId" character varying(1000) COLLATE pg_catalog."default" NOT NULL,
    "Document" jsonb,
    "RevisionNumber" bigint,
    "LastModified" timestamp without time zone,
    "Checksum" bigint,
    "IsActive" boolean,
     PRIMARY KEY ("Id")
);

CREATE TABLE IF NOT EXISTS  "UserSessions"(
    "AccessToken" UUID NOT NULL,
    "UserId" VARCHAR (1000) NOT NULL,
    "LastActivity" TIMESTAMP NOT NULL,
    PRIMARY KEY ("AccessToken")
);