#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xbc257400, "module_layout" },
	{ 0xf7c510ad, "class_destroy" },
	{ 0x37a0cba, "kfree" },
	{ 0xad3e5417, "cdev_del" },
	{ 0x9bce482f, "__release_region" },
	{ 0x9ce4bded, "device_destroy" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0x7fb1b141, "device_create" },
	{ 0xa316da0c, "cdev_add" },
	{ 0x1f648773, "cdev_init" },
	{ 0xadf42bd5, "__request_region" },
	{ 0x59d8223a, "ioport_resource" },
	{ 0x91715312, "sprintf" },
	{ 0xd6367922, "kmem_cache_alloc_trace" },
	{ 0xa98b8bdc, "kmalloc_caches" },
	{ 0xbd59f82a, "__class_create" },
	{ 0x50eedeb8, "printk" },
	{ 0x29537c9e, "alloc_chrdev_region" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "61A800D34F9DBA476D41272");
