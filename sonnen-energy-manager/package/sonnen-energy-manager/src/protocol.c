#include "protocol.h"
#include "sonnen.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

/**
 * Send data request message
 */
int 
protocol_send_data_request(int sock_fd) 
{
    MsgDataRequest msg;

    msg.type = MSG_TYPE_DATA_REQUEST;
    msg.timestamp = (uint64_t)time(NULL);

    ssize_t bytes_written = write(sock_fd, &msg, sizeof(msg));
    if (bytes_written != sizeof(msg)) {
        log_error("PROTOCOL", "Failed to send data request: %s", strerror(errno));
        return -1;
    }

    return 0;
}

/**
 * Send data response message
 */
int 
protocol_send_data_response(int sock_fd, const SolarData *data) 
{
    MsgDataResponse msg;
    ssize_t  bytes_written;

    msg.type = MSG_TYPE_DATA_RESPONSE;
    msg.timestamp = (uint64_t)data->timestamp;
    msg.pv_production_watts = data->pv_production_watts;
    msg.household_consumption_watts = data->household_consumption_watts;

    bytes_written = write(sock_fd, &msg, sizeof(msg));
    if (bytes_written != sizeof(msg)) {
        log_error("PROTOCOL", "Failed to send data response: %s", strerror(errno));
        return -1;
    }

    return 0;
}

/**
 * Send battery command message
 */
int 
protocol_send_battery_command(int sock_fd, const BatteryCommand *cmd) 
{
    MsgBatteryCommand msg;

    msg.type = MSG_TYPE_BATTERY_COMMAND;
    msg.timestamp = (uint64_t)cmd->timestamp;
    msg.power_watts = cmd->power_watts;
    msg.action = cmd->action;

    ssize_t bytes_written = write(sock_fd, &msg, sizeof(msg));
    if (bytes_written != sizeof(msg)) {
        log_error("PROTOCOL", "Failed to send battery command: %s", strerror(errno));
        return -1;
    }

    return 0;
}

/**
 * Receive message (generic, works for any message type)
 */
int 
protocol_receive_message(int sock_fd, void *buffer, size_t max_size, 
                         uint8_t *msg_type) 
{
    /* First read the message type (1 byte) */
    ssize_t bytes_read = read(sock_fd, msg_type, 1);
    if (bytes_read != 1) {
        if (bytes_read < 0) {
            log_error("PROTOCOL", "Failed to read message type: %s", strerror(errno));
        } else if (bytes_read == 0) {
            /* log_debug("PROTOCOL", "Connection closed by peer"); */
        }
        return -1;
    }

    /* Determine remaining size based on message type */
    size_t remaining_size = 0;

    switch (*msg_type) {
        case MSG_TYPE_DATA_REQUEST:
            remaining_size = sizeof(MsgDataRequest) - 1;
            break;
        case MSG_TYPE_DATA_RESPONSE:
            remaining_size = sizeof(MsgDataResponse) - 1;
            break;
        case MSG_TYPE_BATTERY_COMMAND:
            remaining_size = sizeof(MsgBatteryCommand) - 1;
            break;
        default:
            log_error("PROTOCOL", "Unknown message type: %d", *msg_type);
            return -1;
    }

    if (remaining_size > max_size - 1) {
        log_error("PROTOCOL", "Buffer too small for message type %d (need %zu, have %zu)", 
                  *msg_type, remaining_size + 1, max_size);
        return -1;
    }

    /* Store type in buffer */
    ((uint8_t*)buffer)[0] = *msg_type;

    /* Read remaining bytes */
    bytes_read = read(sock_fd, (uint8_t*)buffer + 1, remaining_size);
    if (bytes_read != (ssize_t)remaining_size) {
        if (bytes_read < 0) {
            log_error("PROTOCOL", "Failed to read message body: %s", strerror(errno));
        } else if (bytes_read == 0) {
            log_warning("PROTOCOL", "Connection closed during message read");
        } else {
            log_error("PROTOCOL", "Partial read: expected %zu bytes, got %zd bytes", 
                      remaining_size, bytes_read);
        }
        return -1;
    }

    /* log_debug("PROTOCOL", "Received %s message (%zu bytes)", 
              protocol_type_to_string(*msg_type), remaining_size + 1); */

    return 0;
}

/**
 * Parse data response message
 */
int 
protocol_parse_data_response(const MsgDataResponse *msg, SolarData *data) 
{
    if (!msg || !data) {
        log_error("PROTOCOL", "NULL pointer in parse_data_response");
        return -1;
    }

    data->pv_production_watts = msg->pv_production_watts;
    data->household_consumption_watts = msg->household_consumption_watts;
    data->timestamp = (time_t)msg->timestamp;

    /* log_debug("PROTOCOL", "Parsed data response: PV=%dW, Consumption=%dW", 
              data->pv_production_watts, data->household_consumption_watts); */

    return 0;
}

/**
 * Parse battery command message
 */
int 
protocol_parse_battery_command(const MsgBatteryCommand *msg, BatteryCommand *cmd) 
{
    if (!msg || !cmd) {
        log_error("PROTOCOL", "NULL pointer in parse_battery_command");
        return -1;
    }

    cmd->power_watts = msg->power_watts;
    cmd->action = msg->action;
    cmd->timestamp = (time_t)msg->timestamp;

    /* log_debug("PROTOCOL", "Parsed battery command: %s %dW", 
              protocol_action_to_string(cmd->action), cmd->power_watts); */

    return 0;
}

/**
 * Convert action code to string
 */
const char* 
protocol_action_to_string(uint8_t action) 
{
    return (action == ACTION_CHARGE) ? "charge" : "discharge";
}

/**
 * Convert message type to string
 */
const char* 
protocol_type_to_string(uint8_t msg_type) 
{
    switch (msg_type) {
        case MSG_TYPE_DATA_REQUEST:   return "DATA_REQUEST";
        case MSG_TYPE_DATA_RESPONSE:  return "DATA_RESPONSE";
        case MSG_TYPE_BATTERY_COMMAND: return "BATTERY_COMMAND";
        case MSG_TYPE_ERROR:          return "ERROR";
        default:                      return "UNKNOWN";
    }
}
