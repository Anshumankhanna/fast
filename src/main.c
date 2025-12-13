#include <assert.h>
#include <corecrt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <direct.h>
#include <windows.h>
#include <winnt.h>

#define INPUT_SIZE 128
#define BUFFER_SIZE 1024
// does `malloc()` on the `_ptr` and allocates `_size` number for elements to `_ptr`,
// Return value: `1` on success and `0` on failure
#define check_malloc(_ptr, _size) (_ptr = malloc(sizeof(*_ptr) * (_size))) == NULL
// does `realloc()` on the `_ptr` and allocates `_size` number for elements to `_ptr`,
// Return value: `1` on success and `0` on failure
#define check_realloc(_ptr, _size) (_ptr = realloc(_ptr, sizeof(*_ptr) * (_size))) == NULL
// does `strcpy_s()` on the `_dest` and copies `_size` number of characters from `_src` to `_dest`,
// `_size` should be `strlen(_src) + 1`, we haven't added this to macro since we can have literal `_src` here,
// Return value: `1` on success and `0` on failure
#define check_strcpy_s(_dest, _size, _src) strcpy_s(_dest, _size, _src) != 0
// does `strcat_s()` on the `_dest` and concatenates `_size` number of characters from `_src` to `_dest`,
// `_size` should be `strlen(_dest) + strlen(_src) + 1`, we haven't added this to macro since we can have literal `_src` here,
// Return value: `1` on success and `0` on failure
#define check_strcat_s(_dest, _size, _src) strcat_s(_dest, sizeof(*_dest) * (_size), _src) != 0

enum Result {
	Ok,
	Error
};

struct string {
	char *data;
	size_t size;
};
// create a `struct string` from a literal.
#define string_new_literal(LITERAL) (struct string) { .data = LITERAL, .size = sizeof(LITERAL) - 1 }
// create a `struct string` with no data.
#define string_blank (struct string) { .data = NULL, .size = 0 }
// create a `struct string` from given `char *`.
struct string string_new(const char *);
// frees the data inside the `string` pointer but the pointer itself isn't freed.
void string_delete(struct string *);
// merges the two strings given and returns `prefix + suffix`,
// returns `string_blank` on failure.
struct string string_merge(const struct string *prefix, const struct string *suffix);

struct string_array {
	struct string *arr;
	size_t size;
	size_t capacity;
};
// create a `struct string_array` with no data.
#define string_array_new (struct string_array) { .arr = NULL, .size = 0, .capacity = 0 }
// frees the data inside the `string_array` pointer but the pointer itself isn't freed.
void string_array_delete(struct string_array *);
// returns `Error` if we are unable to fill all `size_t` elements into the string_array else returns `Ok`.
enum Result string_array_fill_char(struct string_array *, const char *const *, const size_t);
// returns `Error` if we are unable to push the current element in string_array else returns `Ok`.
enum Result string_array_push_char(struct string_array *, const char *);
// this is just a way to push multiple elements.
void string_array_print(const struct string_array *);

// linked list of command_data_nodes.
struct command_data_ll {
	// storing the commands, alias is the word that the user types and the command is the actual command that runs in the CLI.
	struct command_data_node {
		struct string *command;
		struct string *alias;
		struct command_data_node *prev;
		struct command_data_node *next;
	} *head;
	struct command_data_node *tail;
	size_t size;
};

// creating a macro here not because we'll use this multiple times but just to keep a standard at the top of the file.
// we'll always statically allocate this struct as we have commands to `push` and `fill` that can be used later on to mutate it.
#define command_data_ll_new (struct command_data_ll) { .head = NULL, .tail = NULL, .size = 0 }
// frees the data inside the `command_data_ll` pointer but the pointer itself isn't freed.
void command_data_ll_delete(struct command_data_ll *);
// returns `Error` if we are unable to push the current element in the linked list else returns `Ok`.
enum Result command_data_ll_push(struct command_data_ll *, const struct string *, const struct string *);
// returns `Error` if we are unable to push all elements from string_array in the linked list else returns `Ok`.
enum Result command_data_ll_fill(struct command_data_ll *, const struct string_array *);
// we are returning a `const` value so that it cannot be changed which prevents errors.
struct string *command_data_ll_find(struct command_data_ll *, const struct string *);
void command_data_ll_print(const struct command_data_ll *);

