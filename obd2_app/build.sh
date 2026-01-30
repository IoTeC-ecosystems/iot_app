#!/bin/bash
#
# Build and test OBD II module
# Usage: ./build.sh [example|simple|clean]
#

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR/.."

COLOR_GREEN='\033[0;32m'
COLOR_RED='\033[0;31m'
COLOR_YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_header() {
    echo -e "${COLOR_YELLOW}=== $1 ===${NC}"
}

print_success() {
    echo -e "${COLOR_GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${COLOR_RED}✗ $1${NC}"
}

# Clean build
if [ "$1" = "clean" ]; then
    print_header "Cleaning build"
    make clean
    rm -rf bin/
    print_success "Cleaned"
    exit 0
fi

# Check prerequisites
print_header "Checking prerequisites"

if ! command -v gcc &> /dev/null; then
    print_error "gcc not found"
    exit 1
fi
print_success "gcc found"

if ! command -v make &> /dev/null; then
    print_error "make not found"
    exit 1
fi
print_success "make found"

# Build OBD II module
print_header "Building OBD II Module"

if [ -z "$1" ] || [ "$1" = "example" ]; then
    print_header "Building example_main.c"
    make all EXTERNAL_MODULE_DIRS=$(pwd) USEMODULE=obd2_app 2>&1 | tail -20
    if [ $? -eq 0 ]; then
        print_success "Build successful"
        echo ""
        echo "Run with:"
        echo "  ./bin/native64/iot_app.elf"
    else
        print_error "Build failed"
        exit 1
    fi
fi

# Build simple test
if [ "$1" = "simple" ]; then
    print_header "Building simple_test.c"
    # Would need a separate build config - just show instructions
    echo "To build simple_test.c:"
    echo "  gcc -I./obd2_app/include -o test_obd2 obd2_app/simple_test.c obd2_app/obd2.c"
    echo ""
    echo "Or modify main.c to use simple_test.c instead"
fi

echo ""
echo "For troubleshooting, see obd2_app/README.md"
