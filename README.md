# AzerothCore Module: Playerbots Auto-Guild

> **Disclaimer:** This module requires the [mod-playerbots](https://github.com/mod-playerbots/mod-playerbots) fork of AzerothCore. Ensure you are using a compatible server build before enabling this module.

## Table of Contents

1. [Overview](#overview)
2. [Features](#features)
3. [Installation](#installation)
4. [Configuration](#configuration)
5. [Usage](#usage)
6. [Troubleshooting](#troubleshooting)

## Overview

This module automatically assigns playerbots to pre-existing faction guilds. When a bot logs in, it is placed into the configured Horde or Alliance guild. A periodic background scan ensures bots that were already online at startup (or that somehow left their guild) are also handled.

## Features

- **Login Assignment** — Bots are assigned to their faction guild immediately upon login.
- **Periodic Safety Scan** — A configurable timer scans all online bots and fixes any that are not in their faction guild.
- **Force Assign Mode** — Optionally removes bots from any existing guild and places them into the correct faction guild.
- **Guild Resolution** — Guilds can be specified by ID (preferred) or by name (looked up at startup).
- **Faction-Aware** — Bots are assigned based on their team (Horde or Alliance).

## Installation

1. **Clone the Module**
   ```bash
   cd /path/to/azerothcore/modules
   git clone https://github.com/dr1s/mod-playerbots-auto-guild.git
   ```

2. **Recompile AzerothCore**

3. **Enable and Configure**
   ```bash
   cp /path/to/azerothcore/modules/mod-playerbots-auto-guild/conf/mod_playerbots_auto_guild.conf.dist /path/to/azerothcore/modules/mod-playerbots-auto-guild/conf/mod_playerbots_auto_guild.conf
   ```

4. **Restart the Server**

## Configuration

Edit `mod_playerbots_auto_guild.conf`:

```ini
# Guild ID for the Horde bot guild. If set to 0, the module will try to resolve by HordeGuildName.
PlayerbotsAutoGuild.HordeGuildId = 0

# Guild ID for the Alliance bot guild. If set to 0, the module will try to resolve by AllianceGuildName.
PlayerbotsAutoGuild.AllianceGuildId = 0

# Guild name for the Horde bot guild. Only used if HordeGuildId is 0.
PlayerbotsAutoGuild.HordeGuildName = ""

# Guild name for the Alliance bot guild. Only used if AllianceGuildId is 0.
PlayerbotsAutoGuild.AllianceGuildName = ""

# If true, bots will be removed from any guild they are currently in and placed into their faction guild.
# If false, bots already in a guild are left alone.
PlayerbotsAutoGuild.ForceAssign = false

# How often (in seconds) the periodic scan runs to ensure all online bots are in their faction guild.
PlayerbotsAutoGuild.CheckIntervalSeconds = 60
```

## Usage

### By Guild ID (Recommended)

Find the guild IDs in the `guild` table of your characters database:

```sql
SELECT guildid, name FROM guild;
```

Then set the IDs in your config:

```ini
PlayerbotsAutoGuild.HordeGuildId = 1
PlayerbotsAutoGuild.AllianceGuildId = 2
```

### By Guild Name

If you prefer to use guild names instead of IDs:

```ini
PlayerbotsAutoGuild.HordeGuildName = "Horde Bots"
PlayerbotsAutoGuild.AllianceGuildName = "Alliance Bots"
```

Guild name resolution happens once at server startup. If the guild is not found, a warning is logged.

### Force Assign

When `ForceAssign` is set to `true`, any bot that is in a guild other than their assigned faction guild will be removed from that guild and added to the correct one. This is useful if bots have been invited to player guilds and you want to enforce strict faction guild membership.

```ini
PlayerbotsAutoGuild.ForceAssign = true
```

## Troubleshooting

- **No bots being assigned**
  - Verify that the guild ID or name is correctly configured.
  - Check server logs for `[PlayerbotsAutoGuild]` messages at startup.
  - Ensure the Playerbots module is loaded.

- **Guild not found warning**
  - If using guild name, confirm the name matches exactly (case-sensitive).
  - If using guild ID, verify the guild exists in the database.

- **Bots not reassigned after config change**
  - Restart the server to reload configuration.
  - The periodic scan will catch bots on the next cycle if the server is running.
