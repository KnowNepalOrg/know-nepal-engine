#!/bin/bash
set -e

# Benchmark runner for know-nepal-engine
# Runs TypeScript and C++ benchmarks, compares results

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ENGINE_DIR="$(dirname "$SCRIPT_DIR")"
PLATFORM_DIR="$ENGINE_DIR/../know-nepal-platform/know-nepal-frontend"

echo "=== Know Nepal Engine Benchmark ==="
echo ""

# Step 1: Extract TypeScript dataset
echo "[1/5] Extracting TypeScript dataset..."
cd "$SCRIPT_DIR"
npx tsx extract-index.ts
echo ""

# Step 2: Process with C++ index-builder
echo "[2/5] Processing dataset with C++ index-builder..."
cd "$ENGINE_DIR"
./build/index-builder.exe benchmark/dataset.json benchmark/processed.json
echo ""

# Step 3: Run TypeScript benchmark
echo "[3/5] Running TypeScript benchmark..."
cd "$SCRIPT_DIR"
npx tsx bench-ts.ts
echo ""

# Step 4: Run C++ benchmark
echo "[4/5] Running C++ benchmark..."
cd "$SCRIPT_DIR"
"$ENGINE_DIR/build/bench-cpp.exe" benchmark/processed.json
echo ""

# Step 5: Compare results
echo "[5/5] Comparing results..."
cd "$SCRIPT_DIR"
npx tsx compare.ts
echo ""

echo "=== Benchmark Complete ==="
echo ""
echo "Files generated:"
echo "  benchmark/dataset.json      - Raw TypeScript dataset"
echo "  benchmark/processed.json    - C++ processed index"
echo "  benchmark/ts_results.json   - TypeScript search results"
echo "  benchmark/cpp_results.json  - C++ search results"
echo "  benchmark/ts_timing.json    - TypeScript timing"
echo "  benchmark/cpp_timing.json   - C++ timing"
echo "  benchmark/parity_report.json - Parity comparison report"
