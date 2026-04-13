#!/bin/bash
#
# Build script for JSN-SR04T Renode Plugin
#
# Prerequisites:
# 1. .NET 8.0 SDK installed
# 2. Renode source code cloned and built
#
# Usage:
#   ./build.sh [path/to/renode]
#

set -e

# Configuration
PLUGIN_NAME="JSN_SR04T_Plugin"
PROJECT_FILE="${PLUGIN_NAME}.csproj"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== JSN-SR04T Renode Plugin Build Script ===${NC}"

# Check for .NET SDK
if ! command -v dotnet &> /dev/null; then
    echo -e "${RED}Error: .NET SDK not found${NC}"
    echo "Please install .NET 8.0 SDK from: https://dotnet.microsoft.com/download"
    exit 1
fi

DOTNET_VERSION=$(dotnet --version)
echo "Found .NET version: $DOTNET_VERSION"

# Find Renode
RENODE_PATH="${1:-}"

if [ -z "$RENODE_PATH" ]; then
    # Try common locations
    if [ -d "$HOME/Projects/renode" ]; then
        RENODE_PATH="$HOME/Projects/renode"
    elif [ -d "../renode" ]; then
        RENODE_PATH="../renode"
    elif [ -d "/opt/renode" ]; then
        RENODE_PATH="/opt/renode"
    fi
fi

if [ -z "$RENODE_PATH" ] || [ ! -d "$RENODE_PATH" ]; then
    echo -e "${YELLOW}Warning: Renode path not specified or not found${NC}"
    echo "Please provide path to Renode source/build directory:"
    echo "  ./build.sh /path/to/renode"
    echo ""
    echo "Alternatively, you can:"
    echo "1. Clone Renode: git clone https://github.com/renode/renode"
    echo "2. Build Renode: cd renode && ./build.sh"
    echo ""
    
    # Try to build without Renode references (for syntax checking)
    echo "Attempting build without Renode references..."
    dotnet build "$PROJECT_FILE" -c Release --no-restore 2>/dev/null || {
        echo -e "${RED}Build failed. Please provide Renode path.${NC}"
        exit 1
    }
else
    echo "Using Renode from: $RENODE_PATH"
    
    # Check for Renode build output
    RENODE_BIN="$RENODE_PATH/bin"
    if [ ! -d "$RENODE_BIN" ]; then
        echo -e "${YELLOW}Warning: Renode bin directory not found at $RENODE_BIN${NC}"
        echo "Building Renode first..."
        (cd "$RENODE_PATH" && ./build.sh) || {
            echo -e "${RED}Failed to build Renode${NC}"
            exit 1
        }
    fi
    
    # Update csproj with correct paths
    sed -i.bak "s|\${RENODE_PATH}|$RENODE_PATH|g" "$PROJECT_FILE"
    
    # Build
    echo "Building plugin..."
    dotnet build "$PROJECT_FILE" -c Release
    
    # Restore original csproj
    mv "$PROJECT_FILE.bak" "$PROJECT_FILE" 2>/dev/null || true
fi

# Output
OUTPUT_DIR="bin/Release/net8.0"
if [ -d "$OUTPUT_DIR" ]; then
    echo ""
    echo -e "${GREEN}Build successful!${NC}"
    echo "Output: $OUTPUT_DIR/${PLUGIN_NAME}.dll"
    echo ""
    echo "To use the plugin:"
    echo "1. Copy to Renode's plugin directory:"
    echo "   cp $OUTPUT_DIR/${PLUGIN_NAME}.dll \$RENODE_PATH/bin/"
    echo ""
    echo "2. Or load via Python extension (recommended):"
    echo "   emulation LoadPythonExtension \"python/sensor_helper.py\""
else
    echo -e "${RED}Build output not found${NC}"
    exit 1
fi
