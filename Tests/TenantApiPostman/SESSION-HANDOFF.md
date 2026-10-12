# Session handoff — Postman test-suite audit & hardening

Written at the end of a session that (a) fixed failing Postman suites in Puma and
ProLife, (b) audited the Puma `TenantApiPostman` suite for assertion quality, and
(c) uncovered several product defects in ImtCore while doing so.

**Current state: the Puma Postgres suite is intentionally RED — 1668 assertions,
9 failures.** Every one of those failures is real and was previously hidden by
self-disabling assertions. Do not "fix" them by restoring tolerance.

---

## 1. Suites and where they stand

| Suite | Command | State |
| --- | --- | --- |
| `Puma/Tests/TenantApiPostman` (Postgres) | `Run-CiTests.ps1 -RepoRoot D:\ImagingTools\Git\Puma -BuildConfig Debug_Qt6_VC17_x64` | **9 failures** (all genuine, see §4) |
| `Puma/Tests/TenantApiPostman` (SQLite) | `Run-CiTests-Sqlite.ps1` (same args) | **Not re-run** after the hardening — expect the same 9 |
| `ProLife/Tests/DeskTicketApiPostman` | `Run-CiTests.ps1 -RepoRoot D:\ImagingTools\Git\ProLife -BuildConfig Debug_Qt6_VC17_x64` | green, 38 req / 77 assertions |
| `ProLife/Tests/PatTokenApiPostman` | same | green, 113 req / 658 assertions |
| `ProLife/Tests/ProLifeApiPostman` | same | green, 102 req / 499 assertions |

Both Puma scripts need `-RepoRoot` passed explicitly (their auto-detection fails
from an agent shell).

---

## 2. Problems that were FIXED

### 2.1 Async document-load race (Puma + ProLife) — fixed

`OpenDocument` returns `isLoading: true` and finishes loading on a background
thread (`imtdoc::CDocumentServiceBase::DoOpenDocument`), so a mutation fired
immediately after it races the load and gets `InvalidDocumentId`.

Measured: the load takes **~10 ms on SQLite but ~400 ms on Postgres**. That is
exactly why these suites passed on SQLite and failed on Postgres.

Three "wait" requests already existed in the Puma collection but were no-ops —
one round-trip (~9 ms) plus `pm.expect(true).to.eql(true)`. They were rewritten
as real bounded polling loops (`postman.setNextRequest` onto themselves, max 25
attempts, 150 ms busy-wait spacing in a pre-request script, and an assertion that
the representation is actually readable).

Same one-line-per-suite fix applied in `ProLife/Tests/DeskTicketApiPostman`
(`Wait for ticket document load`).

### 2.2 `SaveDocument` save-as created duplicate users — fixed in the tests

`imtdoc::TCollectionDocumentServiceWrap::DoSaveDocument` treats *"the passed
`documentName` differs from the document's current name"* as **Save-As** and calls
`InsertNewObject`, creating a **new logical object**, while still returning
`status: Success`.

The `User` type has a name provider that rewrites the stored name after the first
save ("Member A" → "A Member"), so every later `SaveDocument` that kept sending
the hardcoded `documentName: "Member A"` no longer matched and silently forked a
new user with the same login. That is what eventually made
`CUserDocumentValidatorComp` report `Username already exists`.

Verified in isolation on a clean user: saving with a mismatched `documentName`
took `1 distinct DocumentId → 2` while returning `Success`.

Fix in the collection: all four `SaveDocument` calls on `Users` now pass
`documentName: ""` (an empty name skips the save-as branch entirely and performs
a plain update; the name provider sets the name anyway).

> **Caution for whoever reads the DB:** the `Users` table is *revision-based*.
> Many rows share one `DocumentId` (N `InActive` + exactly 1 `Active`) — that is
> normal history, **not** duplication. The real duplicate signal is
> `count(DISTINCT "DocumentId")` per login. I initially misread revisions as
> duplicates; don't repeat that.
>
> ```sql
> SELECT "Document"->>'Id' AS username, count(*) AS revisions,
>        count(DISTINCT "DocumentId") AS logical_users
> FROM "Users" GROUP BY 1 HAVING count(DISTINCT "DocumentId") > 1;
> ```

