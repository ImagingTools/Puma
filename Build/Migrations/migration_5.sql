INSERT INTO public."Roles" (
    "Id", "DocumentId", "TypeId", "Name", "Description", "Document", "TimeStamp", "RevisionInfo", "State", "DataMetaInfo"
)
SELECT 
    gen_random_uuid(), -- Генерируем новый UUID вместо SERIAL
    "DocumentId"::UUID, -- Преобразуем в UUID
    'Role',
    "Document"->>'RoleName',
    "Document"->>'RoleDescription',
    "Document",
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
        WHEN "DocumentId" = 'su' THEN '00000000-0000-0000-0000-000000000000'::UUID -- Заменяем 'su' на дефолтный UUID
        WHEN "DocumentId" ~ '^[0-9a-fA-F-]{36}$' THEN "DocumentId"::UUID -- Оставляем валидные UUID
        ELSE gen_random_uuid() -- Заменяем на случайный UUID для всех других случаев
    END,
	'User',
	"Document"->>'Name',
	"Document"->>'Description',
    "Document",
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
        'UserId', COALESCE("Document"->>'Id', ''),
        'UserName', COALESCE("Document"->>'Name', ''),
        'UserDescription', COALESCE("Document"->>'Description', ''),
	'Mail', COALESCE("Document"->>'Mail', ''),
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
    "Document",
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
