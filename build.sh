#!/bin/bash

# Master Mind Trading System Build Script
# Supports Linux, macOS, and Windows (with WSL/MSYS2)

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
PROJECT_NAME="MasterMindTradingSystem"
BUILD_TYPE="${BUILD_TYPE:-Release}"
BUILD_DIR="${BUILD_DIR:-build}"
INSTALL_PREFIX="${INSTALL_PREFIX:-/usr/local}"
NUM_CORES=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

print_header() {
    echo -e "${BLUE}╔═══════════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${BLUE}║                 MASTER MIND TRADING SYSTEM BUILD                  ║${NC}"
    echo -e "${BLUE}║                                                                   ║${NC}"
    echo -e "${BLUE}║           High-Frequency Cross-Asset Trading Platform            ║${NC}"
    echo -e "${BLUE}╚═══════════════════════════════════════════════════════════════════╝${NC}"
    echo ""
}

print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
    exit 1
}

detect_os() {
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        OS="linux"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        OS="macos"
    elif [[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]]; then
        OS="windows"
    else
        print_error "Unsupported operating system: $OSTYPE"
    fi
    print_info "Detected OS: $OS"
}

check_dependencies() {
    print_info "Checking dependencies..."
    
    # Check for required tools
    command -v cmake >/dev/null 2>&1 || print_error "CMake is required but not installed"
    command -v make >/dev/null 2>&1 || command -v ninja >/dev/null 2>&1 || print_error "Make or Ninja is required but not installed"
    
    # Check for compiler
    if command -v g++ >/dev/null 2>&1; then
        COMPILER="g++"
        print_info "Using compiler: g++ $(g++ --version | head -n1 | cut -d' ' -f4)"
    elif command -v clang++ >/dev/null 2>&1; then
        COMPILER="clang++"
        print_info "Using compiler: clang++ $(clang++ --version | head -n1 | cut -d' ' -f4)"
    else
        print_error "No suitable C++ compiler found (g++ or clang++)"
    fi
    
    # Check CMake version
    CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
    print_info "CMake version: $CMAKE_VERSION"
    
    if [[ $(echo "$CMAKE_VERSION 3.16" | tr " " "\n" | sort -V | head -n1) != "3.16" ]]; then
        print_error "CMake 3.16 or higher is required, found: $CMAKE_VERSION"
    fi
}

install_dependencies() {
    print_info "Installing dependencies for $OS..."
    
    case $OS in
        "linux")
            if command -v apt-get >/dev/null 2>&1; then
                # Debian/Ubuntu
                print_info "Installing dependencies with apt..."
                sudo apt-get update
                sudo apt-get install -y build-essential cmake libssl-dev libsqlite3-dev libboost-all-dev pkg-config
            elif command -v yum >/dev/null 2>&1; then
                # RHEL/CentOS
                print_info "Installing dependencies with yum..."
                sudo yum groupinstall -y "Development Tools"
                sudo yum install -y cmake openssl-devel sqlite-devel boost-devel pkg-config
            elif command -v dnf >/dev/null 2>&1; then
                # Fedora
                print_info "Installing dependencies with dnf..."
                sudo dnf groupinstall -y "Development Tools"
                sudo dnf install -y cmake openssl-devel sqlite-devel boost-devel pkg-config
            else
                print_warning "Unknown Linux distribution. Please install dependencies manually."
            fi
            ;;
        "macos")
            if command -v brew >/dev/null 2>&1; then
                print_info "Installing dependencies with Homebrew..."
                brew install cmake openssl sqlite boost pkg-config
            else
                print_error "Homebrew is required on macOS. Install from https://brew.sh/"
            fi
            ;;
        "windows")
            print_warning "Windows dependencies must be installed manually"
            print_info "Consider using vcpkg: vcpkg install openssl sqlite3 boost-system boost-thread boost-chrono"
            ;;
    esac
}

create_build_directory() {
    print_info "Creating build directory: $BUILD_DIR"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
}

