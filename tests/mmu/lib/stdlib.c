/*
 * stdlib.c
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

#include <stdlib.h>
#include <sys/shared_page.h>
#include <sys/xhyp.h>
#include <sys/io.h>

int strlen(const char *s);
int strnlen(const char *s, int count);
char *strchr(const char *s, int c);
char *strncpy(char *dest, const char *src, int count);
int strncmp(const char *cs, const char *ct, int count);
int strcmp(const char *cs, const char *ct);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, int count);
extern unsigned long strtoul(const char *cp, char **endp, unsigned int base);
int fprintf(int fd, const char *fmt, ...);
extern long strtol(const char *cp, char **endp, unsigned int base);
unsigned long long strtoull(const char *cp, char **endp, unsigned int base);
long long strtoll(const char *cp, char **endp, unsigned int base);
int vsscanf(const char *buf, const char *fmt, va_list args);
int sscanf(const char *buf, const char *fmt, ...);
int sprintf(char *buf, const char *fmt, ...);

#define ZEROPAD	1
#define SIGN	2
#define PLUS	4
#define SPACE	8
#define LEFT	16
#define SPECIAL	32
#define LARGE	64

#define do_div(n, base) ({ \
int res; \
res = ((unsigned long)(n)) % (unsigned)(base); \
n = ((unsigned long)(n)) / (unsigned)(base); \
res; })

#define setnext(buf, end, c) do {if ((buf) <= (end)) *(buf) = (c); ++(buf);} while (0)

#define ASCII_LOW "0123456789abcdefghijklmnopqrstuvwxyz"
#define ASCII_HIGH "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"

int strlen(const char *s)
{
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc)
		;
	return sc - s;
}

int strnlen(const char *s, int count)
{
	const char *sc;

	for (sc = s; count-- && *sc != '\0'; ++sc)
		;
	return sc - s;
}

char *strchr(const char *s, int c)
{
	for (; *s != (char)c; ++s)
		if (*s == '\0')
			return 0;
	return (char *)s;
}

void memset(void *dest, char val, int count)
{
	while (count--)
		*(char *)dest++ = val;
}

void *memcpy(void *dest, const void *src, int count)
{
	char *tmp = dest;
	char *s = (char *)src;

	while (count--)
		*tmp++ = *s++;
	return dest;
}

int strcmp(const char *cs, const char *ct)
{
	char res;

	while (1)
		if ((res = *cs - *ct++) != 0 || !*cs++)
			break;
	return res;
}

int strncmp(const char *cs, const char *ct, int count)
{
	char res = 0;

	while (count) {
		if ((res = *cs - *ct++) != 0 || !*cs++)
			break;
		count--;
	}
	return res;
}

char *strncpy(char *dest, const char *src, int count)
{
	char *tmp = dest;

	while (count) {
		if ((*tmp = *src) != 0)
			src++;
		tmp++;
		count--;
	}
	return dest;
}

char *strcat(char *dest, const char *src)
{
	char *tmp = dest;

	while (*dest)
		dest++;
	while ((*dest++ = *src++) != '\0')
		;
	return tmp;
}

char *strncat(char *dest, const char *src, int count)
{
	char *tmp = dest;

	if (count) {
		while (*dest)
			dest++;
		while ((*dest++ = *src++) != 0) {
			if (--count == 0) {
				*dest = '\0';
				break;
			}
		}
	}
	return tmp;
}

int arg_count(char **vec)
{
	int i;
	for (i = 0; *vec; i++) vec++;
	return i;
}

static int skip_atoi(const char **s)
{
	int i = 0;

	while (isdigit(**s))
		i = i*10 + *((*s)++) - '0';
	return i;
}

#define MAX_DIGIT	80

static char *number(char *str, char *end, long num, int base, int size,
		    int precision, int type)
{
	char c, sign, tmp[MAX_DIGIT];
	int i;
	static const char sdigits[] = ASCII_LOW;
	static const char ldigits[] = ASCII_HIGH;
	const char *digits = (type & LARGE) ? ldigits : sdigits;

	if (type & LEFT)
		type &= ~ZEROPAD;
	if (base < 2 || base > 36)
		return 0;
	c = (type & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (type & SIGN) {
		if ((signed long long)num < 0) {
			sign = '-';
			num = - (signed long long) num;
			size--;
		} else if (type & PLUS) {
			sign = '+';
			size--;
		} else if (type & SPACE) {
			sign = ' ';
			size--;
		}
	}
	if (type & SPECIAL) {
		if (base == 16) size -= 2;
		else if (base == 8) size--;
	}
	i = 0;
	if (num == 0) tmp[i++] = '0';
	else while (num != 0 && i < MAX_DIGIT)
		tmp[i++] = digits[do_div(num, base)];
	if (i > precision) precision = i;
	size -= precision;
	if (!(type & (ZEROPAD + LEFT)))
		while (size-- > 0)
			setnext(str, end, ' ');
	if (sign) setnext(str, end, sign);
	if (type & SPECIAL) {
		if (base == 8)
			setnext(str, end, '0');
		else if (base == 16) {
			setnext(str, end, '0');
			setnext(str, end, digits[MAX_DIGIT/2]);
		}
	}
	if (!(type & LEFT))
		while (size-- > 0)
			setnext(str, end, c);
	while (i < precision--)
		setnext(str, end, '0');
	while (i-- > 0)
		setnext(str, end, tmp[i]);
	while (size-- > 0)
		setnext(str, end, ' ');
	return str;
}

int vsnprintf(char *buf, int size, const char *fmt, va_list args)
{
	int len;
	unsigned long long num;
	int i, base;
	char *str, *end = buf + size - 1;
	char *s;
	int flags;
	int field_width;
	int precision;
	int qualifier;

	if (end < buf - 1) {
		end = (char *) -1;
		size = end - buf + 1;
	}
	for (str = buf; *fmt ; ++fmt) {
		if (*fmt != '%') {
			setnext(str, end, *fmt);
			continue;
		}

		flags = 0;
		repeat:
			++fmt;
			switch (*fmt) {
			case '-': flags |= LEFT; goto repeat;
			case '+': flags |= PLUS; goto repeat;
			case ' ': flags |= SPACE; goto repeat;
			case '#': flags |= SPECIAL; goto repeat;
			case '0': flags |= ZEROPAD; goto repeat;
			}

		field_width = -1;
		if (isdigit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {
			++fmt;
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		precision = -1;
		if (*fmt == '.') {
			++fmt;
			if (isdigit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*') {
				++fmt;
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}

		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
			qualifier = *fmt;
			++fmt;
		}

		base = 10;

		switch (*fmt) {
		case 'c':
			if (!(flags & LEFT))
				while (--field_width > 0)
					setnext(str, end, ' ');
			setnext(str, end, (unsigned char)va_arg(args, int));
			while (--field_width > 0)
				setnext(str, end, ' ');
			continue;
		case 's':
			s = va_arg(args, char *);
			if (!s)
				s = "<NULL>";
			len = strnlen(s, precision);
			if (!(flags & LEFT))
				while (len < field_width--)
					setnext(str, end, ' ');
			for (i = 0; i < len; ++i)
				setnext(str, end, *s++);
			while (len < field_width--)
				setnext(str, end, ' ');
			continue;
		case 'p':
			if (field_width == -1) {
				field_width = 2*sizeof(void *);
				flags |= ZEROPAD;
			}
			str = number(str, end,
				(unsigned long) va_arg(args, void *), 16,
				field_width, precision, flags);
			continue;
		case 'n':
			if (qualifier == 'l') {
				long *ip = va_arg(args, long *);
				*ip = (str - buf);
			} else {
				int * ip = va_arg(args, int *);
				*ip = (str - buf);
			}
			continue;
		case '%':
			setnext(str, end, '%');
			continue;
		case 'o':
			base = 8;
			break;
		case 'X':
			flags |= LARGE;
		case 'x':
			base = 16;
			break;
		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			break;
		default:
			setnext(str, end, '%');
			if (*fmt) setnext(str, end, *fmt);
			else --fmt;
			continue;
		}
		if (qualifier == 'L')
			num = va_arg(args, long long);
		if (qualifier == 'l') {
			num = va_arg(args, unsigned long);
			if (flags & SIGN)
				num = (signed long)num;
		} else if (qualifier == 'h') {
			num = (unsigned short) va_arg(args, int);
			if (flags & SIGN)
				num = (short) num;
		}
		else {
			num = va_arg(args, unsigned int);
			if (flags & SIGN)
				num = (signed int)num;
		}
		str = number(str, end, num, base, field_width,
			     precision, flags);
	}
	if (str <= end) *str = '\0';
	else if (size > 0) *end = '\0';
	return str - buf;
}

int sprintf(char *buf, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsnprintf(buf, 256, fmt, args);
	va_end(args);
	return i;
}

int snprintf(char *buf, int size, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsnprintf(buf, size, fmt, args);
	va_end(args);
	return i;
}

int fprintf(int fd, const char *fmt, ...)
{
	va_list args;
	int i;
	int size = 180;
	char buf[size];

	va_start(args, fmt);
	i = vsnprintf(buf, size, fmt, args);
	va_end(args);
	if (i <= 0) return i;
	_hyp_io_write(1, buf, strnlen(buf, size));
	return i;
}


unsigned long strtoul(const char *cp, char **endp, unsigned int base)
{
	unsigned long result = 0, value;

	if (!base) {
		base = 10;
		if (*cp == '0') {
			base = 8;
			cp++;
			if ((toupper(*cp) == 'X') && isxdigit(cp[1])) {
				cp++;
				base = 16;
			}
		}
	} else if (base == 16) {
		if (cp[0] == '0' && toupper(cp[1]) == 'X')
			cp += 2;
	}
	while (isxdigit(*cp) &&
	       (value = isdigit(*cp) ? *cp-'0' : toupper(*cp)-'A'+10) < base) {
		result = result *base + value;
		cp++;
	}
	if (endp)
		*endp = (char *)cp;
	return result;
}

long strtol(const char *cp, char **endp, unsigned int base)
{
	if (*cp == '-')
		return -strtoul(cp+1, endp, base);
	return strtoul(cp, endp, base);
}


unsigned long long strtoull(const char *cp, char **endp, unsigned int base)
{
	unsigned long long result = 0, value;

	if (!base) {
		base = 10;
		if (*cp == '0') {
			base = 8;
			cp++;
			if ((toupper(*cp) == 'X') && isxdigit(cp[1])) {
				cp++;
				base = 16;
			}
		}
	} else if (base == 16) {
		if (cp[0] == '0' && toupper(cp[1]) == 'X')
			cp += 2;
	}
	while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
	    ? toupper(*cp) : *cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}
	if (endp)
		*endp = (char *)cp;
	return result;
}

long long strtoll(const char *cp, char **endp, unsigned int base)
{
	if (*cp == '-')
		return -strtoull(cp + 1, endp, base);
	return strtoull(cp, endp, base);
}

int vsscanf(const char *buf, const char *fmt, va_list args)
{
	const char *str = buf;
	char *next;
	char digit;
	int num = 0;
	int qualifier;
	int base;
	int field_width;
	int is_sign = 0;

	while (*fmt && *str) {
		if (isspace(*fmt)) {
			while (isspace(*fmt))
				++fmt;
			while (isspace(*str))
				++str;
		}

		if (*fmt != '%' && *fmt) {
			if (*fmt++ != *str++)
				break;
			continue;
		}

		if (!*fmt)
			break;
		++fmt;

		if (*fmt == '*') {
			while (!isspace(*fmt) && *fmt)
				fmt++;
			while (!isspace(*str) && *str)
				str++;
			continue;
		}

		field_width = -1;
		if (isdigit(*fmt))
			field_width = skip_atoi(&fmt);

		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' ||
		    *fmt == 'Z' || *fmt == 'z') {
			qualifier = *fmt++;
			if (qualifier == *fmt) {
				if (qualifier == 'h') {
					qualifier = 'H';
					fmt++;
				} else if (qualifier == 'l') {
					qualifier = 'L';
					fmt++;
				}
			}
		}
		base = 10;
		is_sign = 0;

		if (!*fmt || !*str)
			break;

		switch (*fmt++) {
		case 'c':
		{
			char *s = (char *)va_arg(args, char *);
			if (field_width == -1)
				field_width = 1;
			do {
				*s++ = *str++;
			} while (--field_width > 0 && *str);
			num++;
		}
		continue;
		case 's':
		{
			char *s = (char *)va_arg(args, char *);
			if (field_width == -1)
				field_width = INT_MAX;
			while (isspace(*str))
				str++;

			while (*str && !isspace(*str) && field_width--) {
				*s++ = *str++;
			}
			*s = '\0';
			num++;
		}
		continue;
		case 'n':
		{
			int *i = (int *)va_arg(args, int*);
			*i = str - buf;
		}
		continue;
		case 'o':
			base = 8;
			break;
		case 'x':
		case 'X':
			base = 16;
			break;
		case 'i':
			base = 0;
		case 'd':
			is_sign = 1;
		case 'u':
			break;
		case '%':
			if (*str++ != '%')
				return num;
			continue;
		default:
			return num;
		}

		while (isspace(*str))
			str++;

		digit = *str;
		if (is_sign && digit == '-')
			digit = *(str + 1);

		if (!digit
		    || (base == 16 && !isxdigit(digit))
		    || (base == 10 && !isdigit(digit))
		    || (base == 8 && (!isdigit(digit) || digit > '7'))
		    || (base == 0 && !isdigit(digit)))
				break;

		switch (qualifier) {
		case 'H':
			if (is_sign) {
				char *s = (char *)va_arg(args, char *);
				*s = (char)strtol(str, &next, base);
			} else {
				unsigned char *s = (unsigned char *) va_arg(args, unsigned char *);
				*s = (unsigned char)strtoul(str, &next, base);
			}
			break;
		case 'h':
			if (is_sign) {
				short *s = (short *) va_arg(args, short *);
				*s = (short)strtol(str, &next, base);
			} else {
				unsigned short *s = (unsigned short *)va_arg(args, unsigned short *);
				*s = (unsigned short)strtoul(str, &next, base);
			}
			break;
		case 'l':
			if (is_sign) {
				long *l = (long *)va_arg(args, long *);
				*l = strtol(str, &next, base);
			} else {
				unsigned long *l = (unsigned long*)va_arg(args, unsigned long*);
				*l = strtoul(str, &next, base);
			}
			break;
		case 'L':
			if (is_sign) {
				long long *l = (long long *)va_arg(args, long long *);
				*l = strtoll(str, &next, base);
			} else {
				unsigned long long *l = (unsigned long long *)va_arg(args, unsigned long long *);
				*l = strtoull(str, &next, base);
			}
			break;
		case 'Z':
		case 'z':
		{
			size_t *s = (size_t*)va_arg(args, size_t *);
			*s = (size_t)strtoul(str, &next, base);
		}
		break;
		default:
			if (is_sign) {
				int *i = (int *)va_arg(args, int*);
				*i = (int)strtol(str, &next, base);
			} else {
				unsigned int *i = (unsigned int*)va_arg(args, unsigned int *);
				*i = (unsigned int)strtoul(str, &next, base);
			}
			break;
		}
		num++;

		if (!next)
			break;
		str = next;
	}
	return num;
}

int sscanf(const char *buf, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsscanf(buf, fmt, args);
	va_end(args);
	return i;
}

