# 【EG800Z-CN】TCP 客户端示例

### 项目概述

本案例使用移远通信EG800Z-CN开发板和UniRTOS，调用UniRTOS中Socket相关功能函数编写。让开发板成为TCP客户端，远程连接其他TCP服务器，进行数据交互。

### 功能特性

**阻塞式TCP客户端**

- **端到端连接自动化**：集成“蜂窝网络附着 → PDP上下文激活 → DNS域名解析 → TCP连接建立”全流程，实现从设备上电到与远程服务器建立可靠通信链路。
- **健壮的数据会话管理**：在成功建立TCP连接后，执行预设次数的“发送请求-接收响应”数据交互循环，并内置对send/read操作结果的严格校验，确保会话的可靠性与完整性。

### 开发准备

#### 硬件要求

- EG800Z-CN开发板，[点此购买开发板](https://www.quecmall.com/goods-detail/2c90800b987f06090198aca7bde100a6)

​	<img src=".\media\开发板实物图.jpg">

- USB数据线（TYPE-C），[点此购买](https://detail.tmall.com/item.htm?abbucket=11&id=712043397690&mi_id=0000UuATUkl2Swill--d8ar3-R828dAfvrmApTj3VzPdxhA&ns=1&priceTId=214783fc17750971433067563e1379&skuId=5825460040081&spm=a21n57.1.hoverItem.4&utparam={"aplus_abtest"%3A"d39c694c59ac1c7b55f24ab87fd2bb30"}&xxc=taobaoSearch)

​	<img src=".\media\数据线.png">

- 有效SIM卡（可发短信）

​	<img src=".\media\SIM.png">

#### 软件要求

- Quectel USB驱动，[点此获取](https://www.quectel.com.cn/download/quectel_windows_usb_drivery_v1-0_cn)
- UniRTOS SDK，获取请联系[技术与支持](https://www.quectel.com.cn/contact?tab=t)。
- EPAT工具：移芯平台日志调试工具，[点此获取](https://www.quectel.com.cn/download/epat日志工具)

### 快速上手

#### 下载项目

示例代码位于本案例`src`目录下

#### 添加项目到UniRTOS SDK

CSDK新增Demo，固件编译和烧录请参考UniRTOS板块的**快速启动栏**

#### 硬件连接

​	<img src=".\media\connect.png" width="50%">

1. 按卡槽丝印提示方向拨开卡槽盖，将SIM卡放入，再扣好盖子
2. 使用数据线连接开发板和电脑

#### 软件部署

connectlab新建服务器，将真实的服务器地址和端口填入项目代码中

​	<img src=".\media\connectlab.png" width="80%">

#### 效果展示

实操效果可查看当前目录下media文件夹中的.mp4视频，日志如图

​	<img src="./media/Log.png" width="80%">

### 代码概览

#### 示例流程图

​	<img src="./media/tcp_socket_test.png" width="60%">

#### 主要功能接口

##### unir_test_demo_init

**功能**：TCP 阻塞客户端演示的入口与初始化函数。负责创建独立任务，让 TCP 通信逻辑在后台运行，不阻塞主程序。
**关键操作**：

- 任务创建：调用 **qosa_task_create** 创建名为 `app_block` 的任务，执行 `qcm_socket_app_block_process`。
- 任务配置：栈大小 4096，普通优先级，保证 TCP 流程稳定运行。
- **重要性**：用户需在应用初始化时调用，用于启动整个 TCP 客户端通信功能。

##### qcm_socket_app_block_process

**功能**：TCP 阻塞客户端主处理函数。完成联网、DNS 解析、Socket 创建、连接服务器、收发数据全套流程。
**关键操作**：

- 等待网络就绪：延时 10 秒等待模组注网。
- 激活 PDP 联网：调用 `qcm_socket_app_datacall_active` 确保数据链路可用。
- DNS 解析：通过 `qosa_dns_syn_getaddrinfo` 获取服务器 IP。
- 创建 Socket：`qcm_socket_create` 创建**阻塞式 TCP Socket**。
- 连接服务器：`qcm_socket_connect` 连接目标 IP 与端口。
- 循环收发：最多发送 20 次数据，等待服务器回传（阻塞等待）。
- 关闭连接：通信完成后调用 `qcm_socket_close` 释放资源。
- **重要性**：完整封装 TCP 客户端标准流程，是物联网设备 TCP 通信的核心参考。

##### qcm_socket_app_datacall_active

**功能**：PDP 数据链路激活函数。检查并激活蜂窝网络连接，为 TCP 通信提供网络基础。
**关键操作**：

- 创建 DataCall 对象：`qosa_datacall_conn_new`。
- 查询 IP 信息：判断 PDP 是否已激活。
- 未激活则启动拨号：`qosa_datacall_start` 激活 PDN 连接。
- **重要性**：TCP 通信必须依赖可用网络，此函数确保网络就绪。