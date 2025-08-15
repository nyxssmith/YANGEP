.PHONY: all build run

all: build

build:
	@echo "Initializing build directory..."
	./scripts/make_build_dir.sh
	@echo "Building..."
	./scripts/build.sh

run: build
	@echo "Running "
	./scripts/run.sh

version: build
	@echo "Saving new patch..."
	./scripts/version.sh patch

minor_version: build
	@echo "Saving new minor version..."
	./scripts/version.sh minor

major_version: build
	@echo "Saving new major version..."
	./scripts/version.sh major

update_assests_from_debug_build:
	@echo "Updating assets from debug build..."
	./scripts/update_assets_from_debug_build.sh