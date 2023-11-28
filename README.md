L3 routing platform

kernel_module:
	该文件夹下包含了需要添加在内核中的内核模块的相关代码，包括ODS_kernel和route_check
		ODS_kernel:为ODS组件提供内核中的支持
		route_check:为FO中用于辅助的模块，来源于ASL源码

LA：
	该文件下包含了AgentDaemon的实现源码

LA_Client_aodv：
	该文件下包含了aodv路由协议的实现样例

LA_Client_router_demo：
	该文件下包含了一个general application的demo例子

lib：
	该文件下包含了3个组件的实现源码，包括：FO，ND，ODS
		FO:转发表交互组件
		ND：邻居发现组件
		ODS：按需服务组件

xx_test:
	这些文件夹下包含了对上述组件和样例的测试，可以不用关注