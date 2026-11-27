# Tenant API Postman Tests

## What This Folder Contains

- Tenant_System_Full.postman_collection.json: main end-to-end Postman collection.
- Tenant_System_Full.postman_environment.json: environment values used by Newman/Postman.

Temporary debug reports (_tmp_*) are not required for normal test execution and can be removed.

## How The Tests Work

The suite executes a full tenant lifecycle and cross-tenant security model in one ordered flow:

1. Bootstraps system users and base tenants.
2. Creates and validates invitations, memberships, connections, contracts and their notifications, and RBAC entities.
3. Verifies positive and negative isolation boundaries across users and tenant contexts.
4. Runs advanced edge scenarios for lifecycle depth, ownership transfer, and cleanup.

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

### Access changes (`ws/ws-access-change.js`)

Run with the usual lifetimes, before the short sessions. A user holding a role gets a right added to the role, loses it, then loses the role: each change applies to the very next request of the same token (no waiting, no polling), and the subscription `OnAccessChanged` of the session is told of it. A role the user does not hold tells nothing. With `--product-http`, `--product-ws`, `--product`, `--product-right`, `--product-check` and `--product-check-command` the same runs at the server of a product (ProLife runs it so).

### Short sessions (`ws/ws-token-expiry.js`, `ws/ws-session-security.js`)

The last step of `Run-CiTests.ps1` starts the server again with short sessions: `ServerSettings.ps1` puts the group `Sessions` (`AccessTokenLifetime` 20 s, `SessionLifetime` 75 s; `-JwtLifetime`, `-SessionLifetime`) into the settings file of the test server and restores the file afterwards. A server reads these settings at its start; by default a token lives 3600 s and a session 86400 s.

- `ws-token-expiry.js`: an expired token is refused with 401 and a new subscription with code `Unauthorized`, while an established subscription lives on until its session ends; refresh keeps the session and rotates the refresh token once, also under concurrency; stop and start move the delivery to the new token; a sign-out ends the subscriptions; a refresh moves the end of the session, and at that end the subscription ends as `Unauthorized`. It takes about three minutes of real time.
- `ws-session-security.js`: `Logout` naming the token of another user, an anonymous `Logout` or `RemoveSession`, `RemoveSession` and `GetSession` of the session of another user, and a forged token end or read nothing; each refusal has a positive control.

`Run-CiTests.ps1` (Postgres) and `Run-CiTests-Sqlite.ps1` also fail the run if the server log (`server-stdout.log`/`server-stderr.log`, `server-sqlite-*.log` for SQLite) has a database error - see `ServerLogCheck.ps1`. A server can answer every request correctly while it loses data behind them; the log is the only place that shows it.

## Scenario Coverage Summary

Total request-level scenarios: 1389

- 00 System bootstrap: 2 requests
- 00A Auth SU: 1 requests
- 00B Register users: 4 requests
- 00C Auth all users: 4 requests
- 00E Server tokens of the products: 5 requests
- 00D Organization creators: 4 requests
- 01 Tenant bootstrap: 13 requests
- 01A Tenant document-service create/update: 8 requests
- 02 Memberships and invitations: 13 requests
- 03 Connections: 8 requests
- 04 RBAC (members, roles, groups, permissions): 25 requests
- 05 Isolation and negative checks: 4 requests
- 05A Isolation matrix - cross-tenant boundaries: 10 requests
- 06 Edge - Tenant lifecycle: 22 requests
- 07 Edge - Memberships and invitations: 24 requests
- 08 Edge - Connections: 7 requests
- 09 Edge - A foreign organization cannot be selected: 3 requests
- 10 Edge - RBAC source/revoke matrix: 13 requests
- 11 Edge - Token and visibility probes: 1 requests
- 12 Permission visibility matrix (per actor): 12 requests
- 13 Coverage sweep - remaining tenant APIs: 43 requests
- 13 Edge - No Organization isolation: 40 requests
- 14 Advanced Edge - Lifecycle depth: 60 requests
- 14A Role permissions validation in-tenant: 15 requests
- 14B Scope isolation - Roles/Users/Groups: 18 requests
- 05B User representation tenant isolation (roles in GetUserRepresentation): 19 requests
- 15 QA Regression - Group roles, group search, registration: 52 requests
- 16 Access Control - Sessions, Self-Registration, Unlock: 15 requests
- 17 Permission Inheritance in Organization: 53 requests
- 18 Permission Inheritance without Organization: 29 requests
- 19 QA Regression - Organizations GUI: 46 requests
- 20 Organization permissions by path: 35 requests  [organization permissions are checked by imtauth::HasPermission: a leaf granted as the full path the GUI stores opens what it names, a sibling leaf or a group of the tree - by path or bare id - does not; the reads of the organization (members, connections, connection code, permissions of another member) need their own leaf]
- 21 QA Regression - Organizations GUI audit: 67 requests  [server defects found by the browser audit of the ProLife Organizations page (2026-10-22): an open organization document follows TransferTenantOwnership and saving it does not revert the ownership; GetTenantList sorts by tenantRelationScope; the Status filter as the GUI sends it (Bool, typed by __typename); user history: every revision has an author and a description, a rename is not recorded as a change of the login, joining a group is recorded as such; role and group documents saved as the editor does keep the element id as the representation id, so their history is found]
- 22 Tenant-scoped uniqueness - Role-ID and group name: 54 requests
- 23 Organization permission scope: 90 requests
- 24 Organizations audit - lookups, removal guard, cascades: 100 requests
- 26 Organization management of users, roles and groups: 23 requests
- 25 User data isolation - profiles, sessions, password recovery: 67 requests
- 27 Product boundary: 52 requests
- 28 Server tokens: 114 requests
- 29 Removed roles and groups leave no references: 58 requests
- 31 Organization management levels: 27 requests
- 32 Contract notifications: 133 requests  [generated by `tools/contract-notifications-folder.js`: three organizations negotiate, conclude and leave contracts, give and take back permissions, remove a party, change the terms by a file and by a party added by its code, decline and withdraw an offer; after every step the inbox of every user is compared exactly, so a notification sent twice, to the wrong user (the responsible user replaces the owner while a member, the acting user is never told) or not withdrawn when its step ends fails the step]
- 99 Cleanup: 4 requests

### Assertion policy

Every test asserts the specific outcome the API is contracted to produce. A test
must not accept "success or a denial", "success or already exists", or "any
decided outcome" - an assertion that cannot fail cannot detect a regression, and
the endpoints that had such assertions were exactly the ones whose backing tables
were silently missing. Negative tests pin the refusal *reason* (for example
`/Access denied/`), so an unrelated failure cannot satisfy them.

Documents are opened immediately before they are read and closed afterwards: the
server closes any document left without an active `OnDocumentChanged` subscriber
after `CloseDocumentTimeout` (30s by default), and this suite is pure HTTP, so a
document handle carried across folders expires on any run slower than ~30s.

## Full Scenario Catalog (All Request-Level Scenarios)

### 00 System bootstrap (2)

1. Ensure superuser exists (su)
2. GetApplicationInfo reports the version and build time of Puma

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

### 00E Server tokens of the products (5)

1. SRV su ensures ProLife is connected
2. SRV su ensures Lisa is connected
3. SRV every permission a server may hold
4. SRV the token of ProLife
5. SRV the token of Lisa

### 00D Organization creators (4)

1. OC SU creates the role that allows creating organizations
2. OC SU gives the role to the owners of A and B through a group
3. OC The owner of A may create organizations and sees them
4. OC The outsider may not create organizations

### 01 Tenant bootstrap (13)

1. Create Tenant A
2. SelectTenant A Owner context
3. Create Tenant B
4. SelectTenant B Owner context
5. Get Tenant list contains A and B
6. Tenant list created this year contains A
7. Tenant list created last year leaves A out
8. Tenant list created from today to today contains A
9. Tenant list created in 2020 leaves A out
10. SelectTenant A Owner context (post-lookup)
11. Update Tenant A
12. Ensure Tenant A is active (bootstrap)
13. Tenant A reads back the update and the active state

### 01A Tenant document-service create/update (8)

1. Create sub-organization document without the Add permission in the scope (negative)
2. Refresh auth A owner (No Organization) before tenant draft
3. Create Tenant document (draft)
4. Get Tenant representation (draft)
5. Update Tenant draft from representation (initial)
6. Update Tenant draft from representation (modify)
7. Save Tenant document (final)
8. GetTenant by id (verify document-service tenant)

### 02 Memberships and invitations (13)

1. Create invitation A -> member
2. Get my invitations (A member)
3. Invitation notification reached the member (A member)
4. Accept invitation (A member)
5. Find membership after accept
6. Refresh auth A member after accept
7. SelectTenant A member context after accept
8. GetTenantList A member after accept
9. Create invitation A -> outsider
10. Get my invitations (outsider pending)
11. Reject invitation (outsider)
12. Get my invitations (outsider pending after reject)
13. Tenant A members after reject: member in, outsider out

### 03 Connections (8)

1. Get connection code B
2. Disable connect by code for B
3. Connection request is refused while B does not accept codes
4. Enable connect by code for B
5. Connection code B accepts connections again
6. Create connection request A -> B
7. Approve connection request by B
8. Approved connection is listed for A

### 04 RBAC (members, roles, groups, permissions) (25)

1. Resolve real tenant-A permissions for role fixtures
2. Create Role VIEW document
3. Get Role VIEW representation
4. Update Role VIEW from representation
5. Save Role VIEW document
6. Role VIEW reads back with exactly its permission
7. Create Role EDITOR document
8. Update Role EDITOR from representation
9. Save Role EDITOR document
10. Role EDITOR reads back with exactly its permission
11. Create Group SUPPORT document
12. Update Group SUPPORT from representation
13. Save Group SUPPORT document
14. Group SUPPORT reads back with its role and member
15. Open User document for A member
16. Wait for A member document load
17. Update User from representation (direct+role+group)
18. Save User document (A member)
19. Refresh auth A member
20. SelectTenant A member context (RBAC)
21. GetPermissions A member
22. Forged JWT: GetPermissions grants nothing for a token with an invalid signature
23. Forged JWT: the real member token reads its own profile (control)
24. Forged JWT: a garbage token is refused
25. Forged JWT: a request authenticated with an invalid signature is refused like garbage

