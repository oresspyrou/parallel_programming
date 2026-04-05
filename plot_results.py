import matplotlib.pyplot as plt
import numpy as np

import os
os.makedirs('results', exist_ok=True)
# ============================================================
# Δεδομένα από run_experiments.sh
# ============================================================

threads = [1, 2, 3, 4]
serial_time = 0.014329  

# Χρόνοι εκτέλεσης (seconds) — μέσος όρος 20 runs
A_nolock  = [0.014602, 0.009351, 0.007481, 0.006145]
A_lock    = [0.013500, 0.008814, 0.007219, 0.006064]
A_prog    = [0.013532, 0.008888, 0.007069, 0.006231]
B_uniform = [0.014365, 0.009353, 0.007354, 0.005827]
B_nonunif = [1.854665, 0.927860, 0.621805, 0.474907]

# Speedup = serial / parallel
# Για uniform f(x) το serial είναι 0.014329
# Για non-uniform το serial είναι το 1-thread του B
speedup_A_nolock  = [serial_time / t for t in A_nolock]
speedup_A_lock    = [serial_time / t for t in A_lock]
speedup_B_uniform = [serial_time / t for t in B_uniform]
speedup_B_nonunif = [B_nonunif[0] / t for t in B_nonunif]

# C: Dynamic Scheduling — uniform f(x)
C_chunk10   = [0.017314, 0.013345, 0.030554, 0.034285]
C_chunk100  = [0.013907, 0.009097, 0.008497, 0.007273]
C_chunk1000 = [0.013690, 0.009755, 0.009457, 0.005354]

# C: Dynamic Scheduling — non-uniform f(x)
C_nu_chunk10   = [1.888505, 0.972410, 0.722940, 0.520165]
C_nu_chunk100  = [1.947215, 0.975370, 0.663865, 0.505145]
C_nu_chunk1000 = [1.876965, 0.982820, 0.684815, 0.509950]

# ============================================================
# Γράφημα 1 — Χρόνος εκτέλεσης vs Threads (uniform f)
# ============================================================
plt.figure(figsize=(9, 5))

plt.plot(threads, A_nolock,  marker='o', label='A nolock')
plt.plot(threads, A_lock,    marker='s', label='A lock')
plt.plot(threads, A_prog,    marker='^', label='A progressive')
plt.plot(threads, B_uniform, marker='D', label='B interleaved')
plt.axhline(y=serial_time, color='black', linestyle='--', label='Serial')

plt.title('Χρόνος Εκτέλεσης vs Αριθμός Threads (uniform f)')
plt.xlabel('Αριθμός Threads')
plt.ylabel('Χρόνος (seconds)')
plt.xticks(threads)
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('results/plot1_time_uniform.png', dpi=150)
plt.close()
print("Αποθηκεύτηκε: results/plot1_time_uniform.png")

# ============================================================
# Γράφημα 2 — Speedup vs Threads
# ============================================================
plt.figure(figsize=(9, 5))

ideal = [1, 2, 3, 4]  # ιδανικό linear speedup
plt.plot(threads, ideal,            linestyle='--', color='black', label='Ιδανικό speedup')
plt.plot(threads, speedup_A_nolock,  marker='o', label='A nolock')
plt.plot(threads, speedup_A_lock,    marker='s', label='A lock')
plt.plot(threads, speedup_B_uniform, marker='D', label='B interleaved')
plt.plot(threads, speedup_B_nonunif, marker='^', label='B non-uniform')

plt.title('Speedup vs Αριθμός Threads')
plt.xlabel('Αριθμός Threads')
plt.ylabel('Speedup')
plt.xticks(threads)
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('results/plot2_speedup.png', dpi=150)
plt.close()
print("Αποθηκεύτηκε: results/plot2_speedup.png")

# ============================================================
# Γράφημα 3 — Επίδραση chunk_size στο C (uniform f)
# ============================================================
plt.figure(figsize=(9, 5))

plt.plot(threads, C_chunk10,   marker='o', label='chunk=10')
plt.plot(threads, C_chunk100,  marker='s', label='chunk=100')
plt.plot(threads, C_chunk1000, marker='^', label='chunk=1000')
plt.axhline(y=serial_time, color='black', linestyle='--', label='Serial')

plt.title('C Dynamic: Επίδραση chunk_size vs Threads (uniform f)')
plt.xlabel('Αριθμός Threads')
plt.ylabel('Χρόνος (seconds)')
plt.xticks(threads)
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('results/plot3_chunk_size.png', dpi=150)
plt.close()
print("Αποθηκεύτηκε: results/plot3_chunk_size.png")

# ============================================================
# Γράφημα 4 — Non-uniform: B vs C
# ============================================================
plt.figure(figsize=(9, 5))

plt.plot(threads, B_nonunif,     marker='o', label='B interleaved')
plt.plot(threads, C_nu_chunk10,  marker='s', label='C chunk=10')
plt.plot(threads, C_nu_chunk100, marker='^', label='C chunk=100')
plt.plot(threads, C_nu_chunk1000,marker='D', label='C chunk=1000')

plt.title('Non-uniform f(x): B vs C Dynamic')
plt.xlabel('Αριθμός Threads')
plt.ylabel('Χρόνος (seconds)')
plt.xticks(threads)
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('results/plot4_nonuniform.png', dpi=150)
plt.close()
print("Αποθηκεύτηκε: results/plot4_nonuniform.png")