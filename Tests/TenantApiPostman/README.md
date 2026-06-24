# Tenant API Postman Tests

## What This Folder Contains

- Tenant_System_Full.postman_collection.json: main end-to-end Postman collection.
- Tenant_System_Full.postman_environment.json: environment values used by Newman/Postman.
- permission_visibility_matrix.iteration.json: matrix artifact used by data-driven visibility checks.

Temporary debug reports (_tmp_*) are not required for normal test execution and can be removed.

## How The Tests Work

The suite executes a full tenant lifecycle and cross-tenant security model in one ordered flow:

1. Bootstraps system users and base tenants.
2. Creates and validates invitations, memberships, connections, relationships, grants, and RBAC entities.
3. Verifies positive and negative isolation boundaries across users and tenant contexts.
4. Runs advanced edge scenarios for lifecycle, delegation depth, ownership transfer, and cleanup.

The collection stores dynamic IDs/tokens in environment variables between steps. For skip-capable branches, sentinel defaults are used to avoid unresolved placeholders reaching backend queries.

## Prerequisites

- Running GraphQL backend at http://localhost:7788/Puma/graphql.
- Node.js and Newman installed.
- Valid product/system credentials in Tenant_System_Full.postman_environment.json (product_id and user credentials). 

## Run The Full Suite

From this directory:

```powershell
newman run Tenant_System_Full.postman_collection.json -e Tenant_System_Full.postman_environment.json --reporters cli,json --reporter-json-export run-report.json
```

Minimal run:

```powershell
newman run Tenant_System_Full.postman_collection.json -e Tenant_System_Full.postman_environment.json
```

## Scenario Coverage Summary

Total request-level scenarios: 321

- 00 System bootstrap: 1 requests
- 00A Auth SU: 1 requests
- 00B Register users: 4 requests
- 00C Auth all users: 4 requests
- 01 Tenant bootstrap: 6 requests
- 01A Tenant document-service create/update: 8 requests
- 02 Memberships and invitations: 11 requests
- 03 Connections, relationships, grants: 9 requests
- 04 RBAC (members, roles, groups, permissions): 16 requests
- 05 Isolation and negative checks: 4 requests
- 05A Isolation matrix - cross-tenant boundaries: 15 requests
- 06 Edge - Tenant lifecycle: 17 requests
- 07 Edge - Memberships and invitations: 16 requests
- 08 Edge - Connections and relationships: 6 requests
- 09 Edge - Grants and delegation: 38 requests
- 10 Edge - RBAC source/revoke matrix: 11 requests
- 11 Edge - Token and visibility probes: 2 requests
- 12 Data-driven permission visibility matrix: 4 requests
- 13 Coverage sweep - remaining tenant APIs: 38 requests
- 13 Edge - No Organization isolation: 17 requests
- 14 Advanced Edge - Lifecycle and delegation depth: 81 requests
- 14A Role permissions validation in-tenant: 7 requests
- 99 Cleanup: 5 requests

## Full Scenario Catalog (All Request-Level Scenarios)

### 00 System bootstrap (1)

1. Ensure superuser exists (su)

### 00A Auth SU (1)

1. Authorization SU

### 00B Register users (4)

1. Register user a_owner
2. Register user b_owner
3. Register user a_member
4. Register user outsider

### 00C Auth all users (4)

1. Authorization A Owner
2. Authorization B Owner
3. Authorization A Member
4. Authorization Outsider

### 01 Tenant bootstrap (6)

1. Create Tenant A
2. SelectTenant A Owner context
3. Create Tenant B
4. SelectTenant B Owner context
5. Get Tenant list contains A and B
6. Update Tenant A

### 01A Tenant document-service create/update (8)

1. Create Tenant document while switched context (negative)
2. Refresh auth A owner (No Organization) before tenant draft
3. Create Tenant document (draft)
4. Get Tenant representation (draft)
5. Update Tenant draft from representation (initial)
6. Update Tenant draft from representation (modify)
7. Save Tenant document (final)
8. GetTenant by id (verify document-service tenant)

