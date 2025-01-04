#pragma once


#define REG_MAX_PATH_LEN 2048       
#define REG_MAX_VALUE_LEN 512
#define REG_MAX_NAME_LEN 64

typedef enum {
  NODE_TYPE_HIVE,
  NODE_TYPE_KEY,
  NODE_TYPE_VALUE
} RegNodeType;

typedef struct RegistryNode {
    RegNodeType type;
    char name[REG_MAX_NAME_LEN];
    char value[REG_MAX_VALUE_LEN];
    struct RegistryNode** children;
    int num_children;
    int children_capacity;
    struct RegistryNode* parent;
} RegistryNode;

typedef struct {
    RegistryNode* root;
} Registry;


void registry_add_child(RegistryNode *parent, RegistryNode *child);
RegistryNode* registry_create_node(RegNodeType type, const char* name);
void registry_init(Registry *reg);
RegistryNode* registry_find_node_by_name(RegistryNode *parent, const char* name);
RegistryNode* registry_get_node(const Registry *reg, const char* path);
bool registry_create_path(Registry *reg, const char* path);
bool registry_set_value(Registry *reg, const char* path, const char* value);
bool registry_get_value(const Registry *reg, const char* path, char *out_value, int out_max_len);
static void registry_free_recursive(RegistryNode* node);
static bool registry_delete_key_internal(RegistryNode* parent, const char* key_name);
bool registry_delete_key(Registry *reg, const char* path);
bool registry_delete_path(Registry *reg, const char* path);