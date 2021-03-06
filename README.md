# lxc 源码分析

## main_loop 
* 封装了epoll相关操作，提供事件创建，删除和分发

## nl 
* 封装跟内核的异步通信机制netlink,实现全双工的通信，用户空间中使用标准socket API，支持多播。

## namespace 
*  支持在clone子进程的时候指定	CLONE_NEWNS, CLONE_NEWPID, CLONE_NEWUTS, CLONE_NEWIPC,
	CLONE_NEWUSER, CLONE_NEWNET。

## network 
*  首先，什么是网桥呢？ 可以简单的这么看，网桥是一个连接2个局域网的设备，是一个“底层的路由器”。根据重点地址来选择要发送的局域网网段。使用方法见[这里](http://wiki.dzsc.com/info/8659.html).
&emsp;&emsp如下图：主机A发送的报文被送到交换机S1的eth0口，由于eth0与eth1、eth2桥接在一起，故而报文被复制到eth1和eth2，并且发送出 去，然后被主机B和交换机S2接收到。而S2又会将报文转发给主机C、D。
[网桥的例子](!http://img.dnbcw.info/20101115/peae2458860.jpg)


## lxc_start
*  lxc_caps_init  设置用户为实际用户，设置权能为permitted视图
*  lxc_conf_init~lxc_config_define_load   初始化lxc_conf ,加载资源配额的defines
*  lxc_start  在这里首先是清关闭除了lxc相关的其他fd（lxc_log_fd，fd_to_ignore，0,1,2）。调用__lxc_start。

> 
**lxc_init** 
* 进程的权能检查，初始化unix域套接字服务器，用于在console中接受用户命令
* 设置container的当前状态为STARTING，将本实例的一些基本信息（lxc_name,conf_file,rootfs，console_path等进行setenv），并且调用pre-start阶段的回调
* 接下来创建控制终端tty,并且设置console的peer，用于container跟用户进行交互。
* 初始化一个signalfd，用于console的master和slave进程进行信号传递

>
**must_drop_cap_sys_boot**  
* 检查container是否集成reboot权能

>
**lxc_spawn**
* 设置支持多种命名空间和用户自定义的网络命名空间的新的进程组。创建并保存网络接口的信息
* 通过生成 ${rootfs}.hold保持根文件系统可写
* clone子进程。子进程使用do_start初始化。  子进程和父进程的通过socketpair进行初始化的控制
* 赋予登录用户的ptys权限

>
**lxc_poll**
* 新建epoll专用的描叙符，注册事件，