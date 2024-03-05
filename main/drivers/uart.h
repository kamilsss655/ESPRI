#define ECHO_TEST_TXD 16
#define ECHO_TEST_RXD 17
#define ECHO_TEST_RTS 2
#define ECHO_TEST_CTS 4

#define ECHO_UART_PORT_NUM      2
#define ECHO_UART_BAUD_RATE     250000
#define ECHO_TASK_STACK_SIZE    2048

#define BUF_SIZE (1024)

void UART_run(void);