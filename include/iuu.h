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

#ifndef _IUU_H_
#define _IUU_H_

enum iuu_uart_parity_t {
   IUU_PARITY_NONE = 0x00,
   IUU_PARITY_EVEN = 0x01,
   IUU_PARITY_ODD = 0x02,
   IUU_PARITY_MARK = 0x03,
   IUU_PARITY_SPACE = 0x04
};
typedef enum iuu_uart_parity_t iuu_uart_parity;

enum iuu_status_register_t {
   IUU_SC_INSERTED = 0x01,
   IUU_VERIFY_ERROR = 0x02,
   IUU_SIM_INSERTED = 0x04
};
typedef enum iuu_status_register_t iuu_status_register;

enum iuu_uart_stopbits_t {
   IUU_TWO_STOP_BITS = 0x00,
   IUU_ONE_STOP_BIT = 0x20
};
typedef enum iuu_uart_stopbits_t iuu_uart_stopbits;

//  enum values are important since they are 
//  code for the right values of the internal
//  timersource and t1 reload value */
enum iuu_uart_baudrate_t {
   IUU_BAUD_2400 = 0x0398,
   IUU_BAUD_9600 = 0x0298,
   IUU_BAUD_19200 = 0x0164,
   IUU_BAUD_28800 = 0x0198,
   IUU_BAUD_38400 = 0x01B2,
   IUU_BAUD_57600 = 0x0030,
   IUU_BAUD_115200 = 0x0098
};
typedef enum iuu_uart_baudrate_t iuu_uart_baudrate;

enum iuu_clk_t {
   IUU_CLK_3579000 = 3579000,
   IUU_CLK_3680000 = 3680000,
   IUU_CLK_6000000 = 6000000
};

enum iuu_status_t {
   IUU_FULLCARD_IN = 0x01,
   IUU_DEV_ERROR = 0x02,
   IUU_MINICARD_IN = 0x04
};

enum iuu_vcc_t {
   IUU_VCC_5V = 0x00,           // 5.0V
   IUU_VCC_3V = 0x01            // 3.3V
};

// IUU related errors 
enum iuu_error_t {
   IUU_OPERATION_OK = 0x00,
   IUU_DEVICE_NOT_FOUND = 0x01,
   IUU_INVALID_HANDLE = 0x02,
   IUU_INVALID_PARAMETER = 0x03,
   IUU_INVALID_INTERFACE = 0x04,
   IUU_INVALID_REQUEST_LENGTH = 0x05,
   IUU_UART_NOT_ENABLED = 0x06,
   IUU_WRITE_ERROR = 0x07,
   IUU_READ_ERROR = 0x08,
   IUU_TX_ERROR = 0x09,
   IUU_RX_ERROR = 0x0A
};
typedef enum iuu_error_t iuu_error;

struct usb_infinity {
   struct usb_device *dev;
   struct usb_dev_handle *handle;
   struct usb_endpoint_descriptor *ep_in, *ep_out;
   // Consider to add here a char iuu_fifo_buf[256] datatype
};
typedef struct usb_infinity iuu;

// General IUU commands
iuu_error iuu_ndevs(int *numdev);
iuu_error iuu_start(iuu * inf, int devnum);
iuu_error iuu_stop(iuu * inf);
iuu_error iuu_cts(iuu * inf);
iuu_error iuu_read(iuu * inf, u_int8_t * buf, int len);
iuu_error iuu_write(iuu * inf, u_int8_t * buf, int len);
iuu_error iuu_nop(iuu * inf);
iuu_error iuu_firmware(iuu * inf, char *ver);
iuu_error iuu_name(iuu * inf, char *name);
iuu_error iuu_loader(iuu * inf, char *ver);
iuu_error iuu_status(iuu * inf, u_int8_t * st);
iuu_error iuu_led(iuu * inf, u_int16_t R, u_int16_t G, u_int16_t B,
                  u_int8_t f);
iuu_error iuu_vcc(iuu * inf, enum iuu_vcc_t vcc);
iuu_error iuu_clk(iuu * inf, int freq);
iuu_error iuu_reset(iuu * inf, u_int8_t wt);

// Phoenix interface related commands 
iuu_error iuu_uart_on(iuu * inf);
iuu_error iuu_uart_off(iuu * inf);
iuu_error iuu_uart_set(iuu * inf, iuu_uart_baudrate br,
                       iuu_uart_parity parity,
                       iuu_uart_stopbits stopbits);
iuu_error iuu_uart_baud(iuu * inf, u_int32_t baud, u_int32_t * actual,
                        iuu_uart_parity parity);
iuu_error iuu_uart_rx(iuu * inf, u_int8_t * data, u_int8_t * len);
iuu_error iuu_uart_tx(iuu * inf, u_int8_t * data, u_int8_t len);
iuu_error iuu_uart_txnops(iuu * inf, u_int8_t * data, u_int8_t len,
                          u_int8_t nops);
