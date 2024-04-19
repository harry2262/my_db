#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

typedef struct
{
    u_int32_t id;
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];
} Row;
void print_row(Row *row)
{
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}
typedef struct
{
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;
typedef enum
{
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum
{
    PREPARE_SUCCESS,
    PREPARE_NEGATIVE_ID,
    PREPARE_STRING_TOO_LONG,
    PREPARE_SYNTAX_ERROR,
    PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum
{
    STATEMENT_INSERT,
    STATEMENT_SELECT
} StatementType;
typedef enum
{
    EXECUTE_SUCCESS,
    EXECUTE_TABLE_FULL
} ExecuteResult;
typedef struct
{
    StatementType type;
    Row row_to_insert;
} Statement;
#define size_of_attribute(Struct, Attribute) sizeof(((Struct *)0)->Attribute)
const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;
const uint32_t PAGE_SIZE = 4096;
#define TABLE_MAX_PAGES 100
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;
typedef struct
{
    uint32_t num_rows;
    void *pages[TABLE_MAX_PAGES];
} Table;
void *row_slot(Table *table, uint32_t row_num)
{
    uint32_t page_num = row_num / ROWS_PER_PAGE;
    void *page = table->pages[page_num];
    if (page == NULL)
    {
        page = table->pages[page_num] = malloc(PAGE_SIZE);
    }
    uint32_t row_offset = row_num % ROWS_PER_PAGE;
    uint32_t byte_offset = row_offset * ROW_SIZE;
    return page + byte_offset;
}

Table *new_table()
{
    Table *table = (Table *)malloc(sizeof(Table));
    table->num_rows = 0;
    for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++)
    {
        table->pages[i] = NULL;
    }
    return table;
}

void free_table(Table *table)
{
    for (int i = 0; table->pages[i]; i++)
    {
        free(table->pages[i]);
    }
    free(table);
}
InputBuffer *new_input_buffer()
{
    InputBuffer *input_buffer = malloc(sizeof(InputBuffer));
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;
    return input_buffer;
}
void read_input_buffer(InputBuffer *input_buffer)
{
    ssize_t bytes_read = getline(&input_buffer->buffer, &input_buffer->buffer_length, stdin);
    if (bytes_read == 0)
    {
        printf("Error reading input\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        input_buffer->input_length = bytes_read - 1;
        input_buffer->buffer[bytes_read - 1] = 0;
    }
}
void close_input_buffer(InputBuffer *input_buffer)
{
    free(input_buffer->buffer);
    free(input_buffer);
}
void print_prompt()
{
    printf("db >");
}
void serialize_row(void *destination, Row *source)
{
    memcpy(destination + ID_OFFSET, &(source->id), ID_SIZE);
    memcpy(destination + USERNAME_OFFSET, &(source->username), USERNAME_SIZE);
    memcpy(destination + EMAIL_OFFSET, &(source->email), EMAIL_SIZE);
}
void desearile_row(void *source, Row *destination)
{
    memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
    memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(&(destination->email), source + EMAIL_OFFSET, EMAIL_SIZE);
}

MetaCommandResult do_meta_command(InputBuffer *input_buffer, Table *table)
{
    if (strcmp(input_buffer->buffer, ".exit") == 0)
    {
        close_input_buffer(input_buffer);
        free_table(table);
        exit(EXIT_SUCCESS);
    }
    else
    {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}
PrepareResult prepare_insert(InputBuffer *inputbuffer, Statement *statement)
{
    statement->type = STATEMENT_INSERT;
    char *keyword = strtok(inputbuffer->buffer, " ");
    char *id_string = strtok(NULL, " ");

    char *username = strtok(NULL, " ");
    char *email = strtok(NULL, " ");
    if (id_string == NULL || email == NULL || username == NULL)
    {
        return PREPARE_SYNTAX_ERROR;
    }
    int id = atoi(id_string);
    if(id<0){
        return PREPARE_NEGATIVE_ID;
    }
    if (strlen(username) > COLUMN_USERNAME_SIZE || strlen(email) > COLUMN_EMAIL_SIZE)
    {
        return PREPARE_STRING_TOO_LONG;
    }
    statement->row_to_insert.id = id;
    strcpy(statement->row_to_insert.email, email);
    strcpy(statement->row_to_insert.username, username);
    return PREPARE_SUCCESS;
}
PrepareResult prepare_statement(InputBuffer *input_buffer, Statement *statement)
{
    if (strncmp(input_buffer->buffer, "insert", 6) == 0)
    {
       return prepare_insert(input_buffer, statement);
    }
    if (strncmp(input_buffer->buffer, "select", 6) == 0)
    {
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    }
    return PREPARE_UNRECOGNIZED_STATEMENT;
}
ExecuteResult execute_insert(Statement *statement, Table *table)
{
    if (table->num_rows >= TABLE_MAX_PAGES)
    {
        return EXECUTE_TABLE_FULL;
    }
    Row *row_to_insert = &statement->row_to_insert;
    serialize_row(row_slot(table, table->num_rows), row_to_insert);
    table->num_rows += 1;
    return EXECUTE_SUCCESS;
}
ExecuteResult execute_select(Statement *statement, Table *table)
{
    Row row;
    for (uint32_t i = 0; i < table->num_rows; i++)
    {
        desearile_row(row_slot(table, i), &row);
        print_row(&row);
    }
    return EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement *statement, Table *table)
{
    switch (statement->type)
    {
    case (STATEMENT_INSERT):
        return execute_insert(statement, table);

    case (STATEMENT_SELECT):
        return execute_select(statement, table);
    }
}

int main()
{
    Table *table = new_table();
    InputBuffer *input_buffer = new_input_buffer();

    while (true)
    {
        // printf("%d\n%d\n", input_buffer->input_length, input_buffer->buffer_length);
        print_prompt();
        read_input_buffer(input_buffer);
        // if(strcmp(input_buffer->buffer,".exit")==0){
        //     close_input_buffer(input_buffer);
        //     exit(EXIT_SUCCESS);
        // }
        if (input_buffer->buffer[0] == '.')
        {
            switch (do_meta_command(input_buffer, table))
            {
            case (META_COMMAND_SUCCESS):
                continue;
            case (META_COMMAND_UNRECOGNIZED_COMMAND):
                printf("unrecognized command %s\n", input_buffer->buffer);
                continue;
            }
        }
        Statement statement;
        switch (prepare_statement(input_buffer, &statement))
        {
        case (PREPARE_SUCCESS):
            break;
        case (PREPARE_NEGATIVE_ID):
        printf("ID must be positive.\n");
        	continue;
        case (PREPARE_STRING_TOO_LONG):
            printf("String is too long.\n");
            continue;
        case (PREPARE_SYNTAX_ERROR):
            printf("Syntax error. Could not parse statement.\n");
            continue;
        case (PREPARE_UNRECOGNIZED_STATEMENT):
            printf("Unrecognized keyword at start of '%s'.\n",
                   input_buffer->buffer);
            continue;
        }
        switch (execute_statement(&statement, table))
        {
        case (EXECUTE_SUCCESS):
            printf("Executed.\n");
            break;
        case (EXECUTE_TABLE_FULL):
            printf("Error: Table full.\n");
            break;
        }
    }

    return 0;
}