configure_project() {
    print_info "Configuring project with CMake..."
    print_info "Build type: $BUILD_TYPE"
    print_info "Install prefix: $INSTALL_PREFIX"
    
    CMAKE_ARGS=(
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
        -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX"
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    )
    
    # Add platform-specific arguments
    case $OS in
        "macos")
            # Find OpenSSL installed by Homebrew
            if brew --prefix openssl >/dev/null 2>&1; then
                OPENSSL_ROOT=$(brew --prefix openssl)
                CMAKE_ARGS+=(-DOPENSSL_ROOT_DIR="$OPENSSL_ROOT")
            fi
            ;;
        "windows")
            # Use vcpkg if available
            if [[ -n "$VCPKG_ROOT" ]]; then
                CMAKE_ARGS+=(-DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake")
            fi
            ;;
    esac
    
    cmake "${CMAKE_ARGS[@]}" ..
}

build_project() {
    print_info "Building project with $NUM_CORES cores..."
    
    if command -v ninja >/dev/null 2>&1 && [[ -f "build.ninja" ]]; then
        ninja
    else
        make -j"$NUM_CORES"
    fi
}

create_directories() {
    print_info "Creating necessary directories..."
    mkdir -p ../logs
    mkdir -p ../database
    mkdir -p ../reports
    mkdir -p ../config/examples
}

setup_permissions() {
    if [[ "$OS" != "windows" ]]; then
        print_info "Setting up permissions..."
        chmod +x MasterMindTrader 2>/dev/null || true
        chmod +x MasterMindTest 2>/dev/null || true
    fi
}

run_tests() {
    if [[ "$1" == "--test" ]]; then
        print_info "Running system tests..."
        if [[ -f "MasterMindTest" ]]; then
            ./MasterMindTest
        else
            print_warning "Test executable not found. Skipping tests."
        fi
    fi
}

show_completion() {
    print_info "Build completed successfully!"
    echo ""
    print_info "Executables:"
    echo "  - MasterMindTrader (main application)"
    echo "  - MasterMindTest (test runner)"
    echo ""
    print_info "Next steps:"
    echo "  1. Review configuration: config/mastermind_config.json"
    echo "  2. Set up exchange API credentials (testnet recommended)"
    echo "  3. Run tests: ./MasterMindTrader -t"
    echo "  4. Start in paper mode: ./MasterMindTrader -p"
    echo ""
    print_warning "IMPORTANT: Never run live trading without thorough testing!"
}

# Main execution
main() {
    print_header
    
    # Parse arguments
    INSTALL_DEPS=false
    RUN_TESTS=false
    CLEAN_BUILD=false
    
    for arg in "$@"; do
        case $arg in
            --deps)
                INSTALL_DEPS=true
                ;;
            --test)
                RUN_TESTS=true
                ;;
            --clean)
                CLEAN_BUILD=true
                ;;
            --help)
                echo "Usage: $0 [OPTIONS]"
                echo ""
                echo "Options:"
                echo "  --deps    Install system dependencies"
                echo "  --test    Run tests after building"
                echo "  --clean   Clean build directory first"
                echo "  --help    Show this help message"
                echo ""
                echo "Environment variables:"
                echo "  BUILD_TYPE       Build type (Debug|Release|RelWithDebInfo) [default: Release]"
                echo "  BUILD_DIR        Build directory [default: build]"
                echo "  INSTALL_PREFIX   Install prefix [default: /usr/local]"
                exit 0
                ;;
        esac
    done
    
    # Detect OS and check dependencies
    detect_os
    check_dependencies
    
    # Install dependencies if requested
    if [[ "$INSTALL_DEPS" == true ]]; then
        install_dependencies
    fi
    
    # Clean build if requested
    if [[ "$CLEAN_BUILD" == true && -d "$BUILD_DIR" ]]; then
        print_info "Cleaning build directory..."
        rm -rf "$BUILD_DIR"
    fi
    
    # Build process
    create_build_directory
    configure_project
    build_project
    create_directories
    setup_permissions
    
    # Run tests if requested
    if [[ "$RUN_TESTS" == true ]]; then
        run_tests --test
    fi
    
    show_completion
}

# Execute main function
main "$@" 