/**
 * @file src/os/modules/registry.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru)
 * @brief Реестр данных
 * @version 0.4.0
 * @date 2025-01-04
 * @copyright Copyright SayoriOS Team (c) 2025
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "registry.h"

/**
 * @brief Добавляет дочерний узел к родительскому узлу в реестре.
 * 
 * Эта функция добавляет указанный дочерний узел к списку дочерних узлов родительского узла.
 * Если список дочерних узлов родительского узла полон, то он перераспределяется с увеличением размера.
 *
 * @param parent Указатель на родительский узел, к которому нужно добавить дочерний.
 * @param child Указатель на дочерний узел, который нужно добавить.
 *
 * @code
 * // Пример использования функции registry_add_child
 * RegistryNode* parent = malloc(sizeof(RegistryNode));
 * parent->num_children = 0;
 * parent->children_capacity = 0;
 * parent->children = NULL;
 * 
 * RegistryNode* child1 = malloc(sizeof(RegistryNode));
 * RegistryNode* child2 = malloc(sizeof(RegistryNode));
 * 
 * // Добавляем первого ребенка
 * registry_add_child(parent, child1);
 * 
 * // Добавляем второго ребенка
 * registry_add_child(parent, child2);
 * 
 * // Теперь у parent есть два ребенка в children.
 * // parent->children[0] == child1
 * // parent->children[1] == child2
 * // parent->num_children == 2
 * // child1->parent == parent
 * // child2->parent == parent
 * 
 * // Освобождение памяти (в реальном коде это нужно сделать аккуратно)
 * free(child1);
 * free(child2);
 * free(parent->children);
 * free(parent);
 * @endcode
 *
 * @note Необходимо предусмотреть обработку ошибок выделения памяти.
 */
void registry_add_child(RegistryNode *parent, RegistryNode *child) {
    if(parent->num_children == parent->children_capacity){
        parent->children_capacity += 4;
        parent->children = realloc(parent->children, parent->children_capacity * sizeof(RegistryNode*));
        if(!parent->children) {
             //Обработка ошибки выделения памяти
        }
    }
    
    parent->children[parent->num_children] = child;
    parent->num_children++;
    child->parent = parent;
}

/**
 * @brief Создает новый узел реестра.
 *
 * Эта функция выделяет память для нового узла реестра, инициализирует его поля и возвращает указатель на созданный узел.
 * В случае ошибки выделения памяти возвращает `NULL`.
 *
 * @param type Тип создаваемого узла (например, `REG_NODE_FOLDER`, `REG_NODE_VALUE`).
 * @param name Имя создаваемого узла (максимальная длина `REG_MAX_NAME_LEN`).
 * @return Указатель на созданный узел реестра или `NULL`, если произошла ошибка выделения памяти.
 *
 * @code
 * // Пример использования функции registry_create_node
 * RegistryNode* folderNode = registry_create_node(REG_NODE_FOLDER, "MyFolder");
 * if (folderNode) {
 *  // Успешно создан узел-папка
 *  printf("Создана папка с именем: %s\n", folderNode->name);
 *  // ... дальнейшие действия с узлом
 *  free(folderNode); // Освобождение памяти
 * } else {
 *   // Ошибка выделения памяти
 *   printf("Ошибка при создании узла-папки.\n");
 * }
 * 
 * RegistryNode* valueNode = registry_create_node(REG_NODE_VALUE, "MyValue");
 * if (valueNode) {
 *  // Успешно создан узел-значение
 *   printf("Создано значение с именем: %s\n", valueNode->name);
 *   // ... дальнейшие действия с узлом
 *   free(valueNode); // Освобождение памяти
 * } else {
 *  // Ошибка выделения памяти
 *  printf("Ошибка при создании узла-значения.\n");
 * }
 * @endcode
 *
 * @note В случае успешного создания узла, память для него должна быть освобождена с помощью `free()` когда она больше не нужна.
 * @note Функция использует `strncpy` для копирования имени, что обеспечивает защиту от переполнения буфера.
 */
