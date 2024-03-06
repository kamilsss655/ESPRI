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

#define ECHO_TEST_TXD 16
#define ECHO_TEST_RXD 17
#define ECHO_TEST_RTS 2
#define ECHO_TEST_CTS 4

#define ECHO_UART_PORT_NUM   2
#define ECHO_UART_BAUD_RATE  250000
#define ECHO_TASK_STACK_SIZE 2048

#define BUF_SIZE (1024)

void UART_init(void);
void UART_send(const void *src, size_t size);
void UART_monitor(void *pvParameters);

#endif