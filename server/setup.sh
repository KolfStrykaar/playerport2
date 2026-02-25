#!/bin/bash
set -e

echo "=== LOK MMORPG Server Setup ==="
echo

# Check Node.js
if ! command -v node &> /dev/null; then
    echo "Node.js not found. Installing via nvm..."
    curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.7/install.sh | bash
    export NVM_DIR="$HOME/.nvm"
    [ -s "$NVM_DIR/nvm.sh" ] && . "$NVM_DIR/nvm.sh"
    nvm install 20
    nvm use 20
fi

echo "Node.js version: $(node --version)"
echo "npm version: $(npm --version)"
echo

# Install dependencies
echo "Installing dependencies..."
cd "$(dirname "$0")"
npm install

# Build
echo "Building TypeScript..."
npm run build

echo
echo "=== Setup complete ==="
echo
echo "To start the server:"
echo "  cd $(pwd)"
echo "  npm start"
echo
echo "Or install the systemd service:"
echo "  sudo cp lok-mmorpg.service /etc/systemd/system/"
echo "  sudo systemctl daemon-reload"
echo "  sudo systemctl enable lok-mmorpg"
echo "  sudo systemctl start lok-mmorpg"
