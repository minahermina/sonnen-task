#ifndef SONNEN_PROTOCOL_H
#define SONNEN_PROTOCOL_H

#include <stdint.h>
#include <time.h>
#include <sys/types.h>

/*
 * Protocol Module
 * 
 * Handles all protocol operations for Sonnen Battery communication.
 * Uses packed binary structs over Unix domain sockets.
 * 
 * This module abstracts the protocol layer, making it easy to swap
 * implementations (e.g., switch to JSON, protobuf, or add encryption).
 */

/* Message Types */
typedef enum {
    MSG_TYPE_DATA_REQUEST,
    MSG_TYPE_DATA_RESPONSE,
    MSG_TYPE_BATTERY_COMMAND,
    MSG_TYPE_ERROR
} MessageType;

/* Protocol Messages (packed for network transmission) */

typedef struct __attribute__((packed)) {
    uint8_t type;           // MSG_TYPE_DATA_REQUEST
    uint64_t timestamp;     // Unix timestamp
} MsgDataRequest;

typedef struct __attribute__((packed)) {
    uint8_t type;                      // MSG_TYPE_DATA_RESPONSE
    uint64_t timestamp;                // Unix timestamp
    int32_t pv_production_watts;       // PV panel production
    int32_t household_consumption_watts; // House consumption
} MsgDataResponse;

typedef struct __attribute__((packed)) {
    uint8_t type;           // MSG_TYPE_BATTERY_COMMAND
    uint64_t timestamp;     // Unix timestamp
    int32_t power_watts;    // Power amount
    uint8_t action;         // 0=discharge, 1=charge
} MsgBatteryCommand;

typedef struct {
    int32_t pv_production_watts;
    int32_t household_consumption_watts;
    time_t timestamp;
} SolarData;

typedef struct {
    int32_t power_watts;
    uint8_t action;  // 0=discharge, 1=charge
    time_t timestamp;
} BatteryCommand;

/* Action Constants */
#define ACTION_DISCHARGE 0
#define ACTION_CHARGE 1

/* Protocol Function Prototypes */

/* Send Operations */
int protocol_send_data_request(int sock_fd);
int protocol_send_data_response(int sock_fd, const SolarData *data);
int protocol_send_battery_command(int sock_fd, const BatteryCommand *cmd);

/* Receive Operations */
int protocol_receive_message(int sock_fd, void *buffer, size_t max_size, uint8_t *msg_type);

/* Parse/Convert Operations */
int protocol_parse_data_response(const MsgDataResponse *msg, SolarData *data);
int protocol_parse_battery_command(const MsgBatteryCommand *msg, BatteryCommand *cmd);

/* Utility */
const char* protocol_action_to_string(uint8_t action);
const char* protocol_type_to_string(uint8_t msg_type);

#endif /* SONNEN_PROTOCOL_H */
