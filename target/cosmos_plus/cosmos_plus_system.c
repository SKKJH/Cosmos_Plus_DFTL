
#include "xil_cache.h"
#include "xil_exception.h"
#include "xil_mmu.h"
#include "xparameters_ps.h"
#include "xscugic_hw.h"
#include "xscugic.h"
#include "debug.h"

#include "../../hil/nvme/nvme.h"
#include "../../hil/nvme/nvme_main.h"
#include "../../hil/nvme/host_lld.h"

#ifdef WIN32
	#include "bsp_windows.hl"
#endif

#include "cosmos_plus_global_config.h"

XScuGic GicInstance;

void SYSTEM_initialize(void)
{
	unsigned int u;

	XScuGic_Config *IntcConfig;

	Xil_ICacheDisable();
	Xil_DCacheDisable();
	Xil_DisableMMU();

	// Paging table set
#define MB (1024*1024)
	for (u = 0; u < 4096; u++)			// dyseo 4GB,
	{
		if (u < 0x2)
			Xil_SetTlbAttributes(u * MB, 0xC1E); // cached & buffered
		else if (u < 0x180)		// 384MB
			Xil_SetTlbAttributes(u * MB, 0xC12); // uncached & nonbuffered
		else if (u < 0x400)		// 1024MB
			Xil_SetTlbAttributes(u * MB, 0xC1E); // cached & buffered
		else
			Xil_SetTlbAttributes(u * MB, 0xC12); // uncached & nonbuffered
	}

	Xil_EnableMMU();
	Xil_ICacheEnable();
	Xil_DCacheEnable();
	xil_printf("[!] MMU has been enabled.\r\n");

	xil_printf("\r\n Hello COSMOS+ OpenSSD !!! \r\n");

	Xil_ExceptionInit();

	IntcConfig = XScuGic_LookupConfig(XPAR_SCUGIC_SINGLE_DEVICE_ID);
	XScuGic_CfgInitialize(&GicInstance, IntcConfig, IntcConfig->CpuBaseAddress);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
		(Xil_ExceptionHandler)XScuGic_InterruptHandler,
		&GicInstance);

	XScuGic_Connect(&GicInstance, 61,
		(Xil_ExceptionHandler)dev_irq_handler,
		(void *)0);

	XScuGic_Enable(&GicInstance, 61);

#if !defined(WIN32)
	// Enable interrupts in the Processor.
	Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);
	Xil_ExceptionEnable();
#endif

	dev_irq_init();
}

void SYSTEM_Run(void)
{
	// do target system job
#if defined(WIN32) || (NVME_UNIT_TEST == 1)
	// complete HDMA 
	set_tx_dma_done(TRUE);
	set_rx_dma_done(TRUE);
#endif
}

