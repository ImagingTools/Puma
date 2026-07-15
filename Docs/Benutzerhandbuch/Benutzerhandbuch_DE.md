# Puma-Benutzerhandbuch

**Sprache:** Deutsch  
**Zielgruppe:** Administratoren, Anwendungsentwickler und Betreiber

## 1. Zweck und Geltungsbereich

Puma ist das zentrale System für Authentifizierung und Berechtigungsverwaltung
mehrerer Anwendungen. Benutzer melden sich gegenüber Puma an; Anwendungen
entscheiden anhand der von Puma gelieferten Berechtigungen, welche Funktionen
freigeschaltet werden. Dadurch müssen Benutzer, Rollen und Gruppen nicht in
jeder Anwendung separat gepflegt werden.

Dieses Handbuch beschreibt:

- den Puma-Server mit SQLite- oder PostgreSQL-Datenbank,
- lokale Benutzer, Rollen, Gruppen und produktbezogene Berechtigungen,
- die Einbindung eigener Qt/C++-Anwendungen über `AuthClientSdk`,
- die Einbindung eigener Server über `AuthServerSdk`,
- Windows-Domänenanmeldung über die in ImtCore implementierte LDAP-Schicht,
- Personal Access Tokens (PATs) für nicht interaktive Zugriffe,
- typische Betriebs-, Sicherheits- und Fehlerfälle.

Die Beschreibung basiert auf Puma und der zugrunde liegenden
Authentifizierungsimplementierung in ImagingTools/ImtCore. Sie beschreibt den
Stand der im Quellcode vorhandenen Funktionen; konkrete Menünamen können je
nach eingebetteter Administrationsoberfläche abweichen.

## 2. Puma auf einen Blick

```mermaid
flowchart LR
    U[Benutzer] --> A[Anwendung]
    S[Automatisierung] --> A
    A -->|Login, PAT, Abfragen| SDK[AuthClientSdk]
    SDK -->|HTTPS / WSS| PS[Puma-Server]
    PS --> AUTH[Authentifizierung]
    PS --> RBAC[Rollen und Berechtigungen]
    PS --> DB[(SQLite oder PostgreSQL)]
    AUTH --> LOCAL[Lokale Konten]
    AUTH --> AD[Windows-Domäne]
```

*Abbildung 1: Überblick über die Puma-Systemarchitektur.*

Puma trennt vier Aufgaben:

1. **Identität:** Wer greift zu?
2. **Authentifizierung:** Ist der vorgelegte Nachweis gültig?
3. **Autorisierung:** Welche produktbezogenen Aktionen sind erlaubt?
4. **Persistenz:** Wo werden Benutzer, Rollen, Gruppen, Sitzungen und PATs
   gespeichert?

### 2.1 Varianten des Puma-Servers

| Variante | Datenbank | Typischer Einsatz |
|---|---|---|
| `PumaServerSl` | SQLite | Einzelinstallation, Entwicklung, kleine lokale Installation |
| `PumaServerPg` | PostgreSQL | Zentraler Mehrbenutzerbetrieb und produktive Serverinstallation |

Beide Varianten verwenden dieselbe Puma-Serverbasis. Die jeweilige
Serveranwendung bindet die passenden Repositories und SQL-Skripte für ihre
Datenbank ein.

```mermaid
flowchart TB
    BASE[PumaServerBase] --> HTTP[HTTP-Server]
    BASE --> WS[WebSocket-Server]
    BASE --> USERS[Benutzer / Rollen / Gruppen]
    BASE --> SESS[Sitzungen]
    BASE --> PAT[PAT-Verwaltung]
    BASE --> TEN[Tenant-Dienste]
    SL[PumaServerSl] --> BASE
    PG[PumaServerPg] --> BASE
    SL --> SQL[(SQLite)]
    PG --> POST[(PostgreSQL)]
```

*Abbildung 2: Gemeinsame Serverbasis und datenbankspezifische Varianten.*

### 2.2 Zusammenspiel von Puma und Anwendung

Die Anwendung bindet die Puma-Administration als Seite in ihre Client-UI ein.
Auf dieser Administrationsseite richten berechtigte Administratoren Benutzer,
Rollen, Gruppen und Berechtigungszuordnungen ein. Die Seite greift über das SDK
auf den Puma-Server zu; die Daten werden nicht separat in der Anwendung
verwaltet. Nach der Anmeldung liefert Puma die effektiven Berechtigungen an die
Anwendung zurück. Die Anwendung verwendet sie, um Funktionen in ihrer
fachlichen UI freizugeben.

```mermaid
flowchart LR
    ADM[Administrator] --> PAGE[Administrationsseite in der Client-UI]
    PAGE -->|Benutzer, Rollen, Gruppen und Zuordnungen| SDK[AuthClientSdk]
    SDK --> P[Puma-Server]
    P --> DB[(SQLite oder PostgreSQL)]
    USER[Benutzer] --> UI[Fachliche Client-UI]
    UI -->|Anmeldung| SDK
    P -->|effektive Berechtigungen| SDK
    SDK -->|HasPermission| UI
```

