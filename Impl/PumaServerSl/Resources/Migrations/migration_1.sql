ALTER TABLE "Users" ADD COLUMN "OwnerId" TEXT;
ALTER TABLE "Users" ADD COLUMN "OwnerName" TEXT;
ALTER TABLE "Users" ADD COLUMN "OperationDescription" TEXT;

ALTER TABLE "Roles" ADD COLUMN "OwnerId" TEXT;
ALTER TABLE "Roles" ADD COLUMN "OwnerName" TEXT;
ALTER TABLE "Roles" ADD COLUMN "OperationDescription" TEXT;

ALTER TABLE "UserGroups" ADD COLUMN "OwnerId" TEXT;
ALTER TABLE "UserGroups" ADD COLUMN "OwnerName" TEXT;
ALTER TABLE "UserGroups" ADD COLUMN "OperationDescription" TEXT;