RegistryNode* registry_create_node(RegNodeType type, const char* name){
    RegistryNode* node = (RegistryNode*)malloc(sizeof(RegistryNode));
    if(!node){
        //Обработка ошибки выделения памяти
        return NULL;
    }
    node->type = type;
    strncpy(node->name, name, REG_MAX_NAME_LEN);
    node->value[0] = '\0';
    node->children = NULL;
    node->num_children = 0;
    node->children_capacity = 0;
    node->parent = NULL;
    return node;
}

/**
 * @brief Инициализирует реестр, создавая корневой узел и основные ветви.
 *
 * Эта функция создает корневой узел реестра с именем "root" и две основные ветви: 
 * "HKEY_LOCAL_MACHINE" и "HKEY_USERS". Эти ветви добавляются в качестве дочерних к корневому узлу.
 *
 * @param reg Указатель на структуру реестра, которую необходимо инициализировать.
 *
 * @code
 * // Пример использования функции registry_init
 * Registry my_registry;
 * registry_init(&my_registry);
 *
 * // Теперь my_registry.root содержит дерево реестра:
 * // my_registry.root -> "root"
 * // my_registry.root->children[0] -> "HKEY_LOCAL_MACHINE"
 * // my_registry.root->children[1] -> "HKEY_USERS"
 *
 * // Освобождение памяти (необходимо выполнить в реальном коде):
 * // free(my_registry.root->children[0]);
 * // free(my_registry.root->children[1]);
 * // free(my_registry.root);
 * @endcode
 *
 * @note Функция предполагает, что структура `Registry` уже выделена.
 * @note После использования реестра, память для узлов, созданных в этой функции, должна быть освобождена.
 */
void registry_init(Registry *reg) {
    reg->root = registry_create_node(NODE_TYPE_HIVE, "root");
    RegistryNode* local_machine = registry_create_node(NODE_TYPE_HIVE, "HKEY_LOCAL_MACHINE");
    RegistryNode* users = registry_create_node(NODE_TYPE_HIVE, "HKEY_USERS");

    registry_add_child(reg->root, local_machine);
    registry_add_child(reg->root, users);
}

/**
 * @brief Находит дочерний узел по имени в заданном родительском узле.
 *
 * Эта функция выполняет поиск дочернего узла с заданным именем в списке дочерних узлов родительского узла.
 * Если узел с таким именем найден, функция возвращает указатель на этот узел.
 * В противном случае, если родительский узел равен `NULL`, не имеет дочерних узлов, или узел с заданным именем не найден, функция возвращает `NULL`.
 *
 * @param parent Указатель на родительский узел, в котором производится поиск.
 * @param name Имя дочернего узла, который необходимо найти.
 * @return Указатель на найденный дочерний узел или `NULL`, если узел не найден.
 *
 * @code
 * // Пример использования функции registry_find_node_by_name
 * // Предположим, что есть узел root и у него есть дочерний узел "child1"
 * // RegistryNode* root = ... (уже инициализированный узел)
 * // RegistryNode* child1 = ... (уже инициализированный узел с именем "child1")
 * // registry_add_child(root, child1);
 *
 * // Поиск узла "child1"
 * RegistryNode* found_node = registry_find_node_by_name(root, "child1");
 * if (found_node != NULL) {
 *   printf("Узел с именем 'child1' найден.\n");
 *   // ... дальнейшие действия с узлом
 * } else {
 *   printf("Узел с именем 'child1' не найден.\n");
 * }
 *
 * // Поиск узла "child2", которого нет
 * RegistryNode* not_found_node = registry_find_node_by_name(root, "child2");
 * if (not_found_node == NULL) {
 *    printf("Узел с именем 'child2' не найден.\n");
 * }
 * @endcode
 *
 * @note Функция сравнивает имена узлов с помощью `strcmp`.
 */
RegistryNode* registry_find_node_by_name(RegistryNode *parent, const char* name){
    if(!parent || parent->num_children == 0)
        return NULL;

     for(int i = 0; i < parent->num_children; i++){
         if(strcmp(parent->children[i]->name, name) == 0)
            return parent->children[i];
     }

     return NULL;
}

