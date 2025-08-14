#!/bin/bash
source "$(dirname "$0")/common.sh"

if [[ "$1" != "major" && "$1" != "minor" && "$1" != "patch" ]]; then
    echo "Usage: $0 [major|minor|patch]"
    exit 1
fi

BRANCH_NAME=$(git rev-parse --abbrev-ref HEAD 2>/dev/null)
# if branch has / in its name, replace it with -
if [[ "$BRANCH_NAME" == *"/"* ]]; then
    BRANCH_NAME="${BRANCH_NAME//\//-}"
fi
# Get the short hash of the current commit
GIT_HASH=$(git rev-parse --short HEAD 2>/dev/null)

# Read the current version from the version file
if [[ -f "$VERSION_FILE" ]]; then
    current_version=$(cat "$VERSION_FILE")
else
    current_version="0.0.0-${BRANCH_NAME:-unknown}-${GIT_HASH:-unknown}"
fi
echo "Current version: $current_version"


# Split the version into its components
IFS='.-' read -r major minor patch branch git_hash <<< "$current_version"
# Increment the appropriate version component
case "$1" in
    major)
        major=$((major + 1))
        minor=0
        patch=0
        ;;
    minor)
        minor=$((minor + 1))
        patch=0
        ;;
    patch)
        patch=$((patch + 1))
        ;;
esac
# Construct the new version string
new_version="$major.$minor.$patch-${branch:-unknown}-${GIT_HASH:-unknown}"

echo "Updating version from $current_version to $new_version"

# Write the new version to the version file
echo "$new_version" > "$VERSION_FILE"
# write the new version to the versions file
if [[ -f "$VERSIONS_FILE" ]]; then
    echo "$new_version" >> "$VERSIONS_FILE"
else
    echo "$new_version" > "$VERSIONS_FILE"
fi
if [[ ! -d "$VERSION_DIR" ]]; then
    mkdir -p "$VERSION_DIR"
fi
# Create a new version directory if it doesn't exist
version_dir="$VERSION_DIR/$new_version"
if [[ ! -d "$version_dir" ]]; then
    mkdir -p "$version_dir"
fi
# Copy the executable to the new version directory
cp "$BUILD_DIR/$EXEC_NAME" "$version_dir/"
cp -r "$BUILD_DIR/assets" "$version_dir/assets"