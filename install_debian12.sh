#!/bin/bash

# Install Script for Perfect World 1.7.6 Source (Debian 12)
# Usage: chmod +x install_debian12.sh && ./install_debian12.sh

VERSION="1.4" # Increment this on every update

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}[+] Starting Setup for Debian 12 (v${VERSION})...${NC}"

# ... (omitted lines)

# Debian 12 dependencies for C++20 build
# Note: libmysqlclient-dev is replaced by libmariadb-dev-compat on Debian 12
apt-get install -y --no-install-recommends build-essential cmake gcc g++ make \
    libxml2-dev libssl-dev libpcre3-dev zlib1g-dev \
    libmariadb-dev-compat libmariadb-dev libreadline-dev \
    ant default-jdk dos2unix libxml-dom-perl

# 3. Clone Repository
REPO_DIR="/root/176source"
REPO_URL="https://github.com/hnomkeng/PwServer-176source.git"

if [ -d "$REPO_DIR" ]; then
    echo -e "${YELLOW}[!] Directory $REPO_DIR already exists. Pulling latest changes...${NC}"
    cd "$REPO_DIR"
    git pull
else
    echo -e "${GREEN}[+] Cloning Repository form $REPO_URL...${NC}"
    git clone "$REPO_URL" "$REPO_DIR"
    cd "$REPO_DIR"
fi

# 4. Setup Directory Structure (Fixing paths for build.sh)
# The build script expects 'share' in directory above or home (~/share)
echo -e "${GREEN}[+] Configuring Directory Structure...${NC}"

if [ -d "share" ]; then
    echo -e "${GREEN}[+] Moving 'share' directory to /root/share as expected by build.sh...${NC}"
    rm -rf /root/share
    cp -r share /root/
fi

# 5. Fix Permissions and Line Endings
dos2unix -v build.sh install_debian12.sh
chmod +x build.sh

# Ensure rpcgen is executable in share directory (source of symlink)
if [ -f "/root/share/rpcgen" ]; then
    echo -e "${GREEN}[+] Fixing rpcgen permissions...${NC}"
    chmod +x /root/share/rpcgen
    dos2unix /root/share/rpcgen
fi

# Ensure xmlcoder.pl is executable (called by rpcgen)
if [ -f "/root/share/rpc/xmlcoder.pl" ]; then
    echo -e "${GREEN}[+] Fixing xmlcoder.pl permissions...${NC}"
    chmod +x /root/share/rpc/xmlcoder.pl
    dos2unix /root/share/rpc/xmlcoder.pl
fi

# 6. Prepare Output Directories
echo -e "${GREEN}[+] Creating Output Directories in /home...${NC}"
mkdir -p /home/gamed
mkdir -p /home/gfactiond
mkdir -p /home/gauthd
mkdir -p /home/uniquenamed
mkdir -p /home/gamedbd
mkdir -p /home/gdeliveryd
mkdir -p /home/glinkd
mkdir -p /home/gacd
mkdir -p /home/logservice

# 7. Build
echo -e "${GREEN}[+] Starting Build Process...${NC}"
echo -e "${YELLOW}[+] Log will be saved to /var/log/build_pw.log${NC}"

# Use standard build script and pipe output to log and stdout
./build.sh all 2>&1 | tee /var/log/build_pw.log

echo -e "${GREEN}[+] Setup and Build Finished!${NC}"
