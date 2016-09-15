#include <libudev.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void enumerateDevices(struct udev* udev)
{
    struct udev_list_entry *dev_list_entry;
    struct udev_enumerate *enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "drm");
    udev_enumerate_scan_devices(enumerate);
    struct udev_list_entry *devices    = udev_enumerate_get_list_entry(enumerate);

    printf("%s\t\t%s\t\t%s\n", "Name", "Status", "Enabled");
    fflush(stdout);

    udev_list_entry_foreach(dev_list_entry, devices)
    {
        const char *path = udev_list_entry_get_name(dev_list_entry);
        struct udev_device *dev = udev_device_new_from_syspath(udev, path);
        struct udev_device *parent = udev_device_get_parent(dev);
        if (parent)
        {
            const char* parentSubsystem = udev_device_get_subsystem(parent);
            if (!strcmp(parentSubsystem, "drm"))
            { //parent subsystem is from drm so we've got connector
                const char* sysname = udev_device_get_sysname(dev);
                const char* status = udev_device_get_sysattr_value(dev,"status");
                const char* enabled = udev_device_get_sysattr_value(dev,"enabled");
                printf("%s\t%s\t%s\n", sysname, status, enabled);
                fflush(stdout);
            }
        }
        udev_device_unref(dev);
    }

    udev_enumerate_unref(enumerate);
}

void monitorDevices(struct udev* udev)
{
    struct udev_monitor *mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "drm", NULL);
    udev_monitor_enable_receiving(mon);
    int fd = udev_monitor_get_fd(mon);
    fd_set fds;

    while(1)
    {
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        int ret = select(fd+1, &fds, NULL, NULL, NULL);
        if (ret > 0 && FD_ISSET(fd, &fds))
        {
            struct udev_device *dev = udev_monitor_receive_device(mon);
            if (dev)
            {
                printf("action: %s %s\n", udev_device_get_sysname(dev), udev_device_get_action(dev));
                fflush(stdout);
                udev_device_unref(dev);
            }
        }
    }
    udev_monitor_unref(mon);
}

void printHelp(const char* arg0)
{
    printf("Usage: %s [option]\n", arg0);
    printf("Options:\n");
    printf("\tlist: lists available monitors\n");
    printf("\thelp: shows this text\n");
    printf("\tmonitor: monitors for new devices and prints events (default)\n");
}

int main(int argc, char* argv[])
{
    if(argc == 2 && (!strcmp(argv[1], "help") || !strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")))
    {
        printHelp(argv[0]);
        return 0;
    }
    struct udev *udev = udev_new();
    if (!udev)
    {
        printf("Can't create udev\n"); exit(1);
    }

    if(argc == 2 && !strcmp(argv[1], "list"))
    {
        enumerateDevices(udev);
    }
    else
    {
        monitorDevices(udev);
    }

    udev_unref(udev);

    return 0;
}