// returns `string_blank` if the function fails otherwise a valid `struct string`.
struct string get_current_directory() {
	char directory_buffer[BUFFER_SIZE];
	const DWORD directory_buffer_size = GetModuleFileName(NULL, directory_buffer, BUFFER_SIZE);

	if (directory_buffer_size == 0) {
		return string_blank;
	}
	
	const char *last_slash_pos = strrchr(directory_buffer, '\\');
	
	if (last_slash_pos == NULL) {
		return string_blank;
	}

	const size_t result_size = last_slash_pos - directory_buffer + 1;
	struct string result = { .data = NULL, .size = result_size };

	if (check_malloc(result.data, result.size + 1)) {
		return string_blank;
	}
	if (strncpy_s(result.data, result.size + 1, directory_buffer, result.size) != 0) {
		string_delete(&result);
		return string_blank;
	}

	return result;
}
void run_help(FILE *data_file_ptr) {
	printf("These are all the commands that are possible.\n\n");
	size_t line_index = 0;
	
	do {
		char line[BUFFER_SIZE];

		if (fgets(line, BUFFER_SIZE, data_file_ptr) == NULL) {
			break;
		}

		line[BUFFER_SIZE - 1] = '\0';
		printf("%s", line);

		if (line_index++ % 2 != 0) {
			printf("\n");
		}
	} while (1);

	printf("\nEnd of all commands.\n");
}

int main(int argc, char **argv) {
	int return_val = 0;

	if (argc < 2) {
		perror("You must run this application like this: fast <command>");
		return 1;
	}

	struct string current_directory = get_current_directory();
	if (current_directory.data == NULL) {
		return 1;
	}

	struct string data_file = string_merge(&current_directory, &string_new_literal("commands.txt"));
	if (data_file.data == NULL) {
		return_val = 1;
		goto cleanup_current_directory;
	}

	char alias[INPUT_SIZE];
	if (check_strcpy_s(alias, INPUT_SIZE, argv[1])) {
		goto cleanup_current_directory;
	}

	// we are only allowing single character modes and not `+` modes currently for no good reason.
	char file_mode[2] = "r";
	bool file_mode_read = true;

	if (strncmp(alias, "save", 5) == 0 && !(file_mode_read = false) && check_strcpy_s(file_mode, 2, "a")) {
		goto cleanup_current_directory;
	}
	
	FILE *data_file_ptr = NULL;
	if (fopen_s(&data_file_ptr, data_file.data, file_mode) != 0) {
		return_val = 1;
		goto cleanup_data_file;
	}

	if (!file_mode_read) {
		char working_directory[INPUT_SIZE];
		if (_getcwd(working_directory, INPUT_SIZE) == NULL) {
			goto cleanup_data_file;
		}

		// TODO: implement code to read the file and find out the alias session and replace the command below it to open the last session which would just be opening the last directory we saved in powershell and in neovim.

		// if (fprintf_s(data_file_ptr, "%.*s\n", INPUT_SIZE, working_directory) < 0) {
		// 	goto cleanup_data_file;
		// }

		goto cleanup_data_file;
	}

	struct string_array aliases_and_commands = string_array_new;
	struct command_data_ll list = command_data_ll_new;

	do {
		char line[BUFFER_SIZE];

		if (fgets(line, BUFFER_SIZE, data_file_ptr) == NULL) {
			break;
		}

		const size_t line_size = strlen(line);
		// the last character of `line` would always be `\n` but we don't want to copy that and hence we change it to create a null terminator.
		line[line_size - 1] = '\0';

		if (string_array_push_char(&aliases_and_commands, line) == Error) {
			return_val = 1;
			goto cleanup;
		}
	} while (1);
	
	// need to close this fast so that we can later edit `data.txt`.
	fclose(data_file_ptr);

	if (command_data_ll_fill(&list, &aliases_and_commands) == Error) {
		return_val = 1;
		goto cleanup;
	}

	// don't clear command because it's a pointer to the data from the list.
	const struct string *command = command_data_ll_find(&list, &(struct string) { .data = alias, .size = strlen(alias) });
	if (command == NULL) {
		printf("Couldn't find the command for %.*s\n", INPUT_SIZE, alias);
		return_val = 1;
		goto cleanup;
	}

	printf("Executing command: %.*s\n", (int) command -> size + 1, command -> data);

	if (system(command -> data) == -1) {
		perror("Error occured\n");
		// writing the below line just for the feel.
		goto cleanup;
	}
	
	// TODO: We may want to check if these resources are still occupied until the powershell that we open (in some commands) isn't still closed,
	// if that's the case then we may want to free these resources beforehand.

cleanup:
	command_data_ll_delete(&list);
	string_array_delete(&aliases_and_commands);

cleanup_data_file:
	string_delete(&data_file);
cleanup_current_directory:
	string_delete(&current_directory);

	return return_val;
}

