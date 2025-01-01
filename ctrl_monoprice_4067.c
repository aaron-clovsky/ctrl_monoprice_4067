/*******************************************************************************
Control Monoprice Switch Model 4067 (HRM-2218F) via RS-232

Copyright (c) 2025 Aaron Clovsky

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/

/*******************************************************************************
Headers
*******************************************************************************/
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

/*******************************************************************************
Macros
*******************************************************************************/
#ifndef DEFAULT_MAX_RETRIES
#define DEFAULT_MAX_RETRIES 9
#endif

#ifndef DEFAULT_TIMEOUT_MS
#define DEFAULT_TIMEOUT_MS 500
#endif

/*******************************************************************************
Constants
*******************************************************************************/
const unsigned char request[] = {0x20, 0x3f, 0x00, 0x00, 0xf1, 0x20, 0x08, 0x00,
                                 0x00, 0x55, 0x20, 0x07, 0x00, 0x00, 0x0a};

const unsigned char response[] = {0x80, 0x0B, 0xC0, 0x01, 0x08, 0x48,
                                  0x52, 0x4D, 0x2D, 0x32, 0x32, 0x31,
                                  0x38, 0xE9, 0x80, 0x02, 0x01};

const unsigned char command[][20] = {
    {0x20, 0x02, 0x01, 0x01, 0xa5, 0x20, 0x08, 0x01, 0x00, 0x91,
     0x20, 0x01, 0x01, 0x01, 0x41, 0x20, 0x07, 0x01, 0x00, 0xce},
    {0x20, 0x02, 0x01, 0x02, 0x47, 0x20, 0x08, 0x01, 0x00, 0x91,
     0x20, 0x01, 0x01, 0x02, 0xa3, 0x20, 0x07, 0x01, 0x00, 0xce},
    {0x20, 0x02, 0x01, 0x03, 0x19, 0x20, 0x08, 0x01, 0x00, 0x91,
     0x20, 0x01, 0x01, 0x03, 0xfd, 0x20, 0x07, 0x01, 0x00, 0xce},
    {0x20, 0x02, 0x01, 0x04, 0x9a, 0x20, 0x08, 0x01, 0x00, 0x91,
     0x20, 0x01, 0x01, 0x04, 0x7e, 0x20, 0x07, 0x01, 0x00, 0xce},
    {0x20, 0x02, 0x01, 0x05, 0xc4, 0x20, 0x08, 0x01, 0x00, 0x91,
     0x20, 0x01, 0x01, 0x05, 0x20, 0x20, 0x07, 0x01, 0x00, 0xce},
    {0x20, 0x02, 0x01, 0x06, 0x26, 0x20, 0x08, 0x01, 0x00, 0x91,
     0x20, 0x01, 0x01, 0x06, 0xc2, 0x20, 0x07, 0x01, 0x00, 0xce},
    {0x20, 0x02, 0x01, 0x07, 0x78, 0x20, 0x08, 0x01, 0x00, 0x91,
     0x20, 0x01, 0x01, 0x07, 0x9c, 0x20, 0x07, 0x01, 0x00, 0xce},
    {0x20, 0x02, 0x01, 0x08, 0x39, 0x20, 0x08, 0x01, 0x00, 0x91,
     0x20, 0x01, 0x01, 0x08, 0xdd, 0x20, 0x07, 0x01, 0x00, 0xce}};

/*******************************************************************************
Utility Functions
*******************************************************************************/
void configure_device(int fd) {
    struct termios tty;

    tcgetattr(fd, &tty);

    tty.c_cflag &= ~PARENB;        /* No parity */
    tty.c_cflag &= ~CSTOPB;        /* 1 stop bit */
    tty.c_cflag &= ~CSIZE;         /* Clear character size bits */
    tty.c_cflag |= CS8;            /* 8 data bits */
    tty.c_cflag &= ~CRTSCTS;       /* No hardware flow control */
    tty.c_cflag |= CREAD | CLOCAL; /* Enable receiver, local line */
    tty.c_cflag &= ~HUPCL;         /* Disable modem disconnect */

    tty.c_iflag &= ~(IXON | IXOFF | IXANY); /* Disable software flow control */

    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); /* Raw input */

    cfsetispeed(&tty, B9600); /* Set baud rate */
    cfsetospeed(&tty, B9600);

    tcsetattr(fd, TCSANOW, &tty);
}

