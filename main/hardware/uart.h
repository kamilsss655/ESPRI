/* Copyright 2024 kamilsss655
 * https://github.com/kamilsss655
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#ifndef HARDWARE_UART_H
#define HARDWARE_UART_H

#include <stddef.h>

#define ECHO_TEST_TXD CONFIG_UART_TX_GPIO
#define ECHO_TEST_RXD CONFIG_UART_RX_GPIO

#define ECHO_UART_PORT_NUM   1
#define ECHO_UART_BAUD_RATE  CONFIG_UART_BAUD_RATE
#define ECHO_TASK_STACK_SIZE 2048

#define BUF_SIZE (1024)

void UART_Init(void);
void UART_Send(const void *src, size_t size);
void UART_Monitor(void *pvParameters);

#endif