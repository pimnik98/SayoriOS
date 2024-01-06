#define ADA_LPREF "[ADA]"

typedef int (*ada_fn_t)(void*);

typedef enum {
    T_NONE, T_NUMBER, T_IDENT, T_PLUS, T_MINUS,
    T_STRING, T_DOT, T_SEMICOLON, T_LPAREN, T_RPAREN,
} tokenType;

typedef struct {
    char *data;
    int size;
} ada_string_t;

typedef struct {
    tokenType type;
    char *value;
    uint32_t start_pos;
    uint32_t end_pos;
    uint32_t line;
} token_t;

typedef struct {
    ada_string_t *name;
    ada_fn_t c_ptr;
} ada_function_t;

typedef struct {
    char *data;
    uint32_t size;
    uint32_t pos;
    uint32_t old_pos;
    uint32_t line;
    token_t token;
    ada_function_t *functions;
    uint32_t functions_cnt;
} ada_t;

void ada_handler_fn(ada_t *ada, char *fn_name, ada_fn_t handler);
void ada_new(char *data);
void ada_file(char *filename);