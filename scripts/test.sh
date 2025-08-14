#!/bin/bash
source "$(dirname "$0")/common.sh"

cd "$BUILD_DIR"

# Check if coverage is requested
COVERAGE=false
if [[ "$1" == "--coverage" ]]; then
    COVERAGE=true
    echo "Enabling coverage reporting..."
fi

# Build tests if needed
if [[ ! -f "yangep_tests" ]]; then
    echo "Building tests..."
    make
fi

# Run tests
echo "Running tests..."
if [[ "$COVERAGE" == true ]]; then
    # Run with coverage
    ./yangep_tests --gtest_output=xml:test_results.xml
    
    # Generate coverage report (if lcov is available)
    if command -v lcov &> /dev/null; then
        lcov --capture --directory . --output-file coverage.info
        lcov --remove coverage.info '/usr/*' --output-file coverage.info
        lcov --list coverage.info
    fi
else
    # Run without coverage
    ./yangep_tests
fi

# Check test exit code
TEST_EXIT_CODE=$?
if [[ $TEST_EXIT_CODE -eq 0 ]]; then
    echo "✅ All tests passed!"
else
    echo "❌ Some tests failed!"
    exit $TEST_EXIT_CODE
fi
