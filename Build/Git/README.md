# Git Scripts

This directory contains scripts for managing Git hooks and version information.

## Scripts

### Windows (`.bat` files)

- **InstallHooks.bat** - Installs the post-merge hook for Windows
- **UpdateVersion.bat** - Updates version information from Git revision

### Linux/macOS (`.sh` files)

- **InstallHooks.sh** - Installs the post-merge hook for Linux/macOS
- **UpdateVersion.sh** - Updates version information from Git revision

### Git Hooks

- **post-merge** - Git hook that automatically runs after a merge, detects the OS and executes the appropriate update script

## Usage

### Installing the Hook

**Windows:**
```cmd
cd Build\Git
InstallHooks.bat
```

**Linux/macOS:**
```bash
cd Build/Git
./InstallHooks.sh
```

### Manually Updating Version

**Windows:**
```cmd
cd Build\Git
UpdateVersion.bat
```

**Linux/macOS:**
```bash
cd Build/Git
./UpdateVersion.sh
```

## How It Works

The scripts process `Partitura/PumaVoce.arp/VersionInfo.acc.xtrsvn` template file and generate `VersionInfo.acc` with:

- `$WCREV$` replaced with Git revision count
- `$WCMODS?1:0$` replaced with 1 (dirty) or 0 (clean) working tree status

The post-merge hook automatically runs the version update after each Git merge operation.
