/**************************************************************************//**
 * @file     uart_transfer.c
 * @version  V1.00
 * $Date: 14/11/17 5:36p $
 * @brief    General UART ISP slave Sample file
 *
 * @note
 * SPDX-License-Identifier: Apache-2.0
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

/*!<Includes */
#include <string.h>
#include "NuMicro.h"
#include "uart_transfer.h"
#include "targetdev.h"

#ifdef __ICCARM__
#pragma data_alignment=4
uint8_t uart_rcvbuf[MAX_PKT_SIZE] = {0};

#else
__attribute__((aligned(4))) uint8_t uart_rcvbuf[MAX_PKT_SIZE] = {0};

#endif

uint8_t volatile bUartDataReady = 0;
uint8_t volatile bufhead = 0;

/* please check "targetdev.h" for chip specifc define option */

/*---------------------------------------------------------------------------------------------------------*/
/* INTSTS to handle UART Channel 0 interrupt event                                                            */
/*---------------------------------------------------------------------------------------------------------*/

#if defined (ENABLE_ISP_UART0) || defined (ENABLE_ISP_UART2)
void UART02_IRQHandler(void)
{
    #if defined (ENABLE_ISP_UART0)
	UART_T* uart = UART0;
    #elif defined (ENABLE_ISP_UART2)
	UART_T* uart = UART2;
	#endif

    /*----- Determine interrupt source -----*/
    uint32_t u32IntSrc = uart->INTSTS;

    if (u32IntSrc & 0x11)   /*RDA FIFO interrupt & RDA timeout interrupt*/
    {
        while (((uart->FIFOSTS & UART_FIFOSTS_RXEMPTY_Msk) == 0) && (bufhead < MAX_PKT_SIZE))      /*RX fifo not empty*/
        {
            uart_rcvbuf[bufhead++] = uart->DAT;
        }
    }

    if (bufhead == MAX_PKT_SIZE)
    {
        bUartDataReady = TRUE;
        bufhead = 0;
    }
    else if (u32IntSrc & 0x10)
    {
        bufhead = 0;
    }

}
#elif defined (ENABLE_ISP_UART1)
void UART13_IRQHandler(void)
{
	UART_T* uart = UART1;

    /*----- Determine interrupt source -----*/
    uint32_t u32IntSrc = uart->INTSTS;

    if (u32IntSrc & 0x11)   /*RDA FIFO interrupt & RDA timeout interrupt*/
    {
        while (((uart->FIFOSTS & UART_FIFOSTS_RXEMPTY_Msk) == 0) && (bufhead < MAX_PKT_SIZE))      /*RX fifo not empty*/
        {
            uart_rcvbuf[bufhead++] = uart->DAT;
        }
    }

    if (bufhead == MAX_PKT_SIZE)
    {
        bUartDataReady = TRUE;
        bufhead = 0;
    }
    else if (u32IntSrc & 0x10)
    {
        bufhead = 0;
    }
}
#endif


#ifdef __ICCARM__
#pragma data_alignment=4
extern uint8_t response_buff[64];
#else
extern __attribute__((aligned(4))) uint8_t response_buff[64];
#endif 


void PutString(void)
{
    uint32_t i;

    #if defined (ENABLE_ISP_UART0)
	UART_T* uart = UART0;
    #elif defined (ENABLE_ISP_UART1)
	UART_T* uart = UART1;
    #elif defined (ENABLE_ISP_UART2)
	UART_T* uart = UART2;
	#endif
	
    for (i = 0; i < MAX_PKT_SIZE; i++)
    {
        while ((uart->FIFOSTS & UART_FIFOSTS_TXFULL_Msk));

        uart->DAT = response_buff[i];
    }
	
}


void UART_Init(void)
{
    #if defined (ENABLE_ISP_UART0)
	UART_T* uart = UART0;
    #elif defined (ENABLE_ISP_UART1)
	UART_T* uart = UART1;
    #elif defined (ENABLE_ISP_UART2)
	UART_T* uart = UART2;
	#endif

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Select UART function mode */
    uart->FUNCSEL = ((uart->FUNCSEL & (~UART_FUNCSEL_FUNCSEL_Msk)) | UART_FUNCSEL_MODE);
    /* Set UART line configuration */
    uart->LINE = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;
    /* Set UART Rx and RTS trigger level */
    uart->FIFO = UART_FIFO_RFITL_14BYTES | UART_FIFO_RTSTRGLV_14BYTES;
    /* Set UART baud rate */
    uart->BAUD = (UART_BAUD_MODE2 | UART_BAUD_MODE2_DIVIDER(__HIRC, 115200));
    /* Set time-out interrupt comparator */
    uart->TOUT = (uart->TOUT & ~UART_TOUT_TOIC_Msk) | (0x40);

	#if defined (ENABLE_ISP_UART0) || defined (ENABLE_ISP_UART2)	
    NVIC_SetPriority(UART02_IRQn, 2);
    NVIC_EnableIRQ(UART02_IRQn);
	#elif defined (ENABLE_ISP_UART1)
    NVIC_SetPriority(UART13_IRQn, 2);
    NVIC_EnableIRQ(UART13_IRQn);
	#endif
	
    /* 0x0811 */
    uart->INTEN = (UART_INTEN_TOCNTEN_Msk | UART_INTEN_RXTOIEN_Msk | UART_INTEN_RDAIEN_Msk);

}

