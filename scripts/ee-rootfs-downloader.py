#!/usr/bin/env python3

#usage example : ./scripts/ee-rootfs-downloader.py --rootfs staging_dir/target-aarch64_cortex-a72_musl/root-bcm27xx/ -d ubuntu -r bionic -a arm64
import requests
import argparse
import os
import tarfile
from datetime import datetime

def download_file(url, destination):
    response = requests.get(url, stream=True)
    response.raise_for_status()

    with open(destination, 'wb') as f:
        for chunk in response.iter_content(chunk_size=8192):
            f.write(chunk)

def find_image_info(lines, distro, release, arch):
    for line in lines:
        parts = line.strip().split(';')
        if parts[0] == distro and parts[1] == release and parts[2] == arch:
            return parts[5]
    return None

def extract_tar(file_path, extract_to):
    with tarfile.open(file_path, "r:xz") as tar:
        tar.extractall(path=extract_to)

def main():
    parser = argparse.ArgumentParser(description='Download and extract a specific Linux container image.')
    parser.add_argument('--rootfs', required=True, help='Root filesystem path')
    parser.add_argument('-d', '--distro', required=True, help='Distribution name')
    parser.add_argument('-r', '--release', required=True, help='Release version')
    parser.add_argument('-a', '--arch', required=True, help='Architecture')

    args = parser.parse_args()

    index_url = 'https://images.linuxcontainers.org/meta/1.0/index-system'

    # Download the index file
    index_file = os.path.join(args.rootfs, 'index-system')
    download_file(index_url, index_file)

    # Read the file and find the specified image
    with open(index_file, 'r') as f:
        lines = f.readlines()

    image_info = find_image_info(lines, args.distro, args.release, args.arch)

    if image_info:
        image_url = f"https://images.linuxcontainers.org{image_info.rstrip('/')}/rootfs.tar.xz"
        print(f"Downloading image from: {image_url}")

        image_file = os.path.join(args.rootfs, 'rootfs.tar.xz')
        download_file(image_url, image_file)

        print(f"Extracting to {args.rootfs}")
        extract_tar(image_file, args.rootfs)

    else:
        print("No matching image found")

if __name__ == "__main__":
    main()
