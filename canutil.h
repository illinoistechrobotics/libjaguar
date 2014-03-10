#include "can.h"

#ifndef CANUTIL_H
#define CANUTIL_H

int encode_can_message(CANMessage *message, CANEncodedMsg *encoded_message);
int decode_can_message(CANEncodedMsg *encoded_message, CANMessage *message);

#endif
