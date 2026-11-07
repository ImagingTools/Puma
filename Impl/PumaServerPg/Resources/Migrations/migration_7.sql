-- Tokens of product servers have no owner and name the product using them
ALTER TABLE IF EXISTS "PersonalAccessTokens" ADD COLUMN IF NOT EXISTS "ClientProductId" text;
ALTER TABLE IF EXISTS "PersonalAccessTokens" ALTER COLUMN "UserId" DROP NOT NULL;
DO $$
BEGIN
	IF to_regclass('"PersonalAccessTokens"') IS NOT NULL THEN
		CREATE UNIQUE INDEX IF NOT EXISTS "IX_PersonalAccessTokens_TokenHash" ON "PersonalAccessTokens" ("TokenHash");
		CREATE INDEX IF NOT EXISTS "IX_PersonalAccessTokens_ClientProductId" ON "PersonalAccessTokens" ("ClientProductId") WHERE "ClientProductId" IS NOT NULL;
	END IF;
END $$;
