#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x4cf819e6, "module_layout" },
	{ 0x696f0c3, "kmalloc_caches" },
	{ 0x54b1fac6, "__ubsan_handle_load_invalid_value" },
	{ 0x302e596e, "ip_local_out" },
	{ 0x27da177, "dst_release" },
	{ 0xc3690fc, "_raw_spin_lock_bh" },
	{ 0xa90d0f9d, "dev_get_by_name" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x11089ac7, "_ctype" },
	{ 0x9da249b, "netlink_kernel_release" },
	{ 0x670ecece, "__x86_indirect_thunk_rbx" },
	{ 0x346cedcc, "kfree_skb_reason" },
	{ 0xb9686614, "netlink_unicast" },
	{ 0xf5595723, "init_net" },
	{ 0x4dcf276e, "nf_register_net_hook" },
	{ 0x7fa46beb, "nf_unregister_net_hook" },
	{ 0x390840f6, "ip_route_input_noref" },
	{ 0x8b7ed2c3, "__alloc_skb" },
	{ 0xe46021ca, "_raw_spin_unlock_bh" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x92997ed8, "_printk" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x4f00afd3, "kmem_cache_alloc_trace" },
	{ 0xc5327655, "ip_route_output_flow" },
	{ 0xdbc1f32, "__netlink_kernel_create" },
	{ 0x37a0cba, "kfree" },
	{ 0x3ab7a2db, "__nlmsg_put" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "473C09906119848854EB32D");
