![Ymir logo](banner-no-bg.png)


# Ymir: Advanced Container Management Daemon for OpenWRT


Ymir is an innovative project designed to provide a comprehensive and lightweight daemon, written in C++, for the efficient management of Linux Containers (LXC) on embedded devices. Ymir elevates container management to a new level, making it fully compliant with TR157 specifications for optimal operation in embedded systems.

## Key Features of Ymir:

### Robust Container Resource Management: 

Ymir specializes in managing container resources, ensuring seamless operation and control of LXC containers in resource-constrained environments.

### SDK for Deployment Units Creation: 

With a dedicated SDK, Ymir empowers developers to build deployment units tailored for specific needs. These units can be in various formats, including IPKs or SquashFS, providing flexibility in software deployment.

### Advanced Deployment Capabilities: 

A single container in Ymir can host multiple SquashFS 3rd party software applications. Each application can further encompass one or more services, offering a modular and scalable approach to software deployment.

### Durable File System Management: 

Ymir ensures robustness by using an overlay file system (OverlayFS) mounted on an ext4 loop file. This approach safeguards against corruption, allowing for easy recovery and factory resets without affecting the core container setup. The reset process only erases the overlay file system's content, leaving the overall solution intact and operational.

### Interactive Container Interface: 

Similar to LXD, Ymir utilizes a REST API and AMX-bus for container interactions. This allows users to create, start, stop, and manage containers effortlessly. The REST API is accessible via any HTTP-compliant client, enabling remote management and monitoring.

### User-Friendly Commands for Quick Operations:

```
Install a new container: 
SoftwareModules.addExecEnv(name=Container1, type=busyboxv2, storage=30)
Deploy a new tarball in the container: 
SoftwareModules.addDeploymentUnit(ee_name=Container1, du_url="/root/tarball-Fantom-bcm27xx.tar.gz")
Licensing and Distribution:
```

Ymir is a closed-source project. The distribution and access to the source code are strictly regulated, requiring official authorization from the code owner. This ensures the integrity and security of the software, particularly in commercial and embedded applications.

### Ideal for Developers and System Administrators:

Ymir is the perfect tool for developers and system administrators seeking a powerful yet lightweight solution to manage LXC containers in embedded devices. It combines flexibility, robustness, and ease of use, making it an essential tool for modern embedded systems.


### Roadmap:

Ymir already employs multithreading for managing container processes. We're planning to extend this multithreading approach to other tasks, further boosting Ymir's performance and responsiveness in managing LXC containers. Stay tuned for these exciting developments.
