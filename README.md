# CH32V307_lwIP-1.41_FreeRTOS
将lwIP1.41与FreeRTOS移植到ch32V307上（NETCONN API）  
使用芯片内部的10MPHY层实现TCPIP协议栈    
目前看到网上关于此款mcu移植lwIP的资料比较少，遂根据沁恒提供的官方以太网驱动库和FreeRTOS例程以及老衲五木的博客实现了TCP Echo Sever   
此项目需要使用MounRiver打开，可以在官方赤兔开发板上直接使用    
刚刚移植完成还没有没有进行稳定性测试 后期若不稳定再更新程序