### 05 Isolation and negative checks (4)

1. Outsider cannot create invitation in Tenant A
2. Tenant A invitations: outsider has only the rejected one
3. A owner cannot fetch B connection code
4. Outsider cannot probe membership in Tenant A (IsMember)

### 05A Isolation matrix - cross-tenant boundaries (10)

1. A owner GetTenant(B) denied
2. B owner GetTenant(A) denied
3. Outsider GetTenant(A) denied
4. A owner without organization context cannot read tenant B
5. A owner GetConnections(B) denied
6. B owner GetConnections(A) denied
7. Unknown UUID tenantId GetTenant (negative)
8. Unknown UUID tenantId GetConnections (negative)
9. Outsider UpdateTenant(A) denied
10. Invariant check after denied outsider update

### 06 Edge - Tenant lifecycle (22)

1. 06 Authorization a_owner (No Organization) for independent organizations
2. CreateTenant empty name (negative)
3. CreateTenant whitespace name (negative)
4. CreateTenant duplicate name exact (negative)
5. CreateTenant duplicate name case-insensitive (negative)
6. CreateTenant invalid ownerId (negative)
7. Refused CreateTenant calls left no tenant behind
8. UpdateTenant unknown tenant (negative)
9. UpdateTenant duplicate target name (negative)
10. UpdateTenant empty name (negative)
11. SetTenantActive unknown tenant (negative)
12. RemoveTenant by outsider (negative)
13. Tenant A survives the refused updates and removal
14. CreateTenant for active toggle (positive)
15. Auth A owner for active toggle probe
16. SelectTenant active toggle probe context
17. SetTenantActive false for active toggle probe (positive)
18. GetTenant active toggle probe is inactive
19. SetTenantActive true for active toggle probe (positive)
20. GetTenant active toggle probe is active again
21. RemoveTenant active toggle probe cleanup (positive)
22. Removed probe tenant is gone

### 07 Edge - Memberships and invitations (24)

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
13. Owner sees the revoked invitation as Revoked (positive control)
14. Create invitation A -> outsider for remove membership (positive)
15. Outsider accepts invitation for remove-membership flow (positive)
16. RemoveMembership by owner (positive)
17. IsMember false after RemoveMembership (positive)
18. Register phantom user for delete cascade test
19. AddMembership for phantom user (to test cascade on delete)
20. Phantom membership is listed in tenant A before user removal
21. Remove phantom user - should cascade remove from memberships (no phantoms)
22. Verify phantom user gone from memberships (GetMembershipsByUser and ByTenant)
23. Verify phantom user gone: GetMembershipsByUser is empty
24. Verify phantom user gone: tenant A no longer lists the phantom membership

### 08 Edge - Connections (7)

1. CreateConnectionRequest with invalid code (negative)
2. CreateConnectionRequest by outsider (negative)
3. ApproveConnectionRequest unknown id (negative)
4. ApproveConnectionRequest twice (negative)
5. RemoveConnection unknown id (negative)
6. RemoveConnection by outsider (negative)
7. Connection survives the refused outsider removal

### 09 Edge - A foreign organization cannot be selected (3)

1. Refresh auth B owner
2. SelectTenant B owner context
3. SelectTenant A denied to the B owner

### 10 Edge - RBAC source/revoke matrix (13)

1. Open User doc for revoke matrix
2. Wait for A member document load (revoke matrix)
3. GetPermissions before revoke (revoke matrix baseline)
4. Update User remove direct keep role/group
5. Save User after direct revoke
6. Refresh auth after direct revoke
7. SelectTenant A member context after direct revoke
8. GetPermissions after direct revoke
9. Update User remove roles and groups
10. Save User after full revoke
11. Refresh auth after full revoke
12. SelectTenant A member context after full revoke
13. GetPermissions after full revoke

### 11 Edge - Token and visibility probes (1)

1. GetPermissions by stale token probe

### 12 Permission visibility matrix (per actor) (12)

1. Matrix RolesList :: a_owner (allowed)
2. Matrix RolesList :: b_owner (allowed)
3. Matrix RolesList :: a_member (refused)
4. Matrix RolesList :: outsider (refused)
5. Matrix GroupsList :: a_owner (allowed)
6. Matrix GroupsList :: b_owner (allowed)
7. Matrix GroupsList :: a_member (refused)
8. Matrix GroupsList :: outsider (refused)
9. Matrix GetPermissions :: a_owner
10. Matrix GetPermissions :: b_owner
11. Matrix GetPermissions :: a_member
12. Matrix GetPermissions :: outsider

### 13 Coverage sweep - remaining tenant APIs (43)

1. GetMembershipsByTenant coverage
2. GetMembershipsByUser coverage
3. GetMembership coverage
4. GetOrganizationPermissions (organization tree)
5. UpdateMembershipPermissions + verify (new org perms)
6. Verify organizationPermissions in GetMembership (new)
7. IsMember coverage
8. GetTenantInvitations coverage
9. AddMembership coverage
10. AddMembership coverage: no duplicate membership stored
11. ResendTenantInvitation coverage: create a pending invitation
12. ResendTenantInvitation coverage
13. ResendTenantInvitation coverage: lastSentAt moved forward
14. ResendTenantInvitation unknown id (negative)
15. ResendTenantInvitation coverage: revoke the probe invitation
16. UpdateMembershipPermissions coverage
17. UpdateMembershipPermissions coverage: permissions are cleared
18. TransferTenantOwnership coverage
19. TransferTenantOwnership coverage: owner unchanged
20. GetConnectionRequests coverage
21. RegenerateConnectionCode coverage
22. RegenerateConnectionCode coverage: previous code no longer resolves
23. RejectConnectionRequest coverage
24. CancelConnectionRequest coverage
25. EnsureSystemTenant coverage
26. Reopen user document (coverage)
27. GetUserRepresentation coverage
28. Close user document (coverage)
29. Reopen group document (coverage)
30. GetGroupRepresentation coverage
31. Close group document (coverage)
32. RoleItem coverage
33. GroupItem coverage
34. RoleAdd coverage
35. RoleUpdate coverage
36. RoleUpdate coverage: verify the update landed
37. GroupAdd coverage
38. GroupUpdate coverage
39. GroupUpdate coverage: verify the update landed
40. UserToken coverage
41. Logout coverage
42. Logout coverage: logout own session via header
43. Logout coverage: logged-out token is no longer authenticated

### 13 Edge - No Organization isolation (40)

1. NoOrg viewer: Register dedicated user
2. NoOrg viewer: Resolve user id if already existed
3. NoOrg viewer: Create global viewer role
4. NoOrg viewer: Update global viewer role permissions
5. NoOrg viewer: Save global viewer role document
6. NoOrg viewer: Assign global viewer role to dedicated user
7. NoOrg viewer: Authenticate in No Organization
8. Refresh auth A owner (No Organization)
9. NoOrg seeds: A owner without viewer permissions is refused RolesList
10. Create role in No Organization
11. Update role in No Organization
12. Save role in No Organization
13. Create group in No Organization
14. Update group in No Organization
15. Save group in No Organization
16. Create user in No Organization
17. NoOrg viewer sees neither the personal role of su nor the Tenant-A role
18. NoOrg su sees its own role and not Tenant-A role
19. NoOrg profile probe: create user holding the NoOrg role and the Tenant-A role
20. NoOrg profile probe: authenticate in No Organization
21. NoOrg GetProfile shows only NoOrg roles/groups and empty currentTenantId
22. NoOrg viewer sees neither the personal group of su nor the Tenant-A group
23. NoOrg su sees its own group and not Tenant-A group
24. NoOrg sees NoOrg user and Tenant-A user (global users visibility)
25. Auth A owner for tenant A context switch
26. SelectTenant A owner context for isolation checks
27. NoOrg profile probe: A owner invites the probe user to tenant A
28. NoOrg profile probe: probe user accepts the tenant A invitation
29. NoOrg profile probe: probe user selects tenant A
30. Tenant A opens its own owner user document (positive control)
31. Tenant A closes its own owner user document
32. Tenant A cannot see NoOrg user
33. Tenant A cannot see NoOrg role
34. Tenant A GetProfile shows currentTenantId and tenant roles
35. Tenant A cannot see NoOrg group
36. NoOrg cannot read Tenant B directly
37. Tenant A cannot read Tenant B directly
38. Tenant B opens its own owner user document (positive control)
39. Tenant B closes its own owner user document
40. Tenant B cannot see NoOrg user

### 14 Advanced Edge - Lifecycle depth (60)

