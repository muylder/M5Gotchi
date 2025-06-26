#!/bin/bash

# Absolute paths
BUILD_DIR=".pio/build/m5stack-stamps3"
SRC="$BUILD_DIR/firmware.bin"
DEST_DIR="docs/firmware"
DEST="$DEST_DIR/latest.bin"
VERSION_FILE="docs/version.txt"

# Ensure source exists
if [ ! -f "$SRC" ]; then
    echo "[post_build.sh] ERROR: Firmware not found at $SRC"
    exit 1
fi

# Create destination directory
mkdir -p "$DEST_DIR"

# Copy firmware
cp "$SRC" "$DEST"
echo "[post_build.sh] Copied firmware to $DEST"

# Prompt for version
read -p "[post_build.sh] Add version to docs/version.txt? [y/N]: " choice
if [[ "$choice" =~ ^[Yy]$ ]]; then
    read -p "Enter version number: " version
    if [[ -n "$version" ]]; then
        echo "$version" > "$VERSION_FILE"
        echo "[post_build.sh] Version '$version' saved to $VERSION_FILE"
    else
        echo "[post_build.sh] No version entered. Skipping."
    fi
else
    echo "[post_build.sh] Version entry skipped."
fi