/**
 * @brief Получает узел реестра по указанному пути.
 *
 * Эта функция проходит по указанному пути в реестре, начиная с корневого узла, и возвращает указатель на узел,
 * находящийся в конце пути. Путь разделяется на части по символу обратного слэша `\`.
 * Если какой-либо узел на пути не найден, или если передан неверный указатель на реестр или путь,
 * функция возвращает `NULL`.
 *
 * @param reg Указатель на структуру реестра.
 * @param path Строка, представляющая путь к искомому узлу в реестре. Путь должен быть разделен обратными слешами (например, "root\HKEY_LOCAL_MACHINE\Software").
 * @return Указатель на найденный узел реестра или `NULL`, если узел не найден или произошла ошибка.
 *
 * @code
 * // Пример использования функции registry_get_node
 * // Предположим, что реестр `my_registry` инициализирован и содержит узлы:
 * // "root", "HKEY_LOCAL_MACHINE", и "Software"
 * //
 * //  registry_init(&my_registry);
 * // RegistryNode* hklm = registry_get_node(&my_registry, "root\\HKEY_LOCAL_MACHINE");
 * // RegistryNode* sw   = registry_get_node(&my_registry, "root\\HKEY_LOCAL_MACHINE\\Software");
 *
 * Registry my_registry;
 * registry_init(&my_registry);
 *
 * RegistryNode* node1 = registry_get_node(&my_registry, "root");
 * if (node1 != NULL) {
 *     printf("Узел 'root' найден.\n");
 * } else {
 *     printf("Узел 'root' не найден.\n");
 * }
 *
 * RegistryNode* node2 = registry_get_node(&my_registry, "root\\HKEY_LOCAL_MACHINE");
 * if (node2 != NULL) {
 *     printf("Узел 'root\\HKEY_LOCAL_MACHINE' найден.\n");
 * } else {
 *     printf("Узел 'root\\HKEY_LOCAL_MACHINE' не найден.\n");
 * }
 *
 * RegistryNode* node3 = registry_get_node(&my_registry, "root\\HKEY_LOCAL_MACHINE\\Software");
 * if (node3 != NULL) {
 *      printf("Узел 'root\\HKEY_LOCAL_MACHINE\\Software' найден.\n");
 * } else {
 *      printf("Узел 'root\\HKEY_LOCAL_MACHINE\\Software' не найден.\n");
 * }
 *
 * // Поиск несуществующего узла
 * RegistryNode* node4 = registry_get_node(&my_registry, "root\\NonExistentNode");
 * if (node4 == NULL) {
 *   printf("Узел 'root\\NonExistentNode' не найден.\n");
 * }
 *
 * @endcode
 *
 * @note Функция использует `strtok` для разделения пути на части.
 * @note Функция использует `strncpy` для копирования пути, что обеспечивает защиту от переполнения буфера.
 */
RegistryNode* registry_get_node(const Registry *reg, const char* path){
    if(!reg || !path)
        return NULL;
     
    char path_copy[REG_MAX_PATH_LEN];
    strncpy(path_copy, path, REG_MAX_PATH_LEN);

    char* token = strtok(path_copy, "\\");
    if(!token)
        return NULL;
    
    RegistryNode *current = reg->root;
    
    while(token != NULL){
        current = registry_find_node_by_name(current, token);
        if(!current)
            return NULL;

        token = strtok(NULL, "\\");
    }

    return current;
}