struct string string_new(const char *data) {
	if (data == NULL) {
		return string_blank;
	}

	struct string result = { .data = NULL, .size = strlen(data) };

	if (check_malloc(result.data, result.size + 1)) {
		return string_blank;
	}
	if (check_strcpy_s(result.data, result.size + 1, data)) {
		string_delete(&result);
		return string_blank;
	}

	return result;
}
void string_delete(struct string *ptr) {
	if (ptr == NULL) {
		return ;
	}

	free(ptr -> data);
}
struct string string_merge(const struct string *const prefix, const struct string *const suffix) {
	if (prefix == NULL || suffix == NULL) {
		return string_blank;
	}

	struct string result = { .data = NULL, .size = prefix -> size + suffix -> size};

	if (check_malloc(result.data, prefix -> size + suffix -> size + 1)) {
		return string_blank;
	}
	if (check_strcpy_s(result.data, prefix -> size + 1, prefix -> data)) {
		string_delete(&result);
		return string_blank;
	}
	if (check_strcat_s(result.data, result.size + 1, suffix -> data)) {
		string_delete(&result);
		return string_blank;
	}

	return result;
}

void string_array_delete(struct string_array *arr) {
	if (arr == NULL) {
		return ;
	}

	for (size_t index = 0; index < arr -> size; ++index) {
		string_delete(&arr -> arr[index]);
	}

	free(arr -> arr);
}
enum Result string_array_fill_char(struct string_array *string_arr, const char *const *string_data_arr, const size_t string_data_arr_size) {
	if (string_arr == NULL || string_data_arr == NULL || string_data_arr_size == 0) {
		return Error;
	}

	const size_t required_size = string_arr -> size + string_data_arr_size;

	if (string_arr -> capacity < required_size) {
		if (check_realloc(string_arr -> arr, required_size)) {
			return Error;
		}

		string_arr -> capacity = required_size;
	}

	for (size_t index = string_arr -> size; index < required_size; ++index) {
		if (string_data_arr[index] == NULL) {
			return Error;
		}

		const size_t data_size = strlen(string_data_arr[index]);

		if (check_malloc(string_arr -> arr[index].data, data_size + 1)) {
			return Error;
		}
		if (check_strcpy_s(string_arr -> arr[index].data, data_size + 1, string_data_arr[index])) {
			string_delete(&string_arr -> arr[index]);
			return Error;
		}
		
		string_arr -> arr[index].size = data_size;
		++string_arr -> size;
	}

	return Ok;
}
enum Result string_array_push_char(struct string_array *string_arr, const char *str) {
	if (string_arr == NULL || str == NULL) {
		return Error;
	}
	
	if (string_arr -> capacity < string_arr -> size + 1) {
		if (string_arr -> capacity == 0) {
			string_arr -> capacity = 1;
		} else {
			string_arr -> capacity *= 2;
		}
		
		if (check_realloc(string_arr -> arr, string_arr -> capacity)) {
			string_arr -> capacity /= 2;
			return Error;
		}
	}

	const size_t str_size = strlen(str);

	if (check_malloc(string_arr -> arr[string_arr -> size].data, str_size + 1)) {
		return Error;
	}

	string_arr -> arr[string_arr -> size].size = str_size;

	if (check_strcpy_s(string_arr -> arr[string_arr -> size].data, str_size + 1, str)) {
		goto cleanup_malloc;
		return Error;
	}

	++string_arr -> size;

	return Ok;

cleanup_malloc:
	string_delete(&string_arr -> arr[string_arr -> size]);

	return Error;
}
void string_array_print(const struct string_array *arr) {
	if (arr == NULL) {
		return ;
	}

	printf("{\n");

	for (size_t index = 0; index < arr -> size - 1; ++index) {
		printf("\t{ data = %s, size = %zu },\n", arr -> arr[index].data, arr -> arr[index].size);
	}

	printf("\t{ data = %s, size = %zu }\n", arr -> arr[arr -> size - 1].data, arr -> arr[arr -> size - 1].size);
	printf("}\n");
}

