#!/bin/bash
# ============================================================
# run_experiments.sh
# Compiles all integration programs and measures average
# execution time over multiple runs for each configuration.
# Usage: bash run_experiments.sh
# ============================================================

RUNS=20             # number of runs per configuration
A=0                 # lower bound of integration
B=3.14159265        # upper bound (pi)
N=1000000           # number of trapezoids
THREADS=(1 2 3 4)   # thread counts to test
CHUNKS=(10 100 1000) # chunk sizes to test for dynamic scheduling (C)

# ============================================================
# Step 1: Compile all programs
# ============================================================
echo "=== Compiling all programs ==="

compile() {
    local src=$1
    local out=$2
    g++ -o "$out" "$src" -lm -pthread
    if [ $? -eq 0 ]; then
        echo "  [OK] $src"
    else
        echo "  [FAIL] $src — aborting"
        exit 1
    fi
}

compile integration_serial.cpp          integration_serial
compile integration_A_noLock.cpp        integration_A_noLock
compile integration_A_lock.cpp          integration_A_lock
compile integration_A_progressive.cpp   integration_A_progressive
compile integration_B.cpp               integration_B
compile integration_B_ChangedFunc.cpp   integration_B_ChangedFunc
compile integration_C.cpp               integration_C
compile integration_C_ChangedFunc.cpp   integration_C_ChangedFunc

# ============================================================
# Step 2: Helper — runs a command $RUNS times, returns average
# ============================================================
# Extracts the number after "Time:" from each run and averages them.
run_avg() {
    local total=0
    local count=0
    for i in $(seq 1 $RUNS); do
        # Run the program, keep only the "Time: X.XXXXXX seconds" line
        t=$("$@" 2>/dev/null | grep "^Time:" | awk '{print $2}')
        if [ -n "$t" ]; then
            total=$(awk "BEGIN {print $total + $t}")
            count=$((count + 1))
        fi
    done
    awk "BEGIN {printf \"%.6f\", $total / $count}"
}

# ============================================================
# Step 3: Run experiments
# ============================================================
echo ""
echo "=== Results (average of $RUNS runs, N=$N, a=$A, b=$B) ==="
echo ""

# --- Serial ---
echo "--- Serial ---"
avg=$(run_avg ./integration_serial $A $B $N)
echo "  avg time: $avg seconds"
echo ""

# --- A: Static Block, No Lock ---
echo "--- A: Static Block (No Lock) ---"
for t in "${THREADS[@]}"; do
    avg=$(run_avg ./integration_A_noLock $A $B $N $t)
    echo "  threads=$t  ->  $avg seconds"
done
echo ""

# --- A: Static Block, With Lock ---
echo "--- A: Static Block (With Lock) ---"
for t in "${THREADS[@]}"; do
    avg=$(run_avg ./integration_A_lock $A $B $N $t)
    echo "  threads=$t  ->  $avg seconds"
done
echo ""

# --- A: Progressive (With Lock) ---
# Intermediate prints are suppressed (redirected to /dev/null) for clean timing
echo "--- A: Progressive (With Lock) ---"
for t in "${THREADS[@]}"; do
    avg=$(run_avg ./integration_A_progressive $A $B $N $t)
    echo "  threads=$t  ->  $avg seconds"
done
echo ""

# --- B: Cyclic (Interleaved), uniform f(x) ---
echo "--- B: Cyclic / Interleaved (uniform f) ---"
for t in "${THREADS[@]}"; do
    avg=$(run_avg ./integration_B $A $B $N $t)
    echo "  threads=$t  ->  $avg seconds"
done
echo ""

# --- B: Cyclic (Interleaved), non-uniform f(x) ---
echo "--- B: Cyclic / Interleaved (non-uniform f) ---"
for t in "${THREADS[@]}"; do
    avg=$(run_avg ./integration_B_ChangedFunc $A $B $N $t)
    echo "  threads=$t  ->  $avg seconds"
done
echo ""

# --- C: Dynamic Scheduling, uniform f(x) ---
echo "--- C: Dynamic Scheduling (uniform f) ---"
for t in "${THREADS[@]}"; do
    for c in "${CHUNKS[@]}"; do
        avg=$(run_avg ./integration_C $A $B $N $t $c)
        echo "  threads=$t  chunk=$c  ->  $avg seconds"
    done
done
echo ""

# --- C: Dynamic Scheduling, non-uniform f(x) ---
echo "--- C: Dynamic Scheduling (non-uniform f) ---"
for t in "${THREADS[@]}"; do
    for c in "${CHUNKS[@]}"; do
        avg=$(run_avg ./integration_C_ChangedFunc $A $B $N $t $c)
        echo "  threads=$t  chunk=$c  ->  $avg seconds"
    done
done
echo ""

echo "=== Done ==="
