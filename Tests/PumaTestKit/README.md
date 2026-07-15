# AuthClientSdk / AuthServerSdk Test Kit

## What This Folder Contains

- `Run-AuthSdkTests.ps1`: starts a local Puma test backend if none is already
  running, then runs `pumatest.exe` (the native Qt Test suite under
  `Include/pumatest`) against it.

Unlike `Tests/TenantApiPostman` (Newman/Postman against the raw GraphQL API),
this suite exercises `AuthClientSdk`/`AuthServerSdk` themselves - the actual
C++ libraries applications link against - by driving them exactly the way
`Include/pumatest` is set up to be used.

## How The Tests Work

`pumatest.exe` is one Qt Test binary containing every class registered with
`I_ADD_TEST` in `Include/pumatest`. Running it with no arguments runs all of
them:

- **CAuthClientSdkTest**: superuser bootstrap, login/logout (incl. auto-logout
  on re-login, anonymous-call rejection), user/role/group CRUD, role and user
  permission accessors (`GetRolePermissions`, `GetUserAuthSystem`,
  `GetUserList`), token-based permission lookup.
- **CPersonalAccessTokenTest**: PAT create (incl. raw-token uniqueness and
  anonymous-call rejection), list (incl. product filtering and field
  round-trip), validate (incl. tampered/unknown tokens), revoke (incl. unknown
  id and idempotent double-revoke), expiry (`isActive` reflects expiry, not
  just revocation), ownership authorization (anonymous/self/cross-user/admin
  matrix - see `PatOwnershipAuthorizationTest`), and the
  `GetTokenPermissions()`/PAT interaction documented below.
- **CAuthServerLifecycleTest**: `AuthorizableServer` start/stop/restart and
  `SetFeaturesFilePath()` against a missing file.
- **CAuthorizationControllerTest**: self-contained, in-process GraphQL handler
  test (via `imtgqltest::CGqlSdlRequestTest`) - doesn't talk to any external
  server. Currently not registered with `I_ADD_TEST` (disabled), left as-is.

Of these, only **CAuthClientSdkTest** and **CPersonalAccessTokenTest** need a
real Puma backend reachable at `AuthServerSdk::SetPumaConnectionParam()`'s
target - hardcoded in those two files to host/port `17789`/`18789`, i.e.
`PumaServerSlTest.acc`'s ports (the isolated, disposable-DB test server
variant - **not** `PumaServerSl.exe`'s `7788`/`8788`, which is a developer's
persistent local Puma database and must never be touched by this suite). They
connect to it through their own embedded `AuthServerSdk` instance, which
delegates user/role/group/PAT/login handling to that backend via
`PumaClientEngine`. **CAuthServerLifecycleTest** does not need any backend:
`Start()`/`Stop()` only bind local sockets.

### PAT permission integration

`AuthClientSdk::GetTokenPermissions()` supports both session tokens and
personal access tokens. For a PAT, the server returns the intersection of the
user's permissions and the token's scopes and enforces the token's product
binding. `CPersonalAccessTokenTest::GetPermissionsWithPatTest` covers this
behavior. `ValidatePersonalAccessToken()`'s `productId` field is always empty because
the underlying `ValidateToken` GraphQL query does not return a token id to
look the product scope up with - see `Docs/AuthClientSdk.md`.

## Prerequisites

- Built `pumatest.exe` and `PumaServerSlTest.exe` (e.g.
  `Bin\Debug_Qt6_VC17_x64\`). Rebuild after pulling these changes - the new
  `CAuthServerLifecycleTest.cpp/.h` files are picked up automatically by both
  the QMake (`$$files(...)`) and CMake (`file(GLOB ...)`) build configs, but
  **CMake's `GLOB` is evaluated at configure time**, so re-run CMake
  configure (not just build) if you don't see them compiled in.
- PowerShell (Windows).
- No Postman/Newman/Node.js needed.

## Run The Suite

From this directory:

```powershell
powershell -ExecutionPolicy Bypass -File Run-AuthSdkTests.ps1
```

If a server is already listening on port 17789 (e.g. you started
`PumaServerSlTest.exe` yourself), the script reuses it and leaves it running
afterwards. Otherwise it starts `PumaServerSlTest.exe` itself and stops it
when done.

Pass extra `pumatest.exe`/QTest arguments through `-TestArgs`, e.g. verbose
output:

```powershell
powershell -ExecutionPolicy Bypass -File Run-AuthSdkTests.ps1 -TestArgs "-v2"
```

Point at a Release build:

```powershell
powershell -ExecutionPolicy Bypass -File Run-AuthSdkTests.ps1 -BuildConfig "Release_Qt6_VC17_x64"
```

Start from a clean database (deletes `puma_test.sqlite` first, only when this
script starts the server itself):

```powershell
powershell -ExecutionPolicy Bypass -File Run-AuthSdkTests.ps1 -ResetDatabase
```

## Notes On Test Data

`PumaServerSlTest.exe` uses its own isolated `puma_test.sqlite` (same
database `Tests/TenantApiPostman/Run-CiTests-Sqlite.ps1` resets for the
Postman suite) - never the developer's persistent `puma.sqlite`
(`PumaServerSl.exe`) or the production-style `PumaServerSettings.xml`. The
test classes are additionally written to be safe to run repeatedly against a
non-reset database: they create uniquely-named users/roles/groups/tokens and
remove them again in `cleanupTestCase()` (with defensive re-cleanup at the
start of `initTestCase()`/end of `cleanupTestCase()` for anything left over
from a previously interrupted run).

`AuthServerSdk`'s own embedded local storage (used for `UserSettingsRepository`
and `UserActionSqlRepository`, independent of the `PumaServerSlTest.exe`
connection above) lives in its own `AuthServerSdk.sqlite` file - see
`Impl/AuthServerSdk/AuthServerSdk.acc`'s `DatabaseEngine` element. It no
longer defaults to PostgreSQL.