/**
 * @brief Создает путь в реестре, добавляя отсутствующие узлы.
 *
 * Эта функция создает путь в реестре, начиная с корневого узла. Если какой-либо узел на пути не существует,
 * он будет создан и добавлен в качестве дочернего узла. Если путь уже существует, функция ничего не делает, а просто возвращает `true`.
 * Если передается `NULL` указатель на реестр или путь, или если возникает ошибка при создании узла, функция возвращает `false`.
 *
 * @param reg Указатель на структуру реестра.
 * @param path Строка, представляющая путь к узлу в реестре. Путь должен быть разделен обратными слешами (например, "root\HKEY_LOCAL_MACHINE\Software").
 * @return `true`, если путь успешно создан или уже существовал, `false`, если возникла ошибка или переданы неверные параметры.
 *
 * @code
 * // Пример использования функции registry_create_path
 * Registry my_registry;
 * registry_init(&my_registry);
 *
 * // Создаем путь "root\HKEY_LOCAL_MACHINE\Software"
 * if (registry_create_path(&my_registry, "root\\HKEY_LOCAL_MACHINE\\Software")) {
 *   printf("Путь 'root\\HKEY_LOCAL_MACHINE\\Software' успешно создан.\n");
 *
 *   // Попытка создать тот же путь повторно (должно вернуть true)
 *   if (registry_create_path(&my_registry, "root\\HKEY_LOCAL_MACHINE\\Software"))
 *     printf("Путь 'root\\HKEY_LOCAL_MACHINE\\Software' уже существует.\n");
 *
 *   // Создаем путь "root\\NewKey"
 *   if(registry_create_path(&my_registry, "root\\NewKey")){
 *    printf("Путь 'root\\NewKey' создан.\n");
 *   }
 *
 *   // Попытка создать путь с ошибкой
 *   // registry_create_node может не выделить память
 *   // (В данном примере для теста заменяем ее на функцию, которая всегда возвращает NULL)
 *
 * } else {
 *   printf("Не удалось создать путь 'root\\HKEY_LOCAL_MACHINE\\Software'.\n");
 * }
 *
 *
 * @endcode
 *
 * @note Функция использует `strtok` для разделения пути на части.
 * @note Функция использует `strncpy` для копирования пути, что обеспечивает защиту от переполнения буфера.
 * @note Функция создает узлы типа `NODE_TYPE_KEY`.
 */
bool registry_create_path(Registry *reg, const char* path) {
    if(!reg || !path)
        return false;

    char path_copy[REG_MAX_PATH_LEN];
    strncpy(path_copy, path, REG_MAX_PATH_LEN);

    char* token = strtok(path_copy, "\\");
    if(!token)
        return false;
    
    RegistryNode *current = reg->root;

    while(token != NULL){
        RegistryNode *existing_node = registry_find_node_by_name(current, token);
        if(existing_node){
            current = existing_node;
        } else {
            RegistryNode *new_node = registry_create_node(NODE_TYPE_KEY, token);
            if(!new_node){
                return false;
            }
            registry_add_child(current, new_node);
            current = new_node;
         }

        token = strtok(NULL, "\\");
    }
  
    return true;
}

/**
 * @brief Устанавливает значение узла реестра по указанному пути.
 *
 * Эта функция устанавливает значение узла реестра по указанному пути. Если узел по указанному пути существует и имеет тип `NODE_TYPE_VALUE`, то его значение обновляется.
 * Если узел по указанному пути не существует, то функция пытается создать путь до родительского узла, а затем создать узел типа `NODE_TYPE_VALUE` с заданным значением.
 * Если передан `NULL` указатель на реестр, путь или значение, либо если не удалось создать путь или узел, функция возвращает `false`.
 *
 * @param reg Указатель на структуру реестра.
 * @param path Строка, представляющая путь к узлу в реестре (например, "root\HKEY_LOCAL_MACHINE\Software\MyValue").
 * @param value Строка, представляющая устанавливаемое значение узла.
 * @return `true`, если значение успешно установлено, `false`, если произошла ошибка или переданы неверные параметры.
 *
 * @code
 * // Пример использования функции registry_set_value
 * Registry my_registry;
 * registry_init(&my_registry);
 *
 * // Установка значения для нового узла, который будет создан
 * if (registry_set_value(&my_registry, "root\\HKEY_LOCAL_MACHINE\\Software\\MyValue", "MyValueData")) {
 *   printf("Значение 'MyValueData' успешно установлено для узла 'root\\HKEY_LOCAL_MACHINE\\Software\\MyValue'.\n");
 * } else {
 *    printf("Не удалось установить значение для узла 'root\\HKEY_LOCAL_MACHINE\\Software\\MyValue'.\n");
 * }
 *
 * // Установка значения для существующего узла
 * if(registry_set_value(&my_registry, "root\\HKEY_LOCAL_MACHINE\\Software\\MyValue", "NewData")){
 *   printf("Значение 'NewData' успешно установлено для узла 'root\\HKEY_LOCAL_MACHINE\\Software\\MyValue'.\n");
 * } else {
 *    printf("Не удалось установить значение для узла 'root\\HKEY_LOCAL_MACHINE\\Software\\MyValue'.\n");
 * }
 * // Попытка установить значение для пути, в котором нет родителя
 * if (!registry_set_value(&my_registry, "root\\NotExist\\MyValue", "SomeData")){
 *  printf("Не удалось установить значение для узла 'root\\NotExist\\MyValue'\n");
 * }
 *
 * // Попытка установить значение с null параметром
 * if (!registry_set_value(&my_registry, "root\\HKEY_LOCAL_MACHINE\\Software\\MyValue", nullptr)) {
 *   printf("Не удалось установить значение из-за некорректного параметра 'value'.\n");
 * }
 * @endcode
 *
 * @note Функция использует `strrchr` для выделения имени узла значения из пути.
 * @note Функция использует `strncpy` для копирования пути, имени узла и значения, что обеспечивает защиту от переполнения буфера.
 * @note Если узел типа `NODE_TYPE_VALUE` не существует, он будет создан.
 */
