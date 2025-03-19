INSERT INTO public."Roles" (
    "Id", "DocumentId", "TypeId", "Name", "Description", "Document", "TimeStamp", "RevisionInfo", "State", "DataMetaInfo"
)
SELECT 
    gen_random_uuid(), -- Генерируем новый UUID вместо SERIAL
    "DocumentId"::UUID, -- Преобразуем в UUID
    'Role',
    "Document"->>'RoleName',
    "Document"->>'RoleDescription',
    jsonb_set("Document", '{Uuid}', to_jsonb("DocumentId"::text)),
    COALESCE("LastModified", now()), -- Если NULL, ставим текущее время
    jsonb_build_object(
        'OwnerId', COALESCE("OwnerId", ''),
        'OwnerName', COALESCE("OwnerName", ''),
        'OperationDescription', COALESCE("OperationDescription", ''),
        'RevisionNumber', "RevisionNumber",
        'Checksum', "Checksum"
    ),
    CASE 
        WHEN "IsActive" = TRUE THEN 'Active'
        WHEN "IsActive" = FALSE THEN 'InActive'
        ELSE 'Disabled'
    END::"DocumentState",
    jsonb_build_object(
        'RoleId', COALESCE("Document"->>'RoleId', ''),
        'RoleName', COALESCE("Document"->>'RoleName', ''),
        'RoleDescription', COALESCE("Document"->>'RoleDescription', ''),
	'ProductId', COALESCE("Document"->>'ProductId', '')
    )
FROM public."Roles_new";
DROP TABLE public."Roles_new";


INSERT INTO public."Users" (
    "Id", "DocumentId", "TypeId", "Name", "Description", "Document", "TimeStamp", "RevisionInfo", "State", "DataMetaInfo"
)
SELECT 
    gen_random_uuid(), -- Генерируем новый UUID вместо SERIAL
    CASE 
        WHEN "DocumentId" = 'su' THEN '00000000-0000-0000-0000-000000000000'::UUID
        WHEN "DocumentId" ~ '^[0-9a-fA-F-]{36}$' THEN "DocumentId"::UUID
        ELSE gen_random_uuid()
    END,
	'User',
	"Document"->>'Name',
	"Document"->>'Description',
	CASE 
        WHEN "DocumentId" = 'su' THEN jsonb_set("Document", '{Uuid}', to_jsonb('00000000-0000-0000-0000-000000000000'::text))
        ELSE jsonb_set("Document", '{Uuid}', to_jsonb("DocumentId"::text))
    END,
    COALESCE("LastModified", now()),
    jsonb_build_object(
        'OwnerId', COALESCE("OwnerId", ''),
        'OwnerName', COALESCE("OwnerName", ''),
        'OperationDescription', COALESCE("OperationDescription", ''),
        'RevisionNumber', "RevisionNumber",
        'Checksum', "Checksum"
    ),
    CASE 
        WHEN "IsActive" = TRUE THEN 'Active'
        WHEN "IsActive" = FALSE THEN 'InActive'
        ELSE 'Disabled'
    END::"DocumentState",
    jsonb_build_object(
        'UserId', COALESCE("Document"->>'Id', ''),
        'UserName', COALESCE("Document"->>'Name', ''),
        'UserDescription', COALESCE("Document"->>'Description', ''),
	'Mail', COALESCE("Document"->>'Mail', ''),
	'PasswordHash', COALESCE("Document"->>'PasswordHash', ''),
	'SystemId', COALESCE("Document"->'SystemInfos'->0->>'SystemId', ''),
	'SystemName', COALESCE("Document"->'SystemInfos'->0->>'SystemName', '')
    )
FROM public."Users_new";
DROP TABLE public."Users_new";

INSERT INTO public."UserGroups" (
    "Id", "DocumentId", "TypeId", "Name", "Description", "Document", "TimeStamp", "RevisionInfo", "State", "DataMetaInfo"
)
SELECT 
    gen_random_uuid(),
    "DocumentId"::UUID,
	'Group',
	"Document"->>'Name',
	"Document"->>'Description',
    jsonb_set("Document", '{Uuid}', to_jsonb("DocumentId"::text)),
    COALESCE("LastModified", now()), -- Если NULL, ставим текущее время
    jsonb_build_object(
        'OwnerId', COALESCE("OwnerId", ''),
        'OwnerName', COALESCE("OwnerName", ''),
        'OperationDescription', COALESCE("OperationDescription", ''),
        'RevisionNumber', "RevisionNumber",
        'Checksum', "Checksum"
    ),
    CASE 
        WHEN "IsActive" = TRUE THEN 'Active'
        WHEN "IsActive" = FALSE THEN 'InActive'
        ELSE 'Disabled'
    END::"DocumentState",
        jsonb_build_object(
        'GroupId', COALESCE("Document"->>'Id', ''),
        'GroupName', COALESCE("Document"->>'Name', ''),
        'GroupDescription', COALESCE("Document"->>'Description', '')
    )
FROM public."UserGroups_new";
DROP TABLE public."UserGroups_new";
