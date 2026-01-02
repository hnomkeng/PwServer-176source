#!/bin/bash

# Install Script for Perfect World 1.7.6 Source (Debian 12)
# Usage: chmod +x install_debian12.sh && ./install_debian12.sh

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}[+] Starting Setup for Debian 12...${NC}"

# 1. Check Root
if [ "$EUID" -ne 0 ]; then
  echo -e "${RED}[!] Please run as root${NC}"
  exit 1
fi

# 2. Install Dependencies
echo -e "${GREEN}[+] Installing Dependencies...${NC}"
apt-get update

# Install Git first to ensure cloning works
apt-get install -y git

# Debian 12 dependencies for C++20 build
# Note: libmysqlclient-dev is replaced by libmariadb-dev-compat on Debian 12
apt-get install -y build-essential cmake gcc g++ make \
    libxml2-dev libssl-dev libpcre3-dev zlib1g-dev \
    libmariadb-dev-compat libmariadb-dev libreadline-dev \
    ant default-jdk

# 3. Clone Repository
REPO_DIR="/root/176source"
REPO_URL="https://github.com/code-bynary/176source.git"

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

# 5. Fix Permissions
chmod +x build.sh
chmod +x cnet/rpcgen

# 6. Build
echo -e "${GREEN}[+] Starting Build Process...${NC}"
# Use standard build script
./build.sh all

echo -e "${GREEN}[+] Setup and Build Finished!${NC}"
