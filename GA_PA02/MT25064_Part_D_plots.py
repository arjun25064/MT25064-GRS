# MT25064_Part_D_plots.py
# MT25064
import matplotlib.pyplot as plt
import numpy as np

# Hardcoded data from CSV
msg_sizes = [64, 512, 4096, 65536]
threads = [1, 2, 4, 8]

# Throughput data (Gbps)
A1_throughput = {
    64: [0.125661, 0.269347, 0.517788, 0.874539],
    512: [1.082198, 2.363781, 5.181462, 12.503897],
    4096: [11.959821, 25.694233, 47.512025, 78.715937],
    65536: [62.970082, 121.602579, 233.190674, 170.435484]
}

A2_throughput = {
    64: [1.208641, 2.453130, 4.988763, 7.768833],
    512: [8.151425, 16.237882, 34.310216, 54.940195],
    4096: [37.492166, 71.984552, 150.551662, 235.375510],
    65536: [75.595357, 137.980543, 286.385609, 107.332869]
}

A3_throughput = {
    64: [0.260614, 0.504401, 1.019843, 1.410619],
    512: [2.072762, 4.056152, 8.085418, 11.217402],
    4096: [14.348835, 27.585026, 54.204056, 78.731530],
    65536: [63.038554, 119.534204, 231.394666, 78.697726]
}

# Cache misses
A1_cache = {
    64: [5249556, 20093151, 24607820, 3538466],
    512: [41024946, 111208561, 35415188, 5812446],
    4096: [41730966, 68761699, 58835425, 11229308],
    65536: [92152276, 101334653, 198144683, 401465360]
}

A2_cache = {
    64: [111883618, 17387273, 50240014, 5103976],
    512: [23336061, 36914720, 50836353, 12089152],
    4096: [15055304, 40055190, 48224821, 18944648],
    65536: [14010351, 45713775, 99521525, 460012926]
}

A3_cache = {
    64: [16187188, 64511600, 36807824, 8849472],
    512: [18338502, 27825618, 29005647, 15829334],
    4096: [31466232, 18943808, 39986217, 27019343],
    65536: [49521502, 80371672, 180218141, 31332009]
}

# Cycles
A1_cycles = {
    64: [8090592279, 67886693186, 120680708878, 259115906082],
    512: [10514105885, 52721919288, 119826655135, 258864926787],
    4096: [17101603226, 38650007567, 117446251655, 255554820042],
    65536: [17137236889, 52010364357, 127435866414, 193933464338]
}

A2_cycles = {
    64: [14019239288, 17604530133, 120781701211, 252974855599],
    512: [22821717950, 42147815446, 115863290254, 251764869553],
    4096: [8812719583, 22520824851, 127027685390, 252326062060],
    65536: [3758198895, 45069577346, 130548729293, 165115960054]
}

A3_cycles = {
    64: [23534765105, 25512527387, 124049676816, 249324989309],
    512: [11012461776, 54948800884, 128932696286, 248410655113],
    4096: [28262977189, 49983330464, 118060050738, 248738632756],
    65536: [21536307196, 51497962465, 124126319212, 24626150709]
}

# Total bytes for CPU cycles per byte
A1_bytes = {
    64: [157075808, 336683448, 647235088, 1093173288],
    512: [1352747200, 2954726208, 6476827392, 15629871168],
    4096: [14949776384, 32117791744, 59390031360, 98394921472],
    65536: [78712602571, 152003223446, 291488342016, 213044355019]
}

A2_bytes = {
    64: [1510800640, 3066413056, 6235953280, 9711041408],
    512: [10189281280, 20297352704, 42887770624, 68675243520],
    4096: [46865207296, 89980690432, 188189577216, 294219386880],
    65536: [94494195712, 172475678720, 357982011392, 134166085632]
}

A3_bytes = {
    64: [325767872, 630501696, 1274803152, 1763273720],
    512: [2590952000, 5070190400, 10106772736, 14021752576],
    4096: [17936043520, 34481282048, 67755070464, 98414412288],
    65536: [78798192640, 149417754624, 289243332608, 98372157228]
}

# Plot 1: Throughput vs Message Size
plt.figure(figsize=(10, 6))
for i, th in enumerate(threads):
    A1_vals = [A1_throughput[s][i] for s in msg_sizes]
    A2_vals = [A2_throughput[s][i] for s in msg_sizes]
    A3_vals = [A3_throughput[s][i] for s in msg_sizes]
    
    plt.plot(msg_sizes, A1_vals, marker='o', label=f'A1 (2-copy) {th}T', linestyle='-')
    plt.plot(msg_sizes, A2_vals, marker='s', label=f'A2 (1-copy) {th}T', linestyle='--')
    plt.plot(msg_sizes, A3_vals, marker='^', label=f'A3 (0-copy) {th}T', linestyle=':')

