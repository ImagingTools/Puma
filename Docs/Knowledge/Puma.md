# Puma: overview for the assistant

This document is part of the knowledge base of the AI assistant of Puma. It describes what Puma is,
how its data are organized and how typical tasks are done. The shared concepts of the user
administration (organizations, users, groups, roles, permissions, scopes) are in `Administration.md`
of ImtCore; the detailed rules are in `Permissions-Rules.en.md`.

## What Puma is

Puma is the authentication and authorization server (account service) of the ImagingTools products.
It keeps the users, roles, groups and organizations. Product servers (for example ProLife, Agentino,
Dogma, Claire) sign their users in through Puma and get their roles and permissions from it.

The console of Puma is a web application with these areas:

- **Administration:** users, roles and groups of the current organization; the page **Assistant** configures
  the AI assistant of the organization.
- **Organizations:** organizations, their members, invitations, connections to other organizations,
  their permission scope.
- **Connected products:** the product servers that use Puma; each has its own permission tree.

## Data and boundaries

- Everything is seen in the **current organization** of the session. Users, roles and groups of other
  organizations are invisible and cannot be referenced.
- Roles belong to a **product**: a role of ProLife gives nothing in Dogma. The permission tree of each
  product comes from the product.
- **Members of an organization** are users with a membership; they are added by invitation (the invited
  user accepts it) or by an organization manager.
- **LDAP:** users and groups can be synchronized from a directory (see `LdapGroupFilter.md`). Synchronized
  groups are read-only in Puma.
- **Server tokens** let product servers call Puma; **personal access tokens** let users call the API with a
  limited scope. The assistant never reads or creates tokens.

## Typical tasks

| Task | How |
|---|---|
| Who is in a group? | `GroupItem` of the group: `users` (direct members and inherited ones) |
| Which groups and roles does a user have? | `UserItem` of the user: `groups`, `roles`, `permissions` |
| Which users have no group? | `UsersList` with `groups`, filter the users with an empty value |
| Add users to a group / remove them | `GroupItem`, then `GroupUpdate` with all direct members changed (one step) |
| Give a group a role | `GroupItem`, then `GroupUpdate` with the role added to the direct roles |
| Create a group with members and roles | `GroupAdd` with name, members and roles |
| Find who can do something | `RoleItem` of the roles: `permissions`; then the groups and users with these roles |

The assistant reads with queries and proposes changes as a plan that the user confirms. It acts with
the rights of the user: what the user cannot do in the console, the assistant cannot do either.

## Notes

- Group names and role IDs are unique inside an organization.
- A user added to a group gets its roles at once; removing the user from the group removes them.
- Changes of users (name, e-mail, enabling, password) are done in the console by the administrator, not by
  the assistant.
