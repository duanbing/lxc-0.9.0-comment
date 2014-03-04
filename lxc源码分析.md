# lxc 源码分析

## main_loop 
* 封装了epoll相关操作，提供事件创建，删除和分发

## nl 
* 封装跟内核的异步通信机制netlink,实现全双工的通信，用户空间中使用标准socket API，支持多播。

## namespace 
*  支持在clone子进程的时候指定	CLONE_NEWNS, CLONE_NEWPID, CLONE_NEWUTS, CLONE_NEWIPC,
	CLONE_NEWUSER, CLONE_NEWNET。

## network 
*  首先，什么是网桥呢？ 可以简单的这么看，网桥是一个连接2个局域网的设备，是一个“底层的路由器”。根据重点地址来选择要发送的局域网网段。使用方法见[这里](http://wiki.dzsc.com/info/8659.html)


