/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifdef __cplusplus
extern "C" {
#endif

void *run_tcp_server(uint16_t port); // return the context, NULL on error
err_t tcp_server_close(void *context);
bool tcp_server_is_running(void *context);

err_t tcp_server_send(void *arg, const char *data, int datalen);
int tcp_server_receive(void *arg, char *buffer, int maxlen);

#ifdef __cplusplus
}
#endif