*Abbildung 3: Zentrale Administration und Nutzung der Berechtigungen in der
Anwendung.*

Das Einbinden und berechtigungsabhängige Anzeigen der Administrationsseite ist
Aufgabe der Anwendung. Puma erzwingt weiterhin die Berechtigungen für
Verwaltungsoperationen; das Ausblenden der Seite allein ist keine
Zugriffskontrolle.

## 3. Rollen- und Berechtigungsmodell

Puma verwaltet Berechtigungen nicht als frei editierbare Eigenschaften eines
Benutzers, sondern über Rollen:

```mermaid
flowchart LR
    U[Benutzer] -->|direkt zugeordnet| R[Rolle]
    U -->|Mitglied| G[Gruppe]
    G -->|zugeordnet| R
    R -->|enthält| P[Berechtigungen]
    P -->|gelten für| PROD[Produkt-ID]
```

*Abbildung 4: Beziehungen zwischen Benutzern, Gruppen, Rollen und
Berechtigungen.*

- **Benutzer** besitzen eine interne, stabile Objekt-ID und einen Login-Namen.
- **Rollen** bündeln Berechtigungen und sind produktspezifisch.
- **Gruppen** bündeln Benutzer und erhalten Rollen.
- **Berechtigungen** sind von der Anwendung definierte, groß- und
  kleinschreibungsabhängige IDs.
- **Produkt-ID** begrenzt den Kontext, in dem Rollen und Berechtigungen gelten.

Ein Benutzer erhält die Vereinigungsmenge aus:

- Berechtigungen direkt zugeordneter Rollen und
- Berechtigungen der Rollen seiner Gruppen.

Bei mehreren direkten Rollen, mehreren Gruppen oder mehreren Rollen pro Gruppe
werden alle enthaltenen Berechtigungen vereinigt. Doppelte Berechtigungen
wirken dabei nur einmal; eine Zuordnung zieht keine Berechtigung aus einer
anderen Zuordnung ab.

> **Wichtig:** Verwaltungsoperationen verwenden die interne Benutzer-ID, nicht
> den Login-Namen. Eine Anwendung setzt ihre Produkt-ID vor der Anmeldung.

### 3.1 Verbindliche und optionale Elemente

| Element | Verbindlich oder optional? |
|---|---|
| Produkt-ID | **Verbindlich:** Die Anwendung setzt sie vor der Anmeldung; Rollen und Berechtigungen gelten in diesem Produktkontext. |
| Berechtigungs-ID | **Verbindlich für geschützte Funktionen:** Die Anwendung definiert und prüft sie. |
| Rolle | **Verbindlich für die Vergabe einer Berechtigung:** Berechtigungen werden über Rollen und nicht direkt an Benutzer vergeben. Ein Benutzer darf ohne Rolle bestehen, hat dann aber keine rollenbasierte Berechtigung. |
| Direkte Rollenzuordnung | **Optional:** Geeignet für individuelle Aufgaben. |
| Gruppe | **Optional:** Geeignet, um wiederkehrende Teamzuordnungen zu bündeln. |
| Gruppenrolle | **Optional:** Alternative oder Ergänzung zur direkten Rollenzuordnung. |
| Mehrere Rollen oder Gruppen | **Optional:** Ihre Berechtigungen bilden gemeinsam die Vereinigungsmenge. |

### 3.2 Empfohlenes Administrationsmodell

```mermaid
flowchart TD
    SU[Initialer Superuser] --> AR[Administratorrolle anlegen]
    AR --> AP[Verwaltungsberechtigungen zuordnen]
    SU --> AU[Personalisierte Administratorkonten anlegen]
    AU --> AR
    AR --> DAILY[Tägliche Administration]
    SU --> EMG[Nur für Bootstrap / Notfall]
```

*Abbildung 5: Empfohlenes Modell für Bootstrap und tägliche Administration.*

Der Superuser dient zum initialen Aufbau. Für den Alltag sollten
personalisierte Administratorkonten mit einer passenden Administratorrolle
verwendet werden. So müssen Superuser-Zugangsdaten nicht geteilt werden.

## 4. Inbetriebnahme des Puma-Servers

### 4.1 Vorbereitung

1. Servervariante auswählen.
2. Bei `PumaServerPg` PostgreSQL-Datenbank, Datenbankbenutzer und
   Erreichbarkeit vorbereiten.
3. Bei `PumaServerSl` ist kein separater Datenbankserver und keine
   Datenbankvorbereitung erforderlich. Der Server legt die SQLite-Datenbank an;
   das Dienstkonto benötigt Schreibrechte am vorgesehenen Speicherort.
4. HTTP- und WebSocket-Port festlegen.
5. Für produktive Systeme ein Serverzertifikat und einen privaten Schlüssel
   bereitstellen.
6. Firewall nur für die benötigten Ports öffnen.
7. Schreibrechte für Einstellungen, Datenbank und Protokolle prüfen.

Die persistenten Puma-Einstellungen werden standardmäßig unter dem
anwendungsbezogenen Systempfad in
`Puma/Puma Server/PumaServerSettings.xml` abgelegt.