void set_timer(unsigned int milliseconds) {
    struct itimerval it_val;

    memset(&it_val, 0, sizeof(it_val));

    it_val.it_interval = it_val.it_value;
    it_val.it_value.tv_sec = milliseconds / 1000;
    it_val.it_value.tv_usec = (milliseconds * 1000) % 1000000;
    it_val.it_interval = it_val.it_value;

    if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
        perror("Error: setitimer()");
        exit(1);
    }
}

void bad_response(unsigned char *buffer, ssize_t len) {
    ssize_t i;

    fprintf(stderr, "Unexpected response: ");

    for (i = 0; i < len; i++) {
        fprintf(stderr, "%02X ", (unsigned int)buffer[i]);
    }

    fprintf(stderr, "\n              Check: ");

    for (i = 0; i < len; i++) {
        if (i < sizeof(response)) {
            if (buffer[i] == response[i]) {
                fprintf(stderr, "OK ");
            } else {
                fprintf(stderr, "XX ");
            }
        } else {
            fprintf(stderr, "?? ");
        }
    }

    fprintf(stderr, "\n             Expect: ");

    for (i = 0; i < sizeof(response); i++) {
        fprintf(stderr, "%02X ", (unsigned int)response[i]);
    }

    fprintf(stderr, "\n");
}

void verbose_response(unsigned char *buffer, ssize_t len) {
    ssize_t i;

    fprintf(stderr, "Response size: %zd\n", len);

    for (i = 0; i < len; i++) {
        fprintf(stderr, "%02X ", (unsigned int)buffer[i]);
    }

    fprintf(stderr, "\n");
}

/*******************************************************************************
Signal Handler
*******************************************************************************/
void sigalrm_handler(int sig) {
    /* This exists solely to trigger EINTR in write() and read() */
}

/*******************************************************************************
main()
*******************************************************************************/
int main(int argc, char *argv[]) {
    int opt;
    int verbose = 0;
    int max_retries = DEFAULT_MAX_RETRIES;
    int timeout_ms = DEFAULT_TIMEOUT_MS;
    int input = 0;
    const char *device = argv[1];
    struct sigaction sa;
    int fd;
    ssize_t count;
    ssize_t total;

    if (argc < 2) {
        printf("Usage: %s <DEVICE> [-i input] [-r retries] [-t ms] [-v]\n",
               argv[0]);
        return 1;
    }

    /* Parse options */
    while ((opt = getopt(argc, argv, ":i:r:t:v")) != -1) {
        char *endptr;

        switch (opt) {
        case 'i':
            errno = 0;

            input = (int)strtol(optarg, &endptr, 10);

            if (errno != 0 || endptr == optarg) {
                printf("Error: Invalid input: '%s'\n", optarg);
                exit(2);
            }

            if (input < 1 || input > 8) {
                printf("Error: Input out of range [1-8]\n");
                exit(2);
            }

            break;
        case 'r':
            errno = 0;

            max_retries = (int)strtol(optarg, &endptr, 10);

            if (errno != 0 || endptr == optarg) {
                printf("Error: Invalid number of retries: '%s'\n", optarg);
                exit(2);
            }

            if (max_retries < 0 || max_retries > 99) {
                printf("Error: Number of retries out of range [0-99]\n");
                exit(2);
            }

            break;
        case 't':
            errno = 0;

            timeout_ms = (int)strtol(optarg, &endptr, 10);

            if (errno != 0 || endptr == optarg) {
                printf("Error: Invalid timeout (ms): '%s'\n", optarg);
                exit(2);
            }

            if (timeout_ms < 10 || timeout_ms > 10000) {
                printf("Error: Timeout (ms) out of range [10-10000]\n");
                exit(2);
            }

            break;
        case 'v':
            verbose = 1;
            break;
        case ':':
            printf("-%c requires a value\n", optopt);
            exit(2);
        case '?':
            printf("unknown option: %c\n", optopt);
            exit(2);
        }
    }

    /* Setup SIGALRM handler */
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = &sigalrm_handler;

    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("Error: Unable to handle SIGALRM");
        exit(1);
    }

    /* If input is set then set the input before reading the input */
    if (input != 0) {
        /* Open the serial port */
        if ((fd = open(device, O_RDWR | O_NOCTTY)) == -1) {
            perror("Error opening serial port");
            return 1;
        }

        /* Configure serial port settings */
        configure_device(fd);

        /* Setup alarm */
        set_timer(timeout_ms);

        /* Write to the serial port */
        if ((count = write(fd, command[input - 1], sizeof(command[0]))) == -1) {
            if (errno == EINTR) {
                fprintf(stderr, "Error: write() timed out\n");
            } else {
                perror("Error writing to serial port");
            }

            exit(1);
        } else {
            if (verbose) {
                fprintf(stderr, "Wrote %zd bytes to %s\n", count, device);
            }
        }

        /* Cancel alarm */
        set_timer(0);

        /* Wait a moment */
        usleep(timeout_ms * 1000 * 2);

        close(fd);
    }

