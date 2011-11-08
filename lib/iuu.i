/*
 *  iuutool - a port of WBE's Infinity USB Unlimited SDK
 * 
 *  Copyright (C) 2006 Juan Carlos Borrás
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

%module iuu
%include typemaps.i

%{
#include <iuu.h>
%}

%inline %{
	int error_code(iuu_error e) {
		return (int) e;
	}
%}

/* pity I have to copy this here */
struct usb_infinity {
   struct usb_device *dev;
   struct usb_dev_handle *handle;
   struct usb_endpoint_descriptor *ep_in, *ep_out;
};
typedef struct usb_infinity iuu;

iuu_error iuu_ndevs(int *OUTPUT);
iuu_error iuu_start(iuu*, int);
iuu_error iuu_stop(iuu*);
iuu_error iuu_cts(iuu*);

/*
iuu_error iuu_read(iuu *inf, u_int8_t *buf, int len);
iuu_error iuu_write(iuu *inf, u_int8_t *buf, int len);
*/

iuu_error iuu_nop(iuu*);
iuu_error iuu_firmware(iuu *, char *);
iuu_error iuu_name(iuu*, char *OUTPUT);
iuu_error iuu_loader(iuu*, char *OUTPUT);
iuu_error iuu_status(iuu *inf, unsigned char *OUTPUT);
iuu_error iuu_led(iuu*, short, short, short, unsigned char f);
iuu_error iuu_vcc(iuu *inf, enum iuu_vcc_t vcc);
iuu_error iuu_clk(iuu *inf, int freq);
iuu_error iuu_reset(iuu *inf, unsigned char wt);

// Phoenix interface related commands 
iuu_error iuu_uart_on(iuu *inf);
iuu_error iuu_uart_off(iuu *inf);
iuu_error iuu_uart_set(iuu *inf, iuu_uart_baudrate br,
											 iuu_uart_parity parity, 
											 iuu_uart_stopbits stopbits);
iuu_error iuu_uart_baud(iuu *inf, u_int32_t baud, u_int32_t *actual,
												iuu_uart_parity parity);
iuu_error iuu_uart_rx(iuu *inf, u_int8_t *data, u_int8_t *len);
iuu_error iuu_uart_tx(iuu *inf, u_int8_t *data, u_int8_t len);
iuu_error iuu_uart_trap(iuu *inf, u_int8_t wt, u_int8_t cmdbyte);
iuu_error iuu_uart_break(iuu *inf, u_int8_t wt, u_int8_t cmdbyte);
iuu_error iuu_uart_flush(iuu *inf);



 


