import matplotlib.pyplot as plt
import numpy as np

# Data for each algorithm
algorithms = [
    "Binary Search", "Bitcount", "Bubble Sort", "CRC", "Dijkstra", 
    "Duff's Device", "Fibonacci", "Levenshtein Distance", "Merge Sort",
    "Prime", "Recursion", "SHA256", "Tarai"
]

# Normal and Secure Run values (Binary Size in bytes and Execution Time in seconds)
normal_run_sizes = [4736, 3584, 4736, 3616, 3968, 3904, 3584, 3952, 5072, 3226, 3568, 4896, 3616]
secure_run_sizes = [5152, 4016, 5200, 4032, 4448, 4336, 4608, 4448, 5600, 10768, 19808, 5456, 4128]
normal_run_times = [0.08387207984924316, 0.09552812576293945, 0.37113380432128906, 0.08025240898132324, 0.08474540710449219,
                    0.09779596328735352, 0.616631269454956, 0.0858006477355957, 1.21690034866333, 3.254150390625,
                    5.537966251373291, 0.08490705490112305, 0.09367251396179199]
secure_run_times = [0.08388194404602051, 0.09566017646789551, 0.37116265296936035, 0.08026152320861816, 0.08491897583007812,
                    0.09779952598571777, 10.00898003578186, 0.0860896110534668, 1.21708083152771, 3.255135847091675,
                    5.539685653686523, 0.08517609367370605, 0.09395313262939453]

# Plot 1: Binary Sizes
x = np.arange(len(algorithms))
width = 0.35  # Width of each bar

fig, ax1 = plt.subplots(figsize=(16, 8))
bars1 = ax1.bar(x - width/2, normal_run_sizes, width, label='Normal Run - Binary Size (Bytes)', color='skyblue')
bars2 = ax1.bar(x + width/2, secure_run_sizes, width, label='Secure Run - Binary Size (Bytes)', color='dodgerblue')

# Labeling for Binary Size plot
ax1.set_xlabel('Algorithm', fontsize=14, labelpad=20)
ax1.set_ylabel('Binary Size (Bytes)', fontsize=14, labelpad=20)
ax1.set_xticks(x)
ax1.set_xticklabels(algorithms, rotation=45, ha="right", fontsize=12)
ax1.legend(fontsize=12)

# Set y-axis limits for better space utilization
ax1.set_ylim(0, max(max(normal_run_sizes), max(secure_run_sizes)) * 1.1)  # Add 10% space above the max value

# Annotate bar values for Binary Sizes
for bars in [bars1, bars2]:
    for bar in bars:
        height = bar.get_height()
        ax1.annotate(f'{int(height)}',
                    xy=(bar.get_x() + bar.get_width() / 2, height),
                    xytext=(0, 5),
                    textcoords="offset points",
                    ha='center', va='bottom', fontsize=10)

plt.tight_layout(rect=[0, 0, 1, 0.95])  # Adjust layout to reduce whitespace

# Plot 2: Non-Normalized Execution Times
fig, ax2 = plt.subplots(figsize=(16, 8))
bars3 = ax2.bar(x - width/2, normal_run_times, width, label='Normal Run - Execution Time (s)', color='salmon')
bars4 = ax2.bar(x + width/2, secure_run_times, width, label='Secure Run - Execution Time (s)', color='red')

# Labeling for Execution Time plot
ax2.set_xlabel('Algorithm', fontsize=14, labelpad=20)
ax2.set_ylabel('Execution Time (s)', fontsize=14, labelpad=20)
ax2.set_xticks(x)
ax2.set_xticklabels(algorithms, rotation=45, ha="right", fontsize=12)
ax2.legend(fontsize=12)

# Set y-axis limits for better space utilization
ax2.set_ylim(0, max(max(normal_run_times), max(secure_run_times)) * 1.1)  # Add 10% space above the max value

# Annotate bar values for Execution Times with original values
for i, bars in enumerate([bars3, bars4]):
    for bar in bars:
        height = bar.get_height()
        ax2.annotate(f'{height:.2f}s',
                    xy=(bar.get_x() + bar.get_width() / 2, height),
                    xytext=(0, 5),
                    textcoords="offset points",
                    ha='center', va='bottom', fontsize=10)

plt.tight_layout(rect=[0, 0, 1, 0.95])  # Adjust layout to reduce whitespace

# Show both plots
plt.show()
