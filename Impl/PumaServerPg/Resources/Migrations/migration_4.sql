DO $$
BEGIN
   IF NOT EXISTS (SELECT 1 FROM pg_type WHERE typname = 'DocumentState') THEN
      CREATE TYPE "DocumentState" AS ENUM ('Active', 'InActive', 'Disabled');
   END IF;
END $$;

CREATE TABLE IF NOT EXISTS "Roles"
(
    "Id" UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    "DocumentId" UUID NOT NULL,
    "TypeId" Text,
    "Document" jsonb NOT NULL,
    "Name" Text,
    "Description" Text,
    "RevisionInfo" jsonb,
    "DataMetaInfo" jsonb,
    "Derivates" jsonb,
    "TimeStamp" timestamp without time zone NOT NULL,
    "State" "DocumentState"
);

CREATE TABLE IF NOT EXISTS "Users"
(
    "Id" UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    "DocumentId" UUID NOT NULL,
    "TypeId" Text,
    "Document" jsonb NOT NULL,
    "Name" Text,
    "Description" Text,
    "RevisionInfo" jsonb,
    "DataMetaInfo" jsonb,
    "Derivates" jsonb,
    "TimeStamp" timestamp without time zone NOT NULL,
    "State" "DocumentState"
);

CREATE TABLE IF NOT EXISTS "UserGroups"
(
    "Id" UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    "DocumentId" UUID NOT NULL,
    "TypeId" Text,
    "Document" jsonb NOT NULL,
    "Name" Text,
    "Description" Text,
    "RevisionInfo" jsonb,
    "DataMetaInfo" jsonb,
    "Derivates" jsonb,
    "TimeStamp" timestamp without time zone NOT NULL,
    "State" "DocumentState"
);
