#!/bin/bash
# verify_stability.sh
# Purpose: Run Axiomatic Probes 10 times to verify deterministic behavior.

LOG_FILE="stability_report.txt"
> "$LOG_FILE"

echo "=== STARTING 10x STABILITY TEST ===" | tee -a "$LOG_FILE"

# Build modules first
make clean > /dev/null 2>&1
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules obj-m=axiomatic_probe_v2.o > /dev/null 2>&1
make -C /lib/modules/$(uname -r)/build M=$(pwd) modules obj-m=axiomatic_probe_v3.o > /dev/null 2>&1

# Get inetsw address dynamicall
INETSW_ADDR=$(sudo grep " inetsw$" /proc/kallsyms | awk '{print $1}')
echo "Target inetsw address: 0x$INETSW_ADDR" | tee -a "$LOG_FILE"

if [ -z "$INETSW_ADDR" ]; then
    echo "ERROR: Could not find inetsw address" | tee -a "$LOG_FILE"
    exit 1
fi

for i in {1..10}
do
    echo "--- Iteration $i ---" | tee -a "$LOG_FILE"
    
    # TEST V2: HARDER PUZZLE
    sudo dmesg -c > /dev/null # Clear ring buffer
    sudo insmod axiomatic_probe_v2.ko inetsw_addr=0x$INETSW_ADDR
    sudo rmmod axiomatic_probe_v2
    
    # Capture V2 Output
    V2_OUT=$(sudo dmesg | grep "AXIOM")
    echo "$V2_OUT" >> "$LOG_FILE"
    
    # Verify TCP and MPTCP presence
    if echo "$V2_OUT" | grep -q "protocol = 6" && echo "$V2_OUT" | grep -q "protocol = 262"; then
        echo "  v2 (Protocols): PASS" | tee -a "$LOG_FILE"
    else
        echo "  v2 (Protocols): FAIL" | tee -a "$LOG_FILE"
    fi

    # TEST V3: SLAB GEOMETRY
    sudo dmesg -c > /dev/null
    sudo insmod axiomatic_probe_v3.ko
    sudo rmmod axiomatic_probe_v3
    
    # Capture V3 Output
    V3_OUT=$(sudo dmesg | grep "AXIOM")
    echo "$V3_OUT" >> "$LOG_FILE"
    
    # Verify Slab Sizes (768 vs 832 check)
    if echo "$V3_OUT" | grep -q "struct socket_alloc) = 768"; then
        echo "  v3 (Slab Size): PASS (768 confirmed)" | tee -a "$LOG_FILE"
    else
        echo "  v3 (Slab Size): FAIL" | tee -a "$LOG_FILE"
    fi
    
    sleep 1
done

echo "=== TEST COMPLETE ===" | tee -a "$LOG_FILE"