bool registry_set_value(Registry *reg, const char* path, const char* value){
    if(!reg || !path || !value)
        return false;

    RegistryNode* node = registry_get_node(reg, path);

    if(!node || node->type != NODE_TYPE_VALUE){
        
        char* last_slash = strrchr(path, '\\');
        if(last_slash == NULL){
            return false; 
        }
        int key_len = last_slash - path;
        char key_path[REG_MAX_PATH_LEN];
        strncpy(key_path, path, key_len);
        key_path[key_len] = '\0';
        
        if(!registry_create_path(reg, key_path)){
            return false;
        }

        RegistryNode* parent_node = registry_get_node(reg, key_path);
        if(!parent_node){
            return false;
        }

        char value_name[REG_MAX_NAME_LEN];
        strncpy(value_name, last_slash + 1, REG_MAX_NAME_LEN - 1);
         
       
        RegistryNode* value_node = registry_create_node(NODE_TYPE_VALUE, value_name);
        if(!value_node){
            return false;
        }
        registry_add_child(parent_node, value_node);
        strncpy(value_node->value, value, REG_MAX_VALUE_LEN);
      
        return true;
    } else {
        strncpy(node->value, value, REG_MAX_VALUE_LEN);
        return true;
    }

    return false;
}

/**
 * @brief Получает значение узла реестра по указанному пути.
 *
 * Эта функция извлекает значение узла реестра по указанному пути и копирует его в буфер `out_value`.
 * Если узел по указанному пути не существует или не имеет типа `NODE_TYPE_VALUE`, функция возвращает `false`.
 * Если извлечение значения прошло успешно, функция возвращает `true`.
 *
 * @param reg Указатель на структуру реестра.
 * @param path Строка, представляющая путь к узлу в реестре (например, "root\HKEY_LOCAL_MACHINE\Software\MyValue").
 * @param out_value Буфер, в который будет скопировано значение узла.
 * @param out_max_len Максимальная длина буфера `out_value`.
 * @return `true`, если значение успешно получено, `false`, если произошла ошибка или узел не найден.
 *
 * @code
 * // Пример использования функции registry_get_value
 * Registry my_registry;
 * registry_init(&my_registry);
 * 
 * // Создаем путь и узел значения
 * registry_create_path(&my_registry, "root\\HKEY_LOCAL_MACHINE\\Software");
 * registry_set_value(&my_registry, "root\\HKEY_LOCAL_MACHINE\\Software\\MyValue", "SomeData");
 *
 * // Получаем значение узла
 * char value_buffer[REG_MAX_VALUE_LEN];
 * if (registry_get_value(&my_registry, "root\\HKEY_LOCAL_MACHINE\\Software\\MyValue", value_buffer, REG_MAX_VALUE_LEN)) {
 *   printf("Значение узла: %s\n", value_buffer);
 * } else {
 *   printf("Не удалось получить значение узла 'root\\HKEY_LOCAL_MACHINE\\Software\\MyValue'.\n");
 * }
 *
 * // Попытка получить значение несуществующего узла
 * if (!registry_get_value(&my_registry, "root\\HKEY_LOCAL_MACHINE\\Software\\NonExistentValue", value_buffer, REG_MAX_VALUE_LEN)) {
 *   printf("Не удалось получить значение несуществующего узла.\n");
 * }
  *
 * // Попытка получить значение с NULL указателем
 * if(!registry_get_value(nullptr, "root\\HKEY_LOCAL_MACHINE\\Software\\MyValue", value_buffer, REG_MAX_VALUE_LEN)){
 *  printf("Не удалось получить значение из-за некорректного параметра 'reg'.\n");
 * }
 *
 * @endcode
 *
 * @note Функция использует `registry_get_node` для поиска узла по пути.
 * @note Функция использует `strncpy` для копирования значения узла в выходной буфер, что обеспечивает защиту от переполнения буфера.
 */
