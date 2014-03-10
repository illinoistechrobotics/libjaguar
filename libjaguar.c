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

    // discard bytes until start of frame read
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

    return 0;
}