### 02 Memberships and invitations (11)

1. Create invitation A -> member
2. Get my invitations (A member)
3. Accept invitation (A member)
4. Find membership after accept
5. Refresh auth A member after accept
6. SelectTenant A member context after accept
7. GetTenantList A member after accept
8. Create invitation A -> outsider
9. Get my invitations (outsider pending)
10. Reject invitation (outsider)
11. Get my invitations (outsider pending after reject)

### 03 Connections, relationships, grants (9)

1. Get connection code B
2. Enable connect by code for B
3. Create connection request A -> B
4. Approve connection request by B
5. Direct Create relationship document
6. Direct Update relationship representation
7. Direct Save relationship document
8. Create cross-org grant A -> B
9. Revoke cross-org grant

### 04 RBAC (members, roles, groups, permissions) (16)

1. Create Role VIEW document
2. Get Role VIEW representation
3. Update Role VIEW from representation
4. Save Role VIEW document
5. Create Role EDITOR document
6. Update Role EDITOR from representation
7. Save Role EDITOR document
8. Create Group SUPPORT document
9. Update Group SUPPORT from representation
10. Save Group SUPPORT document
11. Open User document for A member
12. Update User from representation (direct+role+group)
13. Save User document (A member)
14. Refresh auth A member
15. SelectTenant A member context (RBAC)
16. GetPermissions A member

### 05 Isolation and negative checks (4)

1. Outsider cannot create invitation in Tenant A
2. A member cannot create cross-org grant
3. A owner cannot fetch B connection code
4. B owner cannot fetch A relationships as foreign tenant

### 05A Isolation matrix - cross-tenant boundaries (15)

1. A owner GetTenant(B) denied
2. B owner GetTenant(A) denied
3. Outsider GetTenant(A) denied
4. A owner GetConnections(B) denied
5. B owner GetConnections(A) denied
6. A owner GetTenantRelationships(B) denied
7. B owner GetTenantRelationships(A) denied
8. A owner GetCrossOrgGrants(B) denied
9. B owner GetCrossOrgGrants(A) denied
10. Unknown UUID tenantId GetTenant (negative)
11. Unknown UUID tenantId GetConnections (negative)
12. Unknown UUID tenantId GetTenantRelationships (negative)
13. Unknown UUID tenantId GetCrossOrgGrants (negative)
14. Outsider UpdateTenant(A) denied
15. Invariant check after denied outsider update

### 06 Edge - Tenant lifecycle (17)

1. CreateTenant empty name (negative)
2. CreateTenant whitespace name (negative)
3. CreateTenant duplicate name exact (negative)
4. CreateTenant duplicate name case-insensitive (negative)
5. CreateTenant invalid ownerId (negative)
6. UpdateTenant unknown tenant (negative)
7. UpdateTenant duplicate target name (negative)
8. UpdateTenant empty name (negative)
9. SetTenantActive unknown tenant (negative)
10. RemoveTenant by outsider (negative)
11. CreateTenant for active toggle (positive)
12. SelectTenant active toggle probe context
13. SetTenantActive false for active toggle probe (positive)
14. GetTenant active toggle probe is inactive
15. SetTenantActive true for active toggle probe (positive)
16. GetTenant active toggle probe is active again
17. RemoveTenant active toggle probe cleanup (positive)

### 07 Edge - Memberships and invitations (16)

1. Create invitation by outsider (negative)
2. Accept invitation with unknown id (negative)
3. Accept invitation twice (negative)
4. Reject already accepted invitation (negative)
5. Revoke invitation with unknown id (negative)
6. Revoke terminal invitation state (negative)
7. RemoveMembership unknown id (negative)
8. RemoveMembership by outsider (negative)
9. HasMinimumRole matrix check
10. Create invitation A -> outsider for revoke (positive)
11. Revoke pending invitation by owner (positive)
12. Outsider does not see revoked invitation (positive)
13. Create invitation A -> outsider for remove membership (positive)
14. Outsider accepts invitation for remove-membership flow (positive)
15. RemoveMembership by owner (positive)
16. IsMember false after RemoveMembership (positive)

