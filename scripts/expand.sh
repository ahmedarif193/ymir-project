#!/bin/sh

FLAG_FILE="/etc/fs.prepared"

# Check if the script has already been run
if [ -f "$FLAG_FILE" ]; then
    echo "The fdisk script has already been run. Exiting."
    exit 0
fi

echo "p
d
2
p
n
p
2
147456
n 
n 
n 
n 
n 

w
"|fdisk /dev/mmcblk0 

resize2fs /dev/loop0

touch "$FLAG_FILE"
echo "Flag file created at $FLAG_FILE."
