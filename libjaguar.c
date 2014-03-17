#include "libjaguar.h"

int open_jaguar_connection(JaguarConnection *conn, const char *serial_port)
{
    int fd;
    struct termios settings;
    conn->is_connected = false;
    conn->serial_port = serial_port;

    // open serial port
    fd = open(serial_port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
        // could not open serial port
        return 1;
    }
    conn->serial_fd = fd;
    conn->is_connected = true;

    conn->saved_settings = malloc(sizeof(struct termios));

    // save existing serial settings
    tcgetattr(fd, conn->saved_settings);

    // initialize new settings with existing settings
    tcgetattr(fd, &settings);

    // set baud rate to 115200
    cfsetispeed(&settings, B115200);
    cfsetospeed(&settings, B115200);

    // set port to raw mode
    cfmakeraw(&settings);

    // apply settings
    tcsetattr(fd, TCSANOW, &settings);

    // flush buffers
    tcflush(fd, TCIOFLUSH);

    return 0;
}

int close_jaguar_connection(JaguarConnection *conn)
{
    int fd;
    conn->is_connected = false;

    fd = conn->serial_fd;

    // flush buffers
    tcflush(fd, TCIOFLUSH);

    // apply saved settings
    tcsetattr(fd, TCSANOW, conn->saved_settings);

    // free memory for saved settings
    free(conn->saved_settings);

    // close serial port
    close(fd);

    return 0;
}

int send_can_message(JaguarConnection *conn, CANMessage *message)
{
    CANEncodedMsg encoded_message;

    encode_can_message(message, &encoded_message);
    write(conn->serial_fd, encoded_message.data, encoded_message.size);
    // Sleep to allow message to send before proceding
    usleep(1);

    return 0;
}

int recieve_can_message(JaguarConnection *conn, CANMessage *message)
{
    int fd;
    int bytes_read;
    int extra_bytes;
    uint8_t read_byte;
    uint8_t size;
    uint8_t *data_ptr;
    CANEncodedMsg encoded_message;

    fd = conn->serial_fd;

    // discard bytes or wait until start of frame read
    read(fd, &read_byte, 1);
    while(read_byte != START_OF_FRAME) {
        read(fd, &read_byte, 1);
    }

    bytes_read = 0;
    encoded_message.data[0] = START_OF_FRAME;

    read(fd, &size, 1);
    encoded_message.data[1] = size;

    // read bytes into encoded message buffer
    data_ptr = &(encoded_message.data[2]);
    for(bytes_read = 0; bytes_read < size; bytes_read++) {
        read(fd, data_ptr, 1);
        if (*data_ptr == ENCODE_BYTE_A){
            // read the second encoded byte
            data_ptr += 1;
            read(fd, data_ptr, 1);
            extra_bytes += 1;
        }
        data_ptr += 1;
    }

    encoded_message.size = 2 + size + extra_bytes;

    decode_can_message(&encoded_message, message);

    usleep(1);

    return 0;
}

int init_sys_message(CANMessage *message, uint8_t api_index)
{
    message->manufacturer = MANUFACTURER_SYS;
    message->device_type = DEVTYPE_SYS;
    message->api_class = API_SYS;
    message->api_index = api_index;
    return 0;
}

int init_jaguar_message(CANMessage *message, uint8_t api_class, uint8_t api_index)
{
    message->manufacturer = MANUFACTURER_TI;
    message->device_type = DEVTYPE_MOTORCTRL;
    message->api_class = api_class;
    message->api_index = api_index;
    return 0;
}

bool valid_sys_reply(CANMessage *message, CANMessage *reply)
{
    return reply->manufacturer == MANUFACTURER_SYS
            && reply->device_type == DEVTYPE_SYS
            && reply->api_class == API_SYS
            && reply->api_index == message->api_index
            && reply->device == message->device;
}

bool valid_jaguar_reply(CANMessage *message, CANMessage *reply)
{
    return reply->manufacturer == MANUFACTURER_TI 
            && reply->device_type == DEVTYPE_MOTORCTRL
            && reply->api_class == message->api_class
            && reply->api_index == message->api_index
            && reply->device == message->device;
}

bool valid_ack(CANMessage *message, CANMessage *ack)
{
    return ack->manufacturer == MANUFACTURER_TI 
            && ack->device_type == DEVTYPE_MOTORCTRL
            && ack->api_class == API_ACK
            && ack->device == message->device;
}

int sys_heartbeat(JaguarConnection *conn, uint8_t device)
{
    CANMessage message;
    init_sys_message(&message, SYS_HEARTBEAT);
    message.device = device;
    message.data_size = 0;
    send_can_message(conn, &message);

    return 0;
}