1. Advanced inactive: SetTenantActive false for tenant A
2. Advanced inactive: GetTenant A is inactive
3. Advanced inactive: A owner SelectTenant strict behavior
4. Advanced inactive: Refresh auth A member
5. Advanced inactive: A member SelectTenant strict behavior
6. Advanced inactive: refused SelectTenant leaves A member outside any tenant
7. Advanced inactive: SetTenantActive true for tenant A
8. Advanced inactive: GetTenant A is active again
9. Advanced invite: Create invitation A -> outsider for revoked reuse
10. Advanced invite: Resend pending invitation succeeds
11. Advanced invite: Revoke invitation A -> outsider for revoked reuse
12. Advanced invite: Accept revoked invitation is rejected
13. Advanced invite: GetTenantInvitations shows revoked status
14. Advanced invite: IsMember control - A member is a member of A
15. Advanced invite: outsider is not a member of A after the refused accept
16. Advanced membership: Create invitation B -> outsider for post-remove checks
17. Advanced membership: Outsider accepts invitation in B
18. Advanced membership: Remove outsider membership from B
19. Advanced membership: Refresh auth outsider after removal
20. Advanced membership: Outsider tenant list excludes B after removal
21. Advanced membership: Outsider cannot SelectTenant B after removal
22. Advanced membership: IsMember false for outsider in B after removal
23. Advanced membership-role: Create invitation B -> A member
24. Advanced membership-role: Refresh auth A member before B accept
25. Advanced membership-role: A member accepts invitation in B
26. Advanced membership-role: UpdateMembershipPermissions for A member in B
27. Advanced membership-role: GetMembership returns A member in B
28. Advanced membership-role: UpdateMembershipPermissions clears the organization permissions
29. Advanced membership-role: GetMembership shows no organization permissions
30. 14 Authorization b_owner (No Organization) for independent organizations
31. Advanced connection: Create temporary tenant for connection lifecycle
32. Advanced connection: Select temporary tenant context before config
33. Advanced connection: Enable connect by code on temporary tenant
34. Advanced connection: Select temporary tenant context for B owner
35. Advanced connection: Get connection code for temporary tenant
36. Advanced connection: Create request A -> temporary tenant for reject flow
37. Advanced connection: Reject pending request on temporary tenant
38. Advanced connection: Rejected request cannot be approved
39. Advanced connection: Create request A -> temporary tenant for cancel flow
40. Advanced connection: Cancel pending request by source owner
41. Advanced connection: Canceled request cannot be approved
42. Advanced connection: Regenerate connection code for temporary tenant
43. Advanced connection: Old code is rejected after regenerate
44. Advanced connection: Create request with regenerated code
45. Advanced connection: Approve request created with regenerated code
46. Advanced connection: Remove temporary approved connection
47. Advanced connection: Remove temporary connection tenant
48. Advanced ownership: Create temporary tenant for transfer
49. Advanced ownership: Select temporary tenant context before invite
50. Advanced ownership: Invite A member to temporary ownership tenant
51. Advanced ownership: Select temporary ownership tenant context for B owner
52. Advanced ownership: Refresh auth A member before ownership invite accept
53. Advanced ownership: A member accepts ownership tenant invitation
54. Advanced ownership: Transfer tenant to A member
55. Advanced ownership: A member selects temporary ownership tenant context
56. Advanced ownership: GetTenant confirms A member is the owner
57. Advanced ownership: Transfer tenant back to B owner
58. Advanced ownership: GetTenant confirms B owner restored
59. Advanced ownership: Remove A member membership from temporary ownership tenant
60. Advanced ownership: Remove temporary ownership tenant

### 14A Role permissions validation in-tenant (15)

1. RolePerm: Authorization A owner (No Organization)
2. RolePerm: SelectTenant A owner context
3. RolePerm: GetTenantPermissions A
4. RolePerm: GetOrganizationPermissions picks a real permission tenant A roles may not use
5. RolePerm: Create role document (forbidden probe)
6. RolePerm: Update role from representation (forbidden permission)
7. RolePerm: Save role document (forbidden permission - expect rejection)
8. RolePerm: Create role document (allowed probe)
9. RolePerm: Update role from representation (allowed permission)
10. RolePerm: Save role document (allowed permission - expect success)
11. RolePerm: Create role document (system permission probe)
12. RolePerm: Update role from representation (system permission ExportUser)
13. RolePerm: Save role document (system permission ExportUser - expect rejection)
14. RolePerm: RolesList read-back - only the allowed probe role was stored
15. RolePerm: RoleItem read-back - allowed probe role keeps its permission

### 14B Scope isolation - Roles/Users/Groups (18)

1. Scope seed variables
2. Scope auth A owner (No Organization)
3. Scope select tenant A
4. Scope create role in tenant A
5. Scope create group in tenant A
6. Scope auth B owner (No Organization)
7. Scope select tenant B
8. Scope create role in tenant B
9. Scope create group in tenant B
10. Scope RolesList in tenant A
11. Scope RolesList in tenant B
12. Scope RolesList in No Organization
13. Scope GroupsList in tenant A
14. Scope GroupsList in tenant B
15. Scope GroupsList in No Organization
16. Scope UsersList in tenant A
17. Scope UsersList in tenant B
18. Scope UsersList in No Organization

### 05B User representation tenant isolation (roles in GetUserRepresentation) (19)

1. Refresh auth A owner (user repr isolation setup)
2. SelectTenant A owner (user repr isolation setup)
3. Isolation probe: Create role bound to tenant A
4. Isolation probe: Update role permissions
5. Isolation probe: Save role document
6. Isolation probe: Resolve real role objectId
7. Isolation probe: Create group bound to tenant A
8. Isolation probe: Update group
9. Isolation probe: Save group document
10. Isolation probe: Resolve real group objectId
11. Isolation probe: Open a_member user document
12. Isolation probe: Wait for a_member document load
13. Isolation probe: Assign role+group to a_member (tenant A binding)
14. Isolation probe: Save a_member document
15. GetUserRepresentation under tenant A (roles must be tenant-scoped)
16. SelectTenant B owner (cross tenant repr probe)
17. Isolation probe: Open a_member user document as B owner
18. Isolation probe: Wait for a_member document load (B owner)
19. GetUserRepresentation under B (must not return A-tenant role IDs for shared users)

### 15 QA Regression - Group roles, group search, registration (52)

1. QA Regression seed variables
2. QA Regression role granted only through a group
3. QA Regression group carrying the role
4. D-33 GroupsList text search finds a group created with GroupAdd
5. D-33 CreateNewDocument group
6. D-33 UpdateGroupFromRepresentation names the new group
7. D-33 SaveDocument group
8. D-33 GroupsList text search finds a group created through the document service
9. D-36 OpenDocument group carrying the role
10. D-36 GetGroupRepresentation lists the granted permission with its role (waits for load)
11. D-36 CloseDocument group
12. QA Regression member added only to the group
13. D-26 UserItem reports the group permission with its source, not the group role as a direct role
14. D-26 GetProfile reports the role inherited from the group
15. D-26 Authorization of the member grants the inherited permission
16. D-26 OpenDocument member
17. D-26 GetUserRepresentation shows only direct roles (waits for load)
18. D-26 UpdateUserFromRepresentation removes the member from the group (roles as shown in the editor)
19. D-26 SaveDocument member
20. D-26 UserItem after leaving the group has no roles left
21. D-26 Authorization after leaving the group no longer grants the permission
22. D-35 UserUpdate with a direct permission
23. D-35 UserItem reports no permission without roles or groups
24. D-35 Authorization does not grant a direct permission
25. QA Regression legacy member added only to the group
26. D-01 UserItem of the legacy member (round trip source)
27. D-01 UserUpdate writes back the representation and leaves the group
28. D-01 Authorization after the legacy round trip no longer grants the permission
29. D-16 SetProfile by the legacy member renames their own profile
30. D-16 GetRevisionInfoList attributes the profile revision to the requesting user
31. D-16 UserItem shows the self-renamed profile
32. SetProfile of another user by a non-superuser is refused
33. UserItem of the other user keeps its name after the refused SetProfile
34. SetProfile of another user by the superuser is accepted
35. UserItem of the other user carries the superuser rename
36. D-32 RegisterUser with a password violating the policy returns a structured failure
37. D-32 Authorization of the refused registration fails
38. D-37 Child role inheriting the group role
39. D-37 User holding only the child role
40. D-37 UserItem names the parent role as the source
41. D-37 RoleItem of the child role lists the inherited permission
42. D-37 RoleUpdate cannot close a parent role cycle
43. D-37 Authorization after the refused cycle still answers
44. D-37 UserItem has no parent role cycle
45. D-38 UserAdd without a top-level name
46. D-38 UsersList text search finds a user created with UserAdd
47. D-38 OpenDocument names a user created with UserAdd
48. D-38 CloseDocument of the named user
49. D-38 UserUpdate renames the user
50. D-38 UsersList text search finds the renamed user by its new name
51. D-38 UsersList text search no longer finds the renamed user by its old name
52. D-38 cleanup: remove the named user

### 16 Access Control - Sessions, Self-Registration, Unlock (15)

1. AC seed: Auth SU
2. AC role granting a probe permission
3. AC group carrying the role
4. AC anonymous RegisterUser asking for the role and the group
5. AC self-registered user holds neither the requested role nor the group
6. AC Authorization of the self-registered user
7. AC superuser RegisterUser keeps the requested role and group
8. AC superuser-registered user holds the requested role and group
9. AC Authorization of the superuser-registered user carries the probe permission
10. AC anonymous CreateNewSession for the superuser is refused
11. AC CreateNewSession for another user is refused
12. AC CreateNewSession for oneself is allowed
13. AC superuser CreateNewSession for another user is allowed
14. AC anonymous UnlockUser is refused
15. AC UnlockUser by a regular user is refused

### 17 Permission Inheritance in Organization (53)

