#ifndef LIBJAGUAR_H
#define LIBJAGUAR_H

#include "can.h"
#include "canutil.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <termios.h>

typedef struct JaguarConnection {
    int serial_fd;
    bool is_connected;
    const char *serial_port;
    struct termios *saved_settings;
} JaguarConnection;

int open_jaguar_connection(JaguarConnection *conn, const char *serial_port);
int close_jaguar_connection(JaguarConnection *conn);

int send_can_message(JaguarConnection *conn, CANMessage *message);
int recieve_can_message(JaguarConnection *conn, CANMessage *message);

#endif
