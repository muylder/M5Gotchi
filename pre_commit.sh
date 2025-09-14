#!/bin/bash

set -e

REPO_URL="https://devsur11.github.io/M5Gotchi/firmware"

read -p "📄 Enter firmware version (e.g., 0.3.1): " VERSION

mkdir -p firmware

FULL_BIN_PATH=$(find .pio/build/Cardputer-full/ -type f -name "firmware.bin" | head -n 1)
if [ ! -f "$FULL_BIN_PATH" ]; then
    echo "❌ Full firmware.bin not found!"
    exit 1
fi
cp "$FULL_BIN_PATH" firmware/firmware.bin
echo "✅ Full firmware copied to firmware/firmware.bin"

# Step 6: Copy lite binary to /firmware/lite.bin
LITE_BIN_PATH=$(find .pio/build/cardputer-lite -type f -name "firmware.bin" | head -n 1)
if [ ! -f "$LITE_BIN_PATH" ]; then
    echo "❌ Lite firmware.bin not found!"
    exit 1
fi
cp "$LITE_BIN_PATH" firmware/lite.bin
echo "✅ Lite firmware copied to firmware/lite.bin"

LITE_BIN_PATH=$(find .pio/build/m5stick-c -type f -name "firmware.bin" | head -n 1)
if [ ! -f "$LITE_BIN_PATH" ]; then
    echo "❌ Lite firmware.bin not found!"
    exit 1
fi
cp "$LITE_BIN_PATH" firmware/stick.bin
echo "✅ M5Stick firmware copied to firmware/stick.bin"

# Step 7: Create firmware.json with full URL
DATE=$(date +%F)
cat <<EOF > firmware/firmware.json
{
  "version": "$VERSION",
  "file": "$REPO_URL/firmware.bin",
  "date": "$DATE",
  "notes": "Full version"
}
EOF

# Step 8: Create lite.json with full URL
cat <<EOF > firmware/lite.json
{
  "version": "$VERSION",
  "file": "$REPO_URL/lite.bin",
  "date": "$DATE",
  "notes": "Lite version"
}
EOF

cat <<EOF > firmware/stick.json
{
  "version": "$VERSION",
  "file": "$REPO_URL/stick.bin",
  "date": "$DATE",
  "notes": "M5StickC version"
}
EOF

bundle-cardputer-lite
bundle-cardputer-full

echo "✅ Metadata files with full download URLs created"