### 2.3 Expired access token late in the run — fixed

Folder `05B` runs at ~95 % of a 393-request run and reused an `a_owner_token`
last refreshed thousands of requests earlier. On a slower CI agent the JWT
expired; the server answered **401 with a completely empty body** (not JSON), so
the global "valid JSON"/"envelope" assertions failed too, and because the token is
only refreshed from the first request of that folder — which itself 401'd — the
failure cascaded through all 14 requests (42 assertions).

Fix: a `Refresh auth A owner (user repr isolation setup)` request was added at the
head of folder `05B`, matching the existing `Refresh auth A owner (No Organization)`
pattern.

### 2.4 Dead `GetUserMode` tests (ProLife) — removed

`GetUserMode` / `AuthorizationOptions` is only wired in
`ProLife/Partitura/ProLifeVoce.arp/RootDataController.acc`, which nothing
includes, so `ProLifeServerTest` answers `400 "Invalid command request"`. Removed
from `ProLifeApi.postman_collection.json`, from the legacy
`ProLife/Tests/backend/postman_collection.json`, and from the ProLifeApi README.

### 2.5 Broken fixtures that made assertions vacuous — fixed

Exposed only after the assertions were tightened:

| Broken fixture | Consequence |
| --- | --- |
| `permReadX`/`permWriteX` were invented ids (`fixture.permission.read`/`.write`) | `CRoleDocumentValidatorComp` rejects permissions not enabled for the tenant → the **VIEW/EDITOR role documents never saved**, `RoleItem` pointed at a phantom, delegation role resolution returned an empty list, and `rbac_*_expected` stayed `'false'` so the permission assertions skipped themselves |
| `relationshipId` captured from `CreateNewDocument.objectId`, which is `""` at create time | cleanup `RemoveTenantRelationship` could never work |
| cross-tenant leak check queried `urtiMemberDocId` — a document opened in **A owner's** workspace | document workspaces are keyed per user server-side, so B owner always got `Document not found`, and the assertion treated any error as a pass → **the check never verified anything** |
| the collection never sent a `productid` header | see §3.1 — tenant role filtering was silently off |

Added requests: `Resolve real tenant-A permissions for role fixtures` (head of
folder 04, uses `GetTenantPermissions`), `Resolve real relationship objectId after
save` (folder 03), and `Isolation probe: Open a_member user document as B owner` +
its polling wait (folder 05B).

### 2.6 Assertion-quality cleanup — done

- **34 tautological assertions** (`pm.expect(true).to.eql(true)`) removed — each
  reviewed individually and either made strict or turned into a real
  `pm.test.skip` (newman 6.2.2 supports it and reports skips separately).
- **3 further disguised tautologies** removed: `pm.expect(typeof x).to.eql("boolean")`
  is always true. One of them hid that `Logout` returns `ok:false`.
- The assertion that masked §2.2 was tightened — it used to **pass** when the
  duplicate error appeared:
  ```js
  if (hasAlreadyExists) { pm.expect(status === "Success").to.eql(false); return; }
  ```

**Scale note for context:** newman reported 1655 assertions, but ~1179 of those
(71 %) were the same 3 boilerplate global checks repeated per request. Real
domain assertions were 473.

---

## 3. Product defects FOUND (ImtCore) — none currently fixed

### 3.1 Tenant role filtering is fail-open without `productid` — **verified, enforcement reverted**

`imtauthgql::AdaptUserForTenant` (`ImtCore/Include/imtauthgql/imtauthgql.h`, ~line 995)
filters roles and permissions **only when `productId` is non-empty**, while groups
are filtered unconditionally:

```cpp
QByteArrayList filteredRoles;
if (!productId.isEmpty()){ /* ...filter roles... */ }

QByteArrayList filteredGroups;          // no productId guard
```

and the write-back has the same guard:

```cpp
if (anyFilterChange){
    if (!productId.isEmpty()){
        adaptedUserInfoPtr->SetRoles(productId, filteredRoles);
        adaptedUserInfoPtr->SetLocalPermissions(productId, filteredPerms);
    }
    /* groups always reset to the filtered list */
}
```

