# Puma: LDAP group filter

The Puma server mirrors the LDAP (Active Directory) groups into its user groups
(`imtauth::CLdapUserGroupSynchronizerComp`, instance `LdapUserGroupSynchronizer` in
`Partitura/PumaVoce.arp/PumaServerBase.acc`). The synchronization runs only while
*LDAP enabled* and *Synchronize LDAP groups* are switched on in the Puma settings.

## Settings

| Page | Parameter | ID | Default |
|------|-----------|----|---------|
| LDAP | LDAP group filter (base DN) | `LdapGroupFilter` | empty |
| LDAP | LDAP group filter (name) | `LdapGroupNameFilter` | empty (all names) |

The base DN filter is the distinguished name of the directory subtree containing the groups,
for example `DC=atlascopco,DC=group` or `OU=Groups,DC=example,DC=com`.

* Empty: the domain of the server is searched (`defaultNamingContext` of the rootDSE), as before.
* A DN is converted to the ADSI path `LDAP://<DN>` (a `/` in the DN is escaped).
* A complete ADSI path (`LDAP://server/DC=...`) is used unchanged (the provider name is normalized to `LDAP://`).

The synchronizer reads the in-memory parameter in every synchronization cycle. Changes made
through the separate PumaServerConfigurator require a server restart to load the settings file.

Groups outside of the subtree are not read anymore. Their existing mirrors are removed in the
same way as for groups deleted in the directory. Parent groups outside of the subtree are
ignored. If the subtree cannot be read (e.g. invalid DN), an error is logged once and nothing is changed.

The name filter selects groups by their LDAP `name`, case-insensitively. `*` matches any
number of characters, so `MyGrou*` selects names beginning with `MyGrou`. All other
characters are literal. An empty filter matches all names. The in-memory setting is read on
each synchronization cycle; changes made through the separate configurator require a server
restart.

All groups below the base DN are read (`(objectCategory=group)`), the name filter is applied
locally. Only matching groups get a new mirror. Existing LDAP mirrors that no longer match (group
renamed or name filter changed) are retained and still updated (name, description, nesting and
members); a mirror is only removed when its group is deleted in the directory or is outside of the
base DN subtree.

## Members of the groups

With the `UserCollection` reference (in Puma the `UserRepository`) the synchronizer also mirrors the
members of the mirrored groups. The users are read in every cycle after the groups, with the same
account, from the whole domain of the base DN (the `DC=` components, e.g. `DC=example,DC=com` for
`OU=Groups,DC=example,DC=com`): only the enabled user accounts being direct members of the mirrored groups
(matching the name filter or with an existing mirror, `GetMirroredLdapGroups`)
(`(&(objectCategory=person)(objectClass=user)(|(memberOf=<group DN>)...))`, the DNs escaped as defined by
RFC 4515, at most 100 groups per query) with their direct `memberOf` groups. Users of the domain that are
not member of any of these groups are not transferred. All queries must be read completely, otherwise the
users are treated as not readable.

* **Existing user**: a user found by its SID or, for users without SID, by the login
  (user principal name, account name or `DOMAIN\account name`, case-insensitive) is only added to the
  mirrored groups. `DOMAIN` must be the NetBIOS name of the account's own domain (resolved from its SID);
  a login with another domain, or any `DOMAIN\` login if the domain cannot be resolved, is not matched.
  Its other data (name, roles, password, ...) is not changed; a user found by the login
  gets the SID of the directory account, so it stays linked to it. The user is read again right before it
  is written and only the memberships are changed, changes made in the meantime are kept.
* **Unknown user**: a new user is created and added to the groups. The ID is derived from the SID
  (`CLdapUserGroupSynchronizerComp::GetUserObjectId`), the login is the user principal name (account
  name if not set), name, description, mail and SID are taken from the directory and the user is marked
  as LDAP user (system `Ldap`). The user has no password and logs in through LDAP.
  If the login is already used by another account, the user is skipped and a warning is logged.
* **Removed members**: users with SID (also linked by the login) or created by the synchronization follow the directory: they are
  removed from the mirrored groups they are no longer member of (and from removed mirrors). The users
  themselves are never deleted. Other members assigned by an administrator (internal users) are kept.
* Both sides of the membership (group members and groups of the user) are written together with the groups (in one transaction if a
  transaction manager is configured).

If the users cannot be read, an error is logged once and the groups are synchronized without changing
their members. Limitations: the primary group of an account (usually *Domain Users*) is not part of
`memberOf`, and members from other domains (foreign security principals) are not mirrored.

## Preview in the PumaServerConfigurator

The LDAP page of the PumaServerConfigurator shows the section *Filtered LDAP groups* below the
parameters. *Show groups* reads the groups below the configured base DN and matching the name
filter (the current, not yet applied values) and shows them as tree:

* Root groups are the groups without a parent group inside of the filter.
* A group that is member of several groups is shown below each of them.
* Parents not matching the name filter are omitted.
* Groups of a level are sorted by name; the description is shown next to the name.
* Select text within a group entry with the mouse and press **Ctrl+C** to copy it.
  If the tree is taller than the preview, use the vertical scrollbar to browse the remaining groups.

After a change of the filter the tree is read again automatically. The groups are read with the same
code as used by the synchronizer (`CLdapUserGroupSynchronizerComp::ReadLdapGroups` and
`CreateGroupTree`), on the computer running the configurator and with the account of the
configurator user. Reading LDAP groups is supported on Windows only.

While the groups are read, a progress bar and the number of groups read so far are shown and the
button changes to *Cancel*. *Cancel* stops the reading: the tree is cleared and a note is shown that
the reading was cancelled. A running LDAP request can not be interrupted, the worker thread finishes
it in the background and its result is ignored; the reading itself stops at the next found group.

## Implementation

* ImtCore `Include/imtauth/CLdapUserGroupSynchronizerComp` – `BaseDnParam` reference,
  `GetSearchPath`, `GetSearchFilter`, `CreateGroupTree`; `ReadLdapGroups` takes an optional progress function that gets
  the number of groups read and stops the reading by returning `false`. Members: `UserCollection`
  reference, `GetMirroredLdapGroups`, `ReadLdapUsers`, `GetUserSearchFilters`, `GetDomainSearchPath`, `GetUserObjectId`.
* ImtCore `Include/imtauthgui/CLdapGroupTreeController` – QML type
  `com.imtcore.imtauth/LdapGroupTreeController`, reads the groups in a worker thread, provides
  `readGroupCount`, `cancelled` and `cancel()` (registered in `Impl/PumaServerConfigurator/Main.cpp`).
* ImtCore `Qml/imtgui/Settings/SettingsController.qml` – `registerPageExtension(pageId, component)`
  adds content below the parameters of a settings page; the item gets the JSON of the page in `pageData`.
* `Include/pumaqml/Qml/PumaServerConfigurator.qml`, `LdapGroupTreeView.qml` – the preview.