int sys_sync_update(JaguarConnection *conn, uint8_t mask)
{
    CANMessage message;
    init_sys_message(&message, SYS_SYNC_UPDATE);
    message.device = 0;
    message.data_size = 1;
    message.data[0] = mask;
    send_can_message(conn, &message);

    return 0;
}

int sys_halt(JaguarConnection *conn, uint8_t device)
{
    CANMessage message;
    init_sys_message(&message, SYS_HALT);
    message.device = device;
    message.data_size = 0;
    send_can_message(conn, &message);

    return 0;
}

int sys_reset(JaguarConnection *conn, uint8_t device)
{
    CANMessage message;
    init_sys_message(&message, SYS_RESET);
    message.device = device;
    message.data_size = 0;
    send_can_message(conn, &message);

    return 0;
}

int sys_resume(JaguarConnection *conn, uint8_t device)
{
    CANMessage message;
    init_sys_message(&message, SYS_RESUME);
    message.device = device;
    message.data_size = 0;
    send_can_message(conn, &message);

    return 0;
}

int status_output_percent(JaguarConnection *conn, uint8_t device, 
        int16_t *output_percent)
{
    CANMessage message;
    CANMessage reply;
    CANMessage ack;
    init_jaguar_message(&message, API_STATUS, STATUS_OUTPUT_PERCENT);
    message.device = device;
    message.data_size = 0;
    send_can_message(conn, &message);
    recieve_can_message(conn, &reply);
    recieve_can_message(conn, &ack);

    if (valid_jaguar_reply(&message, &reply) && valid_ack(&message, &ack)) {
        *output_percent = reply.data[0] | reply.data[1] << 8;
        return 0;
    } else {
        return 1;
    }
}

int status_temperature(JaguarConnection *conn, uint8_t device, 
        uint16_t *temperature)
{
    CANMessage message;
    CANMessage reply;
    CANMessage ack;
    init_jaguar_message(&message, API_STATUS, STATUS_TEMPERATURE);
    message.device = device;
    message.data_size = 0;
    send_can_message(conn, &message);
    recieve_can_message(conn, &reply);
    recieve_can_message(conn, &ack);

    if (valid_jaguar_reply(&message, &reply) && valid_ack(&message, &ack)) {
        *temperature = reply.data[0] | reply.data[1] << 8;
        return 0;
    } else {
        return 1;
    }
}

int status_position(JaguarConnection *conn, uint8_t device, uint32_t *position)
{
    CANMessage message;
    CANMessage reply;
    CANMessage ack;
    init_jaguar_message(&message, API_STATUS, STATUS_POSITION);
    message.device = device;
    message.data_size = 0;
    send_can_message(conn, &message);
    recieve_can_message(conn, &reply);
    recieve_can_message(conn, &ack);

    if (valid_jaguar_reply(&message, &reply) && valid_ack(&message, &ack)) {
        *position = reply.data[0] | reply.data[1] << 8 | reply.data[2] << 16 
            | reply.data[3] << 24;
        return 0;
    } else {
        return 1;
    }
}

int status_mode(JaguarConnection *conn, uint8_t device, uint8_t *mode)
{
    CANMessage message;
    CANMessage reply;
    CANMessage ack;
    init_jaguar_message(&message, API_STATUS, STATUS_MODE);
    message.device = device;
    message.data_size = 0;
    send_can_message(conn, &message);
    recieve_can_message(conn, &reply);
    recieve_can_message(conn, &ack);

    if (valid_jaguar_reply(&message, &reply) && valid_ack(&message, &ack)) {
        *mode = reply.data[0];
        return 0;
    } else {
        return 1;
    }
}

int voltage_enable(JaguarConnection *conn, uint8_t device)
{
    CANMessage message;
    CANMessage ack;
    init_jaguar_message(&message, API_VOLTAGE, VOLTAGE_ENABLE);
    message.device = device;
    message.data_size = 0;
    send_can_message(conn, &message);
    recieve_can_message(conn, &ack);

    return 1 - valid_ack(&message, &ack);
}

int voltage_disable(JaguarConnection *conn, uint8_t device)
{
    CANMessage message;
    CANMessage ack;
    init_jaguar_message(&message, API_VOLTAGE, VOLTAGE_DISABLE);
    message.device = device;
    message.data_size = 0;
    send_can_message(conn, &message);
    recieve_can_message(conn, &ack);

    return 1 - valid_ack(&message, &ack);
}

