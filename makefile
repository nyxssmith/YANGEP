.PHONY: all build run test test-coverage

all: build

build:
	@echo "Initializing build directory..."
	./scripts/make_build_dir.sh
	@echo "Building..."
	./scripts/build.sh

run: build
	@echo "Removing build assets..."
	./scripts/remove_build_assets.sh
	@echo "Copying fresh assets..."
	./scripts/copy_assets_to_build.sh
	@echo "Running "
	./scripts/run.sh

run_action_editor: build
	@echo "Removing build assets..."
	./scripts/remove_build_assets.sh
	@echo "Copying fresh assets..."
	./scripts/copy_assets_to_build.sh
	@echo "Running "
	./scripts/action_editor.sh

run_preserve_assets: build
	@echo "Running with preserved assets..."
	./scripts/run.sh

steamdeck: build
	@echo "Removing build assets..."
	./scripts/remove_build_assets.sh
	@echo "Copying fresh assets..."
	./scripts/copy_assets_to_build.sh
	@echo "Copying to Steam Deck..."
	./scripts/scp_steamdeck.sh
	


test: build
	@echo "Running tests..."
	./scripts/test.sh

test-coverage: build
	@echo "Running tests with coverage..."
	./scripts/test.sh --coverage

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

latest_core_dump:
	./scripts/get_latest_core_dump.sh