iuu_error iuu_uart_txm(iuu * inf, u_int8_t * data, u_int8_t len,
                       u_int8_t ms);
iuu_error iuu_uart_txmu(iuu * inf, u_int8_t * data, u_int8_t len,
                        u_int8_t mus);
iuu_error iuu_uart_trap(iuu * inf, u_int8_t wt, u_int8_t cmdbyte);
iuu_error iuu_uart_break(iuu * inf, u_int8_t wt, u_int8_t cmdbyte);
iuu_error iuu_uart_flush(iuu * inf);

// EEPROM through device related commands
iuu_error iuu_eeprom_on(iuu * inf);
iuu_error iuu_eeprom_off(iuu * inf);
iuu_error iuu_eeprom_write(iuu * inf, u_int8_t ctrl, u_int8_t addr,
                           u_int8_t data);
iuu_error iuu_eeprom_writex(iuu * inf, u_int8_t ctrl, u_int16_t addr,
                            u_int8_t data);
iuu_error iuu_eeprom_write8(iuu * inf, u_int8_t ctrl, u_int8_t addr,
                            u_int8_t * data);
iuu_error iuu_eeprom_write16(iuu * inf, u_int8_t ctrl, u_int8_t addr,
                             u_int8_t * data);
iuu_error iuu_eeprom_writex32(iuu * inf, u_int8_t ctrl, u_int16_t addr,
                              u_int8_t * data);
iuu_error iuu_eeprom_writex64(iuu * inf, u_int8_t ctrl, u_int16_t addr,
                              u_int8_t * data);
iuu_error iuu_eeprom_read(iuu * inf, u_int8_t ctrl, u_int8_t addr,
                          u_int8_t * data);
iuu_error iuu_eeprom_readx(iuu * inf, u_int8_t ctrl, u_int16_t addr,
                           u_int8_t * data);
iuu_error iuu_eeprom_bread(iuu * inf, u_int8_t ctrl, u_int8_t addr,
                           u_int8_t n, u_int8_t * data);
iuu_error iuu_eeprom_breadx(iuu * inf, u_int8_t ctrl, u_int16_t addr,
                            u_int8_t n, u_int8_t * data);

// AVR based cards related commands
iuu_error iuu_avr_on(iuu * inf);
iuu_error iuu_avr_off(iuu * inf);
iuu_error iuu_avr_1clk(iuu * inf);
iuu_error iuu_avr_reset(iuu * inf);
iuu_error iuu_avr_resetpc(iuu * inf);
iuu_error iuu_avr_inc(iuu * inf);
iuu_error iuu_avr_incn(iuu * inf, u_int8_t n);
iuu_error iuu_avr_pread(iuu * inf, u_int8_t * data);
iuu_error iuu_avr_preadn(iuu * inf, u_int8_t * data, unsigned char n);
iuu_error iuu_avr_pwrite(iuu * inf, u_int8_t * data);
iuu_error iuu_avr_pwriten(iuu * inf, u_int8_t * data, u_int8_t len);
iuu_error iuu_avr_dread(iuu * inf, u_int8_t * data);
iuu_error iuu_avr_dreadn(iuu * inf, u_int8_t * data, u_int8_t len);
iuu_error iuu_avr_dwrite(iuu * inf, u_int8_t data);

// PIC based cards related commands
iuu_error iuu_pic_cmd(iuu * inf, u_int8_t cmd);
iuu_error iuu_pic_cmd_load(iuu * inf, u_int8_t cmd, u_int8_t * data);
iuu_error iuu_pic_cmd_read(iuu * inf, u_int8_t data, u_int8_t * resp);
iuu_error iuu_pic_on(iuu * inf);
iuu_error iuu_pic_off(iuu * inf);
iuu_error iuu_pic_reset(iuu * inf);
iuu_error iuu_pic_inc(iuu * inf);
iuu_error iuu_pic_incn(iuu * inf, u_int8_t n);
iuu_error iuu_pic_pwrite(iuu * inf, u_int8_t * data);
iuu_error iuu_pic_pread(iuu * inf, u_int8_t * data);
iuu_error iuu_pic_preadn(iuu * inf, u_int8_t * data, unsigned char n);
iuu_error iuu_pic_dwrite(iuu * inf, u_int8_t * data);
iuu_error iuu_pic_dread(iuu * inf, u_int8_t * data);

// This ones come handy when testing
iuu_error iuu_get_atr(iuu * inf, u_int8_t * atr, u_int8_t * len);
void iuu_print_atr(u_int8_t * atr, u_int8_t atrl);

void iuu_process_error(iuu_error err, char *filename, int linenumber);

#endif
