-- Tokens of product servers have no owner and name the product using them
ALTER TABLE IF EXISTS "PersonalAccessTokens" ADD COLUMN IF NOT EXISTS "ClientProductId" text;
ALTER TABLE IF EXISTS "PersonalAccessTokens" ALTER COLUMN "UserId" DROP NOT NULL;
