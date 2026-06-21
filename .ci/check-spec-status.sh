#!/bin/bash
# check-spec-status.sh — SPEC-004 Compliance Checker
#
# For every spec in agents/Specs/ with `Status: IMPLEMENTED`, verifies
# that corresponding test files exist under Tests/ and that the full
# test binary compiles successfully.
#
# The spec-to-test mapping heuristic:
#   SPEC-NNN-GameAK-AreaName  →  test_area_name.h (included from test_runtime.cpp)
#
# Specs whose behaviour is verified entirely through test_runtime.cpp
# integration tests are listed in the built-in lookup table.

set -euo pipefail

SPEC_DIR="agents/Specs"
TEST_DIR="Tests"
TEST_RUNNER="$TEST_DIR/test_runtime.cpp"
FAILED=0

# ---- Specs covered directly by test_runtime.cpp integration tests ---------
INTEGRATION_SPECS="
    GameAK-Runtime
    GameAK-RuntimeAPI
    GameAK-DataBlocks
"

# ---- Manual overrides (spec-name → test-file stem) -----------------------
declare -A OVERRIDES
OVERRIDES["GameAK-PlatformArchitecture"]="platform_crtp"
OVERRIDES["GameAK-DataBlocks"]="data_layout"
OVERRIDES["GameAK-RuntimeAPI"]="runtime"
OVERRIDES["GameAK-RuntimeCommands"]="command"
OVERRIDES["GameAK-RuntimeIdentity"]="identity"
OVERRIDES["GameAK-ControllerAPI"]="controller"
OVERRIDES["GameAK-ErrorModel"]="error"
OVERRIDES["GameAK-EventSystem"]="event_loop"
OVERRIDES["GameAK-EventLoops"]="event_loop"
OVERRIDES["GameAK-BitRepresentation"]="bit_representation"
OVERRIDES["GameAK-FiniteStateMachines"]="fsm"
OVERRIDES["GameAK-RuleSystems"]="rule_system"
OVERRIDES["GameAK-SemanticStorageInference"]="semantic"
OVERRIDES["GameAK-RuntimePipelines"]="pipeline"
OVERRIDES["GameAK-EphemeralDataBlocks"]="ephemeral"
OVERRIDES["GameAK-AVLTree"]="avl_tree"
OVERRIDES["GameAK-RBTree"]="rb_tree"

# ---- Helpers --------------------------------------------------------------

die() { echo "FAIL: $*" >&2; FAILED=1; }

to_snake() {
    # Convert PascalCase to snake_case (e.g., "FlatVector" → "flat_vector")
    echo "$1" | sed 's/\([a-z0-9]\)\([A-Z]\)/\1_\2/g' | tr '[:upper:]' '[:lower:]'
}

test_file_exists() {
    local stem="$1"
    # Check for .h included from test_runtime.cpp
    if [ -f "$TEST_DIR/test_${stem}.h" ]; then
        if grep -q "test_${stem}.h" "$TEST_RUNNER" 2>/dev/null; then
            return 0
        fi
        die "$spec_id: test_${stem}.h exists but is not included from $TEST_RUNNER"
        return 1
    fi
    # Check for standalone .cpp
    if [ -f "$TEST_DIR/test_${stem}.cpp" ]; then
        return 0
    fi
    return 1
}

# ---- Main ----------------------------------------------------------------

echo "=== SPEC-004 Compliance Check ==="
echo ""

for spec_file in "$SPEC_DIR"/SPEC-*.md; do
    [ -f "$spec_file" ] || continue

    spec_id=$(basename "$spec_file" .md)
    status=$(grep "^Status:" "$spec_file" | sed 's/^Status:[[:space:]]*//')

    if [ "$status" != "IMPLEMENTED" ]; then
        echo "SKIP $spec_id — status is $status"
        continue
    fi

    # Extract spec title (text after "SPEC-NNN-")
    spec_name="${spec_id#SPEC-*-}"

    # Check if it's an integration-only spec
    if echo "$INTEGRATION_SPECS" | grep -qw "$spec_name"; then
        echo "OK   $spec_id ($spec_name) — covered by integration tests in $TEST_RUNNER"
        continue
    fi

    # Check overrides first
    stem=""
    if [ "${OVERRIDES[$spec_name]+exists}" ]; then
        stem="${OVERRIDES[$spec_name]}"
    else
        # Strip "GameAK-" prefix and convert
        area="${spec_name#GameAK-}"
        stem="$(to_snake "$area")"
    fi

    if test_file_exists "$stem"; then
        echo "OK   $spec_id ($spec_name) → test_${stem}.h"
    else
        die "$spec_id ($spec_name): no test file found (expected test_${stem}.h or test_${stem}.cpp)"
    fi
done

echo ""

# ---- Verify the test binary compiles -------------------------------------
echo "=== Verifying test compilation ==="
if make -q test 2>/dev/null; then
    echo "OK   test binary is up to date"
else
    echo "INFO rebuilding test binary..."
    if make test 2>&1; then
        echo "OK   test binary compiles successfully"
    else
        die "test binary failed to compile"
    fi
fi

echo ""

if [ $FAILED -eq 0 ]; then
    echo "VERDICT: All IMPLEMENTED specs have test coverage."
else
    echo "VERDICT: FAILED — some specs lack test coverage (see above)."
fi

exit $FAILED
