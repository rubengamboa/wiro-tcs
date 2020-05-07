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
	{ 0x41086e, "module_layout" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0xec88c4a4, "pci_unregister_driver" },
	{ 0xcfdbba1a, "__register_chrdev" },
	{ 0x7997cb91, "__pci_register_driver" },
	{ 0xf3cc0b6c, "pci_enable_device" },
	{ 0x42c8de35, "ioremap_nocache" },
	{ 0xc2db98b4, "pci_request_regions" },
	{ 0x93b38b05, "dev_get_drvdata" },
	{ 0xc041dc0c, "pci_release_regions" },
	{ 0xf6d62bd3, "dev_set_drvdata" },
	{ 0xedc03953, "iounmap" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0xb2fd5ceb, "__put_user_4" },
	{ 0xa1c76e0a, "_cond_resched" },
	{ 0xb72397d5, "printk" },
	{ 0x37a0cba, "kfree" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0xf2a644fb, "copy_from_user" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("pci:v000010B5d00009050sv000010B5sd00001172bc*sc*i*");

MODULE_INFO(srcversion, "0A84B6ED9FE373E6EA7CEEA");

static const struct rheldata _rheldata __used
__attribute__((section(".rheldata"))) = {
	.rhel_major = 6,
	.rhel_minor = 10,
	.rhel_release = 754,
};
#ifdef RETPOLINE
	MODULE_INFO(retpoline, "Y");
#endif
