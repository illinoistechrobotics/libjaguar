#include <stdint.h>

#ifndef CAN_H
#define CAN_H

#define DEVTYPE_SYS       0
#define DEVTYPE_MOTORCTRL 2
#define MANUFACTURER_SYS  0
#define MANUFACTURER_TI   2

#define START_OF_FRAME 0xff
#define ENCODE_BYTE_A  0xfe
#define ENCODE_BYTE_B  0xfd

#define HEADER_SIZE    6
#define CAN_ID_SIZE    4
#define MAX_DATA_BYTES 8
#define MAX_MSG_BYTES  22

// API Classes
#define API_SYS      0
#define API_VOLTAGE  0
#define API_SPEED    1
#define API_VOLTCOMP 2
#define API_POSITION 3
#define API_CURRENT  4
#define API_STATUS   5
#define API_CONFIG   7
#define API_ACK      8

// System Control Interface
#define SYS_HALT        0
#define SYS_RESET       1
#define SYS_ASSIGN      2
#define SYS_QUERY       3
#define SYS_HEARTBEAT   5
#define SYS_SYNC_UPDATE 6
#define SYS_FW_UPDATE   7
#define SYS_FW_VER      8
#define SYS_ENUMERATION 9
#define SYS_RESUME      10

// Voltage Control Interface
#define VOLTAGE_ENABLE  0
#define VOLTAGE_DISABLE 1
#define VOLTAGE_SET     2
#define VOLTAGE_RAMP    3

// Speed Control Interface
#define SPEED_ENABLE  0
#define SPEED_DISABLE 1
#define SPEED_SET     2
#define SPEED_P       3
#define SPEED_I       4
#define SPEED_D       5
#define SPEED_REF     6

#define SPEED_ENCODER_1CH     0
#define SPEED_ENCODER_1CH_INV 2
#define SPEED_ENCODER_QUAD    3

// Voltage Compensation Control Interface
#define VOLTCOMP_ENABLE  0
#define VOLTCOMP_DISABLE 1
#define VOLTCOMP_SET     2
#define VOLTCOMP_RAMP    3
#define VOLTCOMP_RATE    4

// Position Control Interface
#define POSITION_ENABLE  0
#define POSITION_DISABLE 1
#define POSITION_SET     2
#define POSITION_P       3
#define POSITION_I       4
#define POSITION_D       5
#define POSITION_REF     6

#define POSITION_ENCODER       0
#define POSITION_POTENTIOMETER 1

// Current Control Interface
#define CURRENT_ENABLE  0
#define CURRENT_DISABLE 1
#define CURRENT_SET     2
#define CURRENT_P       3
#define CURRENT_I       4
#define CURRENT_D       5

// Motor Control Status
#define STATUS_OUTPUT_PERCENT 0
#define STATUS_BUS_VOLTAGE    1
#define STATUS_CURRENT        2
#define STATUS_TEMPERATURE    3
#define STATUS_POSITION       4
#define STATUS_SPEED          5
#define STATUS_LIMIT          6
#define STATUS_FAULT          7
#define STATUS_POWER          8
#define STATUS_MODE           9
#define STATUS_OUTPUT_VOLTS   10

#define FORWARD_LIMIT_REACHED 0
#define REVERSE_LIMIT_REACHED 1

#define CURRENT_FAULT     0
#define TEMPERATURE_FAULT 1
#define BUS_VOLTAGE_FAULT 2

#define STATUS_MODE_VOLTAGE  0
#define STATUS_MODE_CURRENT  1
#define STATUS_MODE_SPEED    2
#define STATUS_MODE_POSITION 3
#define STATUS_MODE_VOLTCOMP 4

// Motor Control Configuration
#define CONFIG_BRUSHES       0
#define CONFIG_ENCODER_LINES 1
#define CONFIG_POT_TURNS     2
#define CONFIG_BREAK_COAST   3
#define CONFIG_LIMIT_MODE    4
#define CONFIG_FORWARD_LIMIT 5
#define CONFIG_REVERSE_LIMIT 6
#define CONFIG_MAX_VOLTAGE   7
#define CONFIG_FAULT_TIME    8

typedef struct CANMessage {
    uint8_t device;
    uint8_t api_class;
    uint8_t api_index;
    uint8_t manufacturer;
    uint8_t device_type;
    uint8_t data_size;
    uint8_t data[MAX_DATA_BYTES];
} CANMessage;

typedef struct CANEncodedMsg {
    uint8_t size;
    uint8_t data[MAX_MSG_BYTES];
} CANEncodedMsg;

#endif