### 4.2 Startablauf

```mermaid
sequenceDiagram
    participant O as Betreiber
    participant P as Puma-Server
    participant D as Datenbank
    O->>P: Server starten
    P->>D: Verbindung und Schema prüfen
    D-->>P: Repositories verfügbar
    P->>P: HTTP/WSS, Authentifizierung und Handler starten
    P-->>O: Betriebsbereit oder Fehler im Protokoll
```

*Abbildung 6: Technischer Startablauf des Puma-Servers.*

Nach dem Start sind insbesondere zu prüfen:

- Datenbankverbindung erfolgreich,
- HTTP- und WebSocket-Port gebunden,
- Zertifikat und Schlüssel geladen,
- keine Migrations- oder Repositoryfehler,
- Client kann den Server erreichen.

### 4.3 Ersteinrichtung

```mermaid
flowchart TD
    A[SDK verbindet sich] --> B{Superuser vorhanden?}
    B -->|Nein| C[Passwort und Kontakt-E-Mail setzen]
    C --> H[Superuser erzeugen]
    B -->|Ja| D[Als Administrator anmelden]
    H --> D
    D --> E[Rollen und Gruppen definieren]
    E --> F[Benutzer anlegen oder Domänenlogin zulassen]
    F --> G[Berechtigungen testen]
```

*Abbildung 7: Ersteinrichtung vom Superuser bis zum Berechtigungstest.*

Der SDK-Ablauf besteht aus `SuperuserExists()`, bei Bedarf
`CreateSuperuser()`, danach Login und Aufbau des Rollenmodells. Bei der
Initialisierung werden das Passwort des Superusers und seine Kontakt-E-Mail
festgelegt. `CreateSuperuser()` übernimmt das Passwort; die Kontakt-E-Mail wird
in der Client-Komposition als `SuperuserMail` des
`RemoteSuperuserController` konfiguriert und beim Erzeugen übermittelt. Die
Puma-Tests verwenden für das initiale Konto den Login `su`; produktive
Zugangsdaten und eine erreichbare Kontakt-E-Mail müssen sicher gewählt und
verwahrt werden.

### 4.4 Transportverschlüsselung

Puma verwendet getrennte Ports für HTTP(S) und WebSocket(S). Sobald Clients
außerhalb eines isolierten Entwicklungsrechners zugreifen, sind HTTPS und WSS
zu verwenden.

```mermaid
flowchart LR
    C[Client] -->|TLS: Zugangsdaten und Token| RP[Reverse Proxy oder Puma TLS]
    RP --> P[Puma]
    P --> D[(Datenbank)]
    CERT[Zertifikat + privater Schlüssel] --> RP
```

*Abbildung 8: TLS-geschützter Transport zwischen Client und Puma.*

Produktionsregeln:

- TLS 1.2 oder höher einsetzen.
- Zertifikatsprüfung nicht deaktivieren.
- Privaten Schlüssel nur für den Serverprozess lesbar machen.
- Passphrasen nicht in Quellcode oder Präsentationen speichern.
- HTTP/WS ohne TLS nur in kontrollierten Testumgebungen verwenden.

## 5. Detaillierte Use Cases

### UC-01: Lokalen Benutzer anlegen und berechtigen

**Akteur:** Administrator  
**Vorbedingung:** Administrator ist angemeldet und besitzt die nötigen
Verwaltungsberechtigungen.

1. Benutzer mit Anzeigename, eindeutigem Login, Startpasswort und E-Mail
   anlegen.
2. Interne Benutzer-ID aus dem Ergebnis übernehmen.
3. Vorhandene Rolle zuordnen oder zuerst eine Rolle anlegen.
4. Optional den Benutzer einer Gruppe hinzufügen.
5. Benutzer meldet sich an.
6. Anwendung prüft die erwarteten Berechtigungen.

```mermaid
sequenceDiagram
    participant AD as Administrator
    participant SDK as AuthClientSdk
    participant P as Puma
    AD->>SDK: CreateUser(...)
    SDK->>P: Benutzer erzeugen
    P-->>SDK: interne Benutzer-ID
    AD->>SDK: AddRolesToUser(ID, Rollen)
    SDK->>P: Rollen zuordnen
    AD->>SDK: AddUsersToGroup(Gruppe, ID)
    SDK->>P: Gruppenmitgliedschaft setzen
```

*Abbildung 9: Anlage eines lokalen Benutzers mit Rollen- und
Gruppenzuordnung.*

**Ergebnis:** Der Benutzer erhält Berechtigungen aus direkten Rollen und
Gruppenrollen. Ein bereits angemeldeter Benutzer muss sich gegebenenfalls neu
anmelden, damit die Anwendung aktualisierte Sitzungsberechtigungen erhält.

### UC-02: Team über eine Gruppe verwalten

**Akteur:** Administrator

1. Fachliche Rolle mit den benötigten Berechtigungen erstellen.
2. Gruppe für Team oder Abteilung erstellen.
3. Rolle der Gruppe zuordnen.
4. Benutzer zur Gruppe hinzufügen.
5. Beim Austritt Benutzer aus der Gruppe entfernen.

