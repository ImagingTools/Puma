#!/bin/bash

# Change to script directory
cd "$(dirname "$0")"

FILE="../../Partitura/PumaVoce.arp/VersionInfo.acc.xtrsvn"

# Fetch and unshallow if needed (only if repository is shallow)
if git rev-parse --is-shallow-repository 2>/dev/null | grep -q "true"; then
    git fetch --prune --unshallow 2>/dev/null
fi

# Get revision count
REV=$(git rev-list --count origin/master 2>/dev/null)
if [ -z "$REV" ]; then
    REV=$(git rev-list --count HEAD 2>/dev/null)
fi
if [ -z "$REV" ]; then
    echo "Failed to compute revision count."
    exit 1
fi

# Check if working tree is dirty
if git diff-index --quiet HEAD --; then
    DIRTY=0
else
    DIRTY=1
fi

echo "Git revision: $REV, dirty: $DIRTY"
echo "Processing file: $FILE"

# Generate output filename
OUT="${FILE%.xtrsvn}"

# Process the file - use separate sed commands for better readability
sed -e "s/\\\$WCREV\\\$/$REV/g" "$FILE" | sed -e "s/\\\$WCMODS?1:0\\\$/$DIRTY/g" > "$OUT"

echo "Wrote $OUT with WCREV=$REV and WCMODS=$DIRTY"
