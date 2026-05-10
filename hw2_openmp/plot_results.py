import csv
import os
import matplotlib.pyplot as plt
from collections import defaultdict

os.makedirs("results", exist_ok=True)

# ============================================================
# Load CSV produced by run_experiments.sh
# ============================================================
data = defaultdict(list)

with open("results/results.csv", newline="") as f:
    reader = csv.DictReader(f)
    for row in reader:
        key = (
            row["program"],
            int(row["n"]),
            int(row["threads"]),
            row["param_name"],
            row["param_value"],
        )
        data[key].append(float(row["avg_time"]))

def get_time(program, n, threads, param_name="-", param_value="-"):
    key = (program, n, threads, param_name, param_value)
    vals = data.get(key)
    return vals[0] if vals else None

threads = [1, 2, 3, 4]

# ============================================================
# Plot 1 — A: Execution time vs Threads (N=1M)
# ============================================================
N = 1000000
serial_t = get_time("serial", N, 1)

noRed = [get_time("A_noReduction", N, t) for t in threads]
red   = [get_time("A_reduction",   N, t) for t in threads]

plt.figure(figsize=(9, 5))
plt.plot(threads, noRed, marker='o', label='A noReduction (critical)')
plt.plot(threads, red,   marker='s', label='A reduction')
plt.axhline(y=serial_t, color='black', linestyle='--', label='Serial')
plt.title('A: Χρόνος Εκτέλεσης vs Threads (N=1M, f=sin(x))')
plt.xlabel('Αριθμός Threads')
plt.ylabel('Χρόνος (seconds)')
plt.xticks(threads)
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('results/plot1_A_time.png', dpi=150)
plt.close()
print("Saved: results/plot1_A_time.png")

# ============================================================
# Plot 2 — A: Speedup vs Threads (N=1M)
# ============================================================
def speedup(serial, times):
    return [serial / t if t else None for t in times]

plt.figure(figsize=(9, 5))
plt.plot(threads, [1, 2, 3, 4], linestyle='--', color='black', label='Ιδανικό')
plt.plot(threads, speedup(serial_t, noRed), marker='o', label='A noReduction')
plt.plot(threads, speedup(serial_t, red),   marker='s', label='A reduction')
plt.title('A: Speedup vs Threads (N=1M)')
plt.xlabel('Αριθμός Threads')
plt.ylabel('Speedup')
plt.xticks(threads)
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('results/plot2_A_speedup.png', dpi=150)
plt.close()
print("Saved: results/plot2_A_speedup.png")

# ============================================================
# Plot 3 — A: Επίδραση N στο Speedup (reduction)
# ============================================================
plt.figure(figsize=(9, 5))
plt.plot(threads, [1, 2, 3, 4], linestyle='--', color='black', label='Ιδανικό')
for n_val in [100000, 1000000, 10000000]:
    s_t = get_time("serial", n_val, 1)
    times = [get_time("A_reduction", n_val, t) for t in threads]
    if all(times):
        sp = speedup(s_t, times)
        plt.plot(threads, sp, marker='o', label=f'N={n_val:,}')
plt.title('A reduction: Speedup για διαφορετικά N')
plt.xlabel('Αριθμός Threads')
plt.ylabel('Speedup')
plt.xticks(threads)
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('results/plot3_A_speedup_N.png', dpi=150)
plt.close()
print("Saved: results/plot3_A_speedup_N.png")

# ============================================================
# Plot 4 — B: static vs dynamic vs guided (N=1M, chunk=1000)
# ============================================================
N = 1000000
chunk = "1000"
b_static  = [get_time("B_static",  N, t, "chunk", chunk) for t in threads]
b_dynamic = [get_time("B_dynamic", N, t, "chunk", chunk) for t in threads]
b_guided  = [get_time("B_guided",  N, t, "chunk", chunk) for t in threads]
serial_1t = get_time("B_static", N, 1, "chunk", chunk)

plt.figure(figsize=(9, 5))
plt.plot(threads, b_static,  marker='o', label='static')
plt.plot(threads, b_dynamic, marker='s', label='dynamic')
plt.plot(threads, b_guided,  marker='^', label='guided')
plt.axhline(y=serial_1t, color='black', linestyle='--', label='1 thread (baseline)')
plt.title(f'B: static vs dynamic vs guided (N=1M, chunk={chunk})')
plt.xlabel('Αριθμός Threads')
plt.ylabel('Χρόνος (seconds)')
plt.xticks(threads)
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('results/plot4_B_schedules.png', dpi=150)
plt.close()
print("Saved: results/plot4_B_schedules.png")

# ============================================================
# Plot 5 — B: Επίδραση chunk_size (static, N=1M, 4 threads)
# ============================================================
N = 1000000
t = 4
chunks = ["100", "1000", "10000"]
times_chunks = [get_time("B_static", N, t, "chunk", c) for c in chunks]

plt.figure(figsize=(9, 5))
plt.bar(chunks, times_chunks, color=['steelblue', 'darkorange', 'green'])
plt.title(f'B static: Επίδραση chunk_size (N=1M, threads=4)')
plt.xlabel('chunk_size')
plt.ylabel('Χρόνος (seconds)')
plt.grid(True, axis='y')
plt.tight_layout()
plt.savefig('results/plot5_B_chunksize.png', dpi=150)
plt.close()
print("Saved: results/plot5_B_chunksize.png")

# ============================================================
# Plot 6 — C: tasks vs recursive (χρόνος vs threads, N=1M)
# ============================================================
nt = "16"
ms = "10000"
c_tasks = [get_time("C_tasks",     1000000, t, "num_tasks", nt) for t in threads]
c_rec   = [get_time("C_recursive", 1000000, t, "min_size",  ms) for t in threads]

plt.figure(figsize=(9, 5))
plt.plot(threads, c_tasks, marker='o', label=f'C tasks (num_tasks={nt})')
plt.plot(threads, c_rec,   marker='s', label=f'C recursive (min_size={ms})')
plt.title('C: Tasks vs Recursive — Χρόνος vs Threads (N=1M)')
plt.xlabel('Αριθμός Threads')
plt.ylabel('Χρόνος (seconds)')
plt.xticks(threads)
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('results/plot6_C_tasks.png', dpi=150)
plt.close()
print("Saved: results/plot6_C_tasks.png")

# ============================================================
# Plot 7 — C recursive: Επίδραση min_size (4 threads)
# ============================================================
min_sizes = ["1000", "10000", "100000"]
times_ms = [get_time("C_recursive", 1000000, 4, "min_size", ms) for ms in min_sizes]

plt.figure(figsize=(9, 5))
plt.bar(min_sizes, times_ms, color=['steelblue', 'darkorange', 'green'])
plt.title('C recursive: Επίδραση min_size (N=1M, threads=4)')
plt.xlabel('min_size')
plt.ylabel('Χρόνος (seconds)')
plt.grid(True, axis='y')
plt.tight_layout()
plt.savefig('results/plot7_C_minsize.png', dpi=150)
plt.close()
print("Saved: results/plot7_C_minsize.png")

print("\nΌλα τα γραφήματα αποθηκεύτηκαν στο results/")