```mermaid
flowchart LR
    U1[Benutzer A] --> G[Gruppe: Qualität]
    U2[Benutzer B] --> G
    U3[Benutzer C] --> G
    G --> R[Rolle: Prüfer]
    R --> P1[Messung lesen]
    R --> P2[Prüfung freigeben]
```

*Abbildung 10: Gemeinsame Berechtigungsvergabe über eine Gruppe.*

**Nutzen:** Rollenänderungen wirken zentral auf alle Gruppenmitglieder.

### UC-03: Anmeldung und funktionsbezogene Freigabe

**Akteur:** Endbenutzer

1. Anwendung konfiguriert Verbindung und Produkt-ID.
2. Benutzer gibt Login und Passwort ein.
3. Puma validiert die Zugangsdaten.
4. Puma erzeugt eine Sitzung und liefert Token, Benutzername, Produkt-ID und
   Berechtigungen.
5. Anwendung gibt nur erlaubte Funktionen frei.
6. Beim Abmelden invalidiert Puma die Sitzung.

```mermaid
sequenceDiagram
    participant U as Benutzer
    participant A as Anwendung
    participant S as AuthClientSdk
    participant P as Puma
    U->>A: Login + Passwort
    A->>S: Login(...)
    S->>P: Authentifizierungsanfrage
    P-->>S: Sitzungstoken + Berechtigungen
    S-->>A: Login-Daten
    A->>A: HasPermission("...") prüfen
    A-->>U: Erlaubte Funktionen anzeigen
    U->>A: Abmelden
    A->>S: Logout()
    S->>P: Sitzung beenden
```

*Abbildung 11: Anmeldung, Berechtigungsprüfung und Abmeldung einer Anwendung.*

Fehler bei ungültigen Zugangsdaten, gesperrtem Konto, fehlender Verbindung
oder fehlenden Serverkomponenten werden als fehlgeschlagener Login gemeldet.

### UC-04: Berechtigungsänderung

1. Administrator ändert Rollen- oder Gruppenzuordnung.
2. Anwendung beendet alte Sitzung oder fordert erneute Anmeldung.
3. Benutzer meldet sich erneut an.
4. Anwendung baut ihre Oberfläche aus den neuen Berechtigungen auf.

Das Ausblenden einer Schaltfläche ersetzt keine serverseitige Prüfung. Jede
schutzbedürftige Serveroperation muss die Berechtigung nochmals validieren.

### UC-05: Benutzer deaktivieren oder entfernen

Der vorhandene Client-SDK stellt `RemoveUser()` als permanente Löschung bereit.
Vor dem Löschen sind fachliche Aufbewahrungs- und Audit-Anforderungen zu
prüfen. Rollen- und Gruppenzuordnungen werden mit dem Benutzer entfernt.
Für eine zeitweise Sperre ist die in der konkreten Administrationsoberfläche
angebotene Kontostatusfunktion zu verwenden; andernfalls Zugriff über
Rollenzuordnung und Sitzungsmanagement entziehen.

### UC-06: Eigene Anwendung anbinden

```mermaid
flowchart TD
    C[CAuthorizationController erzeugen] --> N[SetConnectionParam]
    N --> P[SetProductId]
    P --> L[Login]
    L --> H{Erfolgreich?}
    H -->|Nein| E[Fehler anzeigen / erneut versuchen]
    H -->|Ja| Q[HasPermission]
    Q --> F[Funktion ausführen]
    F --> O[Logout]
```

*Abbildung 12: Minimaler Ablauf einer SDK-basierten Client-Anbindung.*

Die minimale Reihenfolge im C++-Client lautet:

```cpp
AuthClientSdk::CAuthorizationController auth;

AuthClientSdk::ServerConfig server;
server.host = "puma.example.org";
server.httpPort = 443;
server.wsPort = 8443;
server.sslConfig = AuthClientSdk::SslConfig{};

auth.SetConnectionParam(server);
auth.SetProductId("MeineAnwendung");

AuthClientSdk::Login session;
if (auth.Login(login, password, session) &&
    auth.HasPermission("messung.lesen")) {
    // Geschützte Funktion freigeben.
}
auth.Logout();
```

Sicherheitsrelevante Hinweise:

- Passwörter nur über TLS übertragen.
- Sitzungstoken nicht protokollieren.
- `Login()` beendet eine vorherige Sitzung des Controllers automatisch.
- `Logout()` explizit aufrufen; der Destruktor versucht zusätzlich eine
  Best-Effort-Abmeldung.
- `Login()` und `Logout()` nicht parallel auf demselben Controller ausführen.

### UC-07: Eigenen autorisierbaren Server anbinden

`AuthServerSdk::CAuthorizableServer` ist für Serveranwendungen vorgesehen, die
eigene Endpunkte anbieten, aber Puma als zentrale Autoritätsquelle verwenden.

```mermaid
flowchart LR
    CLIENT[Produktclient] --> APP[Eigener Anwendungsserver]
    APP --> ASDK[AuthServerSdk]
    ASDK --> P[Puma]
    P -->|Token und Berechtigungen| ASDK
    ASDK -->|Zugriff erlaubt / verweigert| APP
```