bool registry_get_value(const Registry *reg, const char* path, char *out_value, int out_max_len) {
    RegistryNode* node = registry_get_node(reg, path);
    if(!node || node->type != NODE_TYPE_VALUE){
        return false;
    }

    strncpy(out_value, node->value, out_max_len);
    return true;
}

/**
 * @brief Рекурсивно освобождает память, выделенную для узла реестра и его дочерних узлов.
 *
 * Эта статическая функция рекурсивно обходит дерево реестра, начиная с заданного узла, и освобождает память, выделенную для каждого узла,
 * включая массивы дочерних узлов. Функция корректно обрабатывает ситуацию, когда узел равен `NULL`.
 *
 * @param node Указатель на узел реестра, память которого необходимо освободить.
 *
 * @code
 * // Пример использования функции registry_free_recursive
 * // Предположим, что у нас есть дерево реестра, где root - корневой узел
 * // RegistryNode* root = ... (уже инициализированное дерево)
 * 
 * // Освобождаем память рекурсивно
 * registry_free_recursive(root);
 * // root после вызова функции уже не валиден
 * @endcode
 *
 * @note Функция должна вызываться только для узла, память под который выделена с помощью `malloc` (или аналогичной функции).
 * @note После вызова этой функции, указатель на переданный узел и все его дочерние узлы становятся недействительными.
 * @note Функция использует рекурсию для обхода дерева реестра.
 */
static void registry_free_recursive(RegistryNode* node) {
    if (!node) {
        return;
    }
    
    for (int i = 0; i < node->num_children; i++) {
        registry_free_recursive(node->children[i]);
    }
   
    free(node->children);
    free(node);
    
}

/**
 * @brief Внутренняя функция для удаления ключа (узла) из реестра по имени.
 *
 * Эта статическая функция удаляет дочерний узел с заданным именем из родительского узла. 
 * Функция ищет дочерний узел с совпадающим именем, рекурсивно освобождает память, выделенную для этого узла и его потомков,
 * и затем удаляет узел из списка дочерних узлов родительского узла, сдвигая оставшиеся узлы.
 *
 * @param parent Указатель на родительский узел, из которого нужно удалить дочерний узел.
 * @param key_name Имя дочернего узла, который нужно удалить.
 * @return `true`, если дочерний узел был успешно удален, `false` в противном случае (если родительский узел равен `NULL`, имя ключа равно `NULL` или дочерний узел с указанным именем не найден).
 *
 * @code
 * // Пример использования функции registry_delete_key_internal
 * // Предположим, что у нас есть узел root и дочерний узел с именем "key1"
 * // RegistryNode* root = ... (уже инициализированный узел)
 * // RegistryNode* key1 = ... (уже инициализированный дочерний узел с именем "key1")
 * // registry_add_child(root, key1);
 *
 * // Удаляем дочерний узел с именем "key1"
 * if (registry_delete_key_internal(root, "key1")) {
 *  printf("Узел 'key1' успешно удален.\n");
 *  // key1 после вызова функции не валиден, как и потомки key1
 *  // root после вызова функции, имеет num_children меньше на 1
 * } else {
 *  printf("Не удалось удалить узел 'key1'.\n");
 * }
 * // Попытка удалить несуществующий узел
 * if (!registry_delete_key_internal(root, "nonexistent_key")){
 *    printf("Не удалось удалить узел 'nonexistent_key'.\n");
 * }
 *
 * @endcode
 *
 * @note Функция является внутренней и предназначена для использования внутри модуля.
 * @note Функция использует `strcmp` для сравнения имен узлов.
 * @note После вызова этой функции, указатель на удаленный дочерний узел и все его дочерние узлы становятся недействительными.
 */
