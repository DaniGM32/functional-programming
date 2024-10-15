#include "functional.h"
#include "tasks.h"
#include "tests.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// destructorii i am folosit doar pt functiile din tasks.c

void destructor_for_array(void *elem) {
	array_t *list = (array_t *)elem;
	free(list->data);
}

void destructor_for_boolean(void *elem) {
	bool *bool_var = (bool *)elem;
	free(bool_var);
}

void destructor_for_student(void *elem) {
	student_t *student = (student_t *)elem;
	free(student->name);
}

void destructor_for_number(void *elem) {
	number_t *number = (number_t *)elem;
	free(number->string);
}

void aux_reverse(void *acc, void *param_elem) {
	array_t *list = (array_t *)acc;
	list->data = realloc(list->data, (list->len + 1) * list->elem_size);
	// memmove de mai jos muta toate elementele cu o pozitie in dreapta
	// practic adaug cate un element la inceputul listei
	// mut toate elementele cu o pozitie in dreapta
	// si adaug un nou element
	memmove((char *)list->data + list->elem_size,
			list->data,
			list->len * list->elem_size);
	memcpy(list->data, param_elem, list->elem_size);
	list->len++;
}

array_t reverse(array_t list) {
	array_t rev_list;
	rev_list.destructor = list.destructor;
	rev_list.elem_size	= list.elem_size;
	rev_list.len = 0;
	rev_list.data = malloc(list.len * list.elem_size);
	reduce(aux_reverse, &rev_list, list);
	return rev_list;
}

void aux_create_number(void *dest, void **parameter_lists) {
	int integer_part = *(int *)parameter_lists[0];
	int fractional_part = *(int *)parameter_lists[1];

	number_t *number = (number_t *)dest;
	number->integer_part = integer_part;
	number->fractional_part = fractional_part;

	int integer = number->integer_part;
	int fractional = number->fractional_part;

	int string_length = snprintf(NULL, 0, "%d.%d", integer, fractional);
	number->string = (char *)malloc(string_length + 1);
	sprintf(number->string, "%d.%d", integer, fractional);
}

array_t create_number_array(array_t integer_part, array_t fractional_part) {
	if (integer_part.len != fractional_part.len)
		printf("dimensiunile listelor nu-s egale\n");

	array_t number_array = map_multiple(aux_create_number,
										sizeof(number_t),
										destructor_for_number,
										2,
										integer_part,
										fractional_part);
	return number_array;
}

boolean average_above_five(void *student) {
	if (((student_t *)student)->grade > minimal_grade)
		return 1;
	return 0;
}

void get_student_names(void *dest, void *parameter_list) {
	char *name = ((student_t *)parameter_list)->name;
	*(char **)dest = strdup(name);
}

array_t get_passing_students_names(array_t list) {
	array_t first_filtered_list, second_filtered_list;
	first_filtered_list = filter(average_above_five, list);
	second_filtered_list = map(get_student_names,
							   sizeof(char *),
							   destructor_for_student,
							   first_filtered_list);
	return second_filtered_list;
}

void aux_reduce(void *acc, void *list_elem) {
	*(int *)acc += *(int *)list_elem;
}

void aux_check_bigger_sum(void *new_elem, void **parameter_list) {
	int sum = 0;
	reduce(aux_reduce, &sum, *(array_t *)parameter_list[0]);
	int integer_ref = *(int *)parameter_list[1];
	if (sum >= integer_ref)
		*(bool *)new_elem = 1;
	else
		*(bool *)new_elem = 0;
}

array_t check_bigger_sum(array_t list_list, array_t int_list) {
	array_t new_list;
	new_list = map_multiple(aux_check_bigger_sum,
							sizeof(false),
							NULL,
							2,
							list_list, int_list);
	return new_list;
}

boolean different_than_NULL(void *elem) {
	if (*(char **)elem)
		return 1;
	return 0;
}

void aux_even_strings(void *acc, void *param_elem) {
	array_t *result = (array_t *)acc;
	int current_position = result->len;
	// daca pozitia e para, adaug elementul
	if (current_position % 2 == 0) {
		int length = result->len;
		int size = result->elem_size;
		*(char **)(result->data + length * size) = strdup(*(char **)param_elem);
	}

	result->len++;
}

array_t get_even_indexed_strings(array_t list) {
	array_t even_indexed_list;
	even_indexed_list.destructor = list.destructor;
	even_indexed_list.elem_size = list.elem_size;
	even_indexed_list.len = 0;
	even_indexed_list.data = calloc(list.len, list.elem_size);

	reduce(aux_even_strings, &even_indexed_list, list);
	even_indexed_list = filter(different_than_NULL, even_indexed_list);

	for_each(list.destructor, list);
	free(list.data);

	return even_indexed_list;
}

// aux_fill_row are scopul de a completa fiecare element al unei linii
void aux_fill_row(void *acc, void *param_elem) {
	int *n = (int *)acc;
	int *row_elem = (int *)param_elem;
	*row_elem = *n;
	(*n)++;
}

// aux_fill_matrix are scopul de a umple o linie a matricei
void aux_fill_matrix(void *acc, void *param_elem) {
	int n = *(int *)acc;
	// pun in crt primul element din linie
	// si il incrementez la fiecare pas cand completez un element
	int crt = *(int *)param_elem;
	array_t *row = (array_t *)param_elem;
	row->elem_size = sizeof(int);
	row->len = n;
	row->data = malloc(row->len * row->elem_size);
	row->destructor = destructor_for_array;
	// int *p = row->data;
	reduce(aux_fill_row, &crt, *row);
}

// fill_first_row are scopul de a umple prima linie a matricei
void fill_first_row(void *acc, void *param_elem) {
	int *n = (int *)acc;
	int *row_elem = (int *)param_elem;
	*row_elem = *n;
	(*n)++;
}

array_t generate_square_matrix(int n) {
	array_t matrix;
	matrix.len = n;
	matrix.elem_size = sizeof(array_t);
	matrix.data = malloc(n * matrix.elem_size);
	matrix.destructor = destructor_for_array;
	int one = 1;
	// primul reduce completeaza prima linie
	// fac asta ca sa pot sa salvez nr de inceput al fiecarei linii
	reduce(fill_first_row, &one, matrix);

	// al doilea reduce completeaza restul liniilor
	reduce(aux_fill_matrix, &n, matrix);
	return matrix;
}