*Abbildung 13: Puma-Anbindung eines eigenen autorisierbaren Servers.*

Der Anwendungsserver setzt:

1. seine Produkt-ID,
2. die Verbindung zum zentralen Puma-Server,
3. seine eigenen HTTP-/WebSocket-Ports,
4. optional eine Featuredatei und TLS-Konfiguration,
5. anschließend `Start()` und beim Herunterfahren `Stop()`.

## 6. SDK-Schicht

### 6.1 AuthClientSdk

Die Fassade `AuthClientSdk::CAuthorizationController` bietet:

| Bereich | Zentrale Operationen |
|---|---|
| Verbindung | `SetConnectionParam()`, `SetProductId()` |
| Sitzung | `Login()`, `Logout()`, `GetToken()` |
| Autorisierung | `HasPermission()`, `GetTokenPermissions()` |
| Bootstrap | `SuperuserExists()`, `CreateSuperuser()` |
| Benutzer | Auflisten, lesen, anlegen, löschen, Passwort ändern |
| Rollen | Auflisten, lesen, anlegen, löschen, Berechtigungen zuordnen |
| Gruppen | Auflisten, lesen, anlegen, löschen, Benutzer/Rollen zuordnen |
| PAT | Erstellen, auflisten, validieren und widerrufen |

`ServerConfig` enthält Host, HTTP-Port, WebSocket-Port und optionale
TLS-Einstellungen. Rollen und Berechtigungen sind an die mit
`SetProductId()` konfigurierte Anwendung gebunden.

### 6.2 AuthServerSdk

Das Server-SDK kapselt einen autorisierbaren HTTP-/WebSocket-Server. Seine
Netzwerkverbindung zum Puma-Backend ist von den Ports getrennt, auf denen der
eigene Server Clients bedient. Bei verteilten Installationen müssen daher
beide Verbindungsrichtungen konfiguriert und abgesichert werden.

### 6.3 UI-Komponenten

Puma enthält Widgets beziehungsweise QML-Komponenten für Login und
Administration. Sie bauen auf denselben Authentifizierungs- und
Verwaltungsschnittstellen auf. Eine eigene Oberfläche darf die
serverseitigen Berechtigungsprüfungen nicht ersetzen.

## 7. LDAP-/Windows-Domänenanmeldung

### 7.1 Funktionsweise

Die aktuelle ImtCore-Implementierung verwendet auf Windows die
Windows-Domänenfunktionen, insbesondere die Prüfung über `LogonUser`.
Sie ist damit auf Windows-/Active-Directory-Umgebungen ausgerichtet und kein
allgemein konfigurierbarer OpenLDAP-Client.

```mermaid
sequenceDiagram
    participant U as Domänenbenutzer
    participant A as Anwendung
    participant P as Puma
    participant W as Windows-Domäne
    U->>A: DOMÄNE\login + Passwort
    A->>P: Login für Produkt
    P->>W: Zugangsdaten prüfen
    W-->>P: erfolgreich
    P->>P: Benutzer per Login/SID finden
    alt Erster Login
        P->>P: Benutzer lokal spiegeln
        P->>P: Default-Rolle des Produkts zuordnen
    else Bekannter Benutzer
        P->>P: Login, SID, E-Mail und Produktbezug ergänzen
    end
    P-->>A: Sitzung + Berechtigungen
```

*Abbildung 14: Anmeldung eines Windows-Domänenbenutzers über Puma.*

Bei erfolgreichem ersten Domänenlogin:

- legt Puma einen internen Benutzerdatensatz an,
- kennzeichnet das Authentifizierungssystem als `LDAP`,
- übernimmt, soweit verfügbar, SID, Anzeigename und E-Mail,
- erzeugt bei Bedarf die produktbezogenen Rollen `Guest` und `Default`,
- weist dem Benutzer für das Produkt die Default-Rolle zu.

Anschließend können Administratoren dem gespiegelt angelegten Benutzer weitere
Puma-Rollen und Gruppen zuordnen. Das Passwort wird weiterhin gegenüber der
Windows-Domäne geprüft.

### 7.2 Aktivierung und Deaktivierung

`LdapEnabled` ist in der Puma-Standardeinstellung aktiviert und wird über den
Einstellungsbereich **LDAP** bereitgestellt. Wenn ausschließlich lokale
Puma-Konten verwendet werden, sollte die Funktion deaktiviert werden, um
unnötige Domänenprüfungen und irreführende Meldungen zu vermeiden.

### 7.3 Voraussetzungen

- Puma läuft unter Windows.
- Der Server kann die Domäne und einen Domain Controller erreichen.
- Betriebssystem, DNS und Vertrauensstellung sind korrekt konfiguriert.
- Benutzer verwendet einen von Windows akzeptierten Login, typischerweise
  `DOMÄNE\benutzer`.
- LDAP ist in Puma aktiviert.

### 7.4 Fehlersuche

