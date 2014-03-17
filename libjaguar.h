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
#include <unistd.h>

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

int init_sys_message(CANMessage *message, uint8_t api_index);
int init_jaguar_message(CANMessage *message, uint8_t api_class, uint8_t api_index);

bool valid_sys_reply(CANMessage *message, CANMessage *reply);
bool valid_jaguar_reply(CANMessage *message, CANMessage *reply);
bool valid_ack(CANMessage *message, CANMessage *ack);

int sys_heartbeat(JaguarConnection *conn, uint8_t device);
int sys_halt(JaguarConnection *conn, uint8_t device);
int sys_reset(JaguarConnection *conn, uint8_t device);
int sys_resume(JaguarConnection *conn, uint8_t device);
int sys_sync_update(JaguarConnection *conn, uint8_t group);

int voltage_enable(JaguarConnection *conn, uint8_t device);
int voltage_disable(JaguarConnection *conn, uint8_t device);
int voltage_set(JaguarConnection *conn, uint8_t device, int16_t voltage);
int voltage_set_sync(JaguarConnection *conn, uint8_t device, int16_t voltage, 
        uint8_t group);
int voltage_get(JaguarConnection *conn, uint8_t device, int16_t *voltage);
int voltage_ramp(JaguarConnection *conn, uint8_t device, uint16_t ramp);

int position_enable(JaguarConnection *conn, uint8_t device, 
        int32_t position);
int position_disable(JaguarConnection *conn, uint8_t device);
int position_set(JaguarConnection *conn, uint8_t device, 
        int32_t position);
int position_set_sync(JaguarConnection *conn, uint8_t device, int32_t position,
        uint8_t group);
int position_get(JaguarConnection *conn, uint8_t device, int32_t *position);
int position_p(JaguarConnection *conn, uint8_t device, int32_t p);
int position_i(JaguarConnection *conn, uint8_t device, int32_t i);
int position_d(JaguarConnection *conn, uint8_t device, int32_t d);
int position_pid(JaguarConnection *conn, uint8_t device, int32_t p, int32_t i, 
        int32_t d);
int position_ref_encoder(JaguarConnection *conn, uint8_t device);

int status_output_percent(JaguarConnection *conn, uint8_t device, 
        int16_t *output_percent);
int status_temperature(JaguarConnection *conn, uint8_t device, 
        uint16_t *temperature);
int status_position(JaguarConnection *conn, uint8_t device, uint32_t *position);
int status_mode(JaguarConnection *conn, uint8_t device, uint8_t *mode);

int config_encoder_lines(JaguarConnection *conn, uint8_t device, uint16_t lines);
int get_encoder_lines(JaguarConnection *conn, uint8_t device, uint16_t *lines);

#endif
