/*
 * io.h
 *
 * io definitions
 *
 * Author: Pierre Morel <pmorel@mnis.fr>
 *
 * $LICENSE:
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef __IO_H
#define __IO_H

extern int _hyp_io_open(int driver, unsigned int flags);
extern int _hyp_io_write(int fd, void *buffer, int count);
extern int _hyp_io_read(int fd, void *buffer, int count);
extern int _hyp_io_ioctl(int fd, unsigned int cmd, void *buffer, unsigned int flags);
extern int _hyp_io_close(int fd, unsigned int flags);
extern void console_init(void);

extern int puts(char *s);


#define HYP_IO_CONSOLE_ID	1
#define HYP_IO_NET_ID		2

#define HYP_IO_WRONLY		0x0001
#define HYP_IO_RDONLY		0x0002
#define HYP_IO_RDWR		0x0003
#define HYP_IO_ASYNC		0x0004
#define	HYP_IO_CLOSE_DRAIN	0x1000

#endif