| Symptom | Prüfung |
|---|---|
| Domänenlogin scheitert, lokaler Login funktioniert | Domänenerreichbarkeit, DNS, Uhrzeit, Loginformat und `LdapEnabled` prüfen |
| Benutzer wird doppelt angelegt | Einheitliches Loginformat und SID-Auflösung prüfen |
| Benutzer hat nach erstem Login zu wenig Rechte | Default-Rolle und weitere Rollen-/Gruppenzuordnung prüfen |
| Lokale Logins erzeugen Domänenfehler im Protokoll | LDAP deaktivieren, wenn nicht benötigt |
| Linux-Server authentifiziert nicht gegen AD | Aktuelle Implementierung ist Windows-spezifisch |

## 8. Personal Access Tokens (PAT)

### 8.1 Einsatz

PATs sind langlebige Zugangsnachweise für Automatisierung, CI/CD,
Überwachungsdienste und Service-zu-Service-Kommunikation. Ein PAT gehört zu
einem Benutzer, enthält eine Produkt-ID und explizite Berechtigungsscopes.

```mermaid
flowchart LR
    OWNER[Benutzer oder Administrator] -->|PAT erzeugen| P[Puma]
    P -->|Secret genau bei Erstellung| VAULT[Secret Store]
    JOB[Automatisierter Job] -->|PAT| API[Geschützte Anwendung]
    API -->|PAT prüfen| P
    P -->|Benutzer + Scopes| API
```

*Abbildung 15: Erstellung, Speicherung und Verwendung eines PAT.*

### 8.2 Lebenszyklus

```mermaid
stateDiagram-v2
    [*] --> Aktiv: Erstellen
    Aktiv --> Aktiv: Verwenden / LastUsed aktualisieren
    Aktiv --> Abgelaufen: Ablaufzeit erreicht
    Aktiv --> Widerrufen: Revoke
    Widerrufen --> [*]
    Abgelaufen --> [*]
```

*Abbildung 16: Zustände im Lebenszyklus eines PAT.*

Ein Token ist gültig, wenn es existiert, aktiv, nicht widerrufen und nicht
abgelaufen ist. Widerrufene Datensätze bleiben in der Liste sichtbar und
werden als inaktiv gemeldet.

### 8.3 PAT erstellen

**Vorbedingung:** Der Eigentümer oder ein Administrator ist mit einer
Sitzung angemeldet.

1. Zweckbezogenen Namen vergeben, zum Beispiel `CI Produktion Lesen`.
2. Zielbenutzer und Produkt-ID festlegen.
3. Nur die minimal nötigen Scopes auswählen.
4. Möglichst ein Ablaufdatum im ISO-8601-Format setzen.
5. Secret unmittelbar in einem Secret Store speichern.
6. Secret nicht in Quellcode, Build-Protokolle oder Tickets kopieren.

Anonyme Aufrufer dürfen keine PATs erstellen. Ein normaler Benutzer kann
eigene PATs verwalten, aber nicht die PATs anderer Benutzer. Administratoren
können PATs anderer Benutzer verwalten.

### 8.4 PAT verwenden

Das SDK-Datenmodell unterscheidet `TokenType::Session` und
`TokenType::PersonalAccessToken`. Für einen nicht interaktiven Zugriff wird
das PAT über `ValidatePersonalAccessToken()` geprüft; die Anwendung verwendet
anschließend ausschließlich die zurückgegebenen Scopes und prüft zusätzlich
den Produktkontext.

```mermaid
sequenceDiagram
    participant J as Automatisierter Job
    participant A as Anwendung
    participant P as Puma
    J->>A: Anfrage mit PAT
    A->>P: PAT validieren
    P->>P: Hash, Aktivstatus und Ablauf prüfen
    P-->>A: gültig + Benutzer-ID + Scopes
    A->>A: benötigten Scope prüfen
    A-->>J: Ergebnis oder Zugriff verweigert
```

*Abbildung 17: Validierung eines PAT für einen automatisierten Zugriff.*

### 8.5 PAT widerrufen

1. Token anhand von Name, Produkt, Erstellungszeit und letzter Nutzung
   identifizieren.
2. Token-ID widerrufen.
3. Validierung muss danach fehlschlagen.
4. Bei vermutetem Secret-Abfluss abhängige Systeme und Protokolle prüfen.
5. Ersatz-PAT mit reduziertem Scope und neuem Ablaufdatum ausstellen.

### 8.6 Bekannte Schnittstelleneigenschaft

Die aktuelle GraphQL-Validierungsantwort liefert Benutzer-ID und Scopes, aber
nicht die Token-ID. Deshalb kann `ValidatePersonalAccessToken()` die
`productId` im Validierungsergebnis derzeit nicht rekonstruieren. Der
Produktkontext muss beim ausstellenden beziehungsweise konsumierenden System
zusätzlich bekannt und geprüft sein.

## 9. Betrieb und Sicherheit

### 9.1 Verantwortlichkeiten

```mermaid
flowchart TB
    OP[Betreiber] --> TLS[TLS, Firewall, Updates]
    OP --> DB[Backup und Datenbank]
    ADM[Administrator] --> IAM[Benutzer, Rollen, Gruppen]
    ADM --> TOK[PAT-Widerruf und Reviews]
    DEV[Entwickler] --> PERM[Berechtigungsprüfungen]
    DEV --> SEC[Token sicher behandeln]
```

