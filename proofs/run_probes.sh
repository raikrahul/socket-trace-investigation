#!/bin/bash
echo "Running Linus Protocol Probes..."
make clean && make

echo "[+] Operation MSR"
sudo -n insmod msr_reader.ko
sudo -n rmmod msr_reader
dmesg | tail -n 5 | grep "AXIOM_MSR"

echo "[+] Operation Slab Anatomy"
sudo -n insmod slab_anatomy.ko
sudo -n rmmod slab_anatomy
dmesg | tail -n 5 | grep "AXIOM_SLAB"

echo "[+] Operation FD Math"
sudo -n insmod fd_math_probe.ko
sudo -n rmmod fd_math_probe
dmesg | tail -n 5 | grep "AXIOM_FD"

echo "Done."
