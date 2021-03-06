/*
 * Turns all relays on or off on a USB-RLY02.
 *
 * Based on: http://www.robot-electronics.co.uk/files/linux_rly02.c
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define STATE_ON  "on"
#define STATE_OFF "off"

void usage() {
    fprintf(stderr, "usage: %s device %s|%s\n", getprogname(), STATE_ON,
        STATE_OFF);
    exit(EXIT_FAILURE);
}

void print_error_and_exit(char *message) {
    perror(message);
    exit(EXIT_FAILURE);
}

void write_to_device(int fd, int command) {
    char sbuf[2];
    sbuf[0] = command;
    if (write(fd, sbuf, 1) < 0) {
        print_error_and_exit("Error while writing output");
    }
    if (tcdrain(fd) < 0) {
        print_error_and_exit("Error while waiting for output to be written");
    }
}

int parse_command(char *state) {
    if (strcmp(state, STATE_ON) == 0) {
        return 0x64;
    } else if (strcmp(state, STATE_OFF) == 0) {
        return 0x6E;
    }
    return -1;
}

int main(int argc, char *argv[]) {
    struct termios defaults;
    struct termios config;
    if (argc != 3) {
        usage();
    }
    const char *device = argv[optind++];
    int command = parse_command(argv[optind]);
    if (command == -1) {
        usage();
    }
    int fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        char *message;
        asprintf(&message, "Failed to open device %s", device);
        print_error_and_exit(message);
    }
    if (tcgetattr(fd, &defaults) < 0) {
        print_error_and_exit("Failed to read port defaults");
    }
    cfmakeraw(&config);
    if (tcsetattr(fd, TCSANOW, &config) < 0) {
        print_error_and_exit("Failed to configure port");
    }
    write_to_device(fd, command);
    if (tcsetattr(fd, TCSANOW, &defaults) < 0) {
        print_error_and_exit("Failed to restore port defaults");
    }
    close(fd);
    return EXIT_SUCCESS;
}
