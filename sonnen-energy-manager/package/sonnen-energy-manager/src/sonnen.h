#ifndef SONNEN_H
#define SONNEN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "protocol.h"
#include "utils.h"
/*
 * Sonnen Battery Energy Manager
 * 
 * Main header file with common declarations and utilities
 */

/* Configuration */
#define SOCKET_PATH "/var/run/sonnen-battery.sock"
#define BACKLOG 5
#define POLL_INTERVAL_SEC 5

/* Server Functions (implemented in server.c) */
int server_create_socket(const char *path);
void server_cleanup(int server_fd);
void server_handle_client(int client_fd);
void server_generate_solar_data(SolarData *data);
void server_process_battery_command(const BatteryCommand *cmd);

/* Client Functions (implemented in client.c) */
int client_connect_socket(const char *path);
int client_request_data(int sock_fd, SolarData *data);
int client_send_command(int sock_fd, const BatteryCommand *cmd);
void client_calculate_battery_action(const SolarData *data, BatteryCommand *cmd);

#endif /* SONNEN_H */
