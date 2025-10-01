#include <sys/stat.h>
#include "sonnen.h"

/**
 * Create and configure server socket
 */
int
server_create_socket(const char *path)
{
    struct sockaddr_un addr;
    int sock_fd;
    mode_t old_umask;

    /* Remove existing socket file */
    if (unlink(path) == -1 && errno != ENOENT) {
        log_error("SERVER", "Failed to unlink socket: %s", strerror(errno));
        return -1;
    }

    old_umask = umask(0077); /* (only owner can use) */

    /* Create socket */
    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        log_error("SERVER", "Failed to create socket: %s", strerror(errno));
        return -1;
    }

    /* Validate socket path length */
    if (strlen(path) >= sizeof(addr.sun_path)) {
        log_error("SERVER", "Socket path too long: %s", path);
        close(sock_fd);
        return -1;
    }

    /* Configure socket address */
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    /* Bind the socket */
    if (bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        log_error("SERVER", "Failed to bind socket: %s", strerror(errno));
        close(sock_fd);
        return -1;
    }

    umask(old_umask);

    /* Listen for connections */
    if (listen(sock_fd, BACKLOG) == -1) {
        log_error("SERVER", "Failed to listen on socket: %s", strerror(errno));
        close(sock_fd);
        return -1;
    }

    return sock_fd;
}

/**
 * Generate simulated solar data
 */
void
server_generate_solar_data(SolarData *data)
{
    /* Simulate PV production (0-5000W) */
    data->pv_production_watts = rand() % 5001;

    /* Simulate household consumption (500-3000W) */
    data->household_consumption_watts = 500 + (rand() % 2501);

    data->timestamp = time(NULL);
}

void server_process_battery_command(const BatteryCommand *cmd) {
    log_info("SERVER", "Battery Command: %s %dW",
             protocol_action_to_string(cmd->action), cmd->power_watts);
}

/**
 * Handle client connection
 */
void
server_handle_client(int client_fd)
{
    uint8_t buffer[256];
    uint8_t msg_type;
    SolarData data;
    BatteryCommand cmd;
    MsgBatteryCommand *msg;
    int ret;

    /* Receive message */
    ret = protocol_receive_message(client_fd, buffer, sizeof(buffer), &msg_type);
    if (ret < 0) {
        log_error("SERVER", "Failed to receive message from client");
        return;
    }

    /* Process based on message type */
    switch (msg_type) {
        case MSG_TYPE_DATA_REQUEST: {
            server_generate_solar_data(&data);

            log_info("SERVER", "Data Request - PV: %dW, Consumption: %dW",
                     data.pv_production_watts, data.household_consumption_watts);

            protocol_send_data_response(client_fd, &data);
            break;
        }

        case MSG_TYPE_BATTERY_COMMAND: {
            msg = (MsgBatteryCommand*)buffer;

            ret = protocol_parse_battery_command(msg, &cmd);
            if (ret == 0) {
                server_process_battery_command(&cmd);
            }else{
                log_error("SERVER", "Failed to parse battery command");
            }
            break;
        }

        default:
            log_error("SERVER", "Unexpected message type: %d\n", msg_type);
            break;
    }
}

/**
 * Cleanup server resources
 */
void
server_cleanup(int server_fd)
{
    if (server_fd >= 0) {
        close(server_fd);
    }

    if (unlink(SOCKET_PATH) == -1 && errno != ENOENT) {
        log_warning("SERVER", "Failed to unlink socket during cleanup: %s", 
                    strerror(errno));
    }
}

/**
 * Main server entry point
 */
int
main(void)
{
    int server_fd, client_fd;

    /* Initialize */
    setup_signal_handlers();
    srand(time(NULL));
    openlog("sonnen-server", LOG_PID | LOG_CONS, LOG_USER);

    log_info("SERVER", "Starting Sonnen Battery Server");

    /* Create server socket */
    server_fd = server_create_socket(SOCKET_PATH);
    if (server_fd < 0) {
        log_error("SERVER", "Failed to create server socket");
        return EXIT_FAILURE;
    }

    log_info("SERVER", "Listening on %s", SOCKET_PATH);

    /* Main server loop */
    while (g_keep_running) {
        client_fd = accept(server_fd, NULL, NULL);

        if (client_fd < 0) {
            if (errno == EINTR) {
                log_debug("SERVER", "accept() interrupted by signal");
                continue;
            }
            log_error("SERVER", "accept() failed: %s", strerror(errno));
            continue;
        }

        log_debug("SERVER", "Client connected (fd=%d)", client_fd);
        server_handle_client(client_fd);
        close(client_fd);
    }

    /* Cleanup */
    log_info("SERVER", "Shutting down gracefully");
    server_cleanup(server_fd);
    closelog();

    return EXIT_SUCCESS;
}