### 08 Edge - Connections and relationships (6)

1. CreateConnectionRequest with invalid code (negative)
2. CreateConnectionRequest by outsider (negative)
3. ApproveConnectionRequest unknown id (negative)
4. ApproveConnectionRequest twice (negative)
5. RemoveConnection unknown id (negative)
6. RemoveConnection by outsider (negative)

### 09 Edge - Grants and delegation (38)

1. CreateCrossOrgGrant without relationship (negative)
2. CreateCrossOrgGrant by outsider (negative)
3. RevokeCrossOrgGrant unknown id (negative)
4. RevokeCrossOrgGrant twice (negative)
5. RevokeCrossOrgGrant by outsider (negative)
6. RemoveCrossOrgGrants unknown only (negative)
7. RemoveCrossOrgGrants mixed IDs
8. Delegation baseline: Refresh auth B owner
9. Delegation baseline: SelectTenant B owner context
10. Delegation baseline: SelectTenant A denied before grant
11. Delegation: Resolve role R id (R_VIEW_ONLY)
12. Delegation: CreateCrossOrgGrant A -> B with role R
13. Delegation: B owner sees tenant A in organization list
14. Delegation: B owner can switch to tenant A after grant
15. Delegation: B owner GetPermissions includes role R permissions
16. Delegation: B owner switch back to tenant B (member coverage)
17. Delegation: Create invitation B -> outsider member
18. Delegation: Outsider sees pending B invitation
19. Delegation: Outsider accepts B invitation
20. Delegation: Refresh auth outsider for grant checks
21. Delegation: Outsider select tenant B context
22. Delegation: Outsider sees tenant A in org list via grant
23. Delegation: Outsider can select tenant A via grant
24. Delegation: Outsider GetPermissions includes role R permissions
25. Delegation: A owner GetCrossOrgGrants includes delegated grant
26. Delegation cleanup: Revoke delegated grant
27. Delegation post-revoke: Refresh auth B owner
28. Delegation post-revoke: SelectTenant B owner context
29. Delegation post-revoke: B owner org list does not include A
30. Delegation post-revoke: B owner cannot select tenant A
31. Delegation post-revoke: Refresh auth outsider
32. Delegation post-revoke: Outsider select tenant B context
33. Delegation post-revoke: Outsider org list does not include A
34. Delegation post-revoke: Outsider cannot select tenant A
35. Delegation post-revoke: Outsider GetPermissions drops role R
36. Delegation post-revoke: GetCrossOrgGrants excludes revoked grant
37. Delegation cleanup: Remove outsider membership from B (positive)
38. Delegation cleanup: Outsider is not member in B anymore

### 10 Edge - RBAC source/revoke matrix (11)

1. Open User doc for revoke matrix
2. Update User remove direct keep role/group
3. Save User after direct revoke
4. Refresh auth after direct revoke
5. SelectTenant A member context after direct revoke
6. GetPermissions after direct revoke
7. Update User remove roles and groups
8. Save User after full revoke
9. Refresh auth after full revoke
10. SelectTenant A member context after full revoke
11. GetPermissions after full revoke

### 11 Edge - Token and visibility probes (2)

1. GetPermissions by stale token probe
2. Functional visibility probe: RolesList with low-privilege user

### 12 Data-driven permission visibility matrix (4)

1. Matrix RolesList visibility probe
2. Matrix GroupsList visibility probe
3. Matrix GetPermissions probe
4. Matrix GetTenant isolation probe

### 13 Coverage sweep - remaining tenant APIs (38)

