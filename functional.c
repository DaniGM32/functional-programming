#include "functional.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#define int_max 2147483647

void for_each(void (*func)(void *), array_t list) {
	for (int i = 0; i < list.len; i++)
		(*func)((char *)list.data + i * list.elem_size);
}

array_t map(void (*func)(void *, void *),
			int new_list_elem_size,
			void (*new_list_destructor)(void *),
			array_t list) {
	array_t new_list;
	new_list.data = (void *)malloc(list.len * new_list_elem_size);
	new_list.elem_size = new_list_elem_size;
	new_list.destructor = new_list_destructor;
	new_list.len = 0;
	for (int i = 0; i < list.len; i++) {
		(*func)((char *)new_list.data + new_list.len * new_list_elem_size,
				(char *)list.data + i * list.elem_size);
		new_list.len++;
	}
	for (int i = list.len - 1; i >= 0; i--)
		list.destructor((char *)list.data + i * list.elem_size);
	free(list.data);
	return new_list;
}

array_t filter(boolean(*func)(void *), array_t list) {
	int nr = 0;
	for (int i = 0; i < list.len; i++)
		if ((*func)((char *)list.data + i * list.elem_size) == 1)
			nr++;
	array_t	new_list;
	new_list.data = (void *)malloc(nr * list.elem_size);
	if (!new_list.data)
		return list;
	new_list.elem_size = list.elem_size;
	new_list.destructor = list.destructor;
	new_list.len = 0;
	for (int i = 0; i < list.len; i++) {
		if ((*func)((char *)list.data + i * list.elem_size) == 1) {
			memcpy((char *)new_list.data + new_list.len * list.elem_size,
				   (char *)list.data + i * list.elem_size, list.elem_size);
			new_list.len++;
		} else {
			if (list.destructor)
				list.destructor((char *)list.data + i * list.elem_size);
		}
	}
	free(list.data);
	return new_list;
}

void *reduce(void (*func)(void *, void *), void *acc, array_t list) {
	for (int i = 0; i < list.len; i++)
		(*func)(acc, ((char *)list.data + i * list.elem_size));
	return acc;
}

void for_each_multiple(void(*func)(void **), int varg_c, ...) {
	array_t  *all_lists = (array_t *)malloc(varg_c * sizeof(array_t));
	if (!all_lists)
		return;

	va_list arguments;
	va_start(arguments, varg_c);
	for (int i = 0; i < varg_c; i++) {
		if (all_lists)
			all_lists[i] = va_arg(arguments, array_t);
	}
	va_end(arguments);

	int minimal_len = int_max;
	for (int i = 0; i < varg_c; i++)
		if (all_lists[i].len)
			if (all_lists[i].len < minimal_len)
				minimal_len = all_lists[i].len;

	for (int j = 0; j < minimal_len; j++) {
		void **current_list = (void **)malloc(varg_c * sizeof(void *));
		for (int i = 0; i < varg_c; i++) {
			if (all_lists[i].data)
				current_list[i] = (char *)all_lists[i].data +
									j * all_lists[i].elem_size;
		}
		(*func)(current_list);
		free(current_list);
	}
	free(all_lists);
}

array_t map_multiple(void (*func)(void *, void **),
					 int new_list_elem_size,
					 void (*new_list_destructor)(void *),
					 int varg_c, ...) {
	array_t  *all_lists = (array_t *)malloc(varg_c * sizeof(array_t));
	if (!all_lists)
		return (array_t){0};

	va_list arguments;
	va_start(arguments, varg_c);
	for (int i = 0; i < varg_c; i++) {
		if (all_lists)
			all_lists[i] = va_arg(arguments, array_t);
	}
	va_end(arguments);

	int minimal_len = int_max;
	for (int i = 0; i < varg_c; i++) {
		if (all_lists[i].len)
			if (all_lists[i].len < minimal_len)
				minimal_len = all_lists[i].len;
	}

	array_t new_list;
	new_list.data = (void *)malloc(minimal_len * new_list_elem_size);
	new_list.elem_size = new_list_elem_size;
	new_list.destructor = (*new_list_destructor);
	new_list.len = minimal_len;

	for (int j = 0; j < minimal_len; j++) {
		void *current_list[20];
		for (int i = 0; i < varg_c; i++) {
			if (all_lists[i].data) {
				int size = all_lists[i].elem_size;
				current_list[i] = (char *)all_lists[i].data + j * size;
			}
		}
		(*func)(((char *)new_list.data + j * new_list_elem_size), current_list);
	}
	for (int i = 0; i < varg_c; i++) {
		int length = all_lists[i].len;
		for (int j = 0; j < length; j++) {
			if (all_lists[i].destructor) {
				int size = all_lists[i].elem_size;
				all_lists[i].destructor(all_lists[i].data + j * size);
			}
		}
		free(all_lists[i].data);
	}
	free(all_lists);
	return new_list;
}

void *reduce_multiple(void(*func)(void *, void **), void *acc, int varg_c, ...)
{
	array_t  *all_lists = (array_t *)malloc(varg_c * sizeof(array_t));
	if (!all_lists)
		return NULL;

	va_list arguments;
	va_start(arguments, varg_c);
	for (int i = 0; i < varg_c; i++) {
		if (all_lists)
			all_lists[i] = va_arg(arguments, array_t);
	}
	va_end(arguments);

	int minimal_len = int_max;
	for (int i = 0; i < varg_c; i++) {
		if (all_lists[i].len)
			if (all_lists[i].len < minimal_len)
				minimal_len = all_lists[i].len;
	}

	for (int j = 0; j < minimal_len; j++) {
		void *current_list[20];
		for (int i = 0; i < varg_c; i++) {
			if (all_lists[i].data) {
				int size = all_lists[i].elem_size;
				current_list[i] = (char *)all_lists[i].data + j * size;
			}
		}
		(*func)(acc, current_list);
	}
	for (int i = 0; i < varg_c; i++) {
		int length = all_lists[i].len;
		for (int j = 0; j < length; j++) {
			if (all_lists[i].destructor) {
				int size = all_lists[i].elem_size;
				all_lists[i].destructor(all_lists[i].data + j * size);
			}
		}
	}

	free(all_lists);
	return acc;
}
