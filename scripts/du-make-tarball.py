#!/usr/bin/env python3

import shutil
import os
import subprocess
import tarfile
import json
import glob
import re
import sys

# Check if at least one argument is passed

def parse_package_info(package_info):
    packages = package_info.strip().split("\n\n")
    package_list = []

    for package in packages:
        lines = package.split("\n")
        package_dict = {}
        current_key = None

        for line in lines:
            if ':' in line:
                if line.startswith(' '):
                    package_dict[current_key] += ' ' + line.strip()
                else:
                    key, value = line.split(":", 1)
                    package_dict[key.strip()] = value.strip()
                    current_key = key.strip()
            elif current_key:
                package_dict[current_key] += ' ' + line.strip()

        # Add to package_list only if package_dict is not empty
        if package_dict:
            package_list.append(package_dict)

    return package_list

def read_packages_from_directory(directory):
    all_packages = []

    # Walk through the directory
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith("Packages.manifest"):
                with open(os.path.join(root, file), 'r') as file_content:
                    package_info = file_content.read()
                    all_packages.extend(parse_package_info(package_info))

    return all_packages

# Read and parse the metadata.json file
def read_metadata(file_path):
    with open(file_path, 'r') as file:
        return json.load(file)
    
def parse_config(config_path):
    # Define patterns to search for
    arch_pattern = r'^CONFIG_TARGET_ARCH_PACKAGES="([^"]+)"'
    root_pattern = r'^CONFIG_TARGET_BOARD="([^"]+)"'

    # Initialize variables
    arch = None
    root_board = None
    # Read and parse the config file
    with open(config_path, 'r') as file:
        for line in file:
            arch_match = re.search(arch_pattern, line)
            root_match = re.search(root_pattern, line)

            if arch_match:
                arch = arch_match.group(1)
            if root_match:
                root_board = root_match.group(1)

    return arch, root_board

def construct_paths(arch, root_board):
    if arch and root_board:
        squashfs_path = f'staging_dir/target-{arch}_musl/root-{root_board}/'
        ipk_path = f'bin/packages/{arch}'
        return squashfs_path, ipk_path
    else:
        raise ValueError("Could not determine paths from .config")

def make_tarball(metadata, squashfs_path, ipk_path, tarball_path, metadata_file):
    # Create a temporary directory and ensure it's empty
    temp_dir = 'temp_tarball_dir'
    if os.path.exists(temp_dir):
        shutil.rmtree(temp_dir)
    os.makedirs(temp_dir)

    # Initialize list for storing ipk filenames
    ipk_files = []

    # Handle based on the type in metadata
    if metadata.get("Type") == "squashfs":
        # Run mksquashfs to create the SquashFS file
        squashfs_command = f"mksquashfs {squashfs_path} {temp_dir}/rootfs.squashfs -comp xz"
        subprocess.run(squashfs_command, shell=True, check=True)
    elif metadata.get("Type") == "ipk":
        # Find all ipk files and copy them to the temporary directory
        for ipk_file in glob.glob(f'{ipk_path}/**/*.ipk', recursive=True):
            shutil.copy(ipk_file, temp_dir)
            ipk_files.append(os.path.basename(ipk_file))

        # Update metadata with ipk filenames
        metadata["ipk"] = ipk_files
        print(metadata)

    # Write the metadata to a file inside the temporary directory
    with open(os.path.join(temp_dir, metadata_file), 'w') as file:
        json.dump(metadata, file, indent=4)

    # Create a tarball with files at the root
    with tarfile.open(tarball_path, "w:gz") as tar:
        for filename in os.listdir(temp_dir):
            file_path = os.path.join(temp_dir, filename)
            tar.add(file_path, arcname=filename)

    # Clean up by removing the temporary directory
    shutil.rmtree(temp_dir)

root_path="."

if len(sys.argv) > 1:
    root_path = sys.argv[1]

packages_directory = f'{root_path}/bin/packages'
metadata_file = f'{root_path}/metadata.json'
config_path = f'{root_path}/.config'

package_data = read_packages_from_directory(packages_directory)
metadata = read_metadata(metadata_file)

final_json = {"packages": package_data}
final_json.update(metadata)

final_json_str = json.dumps(final_json, indent=4)

print(final_json_str)

arch, root_board = parse_config(config_path)
squashfs_path, ipk_path = construct_paths(arch, root_board)

print(squashfs_path)
print(ipk_path)

#TODO Validate Json
name = metadata.get('Name', 'defaultName')
unsigned_tarball_output = f'{root_path}/bin/tarball-{name}-{root_board}.tar.gz'

make_tarball(metadata, squashfs_path, ipk_path, unsigned_tarball_output, 'metadata.json')