plt.xlabel('Message Size (bytes)', fontsize=12)
plt.ylabel('Throughput (Gbps)', fontsize=12)
plt.title('Throughput vs Message Size\nSystem: Intel ThinkCentre M70s Gen 3', fontsize=14)
plt.xscale('log')
plt.yscale('log')
plt.grid(True, alpha=0.3)
plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left', fontsize=8)
plt.tight_layout()
plt.savefig('plot1_throughput_vs_msgsize.png', dpi=300, bbox_inches='tight')
plt.close()

# Plot 2: Throughput vs Thread Count (for size=4096)
plt.figure(figsize=(10, 6))
size = 4096
plt.plot(threads, A1_throughput[size], marker='o', label='A1 (2-copy)', linewidth=2)
plt.plot(threads, A2_throughput[size], marker='s', label='A2 (1-copy)', linewidth=2)
plt.plot(threads, A3_throughput[size], marker='^', label='A3 (0-copy)', linewidth=2)

plt.xlabel('Thread Count', fontsize=12)
plt.ylabel('Throughput (Gbps)', fontsize=12)
plt.title(f'Throughput vs Thread Count (Message Size = {size} bytes)\nSystem: Intel ThinkCentre M70s Gen 3', fontsize=14)
plt.grid(True, alpha=0.3)
plt.legend(fontsize=10)
plt.xticks(threads)
plt.tight_layout()
plt.savefig('plot2_throughput_vs_threads.png', dpi=300)
plt.close()

# Plot 3: Cache Misses vs Message Size
plt.figure(figsize=(10, 6))
# Use thread=2 as example
th_idx = 1
A1_vals = [A1_cache[s][th_idx] for s in msg_sizes]
A2_vals = [A2_cache[s][th_idx] for s in msg_sizes]
A3_vals = [A3_cache[s][th_idx] for s in msg_sizes]

plt.plot(msg_sizes, A1_vals, marker='o', label='A1 (2-copy)', linewidth=2)
plt.plot(msg_sizes, A2_vals, marker='s', label='A2 (1-copy)', linewidth=2)
plt.plot(msg_sizes, A3_vals, marker='^', label='A3 (0-copy)', linewidth=2)

plt.xlabel('Message Size (bytes)', fontsize=12)
plt.ylabel('Cache Misses', fontsize=12)
plt.title(f'Cache Misses vs Message Size (Threads = {threads[th_idx]})\nSystem: Intel ThinkCentre M70s Gen 3', fontsize=14)
plt.xscale('log')
plt.yscale('log')
plt.grid(True, alpha=0.3)
plt.legend(fontsize=10)
plt.tight_layout()
plt.savefig('plot3_cache_vs_msgsize.png', dpi=300)
plt.close()

# Plot 4: CPU Cycles per Byte
plt.figure(figsize=(10, 6))
for i, th in enumerate(threads):
    A1_cpb = [A1_cycles[s][i] / A1_bytes[s][i] for s in msg_sizes]
    A2_cpb = [A2_cycles[s][i] / A2_bytes[s][i] for s in msg_sizes]
    A3_cpb = [A3_cycles[s][i] / A3_bytes[s][i] for s in msg_sizes]
    
    plt.plot(msg_sizes, A1_cpb, marker='o', label=f'A1 {th}T', alpha=0.7)
    plt.plot(msg_sizes, A2_cpb, marker='s', label=f'A2 {th}T', alpha=0.7)
    plt.plot(msg_sizes, A3_cpb, marker='^', label=f'A3 {th}T', alpha=0.7)

plt.xlabel('Message Size (bytes)', fontsize=12)
plt.ylabel('CPU Cycles per Byte', fontsize=12)
plt.title('CPU Cycles per Byte Transferred\nSystem: Intel ThinkCentre M70s Gen 3', fontsize=14)
plt.xscale('log')
plt.yscale('log')
plt.grid(True, alpha=0.3)
plt.legend(bbox_to_anchor=(1.05, 1), loc='upper left', fontsize=8)
plt.tight_layout()
plt.savefig('plot4_cycles_per_byte.png', dpi=300, bbox_inches='tight')
plt.close()

print("All plots generated successfully!")
print("- plot1_throughput_vs_msgsize.png")
print("- plot2_throughput_vs_threads.png")
print("- plot3_cache_vs_msgsize.png")
print("- plot4_cycles_per_byte.png")    