1. IH auth A owner
2. IH A owner selects tenantAId
3. IH auth B owner
4. IH B owner selects tenantBId
5. IH auth A member
6. IH A member selects tenantAId
7. IH A member baseline permissions
8. IH pick tenant A permissions the member does not hold yet
9. IH pick a tenant B permission the member does not hold
10. IH create role document parent
11. IH fill role parent
12. IH save role parent
13. IH create role document child (parent role)
14. IH fill role child (parent role)
15. IH save role child (parent role)
16. IH child role stores its parent role
17. IH create role document of the parent group
18. IH fill role of the parent group
19. IH save role of the parent group
20. IH create group document parent
21. IH fill group parent
22. IH save group parent
23. IH create group document child with the member
24. IH fill group child with the member
25. IH save group child with the member
26. IH child group stores its parent group
27. IH create group document carrying the child role
28. IH fill group carrying the child role
29. IH save group carrying the child role
30. IH member gets permissions through a parent role and a parent group
31. IH member profile lists the inherited permissions
32. IH child role drops its parent role
33. IH member loses the parent role permission
34. IH child group drops its parent group
35. IH member loses the parent group permission
36. IH create role document in tenant B
37. IH fill role in tenant B
38. IH save role in tenant B
39. IH create group document in tenant B
40. IH fill group in tenant B
41. IH save group in tenant B
42. IH A owner sets a tenant B role as parent role
43. IH child role does not store the tenant B parent role
44. IH member does not inherit through a tenant B parent role
45. IH child role drops the tenant B parent role
46. IH A owner sets a tenant B group as parent group
47. IH child group does not store the tenant B parent group
48. IH member does not inherit through a tenant B parent group
49. IH A owner gives the member group a tenant B role
50. IH member profile lists only the roles of organization A
51. IH member reads the role it holds through its group
52. IH member cannot read the tenant B role of its group
53. IH group drops the tenant B role

### 18 Permission Inheritance without Organization (29)

1. PI seed: Auth SU
2. PI grandparent role R3
3. PI parent role R2 inherits R3
4. PI role R1 inherits R2
5. PI role of the grandparent group
6. PI role of the parent group
7. PI grandparent group G3
8. PI parent group G2 under G3
9. PI group G1 under G2
10. PI user holding R1
11. PI user in G1
12. PI auth U1
13. PI auth U2
14. PI U1 permissions through the role chain
15. PI U1 can list roles through the inherited ViewRoles
16. PI U2 permissions through the group chain
17. PI U2 cannot list roles
18. PI R2 drops its parent role
19. PI U1 loses the grandparent permissions
20. PI U1 can no longer list roles
21. PI G2 drops its parent group
22. PI U2 loses the grandparent group permission
23. PI G2 cannot take G1 as parent group (cycle)
24. PI role of another product carrying ViewRoles
25. PI user holding only the ProLife role
26. PI auth the ProLife-only user
27. PI ProLife-only user has no Puma permissions
28. PI ProLife-only user cannot list Puma roles
29. PI the ProLife-only user holds the ProLife role permissions in ProLife

### 19 QA Regression - Organizations GUI (46)

1. ORG Find the membership of the A owner
2. ORG RemoveMembership of the owner is refused with the reason
3. ORG Get connection code A
4. ORG CreateConnectionRequest with the own code names the reason
5. ORG CreateConnectionRequest with an unknown code names the reason
6. ORG Enable connect by code for B
7. ORG Get connection code B
8. ORG CreateConnectionRequest to a connected organization names the reason
9. ORG Create user document with a proposed id
10. ORG Update the new user without a password
11. ORG Saving a new user without a password is refused
12. ORG Update the new user with a password
13. ORG Save the new user
14. ORG The new user is stored under the proposed id
15. ORG The automatic membership references the stored user
16. ORG GroupAdd parent group for the cycle probe
17. ORG GroupAdd child group of the parent
18. ORG GroupUpdate cannot close a parent group cycle
19. ORG Authorize the A owner outside of any organization
20. ORG Create a new organization document outside of any organization
21. ORG Fill the new organization
22. ORG Save the new organization
23. ORG The stored organization is not edited outside of it
24. ORG The stored organization is not saved outside of it
25. ORG Switch into the new organization
26. ORG The new organization is edited from inside it
27. ORG Read the current name of A
28. ORG Renaming the open organization to a taken name is refused
29. ORG The refused name does not stay in the open document
30. ORG The refused change leaves no unsaved change
31. ORG Opening A from inside X is refused
32. ORG Re-authorize the A owner outside of any organization before opening A
33. ORG Opening A outside of any organization is refused
34. ORG Switch into A
35. ORG Open A from inside A
36. ORG The open A document is not read from X
37. ORG The open A document is not changed from X
38. ORG The open A document is not saved from X
39. ORG Close the A document
40. ORG Close the X document
41. ORG Re-authorize the A owner outside of any organization before the duplicate name probe
42. ORG Create a second organization document
43. ORG Name the second organization like X
44. ORG A duplicate organization name is refused in organization terms
45. ORG Close the second organization document
46. ORG Remove organization X

### 20 Organization permissions by path (35)

1. OPP: Authorization A member
2. OPP: SelectTenant A for the member
3. OPP: Owner grants Members/Edit/Invite as its full path
4. OPP: The membership stores exactly Members/Edit/Invite
5. OPP: A leaf granted as its full path lets the member invite
6. OPP: Owner revokes that invitation
7. OPP: The invitation is stored as revoked
8. OPP: Owner grants only Members/View
9. OPP: The membership stores exactly Members/View
10. OPP: A sibling leaf does not let the member invite
11. OPP: Owner grants the group Members/Edit
12. OPP: The membership stores exactly the group Members/Edit
13. OPP: A group of the tree is no permission of its own
14. OPP: Owner grants the group Edit of the members by its bare id
15. OPP: The refused bare id left the membership unchanged
16. OPP: A group named by its bare id is no permission either
17. OPP: The refused invitations left no pending invitation
18. OPP: Owner grants only Members/View for the reads
19. OPP: The membership stores exactly Members/View for the reads
20. OPP: Members/View opens the list of members
21. OPP: Members/View does not open the connections
22. OPP: Members/View does not open the permissions of another member
23. OPP: Owner grants only Connections/View
24. OPP: The membership stores exactly Connections/View
25. OPP: Connections/View opens the connections
26. OPP: Connections/View does not open the connection code
27. OPP: Connections/View does not open the list of members
28. OPP: Owner grants only Connections/ViewCode
29. OPP: The membership stores exactly Connections/ViewCode
30. OPP: Connections/ViewCode opens the connection code
31. OPP: Owner grants only Permissions/View
32. OPP: The membership stores exactly Permissions/View
33. OPP: Permissions/View opens the permissions of another member
34. OPP: Owner grants nothing again
35. OPP: The membership stores no organization permission

### 21 QA Regression - Organizations GUI audit (67)

1. AUD Authorize the A owner outside of any organization
2. AUD Authorize the A member outside of any organization
3. AUD Create the ownership transfer organization
4. AUD Switch into the ownership transfer organization
5. AUD Invite the A member into the ownership transfer organization
6. AUD The A member accepts the invitation
7. AUD Open the organization before the transfer
8. AUD Wait for the organization document load
9. AUD Transfer the ownership to the A member
10. AUD GetTenant names the new owner
11. AUD The organization representation names the new owner
12. AUD The previous owner edits the open organization document
13. AUD Saving the open document keeps the new owner
14. AUD Close the ownership transfer organization document
15. AUD Re-authorize the A owner outside of any organization for the list probes
16. AUD The organization list of the A owner
17. AUD The organization list sorts by the Role column
18. AUD The organization list sorts by the Role column descending
19. AUD Create the organization to deactivate
20. AUD Switch into the organization to deactivate
21. AUD Deactivate the organization
22. AUD Re-authorize the A owner outside of any organization for the status filters
23. AUD The Inactive status filter lists only inactive organizations
24. AUD The Active status filter lists only active organizations
25. AUD Switch into A for the user history probe
26. AUD Create a user document in A
27. AUD Fill the new user
28. AUD Save the new user
29. AUD Every revision of the new user names its author and its change
30. AUD Rename the user
31. AUD Save the renamed user
32. AUD The rename is not recorded as a change of the login
33. AUD Close the user document
34. AUD Add the user to a new group
35. AUD The group membership revision of the user names the group
36. AUD Create a role document as the editor does
37. AUD Fill the new role
38. AUD Save the new role
39. AUD Rename the role
40. AUD Save the renamed role
41. AUD Close the new role document
42. AUD Open the stored role
43. AUD Wait for the role document load
44. AUD The role representation carries the stored role id
45. AUD The role history is found by the id of its representation
46. AUD Close the stored role document
47. AUD Create a group document as the editor does
48. AUD Fill the new group
49. AUD Save the new group
50. AUD Rename the group
51. AUD Save the renamed group
52. AUD Close the new group document
53. AUD Open the stored group
54. AUD Wait for the group document load
55. AUD The group representation carries the stored group id
56. AUD The group history is found by the id of its representation
57. AUD Close the stored group document
58. AUD Authorize the A member
59. AUD Switch the A member into the ownership transfer organization
60. AUD The ownership transfer organization accepts connections by code
61. AUD Connection code of the ownership transfer organization
62. AUD A requests a connection to the ownership transfer organization
63. AUD The ownership transfer organization approves the connection
64. AUD Remove the audit connection
65. AUD Re-authorize the A owner outside of any organization for the cleanup
66. AUD Remove the deactivated organization
67. AUD Remove the ownership transfer organization

### 22 Tenant-scoped uniqueness - Role-ID and group name (54)