static bool registry_delete_key_internal(RegistryNode* parent, const char* key_name){
    if(!parent || !key_name){
        return false;
    }
    for (int i = 0; i < parent->num_children; ++i) {
        if (strcmp(parent->children[i]->name, key_name) == 0) {
            registry_free_recursive(parent->children[i]);
        
            for (int j = i; j < parent->num_children - 1; j++){
                parent->children[j] = parent->children[j+1];
            }
            parent->num_children--;
            return true;
        }
    }
    return false;
}

/**
 * @brief Удаляет ключ (узел) реестра по указанному пути.
 *
 * Эта функция удаляет узел реестра, находящийся по указанному пути. Она находит родительский узел,
 * используя путь, и вызывает внутреннюю функцию `registry_delete_key_internal` для удаления целевого узла.
 * Если путь не существует, или если путь указывает на корень реестра, или если удаление не удалось, функция возвращает `false`.
 *
 * @param reg Указатель на структуру реестра.
 * @param path Строка, представляющая путь к узлу, который нужно удалить (например, "root\HKEY_LOCAL_MACHINE\Software\MyKey").
 * @return `true`, если узел был успешно удален, `false` в противном случае.
 *
 * @code
 * // Пример использования функции registry_delete_key
 * Registry my_registry;
 * registry_init(&my_registry);
 *
 * // Создаем путь и узел для удаления
 * registry_create_path(&my_registry, "root\\HKEY_LOCAL_MACHINE\\Software");
 * registry_create_path(&my_registry, "root\\HKEY_LOCAL_MACHINE\\Software\\MyKey");
 *
 * // Удаляем узел "MyKey"
 * if (registry_delete_key(&my_registry, "root\\HKEY_LOCAL_MACHINE\\Software\\MyKey")) {
 *     printf("Узел 'root\\HKEY_LOCAL_MACHINE\\Software\\MyKey' успешно удален.\n");
 * } else {
 *      printf("Не удалось удалить узел 'root\\HKEY_LOCAL_MACHINE\\Software\\MyKey'.\n");
 * }
 *
 * // Пытаемся удалить несуществующий узел
 * if(!registry_delete_key(&my_registry, "root\\HKEY_LOCAL_MACHINE\\Software\\NonExistentKey")){
 *    printf("Не удалось удалить узел 'root\\HKEY_LOCAL_MACHINE\\Software\\NonExistentKey'.\n");
 * }
 *
 * // Попытка удалить корень
 * if(!registry_delete_key(&my_registry, "root")){
 *   printf("Невозможно удалить корневой узел 'root'.\n");
 * }
 *
 *
 * @endcode
 *
 * @note Функция использует `strrchr` для выделения имени узла из пути.
 * @note Функция использует `strncpy` для копирования пути и имени узла, что обеспечивает защиту от переполнения буфера.
 * @note Функция не позволяет удалять корневой узел реестра.
 */
bool registry_delete_key(Registry *reg, const char* path) {
    if (!reg || !path) {
        return false;
    }

    char path_copy[REG_MAX_PATH_LEN];
    strncpy(path_copy, path, REG_MAX_PATH_LEN);

    char* last_slash = strrchr(path_copy, '\\');
    if (last_slash == NULL) {
        // Если нет слэша, то удаляем корень, чего делать нельзя
        return false;
    }

    int key_len = last_slash - path_copy;
    char key_path[REG_MAX_PATH_LEN];
    strncpy(key_path, path_copy, key_len);
    key_path[key_len] = '\0';

    char key_name[REG_MAX_NAME_LEN];
    strncpy(key_name, last_slash + 1, REG_MAX_NAME_LEN - 1);

    RegistryNode *parent_node = registry_get_node(reg, key_path);
    if (!parent_node) {
        return false; // Путь не найден
    }

    return registry_delete_key_internal(parent_node, key_name);
}

