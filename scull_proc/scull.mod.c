#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
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
	{ 0x9412fa01, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x43b32b30, __VMLINUX_SYMBOL_STR(seq_release) },
	{ 0xc3e1132a, __VMLINUX_SYMBOL_STR(seq_read) },
	{ 0x56e9db97, __VMLINUX_SYMBOL_STR(seq_lseek) },
	{ 0xcc72f4ce, __VMLINUX_SYMBOL_STR(proc_create_data) },
	{ 0x987fde2e, __VMLINUX_SYMBOL_STR(cdev_add) },
	{ 0xb46aca86, __VMLINUX_SYMBOL_STR(cdev_init) },
	{ 0xac7f6dfd, __VMLINUX_SYMBOL_STR(__mutex_init) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x29537c9e, __VMLINUX_SYMBOL_STR(alloc_chrdev_region) },
	{ 0xd8e484f0, __VMLINUX_SYMBOL_STR(register_chrdev_region) },
	{ 0x7485e15e, __VMLINUX_SYMBOL_STR(unregister_chrdev_region) },
	{ 0x353c3b0c, __VMLINUX_SYMBOL_STR(remove_proc_entry) },
	{ 0xd743757b, __VMLINUX_SYMBOL_STR(cdev_del) },
	{ 0x4f6b400b, __VMLINUX_SYMBOL_STR(_copy_from_user) },
	{ 0xfb578fc5, __VMLINUX_SYMBOL_STR(memset) },
	{ 0xd2b09ce5, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0x4f8b5ddb, __VMLINUX_SYMBOL_STR(_copy_to_user) },
	{ 0x6e938e79, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x6d0fc37d, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x1eecf3fc, __VMLINUX_SYMBOL_STR(mutex_unlock) },
	{ 0xa05f372e, __VMLINUX_SYMBOL_STR(seq_printf) },
	{ 0x9c73f8e4, __VMLINUX_SYMBOL_STR(mutex_lock_interruptible) },
	{ 0x4cbdb485, __VMLINUX_SYMBOL_STR(seq_open) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "EC15EE6BD1F2E9A734795DA");
