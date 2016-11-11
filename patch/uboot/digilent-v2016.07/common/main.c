/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* #define	DEBUG	*/

#include <common.h>
#include <autoboot.h>
#include <cli.h>
#include <console.h>
#include <version.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak void show_boot_progress(int val) {}

static void run_preboot_environment_command(void)
{
#ifdef CONFIG_PREBOOT
	char *p;

	p = getenv("preboot");
	if (p != NULL) {
# ifdef CONFIG_AUTOBOOT_KEYED
		int prev = disable_ctrlc(1);	/* disable Control C checking */
# endif

		run_command_list(p, -1, 0);

# ifdef CONFIG_AUTOBOOT_KEYED
		disable_ctrlc(prev);	/* restore Control C checking */
# endif
	}
#endif /* CONFIG_PREBOOT */
}

#define VDMA_BASEADDR_PHY		(0x83000000)
#define VIDEO_BASEADDR0  (0x2000000)
#define VIDEO_BASEADDR1  (0x3000000)
#define VIDEO_BASEADDR2  (0x4000000)
#define H_ACTIVE            800
#define V_ACTIVE            480
#define VTC_BASEADDR_PHY (0x83c00000)
#define MIZ702_VTC_S00_AXI_SLV_REG0_OFFSET 0
#define MIZ702_VTC_S00_AXI_SLV_REG1_OFFSET 4
#define MIZ702_VTC_S00_AXI_SLV_REG2_OFFSET 8
#define MIZ702_VTC_S00_AXI_SLV_REG3_OFFSET 12
#define MIZ702_VTC_S00_AXI_SLV_REG4_OFFSET 16
#define MIZ702_VTC_S00_AXI_SLV_REG5_OFFSET 20
#define MIZ702_VTC_S00_AXI_SLV_REG6_OFFSET 24
#define MIZ702_VTC_S00_AXI_SLV_REG7_OFFSET 28




//for vdma register write directly
static void vdma_register_write_for_test(void)
{
#define VDMA_BASEADDR_WILD	(VDMA_BASEADDR_PHY)
	//char * pvdma_base_vir = ioremap(VDMA_BASEADDR_PHY, 1024*1024);
	writel(0x4,(VDMA_BASEADDR_WILD+0x30));
	writel(0x8,(VDMA_BASEADDR_WILD+0x30));
	writel(VIDEO_BASEADDR0,(VDMA_BASEADDR_WILD+0xAC));
	writel(VIDEO_BASEADDR0,(VDMA_BASEADDR_WILD+0xAC+4));
	writel(VIDEO_BASEADDR0,(VDMA_BASEADDR_WILD+0xAC+8));
	writel(0x00000C80,(VDMA_BASEADDR_WILD+0xA8));
	writel(H_ACTIVE*4,(VDMA_BASEADDR_WILD+0xA4));
	writel(0x3,(VDMA_BASEADDR_WILD+0x30));
	writel(V_ACTIVE,(VDMA_BASEADDR_WILD+0xA0));
	writel(0x4,(VDMA_BASEADDR_WILD+0x00));
	writel(0x8,(VDMA_BASEADDR_WILD+0x00));
	writel(VIDEO_BASEADDR0,(VDMA_BASEADDR_WILD+0x05c));
	writel(VIDEO_BASEADDR0,(VDMA_BASEADDR_WILD+0x060));
	writel(VIDEO_BASEADDR0,(VDMA_BASEADDR_WILD+0x064));
	writel(0x00000C80,(VDMA_BASEADDR_WILD+0x058));
	writel(H_ACTIVE*4,(VDMA_BASEADDR_WILD+0x054));
	writel(0x00000003,(VDMA_BASEADDR_WILD+0x000));
	writel(V_ACTIVE,(VDMA_BASEADDR_WILD+0x050));
	//iounmap(pvdma_base_vir);


#define VTC_BASEADDR		(VTC_BASEADDR_PHY)
	//char * pVTC_base_vir = ioremap(VTC_BASEADDR_PHY, 1024*1024);

#if 1	
	//vtc config
	writel(((1078<<16)|536),(VTC_BASEADDR+MIZ702_VTC_S00_AXI_SLV_REG1_OFFSET));
	writel(((22<<16)|11),(VTC_BASEADDR+MIZ702_VTC_S00_AXI_SLV_REG2_OFFSET));
	writel(((210<<16)|22),(VTC_BASEADDR+MIZ702_VTC_S00_AXI_SLV_REG3_OFFSET));
	writel(((46<<16)|23),(VTC_BASEADDR+MIZ702_VTC_S00_AXI_SLV_REG4_OFFSET));
	writel(0x04,(VTC_BASEADDR+MIZ702_VTC_S00_AXI_SLV_REG5_OFFSET));
#endif

	printf("led register = [0x%x]\n",readl(VTC_BASEADDR+MIZ702_VTC_S00_AXI_SLV_REG1_OFFSET));
	printf("vtc register = [0x%x]\n",readl(0x43c00010));

	
	//iounmap(pVTC_base_vir);
	printf("set register completed\n");

	//do {
	//}while(1);

}




/* We come here after U-Boot is initialised and ready to process commands */
void main_loop(void)
{
	const char *s;

	vdma_register_write_for_test();

	bootstage_mark_name(BOOTSTAGE_ID_MAIN_LOOP, "main_loop");

#ifdef CONFIG_VERSION_VARIABLE
	setenv("ver", version_string);  /* set version variable */
#endif /* CONFIG_VERSION_VARIABLE */

	cli_init();

	

	run_preboot_environment_command();

#if defined(CONFIG_UPDATE_TFTP)
	update_tftp(0UL, NULL, NULL);
#endif /* CONFIG_UPDATE_TFTP */

	s = bootdelay_process();
	if (cli_process_fdt(&s))
		cli_secure_boot_cmd(s);

	autoboot_command(s);

	cli_loop();
	panic("No CLI available");
}