1. Uniq: Authorization A owner (No Organization)
2. Uniq: SelectTenant A
3. Uniq: Authorization B owner (No Organization)
4. Uniq: SelectTenant B
5. Uniq: RoleAdd in tenant A creates Role-ID R
6. Uniq: RoleAdd in tenant B accepts the same Role-ID R
7. Uniq: RoleAdd of Role-ID R again in tenant A is rejected
8. Uniq: RoleAdd in tenant A creates Role-ID R2
9. Uniq: RoleUpdate renaming R2 to Role-ID R in tenant A is rejected
10. Uniq: RoleUpdate of the R role keeping its Role-ID is accepted
11. Uniq: GroupAdd in tenant A creates group G
12. Uniq: GroupAdd in tenant B accepts the same group name G
13. Uniq: GroupAdd of group name G again in tenant A is rejected
14. Uniq: GroupAdd in tenant A creates group G2
15. Uniq: GroupUpdate renaming G2 to G in tenant A is rejected
16. Uniq: GroupUpdate of group G keeping its name is accepted
17. Uniq: Role document with Role-ID R created in tenant A
18. Uniq: Role document in tenant A takes Role-ID R
19. Uniq: Saving a second Role-ID R in tenant A is rejected
20. Uniq: Rejected role document in tenant A closed
21. Uniq: Role document with Role-ID R2 created in tenant B
22. Uniq: Role document in tenant B takes Role-ID R2 of tenant A
23. Uniq: Saving Role-ID R2 in tenant B is accepted
24. Uniq: Role document in tenant B closed
25. Uniq: Group document with name G created in tenant A
26. Uniq: Group document in tenant A takes name G
27. Uniq: Saving a second group G in tenant A is rejected
28. Uniq: Rejected group document in tenant A closed
29. Uniq: Group document with name G2 created in tenant B
30. Uniq: Group document in tenant B takes name G2 of tenant A
31. Uniq: Saving group G2 in tenant B is accepted
32. Uniq: Group document in tenant B closed
33. Uniq: Role document with Role-ID R created in No Organization
34. Uniq: No Organization role document takes Role-ID R
35. Uniq: Saving Role-ID R in No Organization is accepted
36. Uniq: No Organization role document closed
37. Uniq: Second role document with Role-ID R created in No Organization
38. Uniq: Second No Organization role document takes Role-ID R
39. Uniq: Saving a second Role-ID R in No Organization is rejected
40. Uniq: Rejected No Organization role document closed
41. Uniq: Group document with name G created in No Organization
42. Uniq: No Organization group document takes name G
43. Uniq: Saving group G in No Organization is accepted
44. Uniq: No Organization group document closed
45. Uniq: Second group document with name G created in No Organization
46. Uniq: Second No Organization group document takes name G
47. Uniq: Saving a second group G in No Organization is rejected
48. Uniq: Rejected No Organization group document closed
49. Uniq: RolesList in tenant A
50. Uniq: RolesList in tenant B
51. Uniq: RolesList in No Organization
52. Uniq: GroupsList in tenant A
53. Uniq: GroupsList in tenant B
54. Uniq: GroupsList in No Organization

### 23 Organization permission scope (90)

1. SC SU creates organization S with the A member as its owner
2. SC The creator sets the scope of S
3. SC S holds exactly the chosen scope
4. SC The owner selects S: sign in
5. SC The owner selects S
6. SC Authorization of the outsider for the root organization probe
7. SC The owner of S holds exactly its scope and the access to the organizations
8. SC The owner reads the scope of S
9. SC The owner cannot change the scope of S
10. SC The refused change left the scope of S as it was
11. SC The owner creates the sub-organization S1 from inside S
12. SC S1 is a child of S, created and owned by the owner of S
13. SC The organization list names the parent of S1
14. SC S1 starts with the default scope cut to the scope of S
15. SC The creator enters S1: sign in
16. SC The creator enters S1
17. SC A scope beyond the parent is refused
18. SC The refused superset left S1 as it was
19. SC The creator narrows S1 within the scope of S
20. SC S1 holds exactly the edited scope
21. SC The creator enters the edited S1: sign in
22. SC The creator enters the edited S1
23. SC A parent other than the current organization is refused
24. SC The creator of S1 creates S1A inside S1
25. SC S1A starts with the default scope cut to S1
26. SC The creator opens the S1 document
27. SC The creator opens the S1 document: wait for the load
28. SC The creator reads the S1 document with the parent scope and the names
29. SC The S1 document takes a narrowed scope
30. SC Save the narrowed S1 document
31. SC Close the S1 document
32. SC The saved document narrowed S1
33. SC Saving the narrowed document trims S1A
34. SC The creator restores the scope of S1
35. SC The owner opens a new organization document inside S
36. SC The new document names S as its parent and offers the scope of S
37. SC The document refuses a scope beyond S
38. SC The document takes its name; the creator is the owner until another one is chosen
39. SC The document refuses an owner who does not exist
40. SC The document takes a scope within S and the owner chosen by the creator
41. SC Save the sub-organization document
42. SC The stored document is a child of S
43. SC The stored document holds exactly its scope
44. SC The creator enters the organization owned by another user: sign in
45. SC The creator enters the organization owned by another user
46. SC The creator works in the organization owned by another user
47. SC Close the sub-organization document
48. SC The creator sees itself among the users of the organization owned by another user
49. SC The owner of the document organization enters it: sign in
50. SC The owner of the document organization enters it
51. SC The owner opens the stored organization
52. SC The owner opens the stored organization: wait for the load
53. SC The owner reads the scope read-only with the names of the owner and the creator
54. SC The document refuses a scope change by the owner
55. SC The owner closes the stored organization
56. SC The refused owner change left the document organization as it was
57. SC The creator of S drops the right to create organizations
58. SC S holds the narrowed scope
59. SC An open session of the owner follows the narrowed scope after the cache lifetime
60. SC Narrowing S trims S1
61. SC Narrowing S leaves S1A empty
62. SC The owner selects S again: sign in
63. SC The owner selects S again
64. SC The owner of S follows the narrowed scope
65. SC Without the right in the scope the owner creates no sub-organization
66. SC The creator enters S1 again: sign in
67. SC The creator enters S1 again
68. SC A right the narrowed parent lost is refused
69. SC The refused right left S1 trimmed
70. SC The creator empties the scope of S1
71. SC S1 holds an empty scope
72. SC The owner selects the empty S1: sign in
73. SC The owner selects the empty S1
74. SC An empty scope grants nothing but the access to the organizations
75. SC Without the right the outsider creates no organization
76. SC Without the right the outsider opens no new organization document
77. SC Authorization of the outsider (No Organization)
78. SC Without an organization or an invitation the organizations are hidden
79. SC The owner selects S to invite: sign in
80. SC The owner selects S to invite
81. SC The owner of S invites the outsider
82. SC Sign-in of the invited outsider
83. SC A pending invitation shows the organizations
84. SC The owner of S revokes the invitation
85. SC Sign-in of the outsider after the revoke
86. SC A revoked invitation hides the organizations again
87. SC Remove document sub-organization
88. SC Remove S1A
89. SC Remove S1
90. SC Remove S

### 24 Organizations audit - lookups, removal guard, cascades (100)

1. OA Sign in SU without organization
2. OA Sign in A member without organization
3. OA Sign in outsider without organization
4. OA SU creates organization P owned by the A member
5. OA SU gives P the scope to create sub-organizations
6. OA The A member lists P as its owner
7. OA SU lists P as its creator
8. OA The outsider does not list P
9. OA The outsider cannot read the organizations of the A member
10. OA SU reads the organizations of the A member
11. OA The A member selects P
12. OA The A member invites the outsider to P
13. OA The outsider accepts the invitation to P
14. OA The outsider lists P as a member
15. OA SU deactivates P
16. OA The outsider does not list the inactive P
17. OA SU activates P again
18. OA The outsider lists the active P again
19. OA The A member excludes the outsider from P
20. OA The excluded outsider does not list P
21. OA SU selects P
22. OA SU creates the sub-organization C inside P
23. OA C is a sub-organization of P
24. OA RemoveTenant refuses P while it has a sub-organization
25. OA Removing P alone from the collection is refused
26. OA P and C are still stored
27. OA P is removed together with its sub-organization
28. OA P is gone
29. OA C is gone
30. OA Sign in A owner without organization
31. OA Sign in B owner without organization
32. OA SU creates organization X owned by the A owner
33. OA SU creates organization Y owned by the B owner
34. OA The A owner selects X
35. OA The B owner selects Y
36. OA Y accepts connections by code
37. OA The B owner reads the connection code of Y
38. OA X asks Y for a connection
39. OA Y approves the connection
40. OA The A owner removes the connection of X and Y
41. OA The B owner no longer lists X
42. OA SU creates organization Q owned by the A owner
43. OA The A owner selects Q
44. OA Sign in A member again
45. OA The A owner invites the A member to Q
46. OA The A member accepts the invitation to Q
47. OA The A owner lets the A member invite
48. OA The A member selects Q
49. OA The A member invites the outsider to Q
50. OA The A owner invites the B owner to Q
51. OA The A owner excludes the A member from Q
52. OA Only the invitation sent by the excluded member is revoked
53. OA The outsider has no pending invitation to Q
54. OA Sign in A owner for section E
55. OA Sign in A member for section E
56. OA Sign in SU for section E
57. OA SU creates organization W owned by the A owner
58. OA The A owner selects W
59. OA The A owner invites the A member to W
60. OA The A member accepts the invitation to W
61. OA The A owner lets the A member invite to W
62. OA The A member selects W
63. OA The A member invites the outsider to W
64. OA The A owner leaves the A member out of the members of W
65. OA The invitation sent by the member left out of W is revoked
66. OA The A owner invites the outsider to W again
67. OA An update of W with no members leaves its owner
68. OA The invitation the owner sent is still pending
69. OA The owner of W is still its member
70. OA The A owner cannot remove the unrelated Y by a filter
71. OA Y is still stored
72. OA SU selects W
73. OA The users of W filtered by the A owner are the A owner only
74. OA The count of the users of W filtered by the A owner is 1
75. OA A filter for a user outside W selects nobody
76. OA The users of W without a filter are its people
77. OA SU creates an organization with a semicolon in the name
78. OA The same name with a semicolon in another case is refused
79. OA SU creates an organization with a backslash in the name
80. OA The same name with a backslash in another case is refused
81. OA Sign in B owner for section F
82. OA SU creates organization V owned by the B owner
83. OA The B owner selects V
84. OA V accepts connections by code
85. OA The B owner reads the connection code of V
86. OA W asks V for a connection
87. OA V approves the connection
88. OA Sign in A owner for section G
89. OA The A owner selects Q for section G
90. OA The connection of W and V cannot be removed in the name of Q
91. OA W removes its connection with V
92. OA W asks V for a connection again
93. OA V approves the connection again, the same one
94. OA Cleanup: SU removes X
95. OA Cleanup: SU removes Y
96. OA Cleanup: SU removes Q
97. OA Cleanup: SU removes W
98. OA Cleanup: SU removes V
99. OA Cleanup: SU removes Semicolon
100. OA Cleanup: SU removes Backslash

