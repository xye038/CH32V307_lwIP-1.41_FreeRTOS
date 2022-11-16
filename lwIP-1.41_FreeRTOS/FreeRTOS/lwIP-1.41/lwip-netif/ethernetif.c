/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "lwip/opt.h"
#include "Jubei_eth_driver.h"
#include "string.h"

#if 1 /* don't build, this is only a skeleton, see previous comment */

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/etharp.h"
#include "netif/ppp_oe.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
};


/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void
low_level_init(struct netif *netif)
{
  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;
  WCH_GetMacAddr(&MACAddr[0]);
  /* set MAC hardware address */
  netif->hwaddr[0] = MACAddr[0];
  netif->hwaddr[1] = MACAddr[1];
  netif->hwaddr[2] = MACAddr[2];
  netif->hwaddr[3] = MACAddr[3];
  netif->hwaddr[4] = MACAddr[4];
  netif->hwaddr[5] = MACAddr[5];

  /* maximum transfer unit */
  netif->mtu = 1500;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

  ETH_MACAddressConfig(ETH_MAC_Address0, netif->hwaddr); //向ch32V307MAC地址寄存器写入MAC地址
#if LWIP_IPV6 && LWIP_IPV6_MLD
  /*
   * For hardware/netifs that implement MAC filtering.
   * All-nodes link-local is handled by default, so we must let the hardware know
   * to allow multicast packets in.
   * Should set mld_mac_filter previously. */
  if (netif->mld_mac_filter != NULL) {
    ip6_addr_t ip6_allnodes_ll;
    ip6_addr_set_allnodes_linklocal(&ip6_allnodes_ll);
    netif->mld_mac_filter(netif, &ip6_allnodes_ll, NETIF_ADD_MAC_FILTER);
  }
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */

  /* Do whatever else is needed to initialize interface. */
  Board_ETH_initialization();
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become available since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
    u8 res;
    struct pbuf *q;
    int l = 0;
    u8 *buffer=(u8 *)ETH_GetCurrentTxBufferAddress();
    for(q=p;q!=NULL;q=q->next)
    {
        memcpy((u8_t*)&buffer[l], q->payload, q->len);
        l=l+q->len;
    }
    res=ETH_TxPkt_ChainMode(l);
    if(res==ETH_ERROR)return ERR_MEM;

    return ERR_OK;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *
low_level_input(struct netif *netif)
{
    struct pbuf *p, *q;
    u16_t len;
    int l =0;
    FrameTypeDef frame;
    u8 *buffer;
    p = NULL;
    frame=ETH_RxPkt_ChainMode();
    len=frame.length;//得到包的大小
    buffer=(u8 *)frame.buffer;//得到包数据地址
    p=pbuf_alloc(PBUF_RAW,len,PBUF_POOL);//pbufs内存池分配pbuf
    if(p!=NULL)
    {
        for(q=p;q!=NULL;q=q->next)
        {
            memcpy((u8_t*)q->payload,(u8_t*)&buffer[l], q->len);
            l=l+q->len;
        }
    }
    frame.descriptor->Status=ETH_DMARxDesc_OWN;//设置Rx描述符OWN位，buffer重归ETH DMA
    if((ETH->DMASR&ETH_DMASR_RBUS)!=(u32)RESET)//当Rx Buffer不可用位（RBUS）被设置的时候，重置它，恢复传输
    {
        ETH->DMASR=ETH_DMASR_RBUS;//重置ETH DMA RBUS位
        ETH->DMARPDR=0;//恢复DMA接收
    }
    return p;
}

/**
*网卡接收数据（lwip直接调用）
*netif:网卡结构体指针
 */
err_t
ethernetif_input(struct netif *netif)
{
    err_t err;

    struct pbuf *p;
    p=low_level_input(netif);   //调用low_level_input函数接收数据   eth外设接收到的最原始的数据
    if(p==NULL) return ERR_MEM;
    err=netif->input(p, netif); //调用netif结构体中的input字段（一个函数）来处理数据包   调用ethernet_input回调函数
    if(err!=ERR_OK)
    {
        pbuf_free(p);
        p = NULL;
    }
    return err;
}

/**
调用low_level_init()函数来初始化网络
netif:网卡结构体指针
 */
err_t
ethernetif_init(struct netif *netif)
{
    LWIP_ASSERT("netif!=NULL",(netif!=NULL));
#if LWIP_NETIF_HOSTNAME         //LWIP_NETIF_HOSTNAME
    netif->hostname="lwip";     //
#endif
    netif->name[0]=IFNAME0;     //初始化变量netif的name字段
    netif->name[1]=IFNAME1;     //
    netif->output=etharp_output;//IP层发送数据包函数
    netif->linkoutput=low_level_output;//ARP模块发送数据包函数
    low_level_init(netif);      //底层硬件初始化函数
    return ERR_OK;
}

#endif /* 0 */
