#include "sonnen.h"

/**
 * Connect to server socket
 */
int
client_connect_socket(const char *path)
{
    struct sockaddr_un addr;
    int sock_fd, ret;

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    ret = connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr)); 
    if (ret == -1) {
        perror("connect");
        close(sock_fd);
        return -1;
    }

    return sock_fd;
}

/**
 * Request solar data from server
 */
int
client_request_data(int sock_fd, SolarData *data)
{
    uint8_t buffer[256];
    uint8_t msg_type;
    MsgDataResponse *msg;
    int ret;

    /* Send data request */
    ret = protocol_send_data_request(sock_fd) ;
    if (ret < 0) {
        return -1;
    }

    /* Receive response */
    ret = protocol_receive_message(sock_fd, buffer, sizeof(buffer), &msg_type); 
    if (ret < 0) {
        return -1;
    }

    /* Verify message type */
    if (msg_type != MSG_TYPE_DATA_RESPONSE) {
        log_error("CLIENT", "Expected data response, got type %d\n", msg_type);
        return -1;
    }

    /* Parse response */
    msg = (MsgDataResponse*)buffer;
    return protocol_parse_data_response(msg, data);
}

/**
 * Send battery command to server
 */
int
client_send_command(int sock_fd, const BatteryCommand *cmd)
{
    return protocol_send_battery_command(sock_fd, cmd);
}

/**
 * Calculate battery action based on energy management algorithm
 */
void
client_calculate_battery_action(const SolarData *data, BatteryCommand *cmd)
{
    int32_t surplus = data->pv_production_watts - data->household_consumption_watts;

    if (surplus > 0) {
        cmd->power_watts = surplus;
        cmd->action = ACTION_CHARGE;
    } else {
        cmd->power_watts = -surplus;
        cmd->action = ACTION_DISCHARGE;
    }

    cmd->timestamp = time(NULL);
}

/**
 * Main client entry point
 */
int
main(void)
{
    SolarData data;
    BatteryCommand cmd;
    int sock_fd, ret;

    setup_signal_handlers();
    openlog("sonnen-client", LOG_PID | LOG_CONS, LOG_USER);

    log_info("CLIENT", "Starting Sonnen Battery Client");

    while (g_keep_running) {
        sock_fd = client_connect_socket(SOCKET_PATH);
        if (sock_fd < 0) {
            log_error("CLIENT", "Failed to connect, retrying in %d seconds...\n", 
                    POLL_INTERVAL_SEC);
            sleep(POLL_INTERVAL_SEC);
            continue;
        }

        ret = client_request_data(sock_fd, &data); 
        if (ret == 0) {
            client_calculate_battery_action(&data, &cmd);

            log_info("CLIENT", "PV: %dW, Consumption: %dW -> %s %dW",
                        data.pv_production_watts,
                        data.household_consumption_watts,
                        protocol_action_to_string(cmd.action),
                        cmd.power_watts);

            close(sock_fd);
            sock_fd = client_connect_socket(SOCKET_PATH);

            if (sock_fd >= 0) {
                client_send_command(sock_fd, &cmd);
                close(sock_fd);
            }
        } else {
            log_error("CLIENT", "Failed to get data from server\n");
            close(sock_fd);
        }

        sleep(POLL_INTERVAL_SEC);
    }

    log_info("CLIENT", "Shutting down");
    closelog();

    return EXIT_SUCCESS;
}