### 26 Organization management of users, roles and groups (23)

1. OM SU creates the organization W owned by the A owner
2. OM SU, the creator, empties the permission scope of W
3. OM The permission scope of W is empty
4. OM Sign in the A owner
5. OM The A owner selects W
6. OM The owner creates a role in W
7. OM The owner creates a group in W
8. OM The owner creates a member of W
9. OM The owner invites the A member to W
10. OM Sign in the A member
11. OM The A member accepts the invitation to W
12. OM Sign in the A member again
13. OM The A member selects W
14. OM A member without organization permissions cannot create a role
15. OM The owner lets the A member add roles
16. OM Sign in the A member with the new permission
17. OM The A member selects W again
18. OM The member creates a role in W
19. OM The member still cannot create a group
20. OM Sign in the A owner outside of any organization
21. OM The owner of W cannot create a role of the product
22. OM Cleanup: SU removes the member created in W
23. OM Cleanup: SU removes W

### 25 User data isolation - profiles, sessions, password recovery (67)

1. UI Register the user V
2. UI Register the user X
3. UI Sign in V
4. UI Sign in X
5. UI X cannot read the profile of V
6. UI An anonymous request cannot read the profile of V
7. UI V reads the own profile
8. UI SU reads the profile of V
9. UI SU creates an organization owned by V
10. UI X cannot list the organizations of V
11. UI An anonymous request cannot list the organizations of V
12. UI V lists the own organization
13. UI An anonymous request lists no organization of V
14. UI X lists no organization of V
15. UI V lists the own organization in the collection
16. UI An anonymous request cannot read the organization of V
17. UI X cannot read the organization of V
18. UI V reads the own organization
19. UI X cannot read the organization permissions of V
20. UI V reads the own organization permissions
21. UI X cannot get the id of V
22. UI An anonymous request cannot get the id of V
23. UI V gets the own id
24. UI SU gets the id of V
25. UI X cannot read the session of V
26. UI An anonymous request cannot read the session of V
27. UI X gets the same answer for a session that does not exist
28. UI V reads the own session
29. UI X cannot end the session of V
30. UI An anonymous request cannot end the session of V
31. UI The session of V still works
32. UI CheckEmail of the e-mail of V names no account
33. UI CheckEmail of an unknown e-mail answers alike
34. UI SendEmailCode for V answers success
35. UI SendEmailCode for an unknown login answers alike
36. UI SendEmailCode for an unknown e-mail answers alike
37. UI CheckEmailCode refuses a guessed code of V
38. UI CheckEmailCode answers alike for an unknown login
39. UI An anonymous request without the old password and a code is refused
40. UI An anonymous request with a guessed code is refused
41. UI A guessed code by the e-mail of V is refused
42. UI X without the old password of V is refused
43. UI X with a wrong old password of V is refused
44. UI An unknown login is refused like a wrong password
45. UI A wrong password by an unknown e-mail is refused
46. UI A wrong password by the e-mail of V answers like an unknown e-mail
47. UI The hijack password does not sign V in
48. UI V still signs in with the own password
49. UI V changes the own password with the old one
50. UI V signs in with the new password
51. UI The old password of V no longer signs in
52. UI Register the user W who will be removed
53. UI Sign in W
54. UI Sign in SU for the removed inviter
55. UI SU creates the organization K
56. UI SU selects K
57. UI SU invites W to K
58. UI W accepts the invitation to K
59. UI SU lets W invite
60. UI W selects K
61. UI W invites X to K
62. UI Sign in X again
63. UI X has the invitation of W pending
64. UI SU removes the user W
65. UI X cannot accept the invitation of the removed W
66. UI Cleanup: SU removes K
67. UI Cleanup: SU removes the organization of V

### 27 Product boundary (52)

1. PB seed: auth su
2. PB seed: administrator role of Puma
3. PB seed: role of Puma the administrator holds
4. PB seed: role of Puma with a permission he lacks
5. PB seed: role of Lisa
6. PB seed: group with a role of Lisa
7. PB seed: administrator
8. PB seed: user with a role of Lisa
9. PB seed: auth administrator
10. PB seed: the administrator creates his own role of Puma
11. PB seed: the administrator creates a second role of Puma
12. PB seed: su gives that role a permission the administrator lacks
13. PB seed: su gives the administrator his own role
14. PB seed: auth administrator holding his own role
15. PB positive control: the role of the own product is readable
16. PB a role su created is not readable by the administrator
17. PB role of another product: read naming it
18. PB role of another product: read naming the own product
19. PB role of another product: raw read
20. PB role of another product: update
21. PB role of another product: update naming the own product at the top
22. PB role of another product: update naming the own product everywhere
23. PB role of another product: forged product header
24. PB role of another product: authorize through its server
25. PB role of another product: through the server of that product
26. PB role of another product: add
27. PB role of another product: remove
28. PB role of another product still exists unchanged
29. PB list naming another product
30. PB list naming no product shows only the own product
31. PB the product of a role cannot be changed even by su
32. PB a parent role of another product
33. PB su positive control: updates a role of another product
34. PB raw data: su reads a role of the own product
35. PB raw data: roles cannot be written as raw data
36. PB grant positive control: a role with a held permission
37. PB grant: a new role with a permission he lacks
38. PB grant: adding a permission he lacks to a role
39. PB grant: self-promotion through his own role
40. PB grant: a parent role with a permission he lacks
41. PB grant: a role keeps what he lacks when only the name changes
42. PB grant: a role su created is out of reach of the administrator
43. PB grant: a user gets a role with a permission he lacks
44. PB grant: a group gets a role with a permission he lacks
45. PB grant: su gives anything
46. PB user: roles of another product
47. PB user: update in the own product
48. PB user: the role in the other product survived
49. PB group: roles of another product
50. PB token of another product
51. PB ValidateToken names the product of the token
52. PB token of another product is refused under the header of this product

### 28 Server tokens (114)

1. ST seed: auth su
2. ST seed: auth an ordinary user
3. ST: a token for a product outside the catalog is refused
4. ST: su adds the product to the catalog
5. ST: the same product cannot be added twice
6. ST: a product needs an ID
7. ST: the catalog lists this server and the new product without a token
8. ST: su renames the product
9. ST: the rename is stored
10. ST: the product ID cannot be changed
11. ST: an ordinary user does not see the catalog
12. ST: an ordinary user does not change the catalog
13. ST: su reads the permissions a server may get
14. ST: the permissions for an unknown product are refused
15. ST: the permissions for this server itself are refused
16. ST: an ordinary user does not read the permissions
17. ST: anonymous does not read the permissions
18. ST: su issues a token of StProbe for Puma
19. ST: the catalog shows a product that has never connected
20. ST: su issues a second token of StProbe
21. ST: an edit permission brings its view permission
22. ST: a token needs a name
23. ST: an unknown permission is refused
24. ST: a permission outside the receiver tree is refused
25. ST: a token for the own server is refused
26. ST: an ordinary user is refused
27. ST: anonymous is refused
28. ST: su PAT seed
29. ST: a personal token of su cannot issue
30. ST: the list shows the tokens of the product
31. ST: the list is for su only
32. ST: ValidateToken names both products
33. ST: the server reads what is open to everybody
34. ST: the catalog shows the product online once its server is heard
35. ST: a service principal reads with its own view permission
36. ST: a reading outside the permissions is refused
37. ST: a change outside the permissions is refused
38. ST: a collection read follows the view permission of the collection
39. ST: empty scopes do not read protected users
40. ST: a service without organization creation permission cannot create one
41. ST: an anonymous forwarded request has no service principal
42. ST: a server does not add clients
43. ST: a server validates a token without an extra scope
44. ST: the token is refused under the header of another product
45. ST: the token is refused under the header of Puma
46. ST: a value that is no token is refused
47. ST: an unknown token is refused
48. ST: a server token cannot issue tokens
49. ST: a server token as the token of its own requests is refused for another product
50. ST seed: personal token of su for StProbe
51. ST seed: personal token of su for Lisa
52. ST: a personal token comes through the server of its product
53. ST: the permissions of the server bound even a superuser PAT
54. ST seed: superuser session opened through the product server
55. ST: a forwarded superuser session reads within the server boundary
56. ST: a forwarded superuser session is not a global administrator
57. ST: a forwarded superuser session cannot manage server tokens
58. ST: a personal token of another product does not come through this server
59. ST: a personal token of Puma does not come through the server of another product
60. ST: a personal token of another product is refused without the token of its server
61. ST: the same with the header of its product
62. ST: a superuser session works for another product
63. ST: a personal token of su does not work for another product
64. ST: an ordinary user does not work for another product
65. ST: anonymous does not work for another product
66. ST: su gives the token the view of roles
67. ST: the new permission takes effect at once
68. ST: su takes the view of users away
69. ST: the removed permission is refused at once
70. ST: an unknown permission is not given
71. ST: the permissions of an unknown token are not changed
72. ST: the permissions of a personal token are not changed here
73. ST: an ordinary user does not change the permissions
74. ST: the server of a product reads the permissions of Puma for its roles
75. ST: a superuser session does not read them
76. ST: anonymous does not read them
77. ST: the server of the product publishes its permissions with empty scopes
78. ST: the same tree again changes nothing
79. ST: a server does not publish for another product
80. ST: a forwarded request does not publish
81. ST: su does not publish for a product
82. ST: the catalog shows the published version
83. ST: su reads the published permissions with those of Puma
84. ST: an unknown token is not revoked
85. ST: an ordinary user does not revoke
86. ST: su revokes
87. ST: a revoked token is refused at once
88. ST: a revoked token is reported invalid
89. ST: the permissions of a revoked token are not changed
90. ST: the list marks the revoked token
91. ST: a token in use is not deleted
92. ST: an ordinary user does not delete
93. ST: su deletes a revoked token
94. ST: the list has no deleted token
95. ST: a deleted token is refused
96. ST: a deleted token is not deleted again
97. ST: su revokes the remaining tokens
98. ST: a product with only revoked tokens has no token
99. ST: su adds a client product
100. ST: adding a client again changes nothing
101. ST: the added client is in the catalog without a token
102. ST: the added client gets a token
103. ST: this server is not its own client
104. ST: a client needs an ID
105. ST: an ordinary user does not add clients
106. ST: anonymous does not add clients
107. ST: the refused requests added nothing
108. ST cleanup: the added client is removed
109. ST seed: a token of the product to remove
110. ST: removing the product revokes its tokens
111. ST: the token of a removed product is invalid
112. ST: the server of a removed product is refused
113. ST cleanup: revoke only this suite's ProLife token
114. ST cleanup: revoke only this suite's Lisa token