/*
    Restart from this point if read() times out
*/
retry:

    /* Open the serial port */
    if ((fd = open(device, O_RDWR | O_NOCTTY)) == -1) {
        perror("Error opening serial port");
        return 1;
    }

    /* Configure serial port settings */
    configure_device(fd);

    /* Setup alarm */
    set_timer(timeout_ms);

    /* Write to the serial port */
    if ((count = write(fd, request, sizeof(request))) == -1) {
        if (errno == EINTR) {
            fprintf(stderr, "Error: write() timed out\n");
        } else {
            perror("Error writing to serial port");
        }

        exit(1);
    } else {
        if (verbose) {
            fprintf(stderr, "Wrote %zd bytes to %s\n", count, device);
        }
    }

    /* Setup alarm */
    set_timer(timeout_ms);

    /* Write from the serial port */
    total = 0;

    while (1) {
        unsigned char buffer[sizeof(response) * 2];

        count = read(fd, &buffer[total], sizeof(buffer) - total);

        if (count > 0) {
            total += count;

            if (total < sizeof(response) + 1) {
                continue;
            }

            if (memcmp(buffer, response, sizeof(response)) != 0) {
                bad_response(buffer, total);
                exit(1);
            }

            if (verbose) {
                verbose_response(buffer, total);
            }
        } else if (count == 0) {
            /* If EOF is returned then retry */
            set_timer(0);

            if (verbose) {
                fprintf(stderr, "read() returned EOF, retrying...\n");
            }

            if (max_retries-- <= 0) {
                fprintf(stderr, "Error: Max retries reached\n");
                exit(1);
            }

            close(fd);

            goto retry;
        } else {
            if (errno == EINTR) {
                /* Retry on timeout */
                if (verbose) {
                    fprintf(stderr, "read() timed out, retrying...\n");
                }

                if (max_retries-- <= 0) {
                    fprintf(stderr, "Error: Max retries reached\n");
                    exit(1);
                }

                close(fd);

                goto retry;
            }

            perror("Error: read()");
            exit(1);
        }

        if (buffer[sizeof(response)] > 8) {
            fprintf(stderr, "Error: Invalid input in response: %u\n",
                    (unsigned int)buffer[sizeof(response)]);
            exit(1);
        }

        printf("%u\n", (unsigned int)buffer[sizeof(response)]);

        break;
    }

    close(fd);

    return 0;
}