1. GetMembershipsByTenant coverage
2. GetMembershipsByUser coverage
3. GetMembership coverage
4. IsMember coverage
5. GetTenantInvitations coverage
6. AddMembership coverage
7. ResendTenantInvitation coverage
8. UpdateMembershipRole coverage
9. TransferTenantOwnership coverage
10. GetConnectionRequests coverage
11. RegenerateConnectionCode coverage
12. RejectConnectionRequest coverage
13. CancelConnectionRequest coverage
14. EnsureSystemTenant coverage
15. CreateContract coverage
16. GetContracts coverage
17. UpdateContractStatus coverage
18. TerminateContract coverage
19. SendCrossTenantMessage coverage
20. GetCrossTenantMessages coverage
21. GetCrossTenantMessage coverage
22. UpdateCrossTenantMessageStatus coverage
23. GetOrderRequests coverage
24. GetOrderRequest coverage
25. ConfirmOrderRequest coverage
26. RejectOrderRequest coverage
27. UpdateOrderRequestStatus coverage
28. GetRelationshipRepresentation coverage
29. GetUserRepresentation coverage
30. GetGroupRepresentation coverage
31. RoleItem coverage
32. GroupItem coverage
33. RoleAdd coverage
34. RoleUpdate coverage
35. GroupAdd coverage
36. GroupUpdate coverage
37. UserToken coverage
38. Logout coverage

### 13 Edge - No Organization isolation (17)

1. Refresh auth A owner (No Organization)
2. NoOrg test seeds
3. Create role in No Organization
4. Create group in No Organization
5. Create user in No Organization
6. NoOrg sees NoOrg role and not Tenant-A role
7. NoOrg sees NoOrg group
8. NoOrg sees NoOrg user and not Tenant-A user
9. SelectTenant A owner context for isolation checks
10. Tenant A cannot see NoOrg user
11. Tenant A cannot see NoOrg role
12. Tenant A cannot see NoOrg group
13. NoOrg cannot read Tenant B directly
14. Tenant A cannot read Tenant B directly
15. Tenant B cannot see NoOrg role
16. Tenant B cannot see NoOrg user
17. Tenant B cannot see NoOrg group

### 14 Advanced Edge - Lifecycle and delegation depth (81)

