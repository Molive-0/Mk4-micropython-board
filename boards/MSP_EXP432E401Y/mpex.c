/*
 * Copyright (c) 2015-2017, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== mpex.c ========
 */

#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

#include <ti/drivers/UART.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/PWM.h>
#include <ti/drivers/I2C.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/SD.h>

#include <ti/sysbios/knl/Task.h>

extern int fdOpenSession(void * task);
extern int fdCloseSession(void * task);

#define STACKSIZE 8192U
#define MPHEAPSIZE (65536 + 14384)

extern int mp_main(void * heap, uint32_t heapsize, uint32_t stacksize, UART_Handle uart);

static uint8_t heap[MPHEAPSIZE];

void * mpThread(void * arg)
{
   UART_Handle uart;
   UART_Params params;

   UART_Params_init(&params);
   params.baudRate = 115200;
   params.writeDataMode = UART_DATA_BINARY;
   params.readDataMode = UART_DATA_BINARY;
   uart = UART_open(0, &params);

   fdOpenSession((void *)Task_self());

   mp_main(heap, sizeof(heap), STACKSIZE, uart);

   fdCloseSession((void *)Task_self());

   UART_close(uart);

   return NULL;
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    GPIO_init();
    I2C_init();
    SD_init();
    SPI_init();
    UART_init();
    PWM_init();

    pthread_t thread;
    pthread_attr_t attrs;

    pthread_attr_init(&attrs);
    pthread_attr_setstacksize(&attrs, STACKSIZE);
    pthread_create(&thread, &attrs, mpThread, NULL);
    pthread_attr_destroy(&attrs);

    sleep(100000);

    return NULL;
}