### 29 Removed roles and groups leave no references (58)

1. RR seed: Auth SU
2. RR role to be removed
3. RR kept role inheriting the removed one
4. RR parent group to be removed
5. RR group holding both roles under the parent group
6. RR user holding the removed role directly and through the group
7. RR user holds both permissions before the removal
8. Removing the role
9. RR kept role no longer names the removed parent role
10. RR group no longer holds the removed role
11. RR user no longer holds the removed role
12. RR user lost only the permission of the removed role
13. Removing the parent group
14. RR group no longer names the removed parent group
15. RR user still holds the kept role through its group
16. Removing the group
17. RR user no longer belongs to the removed group
18. RR2 seed: Auth SU
19. RR2 parent role A
20. RR2 role B inheriting A
21. RR2 role C inheriting A
22. RR2 independent role D
23. RR2 role X of the ProLife product
24. RR2 group holding A, C and D
25. RR2 user holding B, C and D and the group
26. RR2 user also holds X in ProLife
27. RR2 user revision before the removal
28. RR2 user holds the permissions of A, B, C and D
29. Removing a parent role together with one of its children and another role
30. RR2 surviving child role no longer names the removed parent
31. RR2 removed child role stays removed
32. RR2 group keeps only the surviving role
33. RR2 user keeps only the surviving role
34. RR2 user keeps the role of the other product
35. RR2 user got one new revision for the dropped roles
36. RR2 user holds only the permission of C
37. Removing the ProLife role
38. RR2 user lost the ProLife role
39. RR2 user keeps its Puma role
40. RR2 group Ga
41. RR2 group Gb
42. RR2 group Gc under Ga and Gb
43. RR2 group Gd under Ga
44. RR2 user in Ga and Gc
45. RR2 group revision before the removal
46. Removing a group that is a parent of two groups and a membership
47. RR2 group with two parents keeps the other one
48. RR2 group with the removed parent only has none left
49. RR2 user keeps only its other group
50. RR2 group got one new revision for the dropped parent
51. Removing a group together with its parent group
52. RR2 removed child group stays removed
53. RR2 user belongs to no group any more
54. Cleanup: removing the edge case users
55. Cleanup: removing the edge case groups
56. Cleanup: removing the edge case role
57. Cleanup: removing the user
58. Cleanup: removing the kept role

### 31 Organization management levels (27)

1. OL Sign in SU
2. OL SU creates the organization L owned by the A owner
3. OL Sign in the A owner
4. OL The A owner works in L
5. OL The owner invites the A member to L
6. OL Sign in the A member
7. OL The A member accepts the invitation to L
8. OL Sign in the A member again
9. OL The A member works in L
10. OL The owner creates the member M2 of L
11. OL The owner finds the membership of M2
12. OL The tree gives the level Administrator
13. OL The owner lets M1 invite only
14. OL M1 that may invite cannot give permissions to M2
15. OL M1 that may invite cannot give itself everything
16. OL M2 got nothing from M1
17. OL The owner lets M1 change the permissions
18. OL M1 cannot give M2 a permission it does not hold
19. OL M2 still has nothing
20. OL M1 gives M2 a permission it holds
21. OL M2 can view the members
22. OL The owner makes M2 an administrator
23. OL M2 holds the level Administrator
24. OL M1 keeps the permissions M2 already holds
25. OL M1 takes a permission away from M2
26. OL M1 cannot give the taken permission back
27. OL SU removes the organization of this folder

### 32 Contract notifications (133)

1. CN Sign in SU
2. CN SU creates X
3. CN SU creates Y
4. CN SU creates Z
5. CN SU adds the A member to X
6. CN Sign in A owner
7. CN A owner works in X
8. CN Sign in A member
9. CN A member works in X
10. CN Sign in B owner
11. CN B owner works in Y
12. CN Sign in outsider
13. CN Outsider works in Z
14. CN Y gives its connection code
15. CN X asks Y for a connection
16. CN Y approves
17. CN Z gives its connection code
18. CN X asks Z for a connection
19. CN Z approves
20. CN X creates the contract C1
21. CN Y cannot change its private data before C1 is sent
22. CN X cannot assign a member of Z to C1
23. CN X saves internal fields of C1
24. CN X makes the A member responsible for C1
25. CN X repeats the same assignment without a new journal entry
26. CN X keeps its internal fields after the assignment
27. CN X sees two private changes in the journal
28. CN A owner of X has nothing of C1
29. CN A member of X has nothing of C1
30. CN B owner of Y has nothing of C1
31. CN Outsider of Z has nothing of C1
32. CN X sends C1
33. CN A owner of X has nothing of C1
34. CN A member of X has nothing of C1
35. CN B owner of Y has InvitationReceived (X) of C1
36. CN Outsider of Z has InvitationReceived (X) of C1
37. CN Y signs C1
38. CN A owner of X has nothing of C1
39. CN A member of X has Signed (Y) of C1
40. CN B owner of Y has nothing of C1
41. CN Outsider of Z has InvitationReceived (X) of C1
42. CN Z asks for changes of C1
43. CN A owner of X has nothing of C1
44. CN A member of X has ChangesRequested (Z), Signed (Y) of C1
45. CN B owner of Y has nothing of C1
46. CN Outsider of Z has nothing of C1
47. CN X changes the terms of C1
48. CN A owner of X has nothing of C1
49. CN A member of X has nothing of C1
50. CN B owner of Y has TermsChanged (X) of C1
51. CN Outsider of Z has TermsChanged (X) of C1
52. CN SU gives Y its scope
53. CN Y gives X a permission before the conclusion
54. CN A owner of X has nothing of C1
55. CN A member of X has nothing of C1
56. CN B owner of Y has TermsChanged (X) of C1
57. CN Outsider of Z has TermsChanged (X) of C1
58. CN Y signs C1 again
59. CN Z signs C1
60. CN A owner of X has nothing of C1
61. CN A member of X has Signed x2 (Z) of C1
62. CN B owner of Y has nothing of C1
63. CN Outsider of Z has nothing of C1
64. CN X signs C1: concluded
65. CN A owner of X has nothing of C1
66. CN A member of X has Concluded, PermissionsReceived (Y) of C1
67. CN B owner of Y has Concluded of C1
68. CN Outsider of Z has Concluded of C1
69. CN Y gives notice on C1
70. CN A owner of X has nothing of C1
71. CN A member of X has Concluded, NoticeGiven (Y), PermissionsReceived (Y) of C1
72. CN B owner of Y has Concluded of C1
73. CN Outsider of Z has Concluded, NoticeGiven (Y) of C1
74. CN Y revokes its notice
75. CN A owner of X has nothing of C1
76. CN A member of X has Concluded, NoticeRevoked (Y), PermissionsReceived (Y) of C1
77. CN B owner of Y has Concluded of C1
78. CN Outsider of Z has Concluded, NoticeRevoked (Y) of C1
79. CN Z stops the renewal of C1
80. CN A owner of X has nothing of C1
81. CN A member of X has Concluded, NoticeRevoked (Y), PermissionsReceived (Y), RenewalStopped (Z) of C1
82. CN B owner of Y has Concluded, RenewalStopped (Z) of C1
83. CN Outsider of Z has Concluded, NoticeRevoked (Y) of C1
84. CN Y finds the permissions it gave
85. CN Y takes the permissions back
86. CN A owner of X has nothing of C1
87. CN A member of X has Concluded, NoticeRevoked (Y), PermissionsReceived (Y), PermissionsWithdrawn (Y), RenewalStopped (Z) of C1
88. CN B owner of Y has Concluded, RenewalStopped (Z) of C1
89. CN Outsider of Z has Concluded, NoticeRevoked (Y) of C1
90. CN Z leaves C1 at once
91. CN A owner of X has nothing of C1
92. CN A member of X has Concluded, NoticeRevoked (Y), PermissionsReceived (Y), PermissionsWithdrawn (Y), RenewalStopped (Z), Withdrawn (Z) of C1
93. CN B owner of Y has Concluded, RenewalStopped (Z), Withdrawn (Z) of C1
94. CN Outsider of Z has Concluded, NoticeRevoked (Y) of C1
95. CN SU removes the A member from X
96. CN Y gives notice again
97. CN A owner of X: the responsible user left, the owner hears has NoticeGiven (Y) of C1
98. CN X creates the contract C2
99. CN X sends C2
100. CN B owner of Y has InvitationReceived (X) of C2
101. CN Outsider of Z has InvitationReceived (X) of C2
102. CN X removes Z from C2
103. CN Outsider of Z: removed, without a link has PartyRemoved (X) unlinked of C2
104. CN B owner of Y: the terms changed, one row has TermsChanged (X) of C2
105. CN Y declines C2
106. CN A owner of X has Declined (Y) of C2
107. CN B owner of Y has nothing of C2
108. CN X creates the contract C3
109. CN X sends C3
110. CN X withdraws C3
111. CN B owner of Y has Cancelled (X) of C3
112. CN A owner of X has nothing of C3
113. CN X creates the contract C4
114. CN X sends C4
115. CN Y signs C4
116. CN A owner of X has Signed (Y) of C4
117. CN X attaches a file of the terms of C4
118. CN A owner of X: the signature of Y is gone has nothing of C4
119. CN B owner of Y: review the file has TermsChanged (X) of C4
120. CN Y sees the attachment of C4
121. CN Y downloads exactly the bytes X attached
122. CN Y signs C4 again
123. CN A owner of X has Signed (Y) of C4
124. CN B owner of Y has nothing of C4
125. CN X adds Z to C4 by its connection code
126. CN A owner of X: the signature of Y is gone again has nothing of C4
127. CN B owner of Y: the terms changed with the new party has TermsChanged (X) of C4
128. CN Outsider of Z: invited has InvitationReceived (X) of C4
129. CN X withdraws C4
130. CN B owner of Y has Cancelled (X) of C4
131. CN Outsider of Z has Cancelled (X) of C4
132. CN A owner of X has nothing of C4
133. CN SU removes the organizations of this folder