*Abbildung 18: Aufteilung der betrieblichen Sicherheitsverantwortung.*

### 9.2 Regelmäßige Kontrollen

- Benutzer ohne aktuellen fachlichen Bedarf entfernen oder sperren.
- Rollen und Gruppen nach dem Least-Privilege-Prinzip prüfen.
- Alte, nie verwendete oder abgelaufene PATs widerrufen.
- Administratorrechte personenbezogen vergeben.
- Datenbank und Einstellungen sichern; Wiederherstellung testen.
- Zertifikatsablauf überwachen.
- Fehlgeschlagene Logins und ungewöhnliche Tokenverwendung untersuchen.
- Server und ImtCore/Puma-Komponenten aktuell halten.

### 9.3 Backup und Wiederherstellung

Ein konsistentes Backup umfasst mindestens Datenbank und Puma-Einstellungen.
Zertifikate und Schlüssel sind getrennt und besonders geschützt zu sichern.
Nach einer Wiederherstellung sind Datenbankmigrationen, Login, Rollen,
Gruppen, Sitzungsbehandlung und PAT-Validierung in einer kontrollierten
Umgebung zu testen.

## 10. Fehlerdiagnose

```mermaid
flowchart TD
    X[Operation fehlgeschlagen] --> N{Server erreichbar?}
    N -->|Nein| NET[Host, Ports, Firewall, DNS]
    N -->|Ja| T{TLS erfolgreich?}
    T -->|Nein| CERT[Zertifikat, CA, Name, Uhrzeit]
    T -->|Ja| A{Login erfolgreich?}
    A -->|Nein| ID[Kontostatus, Passwort, LDAP]
    A -->|Ja| P{Berechtigung vorhanden?}
    P -->|Nein| RBAC[Produkt-ID, Rolle, Gruppe, Scope]
    P -->|Ja| LOG[Server- und Anwendungsprotokoll prüfen]
```

*Abbildung 19: Entscheidungsbaum zur Fehlerdiagnose.*

| Problem | Wahrscheinliche Ursache | Maßnahme |
|---|---|---|
| Verbindung abgelehnt | Falscher Host/Port oder Server nicht gestartet | HTTP- und WS-Port sowie Prozess prüfen |
| TLS-Fehler | Zertifikat nicht vertrauenswürdig oder Name falsch | Zertifikatskette, Hostname und Uhrzeit prüfen |
| Login schlägt fehl | Zugangsdaten, Kontostatus oder LDAP | Authentifizierungsweg gezielt prüfen |
| `HasPermission()` bleibt `false` | Falsche Produkt-ID oder fehlende Rolle | Produkt-ID und effektive Rollen prüfen |
| Benutzeroperation liefert leere ID | Login bereits vorhanden oder Rechte fehlen | Eindeutigkeit und Adminrechte prüfen |
| PAT-Erstellung liefert leeres Secret | Nicht angemeldet, falscher Eigentümer oder leere Scopes | Sitzung, Benutzer-ID und Scopes prüfen |
| PAT ist ungültig | Widerrufen, abgelaufen oder verändert | Tokenmetadaten prüfen und neu ausstellen |
| Einstellungen gehen verloren | Fehlende Schreibrechte | Pfad und Dienstkonto prüfen |

## 11. Abnahme-Checklisten

### Server

- [ ] Passende Datenbankvariante gewählt
- [ ] Datenbankverbindung und Migration erfolgreich
- [ ] HTTPS und WSS mit gültigem Zertifikat aktiv
- [ ] Ports und Firewall dokumentiert
- [ ] Backup und Wiederherstellung getestet
- [ ] Protokollüberwachung eingerichtet

### Berechtigungsmodell

- [ ] Eindeutige Produkt-ID festgelegt
- [ ] Berechtigungs-IDs dokumentiert
- [ ] Rollen nach Aufgaben statt Personen modelliert
- [ ] Gruppen für wiederkehrende Teams angelegt
- [ ] Personalisierte Administratorkonten eingerichtet
- [ ] Negativtests für verweigerte Aktionen durchgeführt

### LDAP

- [ ] Windows- und Domänenvoraussetzungen erfüllt
- [ ] Erster Domänenlogin getestet
- [ ] SID und Benutzerdaten korrekt übernommen
- [ ] Default-Rolle geprüft
- [ ] LDAP deaktiviert, falls nicht benötigt

### PAT

- [ ] Least-Privilege-Scopes vergeben
- [ ] Ablaufdatum gesetzt
- [ ] Secret nur im Secret Store gespeichert
- [ ] Widerruf getestet
- [ ] Rotation und Verantwortlicher dokumentiert

## 12. Weiterführende Dokumentation

- [AuthClientSdk-Referenz](../AuthClientSdk.md)
- [AuthServerSdk-Referenz](../AuthServerSdk.md)
- [Abhängigkeiten](../Dependencies.md)
- [Puma-Sicherheitsrichtlinie](../../SECURITY.md)
- [Kompaktpräsentation](Puma_Kompakt_DE.pptx)

