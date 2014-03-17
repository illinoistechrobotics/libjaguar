#include "can.h"
#include "canutil.h"

int encode_can_message(CANMessage *message, CANEncodedMsg *encoded_message)
{
    int i;
    uint8_t size;
    uint8_t *can_id;
    uint8_t *data_ptr;

    size = HEADER_SIZE + message->data_size;

    // Set start of frame byte
    encoded_message->data[0] = (uint8_t) START_OF_FRAME;

    // Set packet size byte
    encoded_message->data[1] = (uint8_t) (CAN_ID_SIZE + message->data_size);

    // Set CAN identifier
    can_id = &(encoded_message->data[2]);
    can_id[0] = message->device;
    can_id[0] |= message->api_index << 6;
    can_id[1] = message->api_index >> 2;
    can_id[1] |= message->api_class << 2;
    can_id[2] = message->manufacturer;
    can_id[3] = message->device_type;

    // Set data bytes
    data_ptr = &(encoded_message->data[HEADER_SIZE]);
    for (i = 0; i < message->data_size; i++) {
        // Encode special bytes
        if (message->data[i] == START_OF_FRAME) {
            *data_ptr = (uint8_t) ENCODE_BYTE_A;
            data_ptr += 1;
            *data_ptr = (uint8_t) ENCODE_BYTE_A;
            data_ptr += 1;
            size += 1;
        } else if (message->data[i] == ENCODE_BYTE_A) {
            *data_ptr = (uint8_t) ENCODE_BYTE_A;
            data_ptr += 1;
            *data_ptr = (uint8_t) ENCODE_BYTE_B;
            data_ptr += 1;
            size += 1;
        } else {
            // Not a special byte, copy data
            *data_ptr = message->data[i];
            data_ptr += 1;
        }
    }

    encoded_message->size = size;

    return 0;
}

int decode_can_message(CANEncodedMsg *encoded_message, CANMessage *message)
{
    int i;
    uint8_t *can_id;
    uint8_t *data_ptr;
    
    // Get packet size
    message->data_size = (uint8_t) (encoded_message->data[1] - CAN_ID_SIZE);

    // Get CAN identifier
    can_id = &(encoded_message->data[2]);
    message->device = can_id[0] & 0x3F;
    message->api_index = (can_id[0] >> 6) | ((can_id[1] & 0x03) << 2);
    message->api_class = can_id[1] >> 2;
    message->manufacturer = can_id[2];
    message->device_type = can_id[3];
    
    // Get data bytes
    data_ptr = &(encoded_message->data[HEADER_SIZE]);
    for (i = 0; i < message->data_size; i++) {
        // Decode special bytes
        if (*data_ptr == ENCODE_BYTE_A) {
            data_ptr += 1;
            if (*data_ptr == ENCODE_BYTE_A) {
                message->data[i] = (uint8_t) START_OF_FRAME;
                data_ptr += 1;
            } else if (*data_ptr == ENCODE_BYTE_B) {
                message->data[i] = (uint8_t) ENCODE_BYTE_A;
                data_ptr += 1;
            } else {
                // Decoding error
                return 1;
            }
        } else {
            message->data[i] = *data_ptr;
            data_ptr += 1;
        }
    }

    return 0;
}

float fixed16_to_float(uint16_t fx)
{
    float fl;
    fl = (float) (fx >> 8);
    fl += (float) (fx & 0x00ff) / 256.0;
    return fl;
}

float fixed32_to_float(uint32_t fx)
{
    float fl;
    fl = (float) (fx >> 16);
    fl += (float) (fx & 0x0000ffff) / 65536.0;
    return fl;
}