int voltage_set(JaguarConnection *conn, uint8_t device, int16_t voltage)
{
    CANMessage message;
    CANMessage ack;
    init_jaguar_message(&message, API_VOLTAGE, VOLTAGE_SET);
    message.device = device;
    message.data_size = 2;
    message.data[0] = (uint8_t) (voltage & 0x00ff);
    message.data[1] = (uint8_t) (voltage >> 8);
    send_can_message(conn, &message);
    recieve_can_message(conn, &ack);

    return 1 - valid_ack(&message, &ack);
}

int voltage_set_sync(JaguarConnection *conn, uint8_t device, int16_t voltage, 
        uint8_t group)
{
    CANMessage message;
    CANMessage ack;
    init_jaguar_message(&message, API_VOLTAGE, VOLTAGE_SET);
    message.device = device;
    message.data_size = 3;
    message.data[0] = (uint8_t) (voltage & 0x00ff);
    message.data[1] = (uint8_t) (voltage >> 8);
    message.data[2] = group;
    send_can_message(conn, &message);
    recieve_can_message(conn, &ack);

    return 1 - valid_ack(&message, &ack);
}

int voltage_get(JaguarConnection *conn, uint8_t device, int16_t *voltage)
{
    CANMessage message;
    CANMessage reply;
    CANMessage ack;
    init_jaguar_message(&message, API_VOLTAGE, VOLTAGE_SET);
    message.device = device;
    message.data_size = 0;
    send_can_message(conn, &message);
    recieve_can_message(conn, &reply);
    recieve_can_message(conn, &ack);

    if (valid_jaguar_reply(&message, &reply) && valid_ack(&message, &ack)) {
        *voltage = reply.data[0] | reply.data[1] << 8;
        return 0;
    } else {
        return 1;
    }
}

int voltage_ramp(JaguarConnection *conn, uint8_t device, uint16_t ramp)
{
    CANMessage message;
    CANMessage ack;
    init_jaguar_message(&message, API_VOLTAGE, VOLTAGE_RAMP);
    message.device = device;
    message.data_size = 2;
    message.data[0] = (uint8_t) (ramp & 0x00ff);
    message.data[1] = (uint8_t) (ramp >> 8);
    send_can_message(conn, &message);
    recieve_can_message(conn, &ack);

    return 1 - valid_ack(&message, &ack);
}

int position_enable(JaguarConnection *conn, uint8_t device, int32_t position)
{
    CANMessage message;
    CANMessage ack;
    init_jaguar_message(&message, API_POSITION, POSITION_ENABLE);
    message.device = device;
    message.data_size = 4;
    message.data[0] = (uint8_t) (position & 0x000000ff);
    message.data[1] = (uint8_t) (position >> 8 & 0x000000ff);
    message.data[2] = (uint8_t) (position >> 16 & 0x000000ff);
    message.data[3] = (uint8_t) (position >> 24);
    send_can_message(conn, &message);
    recieve_can_message(conn, &ack);

    return 1 - valid_ack(&message, &ack);
}

int position_disable(JaguarConnection *conn, uint8_t device)
{
    CANMessage message;
    CANMessage ack;
    init_jaguar_message(&message, API_POSITION, POSITION_DISABLE);
    message.device = device;
    message.data_size = 0;
    send_can_message(conn, &message);
    recieve_can_message(conn, &ack);

    return 1 - valid_ack(&message, &ack);
}

int position_set(JaguarConnection *conn, uint8_t device, int32_t position)
{
    CANMessage message;
    CANMessage ack;
    init_jaguar_message(&message, API_POSITION, POSITION_SET);
    message.device = device;
    message.data_size = 4;
    message.data[0] = (uint8_t) (position & 0x000000ff);
    message.data[1] = (uint8_t) (position >> 8 & 0x000000ff);
    message.data[2] = (uint8_t) (position >> 16 & 0x000000ff);
    message.data[3] = (uint8_t) (position >> 24);
    send_can_message(conn, &message);
    recieve_can_message(conn, &ack);

    return 1 - valid_ack(&message, &ack);
}

int position_set_sync(JaguarConnection *conn, uint8_t device, int32_t position,
        uint8_t group)
{
    CANMessage message;
    CANMessage ack;
    init_jaguar_message(&message, API_POSITION, POSITION_SET);
    message.device = device;
    message.data_size = 5;
    message.data[0] = (uint8_t) (position & 0x000000ff);
    message.data[1] = (uint8_t) (position >> 8 & 0x000000ff);
    message.data[2] = (uint8_t) (position >> 16 & 0x000000ff);
    message.data[3] = (uint8_t) (position >> 24);
    message.data[4] = group;
    send_can_message(conn, &message);
    recieve_can_message(conn, &ack);

    return 1 - valid_ack(&message, &ack);
}

