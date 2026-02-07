#!/usr/bin/env bash
# MT25064_Part_C_run_experiments.sh
# MT25064

ROLL_NUM="MT25064"
OUT="${ROLL_NUM}_Part_C_results.csv"
PORT_BASE=9000
DURATION=10

MSG_SIZES=(64 512 4096 65536)
THREADS=(1 2 4 8)
VERSIONS=("A1" "A2" "A3")

echo "[INFO] Setting perf permissions..."
# Allow perf to access performance counters
echo -1 | sudo tee /proc/sys/kernel/perf_event_paranoid 2>/dev/null || true
echo 0 | sudo tee /proc/sys/kernel/kptr_restrict 2>/dev/null || true

echo "[INFO] Cleaning and compiling..."
killall -9 MT25064_Part_A1_server MT25064_Part_A2_server MT25064_Part_A3_server MT25064_client perf 2>/dev/null || true
rm -f *.csv *.tmp *.txt 2>/dev/null || true
sleep 1

make clean && make all || exit 1
echo "✓ Compilation done"

echo "Implementation,MessageSize,Threads,TotalBytes,Throughput_Gbps,Cycles,CacheMisses,ContextSwitches" > $OUT

for v in "${VERSIONS[@]}"; do
    for size in "${MSG_SIZES[@]}"; do
        for th in "${THREADS[@]}"; do
            echo ""
            echo "============================================"
            echo "[RUN] $v | size=$size | threads=$th"
            echo "============================================"
            
            PORT=$((PORT_BASE + RANDOM % 1000))
            killall -9 MT25064_Part_${v}_server MT25064_client perf 2>/dev/null || true
            sleep 1
            
            # Start server
            ./MT25064_Part_${v}_server $PORT $size $th 2>/dev/null &
            SERVER_PID=$! # server process ID
            sleep 2
            
            # check if server failed to initiate...
            if ! ps -p $SERVER_PID > /dev/null; then
                echo "   [ERROR] Server failed to start"
                echo "${v},${size},${th},0,0.000000,0,0,0" >> $OUT
                continue
            fi
            
            PERF_OUT="perf_${v}_${size}_${th}.txt"
            
            # Start perf recording - use multiple event groups if needed
            perf stat -p $SERVER_PID \
                -e cycles -e cache-misses -e context-switches \
                -x, -o "$PERF_OUT" sleep $((DURATION + 1)) &
            PERF_PID=$!
            sleep 1
            
            # Launch clients
            for ((i=0; i<th; i++)); do
                ./MT25064_client 127.0.0.1 $PORT $size $DURATION > "bytes_${i}.tmp" 2>&1 &
                sleep 0.1
            done
            
            # Wait for completion
            sleep $((DURATION + 2))
            
            # Stop perf gracefully
            kill -INT $PERF_PID 2>/dev/null || true
            wait $PERF_PID 2>/dev/null || true
            
            # Kill server
            kill -9 $SERVER_PID 2>/dev/null || true
            killall -9 MT25064_client 2>/dev/null || true
            sleep 1
            
            # Calculate bytes
            TOTAL=0
            for ((i=0; i<th; i++)); do
                if [ -f "bytes_${i}.tmp" ]; then
                    B=$(grep -oE '^[0-9]+$' "bytes_${i}.tmp" 2>/dev/null || echo "0")
                    TOTAL=$((TOTAL + B))
                fi
            done
            rm -f bytes_*.tmp
            
            TPUT=$(awk "BEGIN {printf \"%.6f\", ($TOTAL * 8.0) / ($DURATION * 1e9)}")
            
            # Parse perf - handle different output formats
            if [ -f "$PERF_OUT" ]; then
                # Format: value,unit,event,runtime,pct
                # or: value,,event
                CYCLES=$(awk -F',' '$3 ~ /cycles/ || $1 ~ /[0-9]/ && $2 ~ /cycles/ {gsub(/[^0-9]/,"",$1); if($1) {print $1; exit}}' "$PERF_OUT")
                CACHE=$(awk -F',' '$3 ~ /cache-misses/ || $1 ~ /[0-9]/ && $2 ~ /cache-misses/ {gsub(/[^0-9]/,"",$1); if($1) {print $1; exit}}' "$PERF_OUT")
                CTX=$(awk -F',' '$3 ~ /context-switches/ || $1 ~ /[0-9]/ && $2 ~ /context-switches/ {gsub(/[^0-9]/,"",$1); if($1) {print $1; exit}}' "$PERF_OUT")
                
                CYCLES=${CYCLES:-0}
                CACHE=${CACHE:-0}
                CTX=${CTX:-0}
                
                echo "   → $TOTAL bytes, $TPUT Gbps | Cycles: $CYCLES, Cache: $CACHE, Ctx: $CTX"
            else
                CYCLES=0
                CACHE=0
                CTX=0
                echo "   → $TOTAL bytes, $TPUT Gbps | Perf file not found"
            fi
            
            echo "${v},${size},${th},${TOTAL},${TPUT},${CYCLES},${CACHE},${CTX}" >> $OUT
            
            rm -f "$PERF_OUT"
            sleep 1
        done
    done
done

killall -9 MT25064_Part_A1_server MT25064_Part_A2_server MT25064_Part_A3_server MT25064_client perf 2>/dev/null || true

echo ""
echo "============================================"
echo "[DONE] Results saved to: $OUT"
echo "============================================"
echo ""
column -t -s',' "$OUT" 2>/dev/null || cat "$OUT"