1. Advanced inactive: SetTenantActive false for tenant A
2. Advanced inactive: GetTenant A is inactive
3. Advanced inactive: A owner SelectTenant strict behavior
4. Advanced inactive: Refresh auth A member
5. Advanced inactive: A member SelectTenant strict behavior
6. Advanced inactive: A member GetPermissions strict response
7. Advanced inactive: SetTenantActive true for tenant A
8. Advanced inactive: GetTenant A is active again
9. Advanced invite: Create invitation A -> outsider for revoked reuse
10. Advanced invite: Resend pending invitation succeeds
11. Advanced invite: Revoke invitation A -> outsider for revoked reuse
12. Advanced invite: Accept revoked invitation is rejected
13. Advanced invite: GetTenantInvitations shows revoked status
14. Advanced membership: Create invitation B -> outsider for post-remove checks
15. Advanced membership: Outsider accepts invitation in B
16. Advanced membership: Remove outsider membership from B
17. Advanced membership: Refresh auth outsider after removal
18. Advanced membership: Outsider tenant list excludes B after removal
19. Advanced membership: Outsider cannot SelectTenant B after removal
20. Advanced membership: IsMember false for outsider in B after removal
21. Advanced membership-role: Create invitation B -> A member
22. Advanced membership-role: Refresh auth A member before B accept
23. Advanced membership-role: A member accepts invitation in B
24. Advanced membership-role: UpdateMembershipRole for A member in B
25. Advanced membership-role: GetMembership returns A member in B
26. Advanced connection: Create temporary tenant for connection lifecycle
27. Advanced connection: Enable connect by code on temporary tenant
28. Advanced connection: Get connection code for temporary tenant
29. Advanced connection: Create request A -> temporary tenant for reject flow
30. Advanced connection: Reject pending request on temporary tenant
31. Advanced connection: Rejected request cannot be approved
32. Advanced connection: Create request A -> temporary tenant for cancel flow
33. Advanced connection: Cancel pending request by source owner
34. Advanced connection: Canceled request cannot be approved
35. Advanced connection: Regenerate connection code for temporary tenant
36. Advanced connection: Old code is rejected after regenerate
37. Advanced connection: Create request with regenerated code
38. Advanced connection: Approve request created with regenerated code
39. Advanced connection: Remove temporary approved connection
40. Advanced connection: Remove temporary connection tenant
41. Advanced ownership: Create temporary tenant for transfer
42. Advanced ownership: Invite A member to temporary ownership tenant
43. Advanced ownership: Refresh auth A member before ownership invite accept
44. Advanced ownership: A member accepts ownership tenant invitation
45. Advanced ownership: Transfer tenant to A member
46. Advanced ownership: A member selects temporary ownership tenant context
47. Advanced ownership: Transfer tenant back to B owner
48. Advanced ownership: GetTenant confirms B owner restored
49. Advanced ownership: Remove A member membership from temporary ownership tenant
50. Advanced ownership: Remove temporary ownership tenant
51. Advanced grant depth: Resolve role ids from RolesList
52. Advanced grant depth: Create view-role grant A -> B
53. Advanced grant depth: Create editor-role grant A -> B
54. Advanced grant depth: Refresh auth B owner
55. Advanced grant depth: B owner select tenant B context
56. Advanced grant depth: B owner org list includes A with dual grants
57. Advanced grant depth: B owner can select tenant A with dual grants
58. Advanced grant depth: B owner permissions include union
59. Advanced grant depth: Refresh auth A member for dual-grant checks
60. Advanced grant depth: A member select tenant B context
61. Advanced grant depth: A member org list includes A with dual grants
62. Advanced grant depth: A member can select tenant A with dual grants
63. Advanced grant depth: A member permissions include union
64. Advanced grant depth: Revoke editor grant only
65. Advanced grant depth: Refresh auth B owner after editor revoke
66. Advanced grant depth: B owner select tenant B after editor revoke
67. Advanced grant depth: B owner still selects tenant A after editor revoke
68. Advanced grant depth: B owner permissions reduced after editor revoke
69. Advanced grant depth: Refresh auth A member after editor revoke
70. Advanced grant depth: A member select tenant B after editor revoke
71. Advanced grant depth: A member still selects tenant A after editor revoke
72. Advanced grant depth: A member permissions reduced after editor revoke
73. Advanced grant depth: Revoke view grant (final revoke)
74. Advanced grant depth: Refresh auth B owner after all revokes
75. Advanced grant depth: B owner select tenant B after all revokes
76. Advanced grant depth: B owner org list excludes A after all revokes
77. Advanced grant depth: B owner cannot select tenant A after all revokes
78. Advanced grant depth: Refresh auth A member after all revokes
79. Advanced grant depth: A member select tenant B after all revokes
80. Advanced grant depth: A member org list excludes A after all revokes
81. Advanced grant depth: A member cannot select tenant A after all revokes

### 14A Role permissions validation in-tenant (7)

1. RolePerm: Authorization A owner (No Organization)
2. RolePerm: GetProductPermissions all (NoOrg)
3. RolePerm: SelectTenant A owner context
4. RolePerm: GetTenantPermissions A
5. RolePerm: GetProductPermissions tenant-scoped subset
6. RolePerm: RoleUpdate with forbidden permission (negative)
7. RolePerm: RoleUpdate with allowed permission (positive)

### 99 Cleanup (5)

1. Remove relationship
2. Remove connection
3. Remove tenant A
4. Remove tenant B
5. Remove tenant created via document service