## 13. Entwickleranbindung

Eine Anwendung kann Puma auf zwei Arten einbinden:

| Variante | Geeignet für | Abstraktion |
|---|---|---|
| `AuthClientSdk` | Anwendungen, die eine stabile C++-Fassade benötigen | `AuthClientSdk::CAuthorizationController` |
| Partitura | ACF-/ImtCore-Anwendungen, die Authentifizierung und UI deklarativ verdrahten | exportierte ACF-Schnittstellen und GUI-Komponenten |

In beiden Varianten benötigt die Anwendung die Adresse des Puma-Servers, eine
eindeutige Produkt-ID und die für das Produkt definierten Berechtigungen. Die
Produkt-ID muss mit der administrierten Anwendung übereinstimmen. TLS ist für
produktive Verbindungen zu verwenden.

### 13.1 Einbindung über das SDK

1. `AuthClientSdk` bauen und zur Anwendung linken. Das CMake-Beispiel unter
   `Impl/AuthClientSdk/CMake/CMakeLists.txt` zeigt die benötigten Qt- und
   ImtCore-Abhängigkeiten.
2. `AuthClientSdk/AuthClientSdk.h` einbinden.
3. Einen `CAuthorizationController` erzeugen, mit `SetConnectionParam()` den
   HTTP-/WebSocket-Endpunkt und die TLS-Konfiguration setzen und anschließend
   mit `SetProductId()` den Produktkontext festlegen.
4. Mit `Login()` anmelden und Funktionen nur nach erfolgreicher Prüfung durch
   `HasPermission()` freigeben.
5. Für die Administrationsseite die Benutzer-, Rollen- und Gruppenoperationen
   derselben Fassade verwenden. Die Anwendung entscheidet anhand der
   Administrationsberechtigung, ob sie die Seite in der Client-UI anbietet.
6. Beim Sitzungsende `Logout()` aufrufen.

Ein minimales C++-Beispiel befindet sich in [UC-06](#uc-06-eigene-anwendung-anbinden);
die vollständige API beschreibt die
[AuthClientSdk-Referenz](../AuthClientSdk.md). Das SDK wird im aktuellen
CMake-Build nur unter Windows eingebunden.

### 13.2 Einbindung über Partitura

Für eine ACF-/ImtCore-Anwendung dienen die Kompositionen unter
`Impl/AuthClientSdk` als Vorlagen:

- `AuthClientSdk.acc` verdrahtet die Puma-Verbindung und exportiert unter
  anderem `iauth::ILogin`, `iauth::IRightsProvider`,
  `imtauth::IPermissionChecker`, `imtauth::IUserManager`,
  `imtauth::IRoleManager` und `imtauth::IUserGroupManager`.
- `LoginWidget.acc` stellt den Anmeldedialog als `iqtgui::IGuiObject` bereit.
- `AdministrationWidget.acc` stellt die Administrationsseite als
  `iqtgui::IGuiObject` bereit.

Die Integration erfolgt in diesen Schritten:

1. Die Puma- und ImtCore-Paketverzeichnisse sowie die benötigten
   Registry-Dateien in der ACF-Konfiguration der Anwendung bekannt machen.
   `Config/Puma.awc` ist die Referenz für die Puma-Registry-Dateien.
2. Die benötigten `.acc`-Dateien durch den ARX-Compiler verarbeiten. Die
   CMake-Einbindung in `Impl/AuthClientSdk/CMake/CMakeLists.txt` zeigt die
   Generierung und Aufnahme der erzeugten C++-Dateien in das Ziel.
3. In der Anwendungskomposition genau eine Authentifizierungskomposition
   instanziieren und deren exportierte Schnittstellen per
   `Type="Reference"` an die fachlichen Komponenten weiterreichen. So nutzen
   Anmeldung, Berechtigungsprüfung und Administration dieselbe Sitzung.
4. Produktidentität, Puma-Endpunkte und TLS am
   `imtbase::IApplicationInfoController` beziehungsweise
   `imtcom::IServerConnectionInterface` der Komposition konfigurieren.
5. `LoginWidget` und bei Bedarf `AdministrationWidget` in das Seitenmodell der
   Client-UI aufnehmen. Die Administrationsseite nur bei vorhandener
   Administrationsberechtigung anzeigen; serverseitige Prüfungen bleiben
   trotzdem verbindlich.
6. Nach dem Build Anmeldung, Sitzungsende, erlaubte und verweigerte
   Berechtigungen sowie die Benutzer-, Rollen- und Gruppenverwaltung gegen
   eine Testinstanz von Puma prüfen.

Die Partitura-Variante bietet direkten Zugriff auf ImtCore-Schnittstellen,
während die SDK-Variante diese hinter einer C++-Fassade kapselt. Beide
Varianten verwenden dieselben Puma-Endpunkte und dasselbe serverseitige
Berechtigungsmodell; sie dürfen nicht als zwei unabhängige Sitzungen parallel
für dieselbe Client-Funktion verdrahtet werden.