void command_data_ll_delete(struct command_data_ll *list) {
	if (list == NULL) {
		return ;
	}

	struct command_data_node *curr_node = list -> head;

	while (curr_node != NULL) {
		// we can't make this const even though we are not changing its value as we will assign it to a non-const variable later and that displays a warning.
		struct command_data_node *next_node = curr_node -> next;

		string_delete(curr_node -> alias);
		string_delete(curr_node -> command);

		free(curr_node -> alias);
		free(curr_node -> command);
		free(curr_node);

		curr_node = next_node;
	}
}
enum Result command_data_ll_push(struct command_data_ll *list, const struct string *alias, const struct string *command) {
	if (list == NULL) {
		return Error;
	}

	// here we have no need to check if `list -> tail` is currently `NULL`, because even if it is, it doesn't affect the program.
	struct command_data_node *temp_tail = list -> tail;

	// if any of the below don't run as expected then we basically couldn't insert the node properly.
	
	if (check_malloc(list -> tail, 1)) {
		goto exit;
	}
	if (check_malloc(list -> tail -> alias, 1)) {
		goto cleanup_tail;
	}
	if (check_malloc(list -> tail -> command, 1)) {
		goto cleanup_alias;
	}
	if (check_malloc(list -> tail -> alias -> data, alias -> size + 1)) {
		goto cleanup_command;
	}
	if (check_malloc(list -> tail -> command -> data, command -> size + 1)) {
		goto cleanup_alias_data;
	}
	if (check_strcpy_s(list -> tail -> alias -> data, alias -> size + 1, alias -> data)) {
		goto cleanup;
	}
	if (check_strcpy_s(list -> tail -> command -> data, command -> size + 1, command -> data)) {
		goto cleanup;
	}
	
	list -> tail -> alias -> size = alias -> size;
	list -> tail -> command -> size = command -> size;

	list -> tail -> prev = temp_tail;
	list -> tail -> next = NULL;

	if (list -> head == NULL) {
		list -> head = list -> tail;
	} else {
		// if `list -> head` isn't `NULL` then surely `temp_tail` has the value of some node and isn't `NULL` either, hence we can connect the new tail to the old tail, otherwise we won't be able to do this.
		temp_tail -> next = list -> tail;
	}

	++list -> size;

	return Ok;
	
cleanup:
	free(list -> tail -> command -> data);
cleanup_alias_data:
	free(list -> tail -> alias -> data);
cleanup_command:
	free(list -> tail -> command);
cleanup_alias:
	free(list -> tail -> alias);
cleanup_tail:
	free(list -> tail);
exit:
	return Error;
}
enum Result command_data_ll_fill(struct command_data_ll *list, const struct string_array *arr) {
	if (arr -> size % 2 != 0) {
		printf("%zu\n", arr -> size);
		return Error;
	}

	for (size_t consumed = 0; consumed < arr -> size; consumed += 2) {
		if (command_data_ll_push(list, &arr -> arr[consumed], &arr -> arr[consumed + 1]) == Error) {
			return Error;
		}
	}
	
	return Ok;
}
struct string *command_data_ll_find(struct command_data_ll *list, const struct string *alias) {
	if (list == NULL || alias == NULL) {
		return NULL;
	}

	struct command_data_node *node = list -> head;

	while (node != NULL) {
		if (node -> alias -> size == alias -> size && strncmp(node -> alias -> data, alias -> data, alias -> size) == 0) {
			return node -> command;
		}

		node = node -> next;
	}

	return NULL;
}
void command_data_ll_print(const struct command_data_ll *list) {
	if (list == NULL || list -> head == NULL) {
		return ;
	}

	struct command_data_node *node = list -> head;

	printf("{\n");

	while (node -> next != NULL) {
		printf("\t{ alias = { data = %s, size = %zu }, command = { data = %s, size = %zu } },\n", node -> alias -> data, node -> alias -> size, node -> command -> data, node -> command -> size);
		node = node -> next;
	}

	printf("\t{ alias = { data = %s, size = %zu }, command = { data = %s, size = %zu } }\n", node -> alias -> data, node -> alias -> size, node -> command -> data, node -> command -> size);
	printf("}\n");
}

