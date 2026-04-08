INSERT INTO "Roles" (
    "Id", "DocumentId", "TypeId", "Name", "Description", "Document", "TimeStamp", "RevisionInfo", "State", "DataMetaInfo"
)
SELECT
    lower(hex(randomblob(16))),
    "DocumentId",
    'Role',
    json_extract("Document", '$.RoleName'),
    json_extract("Document", '$.RoleDescription'),
    json_set("Document", '$.Uuid', "DocumentId"),
    COALESCE("LastModified", datetime('now')),
    json_object(
        'OwnerId', COALESCE("OwnerId", ''),
        'OwnerName', COALESCE("OwnerName", ''),
        'OperationDescription', COALESCE("OperationDescription", ''),
        'RevisionNumber', "RevisionNumber",
        'Checksum', "Checksum"
    ),
    CASE
        WHEN "IsActive" = 1 THEN 'Active'
        WHEN "IsActive" = 0 THEN 'InActive'
        ELSE 'Disabled'
    END,
    json_object(
        'RoleId', COALESCE(json_extract("Document", '$.RoleId'), ''),
        'RoleName', COALESCE(json_extract("Document", '$.RoleName'), ''),
        'RoleDescription', COALESCE(json_extract("Document", '$.RoleDescription'), ''),
        'ProductId', COALESCE(json_extract("Document", '$.ProductId'), '')
    )
FROM "Roles_new";
DROP TABLE "Roles_new";


INSERT INTO "Users" (
    "Id", "DocumentId", "TypeId", "Name", "Description", "Document", "TimeStamp", "RevisionInfo", "State", "DataMetaInfo"
)
SELECT
    lower(hex(randomblob(16))),
    "DocumentId",
    'User',
    json_extract("Document", '$.Name'),
    json_extract("Document", '$.Description'),
    json_set("Document", '$.Uuid', "DocumentId"),
    COALESCE("LastModified", datetime('now')),
    json_object(
        'OwnerId', COALESCE("OwnerId", ''),
        'OwnerName', COALESCE("OwnerName", ''),
        'OperationDescription', COALESCE("OperationDescription", ''),
        'RevisionNumber', "RevisionNumber",
        'Checksum', "Checksum"
    ),
    CASE
        WHEN "IsActive" = 1 THEN 'Active'
        WHEN "IsActive" = 0 THEN 'InActive'
        ELSE 'Disabled'
    END,
    json_object(
        'UserId', COALESCE(json_extract("Document", '$.Id'), ''),
        'UserName', COALESCE(json_extract("Document", '$.Name'), ''),
        'UserDescription', COALESCE(json_extract("Document", '$.Description'), ''),
        'Mail', COALESCE(json_extract("Document", '$.Mail'), ''),
        'PasswordHash', COALESCE(json_extract("Document", '$.PasswordHash'), ''),
        'SystemId', COALESCE(json_extract("Document", '$.SystemInfos[0].SystemId'), ''),
        'SystemName', COALESCE(json_extract("Document", '$.SystemInfos[0].SystemName'), '')
    )
FROM "Users_new";
DROP TABLE "Users_new";


INSERT INTO "UserGroups" (
    "Id", "DocumentId", "TypeId", "Name", "Description", "Document", "TimeStamp", "RevisionInfo", "State", "DataMetaInfo"
)
SELECT
    lower(hex(randomblob(16))),
    "DocumentId",
    'Group',
    json_extract("Document", '$.Name'),
    json_extract("Document", '$.Description'),
    json_set("Document", '$.Uuid', "DocumentId"),
    COALESCE("LastModified", datetime('now')),
    json_object(
        'OwnerId', COALESCE("OwnerId", ''),
        'OwnerName', COALESCE("OwnerName", ''),
        'OperationDescription', COALESCE("OperationDescription", ''),
        'RevisionNumber', "RevisionNumber",
        'Checksum', "Checksum"
    ),
    CASE
        WHEN "IsActive" = 1 THEN 'Active'
        WHEN "IsActive" = 0 THEN 'InActive'
        ELSE 'Disabled'
    END,
    json_object(
        'GroupId', COALESCE(json_extract("Document", '$.Id'), ''),
        'GroupName', COALESCE(json_extract("Document", '$.Name'), ''),
        'GroupDescription', COALESCE(json_extract("Document", '$.Description'), '')
    )
FROM "UserGroups_new";
DROP TABLE "UserGroups_new";

