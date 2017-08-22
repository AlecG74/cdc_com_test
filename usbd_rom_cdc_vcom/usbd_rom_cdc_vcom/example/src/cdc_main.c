/*
 * @brief Vitual communication port example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#if 0
//#include "board.h"
#include "chip.h"
#include <stdio.h>
#include <string.h>
#include "app_usbd_cfg.h"
#include "cdc_vcom.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

static USBD_HANDLE_T g_hUsb;
static uint8_t g_rxBuff[256];

/* Endpoint 0 patch that prevents nested NAK event processing */
static uint32_t g_ep0RxBusy = 0;/* flag indicating whether EP0 OUT/RX buffer is busy. */
static USB_EP_HANDLER_T g_Ep0BaseHdlr;	/* variable to store the pointer to base EP0 handler */

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

const USBD_API_T *g_pUsbApi;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* EP0_patch part of WORKAROUND for artf45032. */
ErrorCode_t EP0_patch(USBD_HANDLE_T hUsb, void *data, uint32_t event)
{
	switch (event) {
	case USB_EVT_OUT_NAK:
		if (g_ep0RxBusy) {
			/* we already queued the buffer so ignore this NAK event. */
			return LPC_OK;
		}
		else {
			/* Mark EP0_RX buffer as busy and allow base handler to queue the buffer. */
			g_ep0RxBusy = 1;
		}
		break;

	case USB_EVT_SETUP:	/* reset the flag when new setup sequence starts */
	case USB_EVT_OUT:
		/* we received the packet so clear the flag. */
		g_ep0RxBusy = 0;
		break;
	}
	return g_Ep0BaseHdlr(hUsb, data, event);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Handle interrupt from USB
 * @return	Nothing
 */
void USB_IRQHandler(void)
{
	USBD_API->hw->ISR(g_hUsb);
}

/* Find the address of interface descriptor for given class type. */
USB_INTERFACE_DESCRIPTOR *find_IntfDesc(const uint8_t *pDesc, uint32_t intfClass)
{
	USB_COMMON_DESCRIPTOR *pD;
	USB_INTERFACE_DESCRIPTOR *pIntfDesc = 0;
	uint32_t next_desc_adr;

	pD = (USB_COMMON_DESCRIPTOR *) pDesc;
	next_desc_adr = (uint32_t) pDesc;

	while (pD->bLength) {
		/* is it interface descriptor */
		if (pD->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE) {

			pIntfDesc = (USB_INTERFACE_DESCRIPTOR *) pD;
			/* did we find the right interface descriptor */
			if (pIntfDesc->bInterfaceClass == intfClass) {
				break;
			}
		}
		pIntfDesc = 0;
		next_desc_adr = (uint32_t) pD + pD->bLength;
		pD = (USB_COMMON_DESCRIPTOR *) next_desc_adr;
	}

	return pIntfDesc;
}

/**
 * @brief	main routine for blinky example
 * @return	Function should not exit.
 */
int main(void)
{

	USBD_API_INIT_PARAM_T usb_param;
	USB_CORE_DESCS_T desc;
	ErrorCode_t ret = LPC_OK;
	uint32_t prompt = 0, rdCnt = 0;
	USB_CORE_CTRL_T *pCtrl;
	uint32_t i = 0;
	volatile uint32_t delay;

	/* Initialize board and chip */
	SystemCoreClockUpdate();
	//Board_Init();

	/* enable clocks and pinmu
	 * x */
	USB_init_pin_clk();

	/* Init USB API structure */
	g_pUsbApi = (const USBD_API_T *) LPC_ROM_API->usbdApiBase;

	/* initialize call back structures */
	memset((void *) &usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
	usb_param.usb_reg_base = LPC_USB_BASE;
	usb_param.max_num_ep = 4;
	usb_param.mem_base = USB_STACK_MEM_BASE;
	usb_param.mem_size = USB_STACK_MEM_SIZE;

	/* Set the USB descriptors */
	desc.device_desc = (uint8_t *) USB_DeviceDescriptor;
	desc.string_desc = (uint8_t *) USB_StringDescriptor;
#ifdef USE_USB0
	//desc.high_speed_desc = USB_FsConfigDescriptor;
	desc.high_speed_desc = USB_HsConfigDescriptor;
	desc.full_speed_desc = USB_FsConfigDescriptor;
	desc.device_qualifier = (uint8_t *) USB_DeviceQualifier;
#else
	/* Note, to pass USBCV test full-speed only devices should have both
	 * descriptor arrays point to same location and device_qualifier set
	 * to 0.
	 */
	desc.high_speed_desc = USB_FsConfigDescriptor;
	desc.full_speed_desc = USB_FsConfigDescriptor;
	desc.device_qualifier = 0;
#endif

	/* USB Initialization */
	ret = USBD_API->hw->Init(&g_hUsb, &desc, &usb_param);
	if (ret == LPC_OK) {

		/*	WORKAROUND for artf45032 ROM driver BUG:
		    Due to a race condition there is the chance that a second NAK event will
		    occur before the default endpoint0 handler has completed its preparation
		    of the DMA engine for the first NAK event. This can cause certain fields
		    in the DMA descriptors to be in an invalid state when the USB controller
		    reads them, thereby causing a hang.
		 */
		pCtrl = (USB_CORE_CTRL_T *) g_hUsb;	/* convert the handle to control structure */
		g_Ep0BaseHdlr = pCtrl->ep_event_hdlr[0];/* retrieve the default EP0_OUT handler */
		pCtrl->ep_event_hdlr[0] = EP0_patch;/* set our patch routine as EP0_OUT handler */

		/* Init VCOM interface */
		ret = vcom_init(g_hUsb, &desc, &usb_param);
		if (ret == LPC_OK) {
			/*  enable USB interrupts */
			NVIC_EnableIRQ(LPC_USB_IRQ);
			/* now connect */
			USBD_API->hw->Connect(g_hUsb, 1);
		}

	}

	//DEBUGSTR("USB CDC class based virtual Comm port example!\r\n");

	uint32_t size = 225792; 	// size of picture (392*576)
	//uint8_t byRetry = 0;		//received data flag
	uint32_t sendingIndex = 0;	//index of sent data
	//#define tab_size 256	//64;//32;//256;
	//uint8_t  tab[tab_size];


	while (1) {
		/* Check if host has connected and opened the VCOM port */
		if ((prompt == 0) && (vcom_connected() != 0)) {
			//wait to be connected
			prompt = 1;
		}
		/* If VCOM port is opened and received '0' o r '1' */
		if (prompt) {
			rdCnt = vcom_bread(&g_rxBuff[0], 256);
			//function to echo data
			//vcom_write(&g_rxBuff[0], rdCnt);

			if (0 == rdCnt) {
				continue;
			}

			if (g_rxBuff[0] == '0') {
				uint8_t  num[4] ;
				//sending size 8 bytes for int32
				num[0] = (size >> 24) & 0x00FF;
				num[1] = (size >> 16) & 0x00FF;
				num[2] = (size >> 8) & 0x00FF;
				num[3] = size & 0xFF;

				// wait to data sending
				//while (g_vCOM.tx_flags & VCOM_TX_BUSY);

				// send data
				/*do
				{
					sendingIndex += vcom_write(num, 4, sendingIndex);
				}
				while(sendingIndex != 4);
				sendingIndex = 0;*/
				while (0 == vcom_write(num, 4, sendingIndex));

				//while (g_vCOM.tx_flags & VCOM_TX_BUSY);

				//change rx byte array to don't loop
				//g_rxBuff[0] = 0;
			}

			//if data received = '1' send byte array of picture size
			else if (g_rxBuff[0] == '1') {

				//table size of data written
				//#define tab_size 256	//64;//32;//256;
				uint32_t tab_size = 256;	//64;//32;//256;
				uint8_t  tab[tab_size];
				uint32_t ind = 0;
				uint32_t mod = size % tab_size;

				for(i =0 ; i < size; i+=1){

#if 1
					tab[ind] = (uint8_t)(i & 0x00FF);
					ind += 1;

					if(ind == tab_size){

						//tab [252] = (i & 0xFF000000) >> 24;
						//tab [253] = (i & 0x00FF0000) >> 16;
						//tab [254] = (i & 0x0000FF00) >> 8;
						//tab [255] = (i & 0x000000FF) >> 0;

						// wait to data sending
						//while (g_vCOM.tx_flags & VCOM_TX_BUSY);

						// send data
						//do
						//{
						//	sendingIndex += vcom_write(tab, tab_size, sendingIndex);
						//}
						//while(sendingIndex != tab_size);
						//sendingIndex = 0;
						while (0 == vcom_write(tab, tab_size, sendingIndex));

						//delay = 750;

						/* Delay for 250uSec */
						//while(delay--) {}
						//while (g_vCOM.tx_flags & VCOM_TX_BUSY);

						ind =0;
					}
#else
					while (0 == vcom_write(tab, 1, 0));
					delay = 1000;

					/* Delay for 250uSec */
					while(delay--) {}
					//while (g_vCOM.tx_flags & VCOM_TX_BUSY);
#endif

				}
				if(mod > 0){
					for(i =0 ; i < mod; i+=1){
						tab[i] = (uint8_t)(i & 0x00FF);
					}

					// wait to data sending
					//while (g_vCOM.tx_flags & VCOM_TX_BUSY);

					// send data
					/*do
					{
						sendingIndex += vcom_write(tab, mod, sendingIndex);
					}
					while(sendingIndex != mod);
					sendingIndex = 0;*/
					while (0 == vcom_write(tab, mod, sendingIndex));

					//while (g_vCOM.tx_flags & VCOM_TX_BUSY);
				}
				/*if (i == 225280)
				{
					continue;
				}*/

				//change rx byte array to don't loop
				//g_rxBuff[0] = 0;
				//delay = 0x10000;

				/* Delay for 250uSec */
				//while(delay--) {}
				//while (g_vCOM.tx_flags & VCOM_TX_BUSY);

			}


		}

		/* Sleep until next IRQ happens */
		__WFI();
	}
}

/*
 *
 * **********************************************************************************************************************
 *
 * **********************************************************************************************************************
 * **********************************************************************************************************************
 */
#else
/*
 * @brief Vitual communication port example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

//#include "board.h"
#include "chip.h"
#include <stdio.h>
#include <string.h>
#include "app_usbd_cfg.h"
#include "cdc_vcom.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

static USBD_HANDLE_T g_hUsb;
static uint8_t g_rxBuff[256];

/* Endpoint 0 patch that prevents nested NAK event processing */
static uint32_t g_ep0RxBusy = 0;/* flag indicating whether EP0 OUT/RX buffer is busy. */
static USB_EP_HANDLER_T g_Ep0BaseHdlr;	/* variable to store the pointer to base EP0 handler */

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

const USBD_API_T *g_pUsbApi;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* EP0_patch part of WORKAROUND for artf45032. */
ErrorCode_t EP0_patch(USBD_HANDLE_T hUsb, void *data, uint32_t event)
{
	switch (event) {
	case USB_EVT_OUT_NAK:
		if (g_ep0RxBusy) {
			/* we already queued the buffer so ignore this NAK event. */
			return LPC_OK;
		}
		else {
			/* Mark EP0_RX buffer as busy and allow base handler to queue the buffer. */
			g_ep0RxBusy = 1;
		}
		break;

	case USB_EVT_SETUP:	/* reset the flag when new setup sequence starts */
	case USB_EVT_OUT:
		/* we received the packet so clear the flag. */
		g_ep0RxBusy = 0;
		break;
	}
	return g_Ep0BaseHdlr(hUsb, data, event);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Handle interrupt from USB
 * @return	Nothing
 */
void USB_IRQHandler(void)
{
	USBD_API->hw->ISR(g_hUsb);
}

/* Find the address of interface descriptor for given class type. */
USB_INTERFACE_DESCRIPTOR *find_IntfDesc(const uint8_t *pDesc, uint32_t intfClass)
{
	USB_COMMON_DESCRIPTOR *pD;
	USB_INTERFACE_DESCRIPTOR *pIntfDesc = 0;
	uint32_t next_desc_adr;

	pD = (USB_COMMON_DESCRIPTOR *) pDesc;
	next_desc_adr = (uint32_t) pDesc;

	while (pD->bLength) {
		/* is it interface descriptor */
		if (pD->bDescriptorType == USB_INTERFACE_DESCRIPTOR_TYPE) {

			pIntfDesc = (USB_INTERFACE_DESCRIPTOR *) pD;
			/* did we find the right interface descriptor */
			if (pIntfDesc->bInterfaceClass == intfClass) {
				break;
			}
		}
		pIntfDesc = 0;
		next_desc_adr = (uint32_t) pD + pD->bLength;
		pD = (USB_COMMON_DESCRIPTOR *) next_desc_adr;
	}

	return pIntfDesc;
}

/* Sending function */
void write_serial_data(uint8_t *pBuf, uint32_t len){

	uint32_t index = 0;

	do
	{
		index += vcom_write(pBuf, len, index);

	}
	while(index != len);

	//delay = 1000;
	/* Delay for 250uSec */
	//while(delay--) {}
	while (g_vCOM.tx_flags & VCOM_TX_BUSY);
}


/**
 * @brief	main routine for blinky example
 * @return	Function should not exit.
 */
int main(void)
{

	USBD_API_INIT_PARAM_T usb_param;
	USB_CORE_DESCS_T desc;
	ErrorCode_t ret = LPC_OK;
	uint32_t prompt = 0, rdCnt = 0;
	USB_CORE_CTRL_T *pCtrl;
	#define tab_size 256 //512
	uint8_t  tab[tab_size] ;
	volatile uint32_t delay;
	uint32_t size = 225792; // size of picture (392*576)

	/* Initialize board and chip */
	SystemCoreClockUpdate();
	//Board_Init();

	/* enable clocks and pinmux */
	USB_init_pin_clk();

	/* Init USB API structure */
	g_pUsbApi = (const USBD_API_T *) LPC_ROM_API->usbdApiBase;

	/* initialize call back structures */
	memset((void *) &usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
	usb_param.usb_reg_base = LPC_USB_BASE;
	usb_param.max_num_ep = 4;
	usb_param.mem_base = USB_STACK_MEM_BASE;
	usb_param.mem_size = USB_STACK_MEM_SIZE;

	/* Set the USB descriptors */
	desc.device_desc = (uint8_t *) USB_DeviceDescriptor;
	desc.string_desc = (uint8_t *) USB_StringDescriptor;
	desc.string_desc = (uint8_t *) USB_StringDescriptor;

#ifdef USE_USB0
	//desc.high_speed_desc = USB_FsConfigDescriptor;
	desc.high_speed_desc = USB_HsConfigDescriptor;
	desc.full_speed_desc = USB_FsConfigDescriptor;
	desc.device_qualifier = (uint8_t *) USB_DeviceQualifier;
#else
	/* Note, to pass USBCV test full-speed only devices should have both
	 * descriptor arrays point to same location and device_qualifier set
	 * to 0.
	 */
	desc.high_speed_desc = USB_FsConfigDescriptor;
	desc.full_speed_desc = USB_FsConfigDescriptor;
	desc.device_qualifier = 0;
#endif

	/* USB Initialization */
	ret = USBD_API->hw->Init(&g_hUsb, &desc, &usb_param);
	if (ret == LPC_OK) {

		/*	WORKAROUND for artf45032 ROM driver BUG:
		    Due to a race condition there is the chance that a second NAK event will
		    occur before the default endpoint0 handler has completed its preparation
		    of the DMA engine for the first NAK event. This can cause certain fields
		    in the DMA descriptors to be in an invalid state when the USB controller
		    reads them, thereby causing a hang.
		 */
		pCtrl = (USB_CORE_CTRL_T *) g_hUsb;	/* convert the handle to control structure */
		g_Ep0BaseHdlr = pCtrl->ep_event_hdlr[0];/* retrieve the default EP0_OUT handler */
		pCtrl->ep_event_hdlr[0] = EP0_patch;/* set our patch routine as EP0_OUT handler */

		/* Init VCOM interface */
		ret = vcom_init(g_hUsb, &desc, &usb_param);
		if (ret == LPC_OK) {
			/*  enable USB interrupts */
			NVIC_EnableIRQ(LPC_USB_IRQ);
			/* now connect */
			USBD_API->hw->Connect(g_hUsb, 1);
		}

	}

	//DEBUGSTR("USB CDC class based virtual Comm port example!\r\n");


	while (1) {
		/* Check if host has connected and opened the VCOM port */
		if ((prompt == 0) && (vcom_connected() != 0)) {
			//wait to be connected
			prompt = 1;
		}
		/* If VCOM port is opened and received '0' o r '1' */
		if (prompt) {
			rdCnt = vcom_bread(&g_rxBuff[0], 256);
			//function to echo data
			//vcom_write(&g_rxBuff[0], rdCnt);

			if (0 == rdCnt) {
				continue;
			}

			if (g_rxBuff[0] == '0') {
				uint8_t  num[4] ;
				//sending size 8 bytes for int32
				num[0] = (size >> 24) & 0x00FF;
				num[1] = (size >> 16) & 0x00FF;
				num[2] = (size >> 8) & 0x00FF;
				num[3] = size & 0xFF;
				//while(0 == vcom_write( &num, 4));
				//while (g_vCOM.tx_flags & VCOM_TX_BUSY);
				write_serial_data(&num, 4);

				//change rx byte array to don't loop
				//g_rxBuff[0] = 0;
			}

			//if data received = '1' send byte array of picture size
			else if (g_rxBuff[0] == '1') {

				//table size of data written
				uint32_t ind = 0;
				uint32_t mod = size % tab_size;

				for(uint32_t i =0 ; i < size; i+=1){

					tab[ind] = (uint8_t)(i & 0x0000FF);
					ind += 1;

					if(ind == tab_size){
						//wait to data sending
						//while (0 == vcom_write( &tab, tab_size));
						//while (g_vCOM.tx_flags & VCOM_TX_BUSY);
						write_serial_data(&tab, tab_size);

						ind =0;
					}
				}
				if(mod > 0){
					for(uint32_t i =0 ; i < mod; i+=1){
						tab[i] = (uint8_t)(i & 0x00FF);
					}
					write_serial_data(&tab, mod);
					//while (0 == vcom_write( &tab, mod));
					//while (g_vCOM.tx_flags & VCOM_TX_BUSY);
				}
				//change rx byte array to don't loop
				//g_rxBuff[0] = 0;
			}
			delay = 0x10000;

			/* Delay for 250uSec */
			while(delay--) {}
			while (g_vCOM.tx_flags & VCOM_TX_BUSY);

		}
		/* Sleep until next IRQ happens */
		//__WFI();
	}
}
#endif
