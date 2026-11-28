# AI assistant in Puma

Puma has the AI assistant of ImtCore (issue ImagingTools/ImtCore#1206). The general documentation is
`Docs/Assistant/README.md` in ImtCore; this page describes the integration into Puma.

## For the administrators of an organization

The assistant is configured per organization on the page **Administration → Assistant**:

1. Enable the assistant, choose the model provider and the model, and enter the API key of the provider.
2. Save. The members of the organization see the button of the assistant in the top panel.

The API key is stored protected and never shown again; leave the field empty to keep it, or remove it.
The assistant can do only what the user may do: it uses the same commands as the console, with the
rights of the user. Changes (for example adding a user to a group) are shown first and applied only
after the user confirms them.

The assistant knows Puma from its documentation and its public API. It can solve tasks of several steps,
e.g. "find the users without a group, add them to Support and give the group the role Operator": it reads
the data, then proposes a plan. The plan shows every change with the data it sends; the user confirms or
rejects the whole plan. The steps run in order and stop at the first failure; applied steps are not undone
automatically.

What the assistant may use in Puma: reading users, groups, roles, the organization, its members and its
permission scope; creating and changing groups (members, roles, parent groups, name). It never handles
passwords, tokens or sessions.

**Undoing a change:** every confirmed change is recorded as a change set. Ask the assistant to undo it
("undo the last change", "roll back the change to Support"); it shows what will be restored and applies the
rollback after confirmation. Changed groups get their former state, groups created by the change set are
removed; the memberships in the users follow. A change set is not rolled back if a group was changed again
afterwards, or if it removed documents or changed data without revisions. Every user can roll back the own
change sets; `RollbackAny` allows it for the change sets of others. Change sets are kept 90 days.

Permissions (below `Administration/Assistant`): `ViewPage` opens the page, `View` reads the configuration
(without the key), `Edit` changes it, `RollbackAny` rolls back the changes of other users. New organizations have them in their permission scope; for an
existing organization the system administrator adds them to its scope.

## For the operator of the server

| Where | What |
|---|---|
| `Partitura/PumaVoce.arp/PumaServerBase.acc` | Element `Assistant` (`ImtAssistVoce/TenantAssistant`): the GraphQL handlers `Assistant/AssistantController` and `Assistant/ConfigurationController` are in the handlers of the servlet; the actions use `PumaHandlers` with the context of the user. Element `AssistantSecretProtector` protects the API keys. |
| `PumaDatabasePostgres.acc`, `PumaDatabaseSQLite.acc` | Table `AssistantConfigurations` (`ImtAssistVoce/AssistantConfigurationSqlRepository`), created automatically |
| `PumaDatabasePostgres.acc`, `PumaDatabaseSQLite.acc` | Element `ChangeSetJournal` (`ImtDatabasePck/ChangeSetJournal`): tables `ChangeSets` and `ChangeSetEntries`, created with the first change set |
| `Assistant` in `PumaServerBase.acc`: `ChangeSetJournal`, `RollbackCollections`, `RollbackDelegates` | Exported as `AssistantChangeSetJournal`, `AssistantRollbackCollections`, `AssistantRollbackDelegates`; each server sets the journal of its database and the user group repository. The permissions of the rollback are `/Administration/Groups/Edit/Change` and `/Administration/Groups/Edit/Remove` |
| `Include/puma/PumaFeatures.h` | Permissions `/Administration/Assistant/{ViewPage,View,Edit,RollbackAny}` |
| `Include/pumaqml/Qml/PumaMain.qml` | `AssistantCenter.controller` |
| `Docs/Knowledge/Puma.md` and the qrc of the servers (`:/AssistantKnowledge/Puma`) | Knowledge of Puma: this overview, `README.md`, `LdapGroupFilter.md`, this page and the behaviour of the tenant API (`Tests/TenantApiPostman/README.md`); with the ImtCore knowledge (`:/AssistantKnowledge/ImtCore`) |
| `Assistant` in `PumaServerBase.acc`: `AllowedQueries`, `AllowedMutations` | What the assistant may call. Extend the lists to give it more functions; commands of authentication and secret fields stay closed |

Policy of the server, attributes of `Assistant` in `PumaServerBase.acc`:

- `AllowedPresets`: the providers the organizations may choose. Default: all providers with a fixed
  public endpoint (OpenAI, Azure OpenAI, Mistral, Cohere, IONOS, STACKIT, Aleph Alpha, Anthropic, Gemini,
  Vertex AI, Bedrock).
- `AllowCustomEndpoints` (default off): allows an organization to enter its own endpoint, for example a
  self-hosted model server. The server then sends requests to an address chosen by the organization;
  enable it only if the network of the server is restricted.
- `AllowServerCredentials` (default off): organizations without own key use the key of the server from
  the environment variable `IMT_ASSISTANT_API_KEY`.
- `Enabled` (default on): switches the assistant off for all organizations.

**Protection of the API keys:** `AssistantSecretProtector` is `ImtCryptPck/SystemSecretProtector`. On
Windows it encrypts with the data protection API of the server account. On Linux it stores the key as is,
so the database must be protected like the other secrets of the server. A key-based protector is an open
decision (see the pull request).

The server log masks `apiKey` values like passwords.

## Keeping the knowledge current

The production servers have no access to GitHub or the internet: the assistant knows only what is compiled
into the server. The knowledge therefore lives in the repository and is kept current by
`.github/workflows/assistant-knowledge.yml`:

1. After every merged pull request, Claude reads the change (instructions:
   `.github/assistant-knowledge/prompt.md`). If the assistant must know about it, Claude updates
   `Docs/Knowledge` and opens a pull request **Assistant knowledge: update for #N** with a summary and
   suggestions (e.g. commands for `AllowedQueries`). If nothing matters, no pull request is opened.
2. A person reviews and merges it; the next build of the servers compiles the new texts in.
3. On every pull request the workflow checks that all knowledge files are in the resources of the servers
   (`.github/assistant-knowledge/update_qrc.py --check`). After adding a file to `Docs/Knowledge`, run
   `python3 .github/assistant-knowledge/update_qrc.py`.

Set up once: the secret `ANTHROPIC_API_KEY` and, in the settings of the repository, "Allow GitHub Actions
to create and approve pull requests". A run can also be started by hand for a merged pull request
(Actions → Assistant knowledge → Run workflow).