### 99 Cleanup (4)

1. Remove connection
2. Remove tenant A
3. Remove tenant B
4. Remove tenant created via document service

## Notification center (`Notification_Center.postman_collection.json`, `ws/ws-notification-center.js`)

The user notification center of Puma (`ImtCore/Docs/Notifications/NotificationCenter.md`), in two suites that create
their own users and organizations, so they run on any system, after the tenant suite or alone:

```powershell
newman run Notification_Center.postman_collection.json -e Tenant_System_Full.postman_environment.json --env-var base_url=http://localhost:17788/Puma/graphql
node ws/ws-notification-center.js --http http://localhost:17788/Puma/graphql --env Tenant_System_Full.postman_environment.json
```

(add `--env-var su_password=...` / `--su-password ...` when su has another password, e.g. `1` after a restore from a
ProLife backup). `Run-CiTests.ps1` runs both after the tenant suites, `Run-CiTests-Sqlite.ps1` runs the collection on
SQLite.

The collection (183 requests):

- `00 Bootstrap` - four users, organizations A (alice) and B (bob), tenant sessions.
- `01 Access` - every inbox command and sending refuse an anonymous caller; any signed-in user may send and withdraw,
  only for the product of the request and to at most 100 users. This is a known hole kept until a product server has
  an identity of its own (ImagingTools/ImtCore#1059); the senders of the other folders use the session of su.
- `02 Validation` - the length limits of type, keys, subject and actor; arguments: id characters and length, repeated
  ids, more than 10, the standard values, a `Number` that is no number, a `DateTime` that is no time - a refused
  notification stores nothing; long names, values and body are cut; every kind of argument comes back as sent.
- `03 Inboxes` - a personal notification is seen in every tenant of the user, a tenant one only in that tenant, a
  product one only in that product; a non-member gets no tenant notification; nobody sees another user's; the actor is
  not notified; a repeated recipient gets one.
- `04 Groups, dedup and withdrawal` - a group counts its events, keeps the three latest actors newest first and the
  content of its latest event; a read group is closed; a dedup key is stored once and outlives a removal; a withdrawn
  group leaves every inbox and may be told again.
- `05 Inbox operations` - pages newest first, the counts, read and remove only in the own inbox, mark all read up to a
  time (no ids answered), the sync at login (newest undelivered, their count, the server time) and the acknowledgements.
- `06 Catalog` - the types, icons and messages of the organization types in English and Russian.
- `07 Notifications page` - the collection `Notifications`: the inbox of the caller only, filters, search, sorting,
  removal.
- `08 Organization notifications` - invitations (from whom, withdrawn when revoked or answered, the answer to the
  inviter), removed member (personal to the member, `member` to the owner, the organization notifications of the
  member removed), connection requests (`partner`, withdrawn when answered), the ownership
  transfer.

The WebSocket suite (`OnNotificationEvent`): every event reaches exactly the sessions that see its inbox and nothing
else - a personal one every session of the user (unread count -1), a tenant one the sessions in that tenant, an update
of a group, read, all read (a tenant inbox with `createdUntil` and no ids to its sessions, the notifications of the
personal inbox by their ids to every session of the user), removed, withdrawn from two users; a notification
of another product and an acknowledged delivery reach no session.

## WebSocket subscriptions (`ws/ws-subscriptions.js`)

Newman does not execute Postman WebSocket requests — it runs HTTP requests only —
so the subscription side of the tenant API cannot live in the Postman collection.
It is covered by a small Node runner that speaks the same protocol as the client
(`ImtCore/Qml/imtguigql/SubscriptionManager.qml` and `CWebSocketServletComp`):

```
-> {"type":"connection_init"}                                    <- {"type":"connection_ack"}
-> {"id":..,"type":"start","headers":{..},"payload":{"data":Q}}  <- (silence on success)
                                                                 <- {"type":"data","id":..,"payload":{..}}
-> {"id":..,"type":"stop"}                                       <- {"type":"complete","id":..}
                                              refusals arrive as <- {"type":"error","id":..,"payload":[..]}
                                    plus unsolicited keep-alives <- {"type":"ka"}
```

It reuses this folder's environment file for `base_url` / `su_login` / `su_password`,
and writes `junit-report-ws.xml` in the same format TeamCity already consumes.

`Run-CiTests.ps1` runs it right after newman against the same live server, so no
extra build step is needed. Standalone:

```
npm install
node ws/ws-subscriptions.js --http http://localhost:17788/Puma/graphql --ws ws://localhost:18788
```

Options: `--http`, `--ws`, `--env`, `--junit`, `--timeout`, `--grace`, `--su-login`,
`--su-password`, `--product-id`. `--grace` is how long a `start` has to stay unanswered
before it counts as accepted (default 2500 ms) - a successful registration is answered
with silence, so the absence of a refusal is the only acceptance signal on the wire.

The WS port defaults to the HTTP port + 1000 (17788 → 18788, matching
`PumaServerPgTest.acc`); override with `--ws` where that convention does not hold.

### Covered subscriptions

| Subscription | SDL |
|---|---|
| `OnDocumentManagerChanged` | `imtbase/CollectionDocumentService.sdl` |
| `OnDocumentChanged` | `imtbase/CollectionDocumentService.sdl` |
| `OnUndoRedoChanged` | `imtbase/CollectionDocumentService.sdl` |
| `OnMembershipNotification` | `imtauth/TenantMemberships.sdl` |
| `OnConnectionCodesNotification` | `imtauth/Tenants.sdl` |

Beyond "does it deliver", the suite pins down the protocol behaviour that has
actually broken in the field: an error frame must carry the id of the subscription
it refers to (the client attributes failures by id), `stop` on a live subscription
must answer `complete` rather than an error *and* end delivery to it, a repeated
`stop` must not desynchronise the session, and a subscription presenting an invalid
token must be refused as an authentication failure.

Every case asserts on the specific outcome, not on the presence of a frame. A refusal
is matched against the reason the server gives, so a case that stops reaching the check
it was written for - a malformed payload, a broken token - fails instead of passing on
whatever error came back. Notifications are matched on `documentId` and
`documentOperation` rather than counted. On top of that, every inbound frame is audited
against a running ledger of what the suite asked for: an unknown frame type, an error
nobody expected, a notification delivered to an id holding no subscription, or a payload
carrying another command's data fails the final case even though no individual case
looks for it. Cases whose preconditions an earlier failure destroyed report
"precondition not met" rather than asserting against undefined state.

The eight tenant notification subscriptions assert registration only - nothing in the
suite triggers the events that would make them deliver - and they share one grace window
instead of waiting per subscription.

The `SaveDocument` case is a regression probe for the tenant editor: after saving,
the last `OnUndoRedoChanged` frame has to report `isDirty=false`. When it does not,
the Save button stays enabled in the GUI.

### Why the routing cases are here

A ProLife client reaches Puma through the ProLife server, which forwards what it
does not serve itself. The document-service subscriptions are a single command per
operation naming their collection in `input`, where they used to be one command per
collection (`OnUsersDocumentChanged` and friends). Two things went wrong with that
migration, and neither is visible from a single server:

- a forwarder that passed the command on **without** the arguments — the upstream
  publisher selects on them, so nothing bound and the client waited forever;
- a forwarder that decided on the **command alone** — it could no longer tell a
  remotely served collection from a locally served one.

These cases keep the server strict enough that neither mistake can look healthy:
a document-service subscription with no `input`, one naming a collection this server
does not serve, and `OnDocumentChanged` without a document id all have to be refused.
The empty-id case is the same idea for the subscription id itself, which both sides
key everything on.

**This case currently fails, and the failure is real.** `CWebSocketServletComp::RegisterSubscription`
checks for an empty *command* id but never for an empty *subscription* id, and
`CWebSocketRequest` assigns `m_requestId` from the frame's `"id"` unconditionally - so a
`start` carrying `"id": ""` is accepted and bound. Every subscription sent with an empty
id then collides on that one key and receives the others' notifications. The case used to
pass because it sent raw GraphQL as `payload.data` instead of the `{"query": ...}` string
the client sends: `CGqlRequest::ParseQuery` rejected the payload before the id was ever
looked at, so the error frame it asserted on came from the malformed payload rather than
from the id. It now sends the well-formed payload and fails until the server rejects an
empty subscription id.
