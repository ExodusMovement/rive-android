#!/bin/bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
NPM_PACKAGE_DIR="$PROJECT_ROOT/exodus-rive-android-runtime"
MODULE="kotlin"
AAR_FILENAME="rive-android-runtime.aar"

DO_CLEAN=true
DO_PUBLISH=true

# Parse flags
for arg in "$@"; do
  case $arg in
    --no-clean)
      DO_CLEAN=false
      shift
      ;;
    --no-publish)
      DO_PUBLISH=false
      shift
      ;;
    *)
      echo "❓ Unknown option: $arg"
      echo "Usage: $0 [--no-clean] [--no-publish]"
      exit 1
      ;;
  esac
done

# 1. Update submodule
echo "🔄 Updating rive-runtime submodule..."
git submodule update --remote -- submodules/rive-runtime

# 2. Clean & build
cd "$PROJECT_ROOT"
if [ "$DO_CLEAN" = true ]; then
  echo "🧹 Cleaning Gradle build..."
  ./gradlew clean
else
  echo "⚡ Skipping clean"
fi

echo "🚀 Building rive-android $MODULE module..."
./gradlew ":$MODULE:assembleRelease"

OUTPUT_AAR="$PROJECT_ROOT/$MODULE/build/outputs/aar/${MODULE}-release.aar"
DEST_DIR="$NPM_PACKAGE_DIR/android/libs"
DEST_AAR="$DEST_DIR/$AAR_FILENAME"

if [ ! -f "$OUTPUT_AAR" ]; then
  echo "❌ Build failed: $OUTPUT_AAR not found."
  exit 1
fi

# 3. Copy AAR to npm package
echo "📦 Copying built AAR..."
mkdir -p "$DEST_DIR"
rm -f "$DEST_AAR"
cp -f "$OUTPUT_AAR" "$DEST_AAR"

echo "✅ Done! Replaced with fresh build: $DEST_AAR"

# 3.5. Print SHA256 hash
echo "🔑 SHA256 of built AAR:"
shasum -a 256 "$DEST_AAR"

# 4. Handle version bump only if publishing
if [ "$DO_PUBLISH" = true ]; then
  BASE_VERSION=$(cat "$PROJECT_ROOT/VERSION")   # e.g. 10.4.2
  PKG_JSON="$NPM_PACKAGE_DIR/package.json"
  CURRENT_VERSION=$(node -p "require('$PKG_JSON').version")

  if [[ "$CURRENT_VERSION" =~ ^${BASE_VERSION}-exodus\.([0-9]+)$ ]]; then
    SUFFIX=${BASH_REMATCH[1]}
    NEXT_SUFFIX=$((SUFFIX + 1))
  else
    NEXT_SUFFIX=0
  fi

  NEW_VERSION="${BASE_VERSION}-exodus.${NEXT_SUFFIX}"

  echo "🔢 Current npm version: $CURRENT_VERSION"
  echo "➡️  Next npm version: $NEW_VERSION"

  npm version --no-git-tag-version "$NEW_VERSION" --prefix "$NPM_PACKAGE_DIR"

  # 5. Publish npm package
  echo "🎉 Publishing npm package..."
  cd "$NPM_PACKAGE_DIR"
  npm publish --access public
  echo "✅ Published @exodus/rive-android-runtime@$NEW_VERSION"
else
  echo "⚡ Skipping npm publish and package.json version bump"
fi
