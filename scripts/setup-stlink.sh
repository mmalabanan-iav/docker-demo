#!/usr/bin/env bash
# setup-stlink.sh
# Configure udev rules and permissions for ST-Link on Linux

set -e

RULES_FILE="/etc/udev/rules.d/49-stlink.rules"

echo "[*] Creating udev rules for ST-Link at $RULES_FILE ..."
sudo tee $RULES_FILE > /dev/null <<'EOF'
# STMicroelectronics ST-LINK/V2, V2-1, V3
SUBSYSTEM=="usb", ATTR{idVendor}=="0483", ATTR{idProduct}=="3748", MODE="0666", TAG+="uaccess"
SUBSYSTEM=="usb", ATTR{idVendor}=="0483", ATTR{idProduct}=="374b", MODE="0666", TAG+="uaccess"
SUBSYSTEM=="usb", ATTR{idVendor}=="0483", ATTR{idProduct}=="374f", MODE="0666", TAG+="uaccess"
EOF

echo "[*] Reloading udev rules ..."
sudo udevadm control --reload-rules
sudo udevadm trigger

# Detect if system uses 'dialout' or 'uucp' group
if getent group dialout >/dev/null 2>&1; then
    GROUP="dialout"
elif getent group uucp >/dev/null 2>&1; then
    GROUP="uucp"
else
    GROUP=""
fi

if [ -n "$GROUP" ]; then
    echo "[*] Adding user $USER to group $GROUP ..."
    sudo usermod -aG "$GROUP" "$USER"
    echo "[!] You may need to log out and log back in (or run 'newgrp $GROUP') for group changes to take effect."
else
    echo "[!] No 'dialout' or 'uucp' group found. Skipping group add step."
fi

echo "[*] Done. Unplug and replug your ST-Link, then retry OpenOCD."