`productId` comes from the `productid` HTTP header. Observed live, same user,
same object, B owner's JWT decoded to confirm the tenant-B context:

| | under tenant A | under tenant B |
| --- | --- | --- |
| `groups` | `[7dcd2fb9…]` | `[]` — correctly filtered |
| `roles` | `[9a3bf13d…]` | `[9a3bf13d…]` — **leaked** |

Adding the `productid` header made the leak disappear, confirming the diagnosis.

**Status:** a fail-closed enforcement (new `CCS_BAD_REQUEST` → HTTP 400 in
`CAuthenticationManagerComp` / `CHttpGraphQLServletComp` / `CWebSocketServletComp`)
was implemented, compiled clean, but **the user reverted it — it affected other
aspects.** All four files are back to their original state. The leak is therefore
still present in the product.

**Proposed narrower fix (not yet applied, needs the user's go-ahead):** instead of
rejecting the request, when `productId` is empty iterate every product the user
has and filter each product's roles/permissions:

```cpp
const QByteArrayList productsToFilter = productId.isEmpty()
        ? userInfoPtr->GetProducts()
        : QByteArrayList{ productId };
```

This emits no new client-visible errors and changes behaviour *only* in the case
that is currently insecure; calls that already pass a `productId` behave
identically. The delegated-role enrichment block should keep its `productId`
guard — it genuinely needs a product context
(`roleInfoPtr->GetProductId() != productId`).

### 3.2 Delegated cross-org grants confer no permissions — **open, not root-caused**

Confirmed intended semantics (from the user): *a grant from tenant A to tenant B
with role R gives B's users role R over all of A's data, and R may carry various
permissions.*

Observed: after a grant, `GetPermissions` returns `[]`. 8 of the 9 current
failures are this. Adding the `productid` header did **not** fix it (unlike §3.1).

Lead for the next session: `GetPermissions` is served by
`imtauthgql::CAuthorizationControllerComp::OnGetPermissions`
(`ImtCore/Include/imtauthgql/CAuthorizationControllerComp.cpp`, ~line 381), which is a
*different* code path from `AdaptUserForTenant`. Check whether it consults
`imtauth::IDelegatedAccess` at all. Note `AdaptUserForTenant`'s delegated-role
enrichment is itself gated on a non-empty `productId` and on the user **not**
having a direct membership in the current tenant.

### 3.3 `SaveDocument` silent save-as — **open (design question)**

See §2.2. The product returns `Success` while forking a new entity. Combined with
a name provider that rewrites the stored name, any client that caches a document
name and re-sends it will duplicate the entity on the second save. Worth a
decision: reject the mismatch, or make save-as explicit in the API.

### 3.4 HTTP 401/403 responses have an empty body — **open, deliberately untouched**

`CHttpGraphQLServletComp::OnPost` returns `CreateResponse(SC_UNAUTHORIZED, QByteArray(), request)`
— the reason is logged server-side but never sent to the client, contradicting
`ImtCore/Docs/JWT_AUTH_DIAGNOSTICS.md`, which documents bodies like
`"Unauthorized: JWT token expired."`. The WebSocket path
(`CWebSocketServletComp`) does it correctly. This is why the expired-token
failures in §2.3 showed up as `JSONError: No data, empty input at 1:1`.
Not fixed — never approved.

### 3.5 `Logout` ignores its own input argument — **open, minor**

`CAuthorizationControllerComp::OnLogout` takes `input: { accessToken }` but
resolves the session from the request **context/header** instead, so the argument
is dead. The coverage request sends no auth header at all, so it can never log
anyone out; the test now asserts that honestly and explains why.

### 3.6 `GetRelationshipRepresentation` returns `id: ""` — **open**

The 9th current failure. The relationship object is saved without its own `id`
field ever being set (the representation update passes `id: "{{relationshipId}}"`
while that variable is still empty at that point in the flow).

---

## 4. The 9 current failures, precisely

```
Advanced grant depth: A member permissions include union          :: A member has read permission from view grant
Advanced grant depth: A member permissions include union          :: A member write permission follows editor grant availability
Advanced grant depth: A member permissions reduced after editor revoke :: A member read permission persists after editor revoke
Advanced grant depth: B owner permissions include union           :: B owner read permission follows grant visibility model
Advanced grant depth: B owner permissions include union           :: B owner write permission follows editor grant availability
Advanced grant depth: B owner permissions reduced after editor revoke  :: Read permission persists after editor revoke
Delegation: B owner GetPermissions includes role R permissions     :: B owner delegated permissions check is strict and conditional
Delegation: Outsider GetPermissions includes role R permissions    :: Outsider delegated permissions check is strict and conditional
GetRelationshipRepresentation coverage                             :: GetRelationshipRepresentation returns requested relationship
```

First 8 → §3.2. Last one → §3.6.

---

## 5. Suggested next steps

1. **Decide on §3.1** — apply the narrow `productsToFilter` fix, or accept the
   fail-open. This is a cross-tenant data exposure; recommend fixing.
2. **Root-cause §3.2** (8 of 9 failures). Start at `OnGetPermissions`.
3. **Re-run the SQLite suite** — it has not been run since the hardening.
4. **Still not done from the earlier audit:** split the 393-request chain into
   independent sections with their own fixtures. One failure currently cascades
   (seen twice this session). This is a real restructure, not a quick win —
   tenant/role state genuinely flows between folders.
5. Consider adding state-invariant read-backs after mutations generally — the
   suite asserts *responses*, almost never *stored state*, which is precisely why
   §2.2 lived undetected.

---

## 6. Repository state at handoff

| Repo | Path | State |
| --- | --- | --- |
| Puma | `Tests/TenantApiPostman/Tenant_System_Full.postman_collection.json` | **modified, uncommitted** — all of §2.1, §2.2, §2.3, §2.5, §2.6 |
| Puma | `Tests/TenantApiPostman/SESSION-HANDOFF.md` | new, untracked (this file) |
| ProLife | `Tests/DeskTicketApiPostman/DeskTicketApi.postman_collection.json` | **modified, uncommitted** — the §2.1 wait request |
| ProLife | `Tests/ProLifeApiPostman/*`, `Tests/backend/postman_collection.json` | already **committed** — the §2.4 `GetUserMode` removal is in HEAD |
| ImtCore | `Include/imtgql/…`, `Include/imtservergql/…` | **clean** — the §3.1 enforcement was reverted by the user |

> `ProLife/Tests/PatTokenApiPostman/PatTokenApi.postman_collection.json` also shows
> as modified, but **that change is not from this session** (it adds PAT
> scope-escalation tests). Leave it alone.
>
> `ProLife/Tests/ProLifeGui/**` untracked screenshot/report dirs are likewise
> unrelated build output.

Nothing was committed by this session; the Puma and DeskTicket collection edits
are still in the working tree for review.

---

## 7. Environment gotchas (cost real time this session)

- **Rebuilding Puma needs the MSVC environment.** Without it the build fails with
  `fatal error C1083: type_traits / limits` in untouched files — that is a missing
  `INCLUDE`, not a code error:
  ```
  cmd /c '"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul && cmake --build "D:\ImagingTools\Git\Puma\Build\CMake\build\Qt_6_8_0_msvc2022_64-Debug" --target PumaServerPgTest'
  ```
- **ImtCore is NOT built by Puma's tree.** Puma links prebuilt static libs from
  `ImtCore/Bin/Debug_Qt6_VC17_x64/*.lib`, and `ImtCore/Build/CMake/build/` only has
  a **Release** tree configured. Editing ImtCore C++ and rebuilding a Puma target
  produces a binary **without** the change — verified the hard way. For a quick
  sanity check of ImtCore edits, a syntax-only compile works:
  `cl /Zs` with the Acf/AcfSln/IAcf/ImtCore include set + Qt `-external:I` paths,
  `-std:c++17 -permissive- -Zc:__cplusplus`.
- Puma su credentials for manual probing live in
  `Tenant_System_Full.postman_environment.json` (`su` / `Su_ImtCore_2026!`).
- `psql` is at `C:\Program Files\PostgreSQL\18\bin\psql.exe`, password `root`,
  DB `puma_test`. The CI script drops and recreates it at the start of every run.
