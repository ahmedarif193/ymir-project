#include <stdio.h>
#include <stdlib.h>
#include <lxc/lxccontainer.h>

int main() {
    struct lxc_container *c;
    const char *name = "my-container";
    const char *config_path = NULL;
    const char *template = "ubuntu";
    const char *flags = NULL;
    int bdev_type = 0;
    const char *bdev_data = "size=40MB";
    const char *lxcpath = NULL;

    /* Create a new container */
    c = lxc_container_new(name, config_path);
    if (!c) {
        printf("Failed to create container\n");
        exit(1);
    }

    /* Set the container's rootfs size to 40MB */
    if (lxc_container_set_config_item(c, "lxc.rootfs.backend", "btrfs") < 0 ||
        lxc_container_set_config_item(c, "lxc.rootfs.options", bdev_data) < 0) {
        printf("Failed to set rootfs size\n");
        exit(1);
    }

    /* Create the container from a template */
    if (lxc_container_create(c, template, flags, NULL, bdev_type, lxcpath) < 0) {
        printf("Failed to create container\n");
        exit(1);
    }

    /* Start the container */
    if (lxc_container_start(c, 0, NULL) < 0) {
        printf("Failed to start container\n");
        exit(1);
    }

    /* Stop the container */
    if (lxc_container_stop(c) < 0) {
        printf("Failed to stop container\n");
        exit(1);
    }

    /* Destroy the container */
    if (lxc_container_destroy(c) < 0) {
        printf("Failed to destroy container\n");
        exit(1);
    }

    lxc_container_put(c);
    return 0;
}
