/*
 * list.h
 *
 * list handling definitions
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

#ifndef __LIST_H
#define __LIST_H

#include <xhyp/stdlib.h>

struct list {
	struct list *next;
	struct list *prev;
};


static inline int list_empty(struct list *head)
{
	return head->next == head;
}
static inline void list_init(struct list *head)
{
	head->next = head->prev = head;
}
static inline void list_add(struct list *new, struct list *head)
{
	new->next = head->next;
	head->next->prev = new;
	head->next = new;
	new->prev = head;
}
static inline void list_add_tail(struct list *new, struct list *head)
{
	new->prev = head->prev;
	head->prev->next = new;
        head->prev = new;
	new->next = head;
}
static inline void list_del(struct list *head)
{
	head->prev->next = head->next;
	head->next->prev = head->prev;
}


#define list_for_each(l, h) \
	for (l = (h)->next; l != h; l = l->next)

#define list_for_each_safe(l, n, h) \
        for (l = (h)->next, n = l->next; l != (h); \
                l = n, n = l->next)

#define list_entry(ptr, type, member) \
        container_of(ptr, type, member)

#endif