int position_get(JaguarConnection *conn, uint8_t device, int32_t *position)
{
    CANMessage message;
    CANMessage reply;
    CANMessage ack;
    init_jaguar_message(&message, API_POSITION, POSITION_SET);
    message.device = device;
    message.data_size = 0;
    send_can_message(conn, &message);
    recieve_can_message(conn, &reply);
    recieve_can_message(conn, &ack);

    if (valid_jaguar_reply(&message, &reply) && valid_ack(&message, &ack)) {
        *position = reply.data[0] | reply.data[1] << 8 | reply.data[2] << 16 
            | reply.data[3] << 24;
        return 0;
    } else {
        return 1;
    }
}

int position_ref_encoder(JaguarConnection *conn, uint8_t device)
{
    CANMessage message;
    CANMessage ack;
    init_jaguar_message(&message, API_POSITION, POSITION_REF);
    message.device = device;
    message.data_size = 1;
    message.data[0] = (uint8_t) POSITION_ENCODER;
    send_can_message(conn, &message);
    recieve_can_message(conn, &ack);

    return 1 - valid_ack(&message, &ack);
}

int position_p(JaguarConnection *conn, uint8_t device, int32_t p)
{
    CANMessage message;
    CANMessage ack;
    init_jaguar_message(&message, API_POSITION, POSITION_P);
    message.device = device;
    message.data_size = 4;
    message.data[0] = (uint8_t) (p & 0x000000ff);
    message.data[1] = (uint8_t) (p >> 8 & 0x000000ff);
    message.data[2] = (uint8_t) (p >> 16 & 0x000000ff);
    message.data[3] = (uint8_t) (p >> 24);
    send_can_message(conn, &message);
    recieve_can_message(conn, &ack);

    return 1 - valid_ack(&message, &ack);
}

int position_i(JaguarConnection *conn, uint8_t device, int32_t i)
{
    CANMessage message;
    CANMessage ack;
    init_jaguar_message(&message, API_POSITION, POSITION_I);
    message.device = device;
    message.data_size = 4;
    message.data[0] = (uint8_t) (i & 0x000000ff);
    message.data[1] = (uint8_t) (i >> 8 & 0x000000ff);
    message.data[2] = (uint8_t) (i >> 16 & 0x000000ff);
    message.data[3] = (uint8_t) (i >> 24);
    send_can_message(conn, &message);
    recieve_can_message(conn, &ack);

    return 1 - valid_ack(&message, &ack);
}

int position_d(JaguarConnection *conn, uint8_t device, int32_t d)
{
    CANMessage message;
    CANMessage ack;
    init_jaguar_message(&message, API_POSITION, POSITION_D);
    message.device = device;
    message.data_size = 4;
    message.data[0] = (uint8_t) (d & 0x000000ff);
    message.data[1] = (uint8_t) (d >> 8 & 0x000000ff);
    message.data[2] = (uint8_t) (d >> 16 & 0x000000ff);
    message.data[3] = (uint8_t) (d >> 24);
    send_can_message(conn, &message);
    recieve_can_message(conn, &ack);

    return 1 - valid_ack(&message, &ack);
}

int position_pid(JaguarConnection *conn, uint8_t device, int32_t p, int32_t i, 
        int32_t d)
{
    return position_p(conn, device, p) | position_i(conn, device, i) | 
        position_d(conn, device, d);
}

int config_encoder_lines(JaguarConnection *conn, uint8_t device, uint16_t lines)
{
    CANMessage message;
    CANMessage ack;
    init_jaguar_message(&message, API_CONFIG, CONFIG_ENCODER_LINES);
    message.device = device;
    message.data_size = 2;
    message.data[0] = (uint8_t) (lines & 0x00ff);
    message.data[1] = (uint8_t) (lines >> 8);
    send_can_message(conn, &message);
    recieve_can_message(conn, &ack);

    return 1 - valid_ack(&message, &ack);
}

int get_encoder_lines(JaguarConnection *conn, uint8_t device, uint16_t *lines)
{
    CANMessage message;
    CANMessage reply;
    init_jaguar_message(&message, API_CONFIG, CONFIG_ENCODER_LINES);
    message.device = device;
    message.data_size = 0;
    send_can_message(conn, &message);
    recieve_can_message(conn, &reply);
    
    if (valid_jaguar_reply(&message, &reply)) {
        *lines = reply.data[0] | reply.data[1] << 8;
        return 0;
    } else {
        return 1;
    }
}
  
