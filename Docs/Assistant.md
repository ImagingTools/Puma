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

Available functions: search users, groups and roles of the organization, add a user to a group.

Permissions (below `Administration/Assistant`): `ViewPage` opens the page, `View` reads the configuration
(without the key), `Edit` changes it. New organizations have them in their permission scope; for an
existing organization the system administrator adds them to its scope.

## For the operator of the server

| Where | What |
|---|---|
| `Partitura/PumaVoce.arp/PumaServerBase.acc` | Element `Assistant` (`ImtAssistVoce/TenantAssistant`): the GraphQL handlers `Assistant/AssistantController` and `Assistant/ConfigurationController` are in the handlers of the servlet; the actions use `PumaHandlers` with the context of the user. Element `AssistantSecretProtector` protects the API keys. |
| `PumaDatabasePostgres.acc`, `PumaDatabaseSQLite.acc` | Table `AssistantConfigurations` (`ImtAssistVoce/AssistantConfigurationSqlRepository`), created automatically |
| `Include/puma/PumaFeatures.h` | Permissions `/Administration/Assistant/{ViewPage,View,Edit}` |
| `Include/pumaqml/Qml/PumaMain.qml` | `AssistantCenter.controller` |

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
