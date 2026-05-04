#!/usr/bin/env bash
set -e

echo "[1/3] Trying detecting shell..."
CURRENT_SHELL=$(basename "$SHELL")
CONFIG_FILE=""

case "$CURRENT_SHELL" in
    zsh)
        CONFIG_FILE="$HOME/.zshrc"
        ;;
    bash)
        CONFIG_FILE="$HOME/.bashrc"
        ;;
    *)
        CONFIG_FILE="$HOME/.profile"
        ;;
esac

echo "[INFO] Detected shell: $CURRENT_SHELL"
echo "[INFO] Config file: $CONFIG_FILE"

echo "[2/3] Configuring Luarocks in $CONFIG_FILE..."
if ! grep -q 'luarocks path' "$CONFIG_FILE"; then
    echo 'eval "$(luarocks path)"' >> "$CONFIG_FILE"
    echo "[SUCCESS] Line added to $CONFIG_FILE"
else
    echo "[INFO] Already exists a luarocks configuration in $CONFIG_FILE"
fi

echo "[3/3] Compiling project..."
make clean
make

echo "[DONE] Setup completed!"
echo "Open a new terminal or run: source $CONFIG_FILE"
