#!/bin/sh
systemctl is-system-running --wait > /dev/null
exec > /dev/serial0 2>&1
echo "I boot_state=$(systemctl is-system-running)"
/boot/firmware/bench