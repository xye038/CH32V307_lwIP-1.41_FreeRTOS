/********************************** (C) COPYRIGHT *******************************
* File Name          : my_eth_driver.h
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : This file contains the headers of the ETH Driver.
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* SPDX-License-Identifier: Apache-2.0
*******************************************************************************/
#ifndef __JUBEI_ETH_DRIVER__
#define __JUBEI_ETH_DRIVER__

#include "debug.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/init.h"
#include "lwip/timers.h"
#include "lwip/tcp_impl.h"
#include "lwip/ip_frag.h"
#include "lwip/tcpip.h"


/* MII/MDI interface select */
#define PHY_ADDRESS 0x01

#define USE10BASE_T
#ifndef USE10BASE_T
//    #define USE_GIGA_MAC
    #ifndef USE_GIGA_MAC
        #define USE_FAST_MAC
        //#define USE_RMII
    #endif
#endif

#define ETH_RXBUFNB        6
#define ETH_TXBUFNB        2

#if 0
#define USE_LOOP_STRUCT    1
#else
#define USE_CHAIN_STRUCT   1
#endif

#define ROM_CFG_USERADR_ID                   0x1FFFF7E8     //mac地址在flash中的地址

//Static IP ADDRESS
#define IP_ADDR0       192
#define IP_ADDR1       168
#define IP_ADDR2       1
#define IP_ADDR3       50
//子网掩码
#define NETMASK_ADDR0  255
#define NETMASK_ADDR1  255
#define NETMASK_ADDR2  255
#define NETMASK_ADDR3  0
//网关地址
#define GW_ADDR0       192
#define GW_ADDR1       168
#define GW_ADDR2       1
#define GW_ADDR3       1

/* Globe variable */
extern ETH_DMADESCTypeDef  *DMATxDescToSet;
extern ETH_DMADESCTypeDef  *DMARxDescToGet;

extern __attribute__ ((aligned(4))) ETH_DMADESCTypeDef DMARxDscrTab[ETH_RXBUFNB];/* 发送描述符表 */
extern __attribute__ ((aligned(4))) ETH_DMADESCTypeDef DMATxDscrTab[ETH_TXBUFNB];/* 接收描述符表 */
extern __attribute__ ((aligned(4))) uint8_t Rx_Buff[ETH_RXBUFNB][ETH_MAX_PACKET_SIZE];/* 接收队列 */
extern __attribute__ ((aligned(4))) uint8_t Tx_Buff[ETH_TXBUFNB][ETH_MAX_PACKET_SIZE];/* 发送队列 */


extern u8 MACAddr[6];    /*Mac地址*/
extern struct netif lwip_netif;                //定义一个全局的网络接口
typedef struct
{
    u32 length;
    u32 buffer;
    ETH_DMADESCTypeDef *descriptor;
}FrameTypeDef;


FrameTypeDef ETH_RxPkt_ChainMode(void);
uint32_t ETH_TxPkt_ChainMode(u16 FrameLength);
void mac_send(uint8_t * content_ptr, uint16_t content_len);
uint32_t ETH_Init(ETH_InitTypeDef* ETH_InitStruct, uint16_t PHYAddress);
void Ethernet_Configuration(void);
void PHY_control_pin_init(void);
void GETH_pin_init(void);
void FETH_pin_init(void);
void SET_MCO(void);
void WCH_GetMacAddr(uint8_t *p);
void Board_ETH_initialization(void);
void LwIP_Init(void);
err_t ethernetif_input(struct netif *netif);
err_t ethernetif_init(struct netif *netif);
#endif
