#!/bin/bash
# Fast build script - builds in container-local /tmp to avoid WSL I/O overhead

set -e

BUILD_DIR="/tmp/hvc-build"
WORKSPACE="/workspace"
DO_FLASH=0
COPY_BACK=1
MAKE_ARGS=()

# Parse arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        --flash)
            DO_FLASH=1
            COPY_BACK=0
            shift
            ;;
        --no-copy)
            COPY_BACK=0
            shift
            ;;
        *)
            MAKE_ARGS+=("$1")
            shift
            ;;
    esac
done

if [[ ${#MAKE_ARGS[@]} -eq 0 ]]; then
    MAKE_ARGS=("DEBUG=1")
fi

echo "==> Syncing to $BUILD_DIR..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
tar -C "$WORKSPACE" \
    --exclude='./build' \
    --exclude='./.git' \
    -cf - . | tar -C "$BUILD_DIR" -xf -

echo "==> Building in $BUILD_DIR..."
cd "$BUILD_DIR"

if [[ $DO_FLASH -eq 1 ]]; then
    make -j"$(nproc)" -f STM32Make.make flash "${MAKE_ARGS[@]}"
else
    make -j"$(nproc)" -f STM32Make.make "${MAKE_ARGS[@]}"
fi

if [[ $COPY_BACK -eq 1 ]]; then
    if [[ -d "$BUILD_DIR/build" ]]; then
        echo "==> Copying artifacts back..."
        rm -rf "$WORKSPACE/build"
        mkdir -p "$WORKSPACE/build"
        cp -a "$BUILD_DIR/build/." "$WORKSPACE/build/"
        echo "==> Done! Artifacts in $WORKSPACE/build/"
    elif [[ -d "$BUILD_DIR/debug" || -d "$BUILD_DIR/release" ]]; then
        echo "==> Copying artifacts back..."
        rm -rf "$WORKSPACE/build"
        mkdir -p "$WORKSPACE/build"
        if [[ -d "$BUILD_DIR/debug" ]]; then
            mkdir -p "$WORKSPACE/build/debug"
            cp -a "$BUILD_DIR/debug/." "$WORKSPACE/build/debug/"
        fi
        if [[ -d "$BUILD_DIR/release" ]]; then
            mkdir -p "$WORKSPACE/build/release"
            cp -a "$BUILD_DIR/release/." "$WORKSPACE/build/release/"
        fi
        echo "==> Done! Artifacts in $WORKSPACE/build/"
    else
        echo "==> No build artifacts to copy (did you run clean?)"
    fi
fi
