/*
 * stdlib.h
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

#ifndef STDLIB_H
#define STDLIB_H

#define islower(c)	((c) >= 'a' && (c) <= 'z')
#define isupper(c)	((c) >= 'A' && (c) <= 'Z')
#define isdigit(c)	((c) >= '0' && (c) <= '9')
#define isxdigit(c)	(((c) >= '0' && (c) <= '9') || \
			 ((c) >= 'a' && (c) <= 'f') || \
			 ((c) >= 'A' && (c) <= 'F'))
#define isspace(c)	((c) == ' ' || (c) == '\f' || (c) == '\n' || \
			 (c) == '\r' || (c) == '\t' || (c) == '\v')
#define toupper(c)	(islower(c) ? ((c) + 'A' - 'a') : (c))
#define tolower(c)	(isupper(c) ? ((c) + 'a' - 'A') : (c))

#ifndef INT_MAX
#define INT_MAX         ((int)(~0U>>1))
#endif

typedef __builtin_va_list va_list;

#define va_start(v, l) __builtin_va_start(v,l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v,l)


extern int arg_count(char **vec);
extern int strlen(const char *s);
extern int strnlen(const char *s, int count);
extern char *strchr(const char *str, int c);
extern void *memcpy(void *dest, const void *src, int count);
extern void memset(void *dest, char val, int count);
extern int strcmp(const char *cs, const char *ct);
extern int strncmp(const char *cs, const char *ct, int count);
extern char *strncpy(char *dest, const char *src, int count);
extern char *strcat(char *dest, const char *src);
extern char *strncat(char *dest, const char *src, int count);
extern int vsnprintf(char *buf, int size, const char *fmt, va_list args);
extern int snprintf(char *buf, int size, const char *fmt, ...);
extern int fprintf(int fd, const char *fmt, ...);
extern unsigned long strtoul(const char *nptr, char **endptr, unsigned int base);
extern long strtol(const char *nptr, char **endptr, unsigned int base);
extern unsigned long long strtoull(const char *nptr, char **endptr, unsigned int base);
extern long long strtoll(const char *nptr, char **endptr, unsigned int base);
extern int vsscanf(const char *str, const char *fmt, va_list);
extern int sscanf(const char *str, const char *fmt, ...);
extern int filesize(const char *fname);
extern int readfile(const char *fname, char *buf, int size);
extern char *readline(int fd, char *line, int size);
extern int isatty(int fd);
extern int cpu_number(void);
extern int sockserv(int port, char *path);
extern char *getenv(const char *name);


#define atoi(s)	strtol((s), (char **)0, 10)
#define atol(s)	strtol((s), (char **)0, 10)
#define atoll(s)	strtoll((s), (char **)0, 10)
#define atox(s)	strtoul((s), (char **)0, 16)
#define printf(fmt, arg...)	fprintf(1, fmt, ##arg)
#define printk(fmt, arg...)	fprintf(1, fmt, ##arg)


#define size_t	int

#endif /* STDLIB_H */

