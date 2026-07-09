CREATE TABLE IF NOT EXISTS "PersonalAccessTokens"
(
    "Id" UUID NOT NULL DEFAULT gen_random_uuid(),
    "Token" VARCHAR(1000) NOT NULL,
    "UserId" UUID NOT NULL,
    "ProductId" VARCHAR(500),
    "Name" VARCHAR(500) NOT NULL,
    "Description" VARCHAR(2000),
    "Permissions" jsonb NOT NULL DEFAULT '[]'::jsonb,
    "CreationDate" TIMESTAMP WITHOUT TIME ZONE NOT NULL DEFAULT now(),
    "ExpirationDate" TIMESTAMP WITHOUT TIME ZONE,
    "LastUsedDate" TIMESTAMP WITHOUT TIME ZONE,
    "State" "DocumentState" NOT NULL DEFAULT 'Active',
    PRIMARY KEY ("Id")
);

CREATE INDEX IF NOT EXISTS "idx_pat_token" ON "PersonalAccessTokens" ("Token");
CREATE INDEX IF NOT EXISTS "idx_pat_userid" ON "PersonalAccessTokens" ("UserId");
CREATE INDEX IF NOT EXISTS "idx_pat_productid" ON "PersonalAccessTokens" ("ProductId");
