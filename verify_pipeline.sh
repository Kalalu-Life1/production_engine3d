#!/usr/bin/env bash

# Enterprise Safety Rule: Exit immediately if any command fails or has typos
set -euo pipefail

echo "=========================================================="
echo "[CI STAGE 1/4] Cleaning Previous Build Artifacts..."
echo "=========================================================="
rm -rf build/ output.txt

echo "=========================================================="
echo "[CI STAGE 2/4] Regenerating and Compiling Build Tree..."
echo "=========================================================="
cmake -B build
cmake --build build

echo "=========================================================="
echo "[CI STAGE 3/4] Running Automated Math Unit Tests..."
echo "=========================================================="
./build/RunTests

echo "=========================================================="
echo "[CI STAGE 4/4] Profiling Core Memory Stack with Valgrind..."
echo "=========================================================="
# Runs the tests through Valgrind to ensure zero memory leaks exist
valgrind --leak-check=full --error-exitcode=1 ./build/RunTests

echo "=========================================================="
echo "[SUCCESS] PIPELINE PASSED: 100% Accuracy Verified!"
echo "=========================================================="
