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


# ============================================================
# Νέα δεδομένα από 3 N values
# ============================================================
N_values = [100000, 1000000, 10000000]
serial_N = [0.001573, 0.015144, 0.122909]

# A nolock για 3 N values (4 threads)
A_nolock_N100K  = [0.002010, 0.001691, 0.001442, 0.001445]
A_nolock_N1M    = [0.017076, 0.009986, 0.007507, 0.006271]
A_nolock_N10M   = [0.119600, 0.074453, 0.055407, 0.046212]

# Speedup A nolock για 3 N values
speedup_N100K  = [serial_N[0]/t for t in A_nolock_N100K]
speedup_N1M    = [serial_N[1]/t for t in A_nolock_N1M]
speedup_N10M   = [serial_N[2]/t for t in A_nolock_N10M]

# B non-uniform για 3 N values
B_nu_N100K  = [0.194281, 0.104614, 0.074542, 0.057315]
B_nu_N1M    = [1.828735, 0.923860, 0.634070, 0.474833]
B_nu_N10M   = [18.059600, 9.057350, 6.094500, 4.640595]

speedup_Bnu_N100K = [B_nu_N100K[0]/t for t in B_nu_N100K]
speedup_Bnu_N1M   = [B_nu_N1M[0]/t  for t in B_nu_N1M]
speedup_Bnu_N10M  = [B_nu_N10M[0]/t for t in B_nu_N10M]

# C uniform N=10M
C_10M_chunk10   = [0.141179, 0.146358, 0.217492, 0.235427]
C_10M_chunk100  = [0.120068, 0.070269, 0.053351, 0.048045]
C_10M_chunk1000 = [0.114767, 0.062469, 0.047447, 0.039332]

# ============================================================
# Γράφημα 5 — Επίδραση N στο Speedup (A nolock)
# ============================================================
plt.figure(figsize=(9, 5))
plt.plot(threads, [1,2,3,4], linestyle='--', color='black', label='Ιδανικό')
plt.plot(threads, speedup_N100K,  marker='o', label='N=100K')
plt.plot(threads, speedup_N1M,    marker='s', label='N=1M')
plt.plot(threads, speedup_N10M,   marker='^', label='N=10M')
plt.axhline(y=1.0, color='red', linestyle=':', alpha=0.5, label='Speedup=1 (serial)')
plt.title('Επίδραση N στο Speedup — A Static Nolock')
plt.xlabel('Αριθμός Threads')
plt.ylabel('Speedup')
plt.xticks(threads)
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('results/plot5_speedup_vs_N.png', dpi=150)
plt.close()
print("Αποθηκεύτηκε: results/plot5_speedup_vs_N.png")

# ============================================================
# Γράφημα 6 — Speedup B non-uniform για 3 N values
# ============================================================
plt.figure(figsize=(9, 5))
plt.plot(threads, [1,2,3,4], linestyle='--', color='black', label='Ιδανικό')
plt.plot(threads, speedup_Bnu_N100K, marker='o', label='N=100K')
plt.plot(threads, speedup_Bnu_N1M,   marker='s', label='N=1M')
plt.plot(threads, speedup_Bnu_N10M,  marker='^', label='N=10M')
plt.title('Speedup B Non-uniform για διαφορετικά N')
plt.xlabel('Αριθμός Threads')
plt.ylabel('Speedup')
plt.xticks(threads)
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('results/plot6_Bnonunif_speedup_N.png', dpi=150)
plt.close()
print("Αποθηκεύτηκε: results/plot6_Bnonunif_speedup_N.png")

# ============================================================
# Γράφημα 7 — chunk_size για N=10M
# ============================================================
plt.figure(figsize=(9, 5))
plt.plot(threads, C_10M_chunk10,   marker='o', label='chunk=10')
plt.plot(threads, C_10M_chunk100,  marker='s', label='chunk=100')
plt.plot(threads, C_10M_chunk1000, marker='^', label='chunk=1000')
plt.axhline(y=0.122909, color='black', linestyle='--', label='Serial N=10M')
plt.title('C Dynamic: Επίδραση chunk_size για N=10M')
plt.xlabel('Αριθμός Threads')
plt.ylabel('Χρόνος (seconds)')
plt.xticks(threads)
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig('results/plot7_chunk_N10M.png', dpi=150)
plt.close()
print("Αποθηκεύτηκε: results/plot7_chunk_N10M.png")