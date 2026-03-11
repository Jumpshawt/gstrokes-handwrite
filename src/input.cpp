/*
* functions for getting input devices. 
*
*/

#include "input.hpp"
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

std::string find_touchpad() {
    DIR* dir = opendir("/dev/input");
    struct dirent* ent;
    if (!dir) return "";
    while ((ent = readdir(dir)) != NULL) {
        if (std::string(ent->d_name).find("event") != std::string::npos) {
            std::string path = "/dev/input/" + std::string(ent->d_name);
            int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
            if (fd >= 0) {
                struct libevdev* dev = NULL;
                if (libevdev_new_from_fd(fd, &dev) == 0) {
                    if (libevdev_has_event_code(dev, EV_ABS, ABS_X) && libevdev_has_property(dev, INPUT_PROP_POINTER)) {
                        libevdev_free(dev); close(fd); closedir(dir);
                        return path;
                    }
                    libevdev_free(dev);
                }
                close(fd);
            }
        }
    }
    closedir(dir);
    return "";
}
