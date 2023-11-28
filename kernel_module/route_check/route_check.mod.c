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
	{ 0xeb233a45, "__kmalloc" },
	{ 0xa81d3524, "remove_proc_entry" },
	{ 0x3c3ff9fd, "sprintf" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xf6897c7, "proc_mkdir" },
	{ 0x11089ac7, "_ctype" },
	{ 0x9ec6ca96, "ktime_get_real_ts64" },
	{ 0xf5595723, "init_net" },
	{ 0x4dcf276e, "nf_register_net_hook" },
	{ 0x61651be, "strcat" },
	{ 0x7fa46beb, "nf_unregister_net_hook" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x92997ed8, "_printk" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x4f00afd3, "kmem_cache_alloc_trace" },
	{ 0xe240f6e9, "proc_create_data" },
	{ 0x37a0cba, "kfree" },
	{ 0x88db9f48, "__check_object_size" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "B775836982698D464F32123");