/**
 * @brief Удаляет узел реестра и все его дочерние узлы по указанному пути.
 *
 * Эта функция удаляет узел реестра, расположенный по указанному пути, а также все его дочерние узлы.
 * Она находит родительский узел, используя путь, и вызывает внутреннюю функцию `registry_delete_key_internal` для удаления целевого узла.
 * Если путь не существует, или если путь указывает на корень реестра, или если удаление не удалось, функция возвращает `false`.
 *
 * @param reg Указатель на структуру реестра.
 * @param path Строка, представляющая путь к узлу, который нужно удалить (например, "root\HKEY_LOCAL_MACHINE\Software\MyKey").
 * @return `true`, если узел и все его потомки были успешно удалены, `false` в противном случае.
 *
 * @code
 * // Пример использования функции registry_delete_path
 * Registry my_registry;
 * registry_init(&my_registry);
 *
 * // Создаем путь и узлы для удаления
 * registry_create_path(&my_registry, "root\\HKEY_LOCAL_MACHINE\\Software\\MyKey1");
 * registry_create_path(&my_registry, "root\\HKEY_LOCAL_MACHINE\\Software\\MyKey1\\MyKey2");
 *
 * // Удаляем путь "root\\HKEY_LOCAL_MACHINE\\Software\\MyKey1", включающий все вложенные узлы
 * if (registry_delete_path(&my_registry, "root\\HKEY_LOCAL_MACHINE\\Software\\MyKey1")) {
 *   printf("Путь 'root\\HKEY_LOCAL_MACHINE\\Software\\MyKey1' и все его потомки успешно удалены.\n");
 * } else {
 *   printf("Не удалось удалить путь 'root\\HKEY_LOCAL_MACHINE\\Software\\MyKey1'.\n");
 * }
 *
 * // Попытка удалить несуществующий путь
 * if(!registry_delete_path(&my_registry, "root\\HKEY_LOCAL_MACHINE\\Software\\NonExistentKey")){
 *  printf("Не удалось удалить путь 'root\\HKEY_LOCAL_MACHINE\\Software\\NonExistentKey'.\n");
 * }
 *
 * // Попытка удалить куст(корень)
 * if (!registry_delete_path(&my_registry, "root")) {
 *  printf("Нельзя удалить корень реестра 'root'.\n");
 * }
 *
 * @endcode
 *
 * @note Функция использует `strrchr` для выделения имени узла из пути.
 * @note Функция использует `strncpy` для копирования пути и имени узла, что обеспечивает защиту от переполнения буфера.
 * @note Функция не позволяет удалять корневой узел реестра.
 */
bool registry_delete_path(Registry *reg, const char* path) {
    if (!reg || !path) {
        return false;
    }
    char path_copy[REG_MAX_PATH_LEN];
    strncpy(path_copy, path, REG_MAX_PATH_LEN);

    RegistryNode* node = registry_get_node(reg, path_copy);
    if(!node) {
        return false;
    }

    char* last_slash = strrchr(path_copy, '\\');
    if (last_slash == NULL) {
        return false; // Нельзя удалять куст
    }
   
    int key_len = last_slash - path_copy;
    char key_path[REG_MAX_PATH_LEN];
    strncpy(key_path, path_copy, key_len);
    key_path[key_len] = '\0';

    char key_name[REG_MAX_NAME_LEN];
    strncpy(key_name, last_slash + 1, REG_MAX_NAME_LEN - 1);

    RegistryNode* parent = registry_get_node(reg, key_path);
    if(!parent){
        return false;
    }

    if(registry_delete_key_internal(parent, key_name)){
        return true;
    } else{
        return false;
    }
}
