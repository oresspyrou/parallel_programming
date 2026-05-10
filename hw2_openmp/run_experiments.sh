#!/bin/bash
# ============================================================
# run_experiments.sh
# Compiles all integration programs and measures average
# execution time over multiple runs for each configuration.
# Results are saved to results/results.csv.
# Usage: bash run_experiments.sh
# ============================================================

RUNS=20
A=0
B=3.14159265

# N values for A programs (uniform f — fast)
N_A=(100000 1000000 10000000)

# N values for B and C programs (non-uniform f — slower)
N_BC=(100000 1000000)

THREADS=(1 2 3 4)
CHUNKS=(100 1000 10000)
NUM_TASKS=(8 16 64)
MIN_SIZES=(1000 10000 100000)

RESULTS_FILE="results/results.csv"

mkdir -p bin results

# ============================================================
# Step 1: Compile all programs
# ============================================================
echo "=== Compiling ==="

compile() {
    local src=$1
    local out=$2
    g++ -fopenmp -o "$out" "$src" -lm
    if [ $? -eq 0 ]; then echo "  [OK] $src"
    else echo "  [FAIL] $src — aborting"; exit 1
    fi
}

compile src/integration_serial.cpp              bin/integration_serial
compile src/integration_A_noReduction.cpp       bin/integration_A_noReduction
compile src/integration_A_reduction.cpp         bin/integration_A_reduction
compile src/integration_B_static.cpp            bin/integration_B_static
compile src/integration_B_dynamic.cpp           bin/integration_B_dynamic
compile src/integration_B_guided.cpp            bin/integration_B_guided
compile src/integration_C_tasks.cpp             bin/integration_C_tasks
compile src/integration_C_tasks_recursive.cpp   bin/integration_C_tasks_recursive

# ============================================================
# Step 2: Helper — runs a command $RUNS times, returns average
# ============================================================
run_avg() {
    local total=0
    local count=0
    for i in $(seq 1 $RUNS); do
        t=$("$@" 2>/dev/null | grep "Execution time:" | awk '{print $3}')
        if [ -n "$t" ]; then
            total=$(awk "BEGIN {print $total + $t}")
            count=$((count + 1))
        fi
    done
    awk "BEGIN {printf \"%.6f\", $total / $count}"
}

# Appends one row to the CSV
save() {
    echo "$1,$2,$3,$4,$5,$6" >> "$RESULTS_FILE"
}

# ============================================================
# Step 3: Run experiments
# ============================================================
echo "program,n,threads,param_name,param_value,avg_time" > "$RESULTS_FILE"

echo ""
echo "=== Serial ==="
for N in "${N_A[@]}"; do
    avg=$(run_avg ./bin/integration_serial $A $B $N)
    echo "  N=$N  ->  $avg s"
    save "serial" $N 1 "-" "-" $avg
done

echo ""
echo "=== A: noReduction (critical section) ==="
for N in "${N_A[@]}"; do
    echo "  N=$N:"
    for t in "${THREADS[@]}"; do
        avg=$(run_avg ./bin/integration_A_noReduction $A $B $N $t)
        echo "    threads=$t  ->  $avg s"
        save "A_noReduction" $N $t "-" "-" $avg
    done
done

echo ""
echo "=== A: reduction ==="
for N in "${N_A[@]}"; do
    echo "  N=$N:"
    for t in "${THREADS[@]}"; do
        avg=$(run_avg ./bin/integration_A_reduction $A $B $N $t)
        echo "    threads=$t  ->  $avg s"
        save "A_reduction" $N $t "-" "-" $avg
    done
done

echo ""
echo "=== B: static scheduling ==="
for N in "${N_BC[@]}"; do
    echo "  N=$N:"
    for t in "${THREADS[@]}"; do
        for c in "${CHUNKS[@]}"; do
            avg=$(run_avg ./bin/integration_B_static $A 10 $N $t $c)
            echo "    threads=$t  chunk=$c  ->  $avg s"
            save "B_static" $N $t "chunk" $c $avg
        done
    done
done

echo ""
echo "=== B: dynamic scheduling ==="
for N in "${N_BC[@]}"; do
    echo "  N=$N:"
    for t in "${THREADS[@]}"; do
        for c in "${CHUNKS[@]}"; do
            avg=$(run_avg ./bin/integration_B_dynamic $A 10 $N $t $c)
            echo "    threads=$t  chunk=$c  ->  $avg s"
            save "B_dynamic" $N $t "chunk" $c $avg
        done
    done
done

echo ""
echo "=== B: guided scheduling ==="
for N in "${N_BC[@]}"; do
    echo "  N=$N:"
    for t in "${THREADS[@]}"; do
        for c in "${CHUNKS[@]}"; do
            avg=$(run_avg ./bin/integration_B_guided $A 10 $N $t $c)
            echo "    threads=$t  chunk=$c  ->  $avg s"
            save "B_guided" $N $t "chunk" $c $avg
        done
    done
done

echo ""
echo "=== C: tasks (no recursion) ==="
for t in "${THREADS[@]}"; do
    for nt in "${NUM_TASKS[@]}"; do
        avg=$(run_avg ./bin/integration_C_tasks $A 10 1000000 $t $nt)
        echo "  threads=$t  num_tasks=$nt  ->  $avg s"
        save "C_tasks" 1000000 $t "num_tasks" $nt $avg
    done
done

echo ""
echo "=== C: tasks recursive (divide and conquer) ==="
for t in "${THREADS[@]}"; do
    for ms in "${MIN_SIZES[@]}"; do
        avg=$(run_avg ./bin/integration_C_tasks_recursive $A 10 1000000 $t $ms)
        echo "  threads=$t  min_size=$ms  ->  $avg s"
        save "C_recursive" 1000000 $t "min_size" $ms $avg
    done
done

echo ""
echo "=== Done — results saved to $RESULTS_FILE ==="
