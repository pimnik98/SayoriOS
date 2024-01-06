/**
 * @file lib/string.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), NDRAEY >_ (pikachu_andrey@vk.com)
 * @brief Функции для работы со строками
 * @version 0.3.4
 * @date 2022-10-01
 * @copyright Copyright SayoriOS Team (c) 2022-2023 
 */
#include "common.h"
#include "lib/string.h"
#include <emmintrin.h>  // SSE functions and types

bool isalnum(char c){
	return  (c >= '0' && c <= '9') ||
			(c >= 'a' && c <= 'z') ||
			(c >= 'A' && c <= 'Z') ;
}

/**
 * @brief Проверяет, является ли символ формата UTF-8
 *
 * @param с - Символ
 *
 * @return bool - true если да
 */
bool isUTF(char c) {
    if (c == -47 || c == -48){
        return true;
    }
    return false;
}

/**
 * @brief Проверяет, является ли специальным символом
 *
 * @param с - Символ
 *
 * @return bool - true если да
 */
bool isSymbol(char c){
    if (c == -62 || c == -106 || c == -30){
        return true;
    }
    return false;
}
/**
 * @brief Возращает индекс символа
 *
 * @param с - Символ хз зачем
 * @param с1 - Символ нафига
 * @param с2 - Символ хз
 *
 * @return Индекс символа
 */
uint32_t SymConvert(char c, char c1, char c2){
    uint32_t s = 0;
    if (c == -62){
        s = 1000;
    } else if (c == -106){
        s = 2000+(((int) c1)* -1);
    } else if (c == -30){
        s = 3000+(((int) c1)* -1);
    }
    return (isSymbol(c)?((((int) c)* -1)+(((int) c1)* -1)+s):((int) c));
}

/**
 * @brief Возращает индекс символа
 *
 * @param с - Символ
 * @param с1 - Символ
 *
 * @warning зачем нам c и c1 если нужен short c?
 * @return индекс символа
 */
uint32_t UTFConvert(char c, char c1){
    return (isUTF(c)?((((int) c)* -1)+(((int) c1)* -1)):((int) c));
}

/**
 * @brief Возращает длину строки
 *
 * @param str - Строка
 *
 * @return size_t - Длину символов
 */
size_t strlen(const char *str){
	if(str == nullptr) {
		return 0;
	}

    size_t len = 0;
    while (str[len] != 0){
        len++;
    }
    return len;
}

/**
 * @brief Возращает длину строки с учетом UTF-8
 *
 * @param str - Строка
 *
 * @return size_t - Длину символов
 */
size_t mb_strlen(const char *str){
    size_t len = 0;
    const size_t def = strlen(str);

    for(size_t i = 0; i < def;i++){
        if (isUTF(str[i])) continue;
        len++;
    }
    return len;
}

/**
 * @brief Возвращает индекс символа в строке
 *
 * @param str - строка
 * @param find - символ, индекс которого надо найти в строке
 */
size_t struntil(const char* str, const char find) {
    size_t len = 0;

    while(str[len] && str[len] != find)
        len++;

    return len;
}

/**
 * @brief Копирование непересекающихся массивов используя SSE
 *
 * @param dest - Указатель на массив в который будут скопированы данные.
 * @param src - Указатель на массив источник копируемых данных.
 * @param size - Количество байт для копирования
 */
void sse_memcpy(void* restrict dest, const void* restrict src, size_t size) {
    __m128i* d = (__m128i*)dest;
    const __m128i* s = (const __m128i*)src;

    size_t num_elems = size / sizeof(__m128i);
    const size_t remaining_bytes = size % sizeof(__m128i);

	_mm_prefetch(s, _MM_HINT_NTA);

    while(num_elems--) {
		_mm_storeu_si128(d++, _mm_loadu_si128(s++));
    }

	// Copy remaining bytes
    if (remaining_bytes > 0) {
        char* d_char = (char*)d;
        const char* s_char = (const char*)s;

        for (size_t i = 0; i < remaining_bytes; i++) {
            *d_char++ = *s_char++;
        }
    }

	_mm_sfence();
}

/**
 * @brief Копирование непересекающихся массивов
 *
 * @param destination - Указатель на массив в который будут скопированы данные.
 * @param source - Указатель на массив источник копируемых данных.
 * @param n - Количество байт для копирования
 */
void* memcpy(void *restrict destination, const void *restrict source, size_t n){
    size_t *tmp_dest = (size_t *)destination;
    size_t *tmp_src = (size_t *)source;
    size_t len = n / sizeof(size_t);
    size_t i = 0;
    size_t tail = n & (sizeof(size_t) - 1);

    for (; i < len; i++) {
        *tmp_dest++ = *tmp_src++;
    }

    if(tail) {
        char *dest = (char *)destination;
        const char *src = (const char *)source;

        for(i = n - tail; i < n; i++) {
            dest[i] = src[i];
        }
    }

	return destination;
}

/**
 * @brief Заполнение массива указанными символами
 *
 * @param ptr - Указатель на заполняемый массив
 * @param value - Код символа для заполнения
 * @param size_t size - Размер заполняемой части массива в байтах
 */
void* memset(void* ptr, char value, size_t num) {
	uint8_t* p = (uint8_t*)ptr;

	for (size_t i = 0; i < num; i++) {
		p[i] = value;
	}

	return ptr;
}

/**
 * @brief Копирование массивов (в том числе пересекающихся)
 *
 * @param dest - Указатель на массив в который будут скопированы данные.
 * @param src - Указатель на массив источник копируемых данных
 * @param count - Количество байт для копирования
 */
void* memmove(void *dest, void *src, size_t count) {
	void * ret = dest;
	if (dest <= src || (char*)dest >= ((char*)src + count))
	{
		while (count--)
		{
			*(char*)dest = *(char*)src;
			dest = (char*)dest + 1;
			src = (char*)src + 1;
		}
	}
	else
	{
		dest = (char*)dest + count - 1;
		src = (char*)src + count - 1;
		while (count--)
		{
			*(char*)dest = *(char*)src;
			dest = (char*)dest - 1;
			src = (char*)src - 1;
		}
	}
	return ret;
}

/**
 * @brief Сравнение строк
 *
 * @param s1 - Строка 1
 * @param s2 - Строка 2
 *
 * @return Возращает 0 если строки идентичны или разницу между ними
 */
int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s1 == *s2) {
        ++s1;
        ++s2;
    }

    return (*s1 - *s2);
}

/**
 * @brief Сравнение строк
 *
 * @param str1 - Строка 1
 * @param str2 - Строка 2
 *
 * @return bool - Возращает true если строки идентичны
 */
bool strcmpn(const char *str1, const char *str2){
    return strcmp(str1, str2) == 0;
}

/**
 * @brief Копирование строк
 *
 * @param dest - Указатель на строку, в которую будут скопированы данные
 * @param src - Указатель на строку источник копируемых данных
 *
 * @return int - Функция возвращает указатель на строку, в которую скопированы данные.
 */
int strcpy(char* dest, const char* src){
    int i = 0;

    while (src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }

    dest[i] = '\0';

    return i;
}

/**
 * @brief Сравнение массивов
 *
 * @param s1 - Указатель на строку
 * @param s2 - Указатель на строку
 * @param n - Размер сравниваемой части массива в байтах.
 *
 * @return int - Возращает 0 если строки идентичны или разницу между ними
 */
int32_t memcmp(const char *s1, const char *s2, size_t n){
    unsigned char u1, u2;

    for (; n--; s1++, s2++){
        u1 = *(unsigned char *)s1;
        u2 = *(unsigned char *)s2;
        if (u1 != u2){
            return (u1 - u2);
        }
    }
    return 0;
}
/**
 * @brief ???
 *
 * @param str - ???
 * @param c - ???
 *
 * @return size_t - ???
 */
size_t str_bksp(char *str, char c){
    size_t i = strlen(str);
    i--;
    while (i){
        i--;
        if (str[i] == c){
            str[i + 1] = 0;
            return 1;
        }
    }
    return 0;
}
/**
 * @brief ???
 *
 * @param s - ???
 * @param accept - ???
 *
 * @return char* - ???
 */
char *strpbrk(const char *s, const char *accept){
    while (*s != '\0'){
        const char *a = accept;
        while (*a != '\0'){
            if (*a++ == *s){
                return (char *)s;
            }
        }
        ++s;
    }
    return nullptr;
}

/**
 * @brief Определение максимальной длины участка строки, содержащего только указанные символы
 *
 * @param s - Указатель на строку, в которой ведется поиск
 * @param accept - Указатель на строку с набором символов, которые должны входить в участок строки str
 *
 * @return Длина начального участка строки, содержащая только символы, указанные в аргументе sym
 */
size_t strspn(const char *s, const char *accept){
    const char *p;
    const char *a;
    size_t count = 0;

    for (p = s; *p != '\0'; ++p){
        for (a = accept; *a != '\0'; ++a){
            if (*p == *a){
                break;
            }
        }
        if (*a == '\0') {
            return count;
        }

        ++count;
    }
    return count;
}

/**
 * @brief Сравнение строк с ограничением количества сравниваемых символов
 *
 * @param s1 - Строка 1
 * @param s2 - Строка 2
 *
 * @return int - Возращает 0 если строки идентичны или разницу между ними
 */
int32_t strncmp(const char *s1, const char *s2, size_t num){
    for (size_t i = 0; i < num; i++){
        if (s1[i] != s2[i]){
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Разбиение строки на части по указанному разделителю
 *
 * @param s - Указатель на разбиваемую строку
 * @param delim - Указатель на строку, содержащую набор символов разделителей
 *
 * @return int - nullptr – если строку str невозможно разделить на части или указатель на первый символ выделенной части строки.
 */
char *strtok(char *s, const char *delim){
    static char *olds = nullptr;
    char *token;

    if (s == nullptr){
        s = olds;
    }

    s += strspn(s, delim);

    if (*s == '\0'){
        olds = s;
        return nullptr;
    }

    token = s;
    s = strpbrk(token, delim);
    if (s == nullptr){
        olds = token;
    }
    else{
        *s = '\0';
        olds = s + 1;
    }
    return token;
}

/**
 * @brief Копирование строк c ограничением длины
 *
 * @param dest - Указатель на строку, в которую будут скопированы данные
 * @param src - Указатель на строку источник копируемых данных
 * @param n - Ограничение длинны копирования
 *
 * @return char* - Функция возвращает указатель на строку, в которую скопированы данные
 */
char *strncpy(char *dest, const char *src, size_t n) {
    for (size_t i = 0; i < n && src[i] != '\0'; i++) {
		dest[i] = src[i];
	}

	return dest;
}

/**
 * @brief Объединение строк
 *
 * @param s - Указатель на массив в который будет добавлена строка
 * @param t - Указатель на массив из которого будет скопирована строка
 *
 * @return char* - Функция возвращает указатель на массив, в который добавлена строка
 */
char* strcat(char* destination, const char* source) {
    char* ptr = destination;

    // Находим конец строки в destination
    while (*ptr != '\0') {
        ptr++;
    }

    // Копируем символы из source в конец destination
    while (*source != '\0') {
        *ptr = *source;
        ptr++;
        source++;
    }

    // Добавляем завершающий нулевой символ в destination
    *ptr = '\0';

    return destination;
}

/**
 * @brief Вырезает и возвращает подстроку из строки
 *
 * @param dest - Указатель куда будет записана строка
 * @param source - Указатель на исходную строку
 * @param from - Откуда копируем
 * @param length - Количество копируемых байт
 */
void substr(char* restrict dest, const char* restrict source, int from, int length){
    strncpy(dest, source + from, length);
    dest[length] = 0;
}

/**
 * @brief Поиск первого вхождения символа в строку
 *
 * @param _s - Указатель на строку, в которой будет осуществляться поиск.
 * @param _c - Код искомого символа
 *
 * @return char* - Указатель на искомый символ, если он найден в строке str, иначе nullptr.
 */
char *strchr(const char *_s, char _c){
    while (*_s != (char)_c){
        if (!*_s++){
            return 0;
        }
    }
    return (char *)_s;
}

/**
 * @brief Перевод строки в нижний регистр
 *
 * @param as - Указатель на строку.
 */
void strtolower(char* as){
	while(*as != 0)
	{
		if(*as >= 'A' && *as <= 'Z')
			*as += ('a' - 'A');
		as++;
	}
}

/**
 * @brief Перевод строки в верхний регистр
 *
 * @param as - Указатель на строку.
 */
void strtoupper(char* as){
	while(*as != 0)
	{
		if(*as >= 'a' && *as <= 'z')
			*as -= ('a' - 'A');
		as++;
	}
}

/**
 * @brief Проверяет, является ли строка числом
 *
 * @param c - Указатель на строку.
 *
 * @return bool - если строка является числом
 */
bool isNumber(const char* c) {
    for(uint32_t i = 0, len = strlen(c); i < len; i++){
        if (!(c[i] >= '0' && c[i] <= '9')) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Превращает строку в число
 *
 * @param s - Указатель на строку.
 *
 * @return uint32_t - Число
 */
uint32_t atoi(const char s[]){
    int i = 0, n = 0;
    bool minus = *s == '-';

    if(minus)
        i++;

    for (; s[i] >= '0' && s[i] <= '9'; ++i)
        n = (n * 10) + (s[i] - '0');

    n *= minus ? -1 : 1;

    return n;
}

/**
 * @brief Переворачивает строку задом наперед
 *
 * @param  str - строка символов, которая должна быть обращена
 */
void strver(char *str) {
    int32_t j = strlen(str) - 1;

    for (int32_t i = 0; i < j; i++) {
        char c = str[i];
        str[i] = str[j];
        str[j] = c;
        j--;
    }
}

/**
 * @brief Конвертируем число в символы
 *
 * @param n - Число
 * @param buffer - символы
 * @return Длина строки
 */
size_t itoa(int32_t n, char *buffer) {
    char const digits[] = "0123456789";
    char* p = buffer;

    if (n < 0){
        *p++ = '-';
        n = -n;
    }

    int s = n;

    do {
        ++p;
        s = s / 10;
    } while(s);

    *p = '\0';

    do {
        *--p = digits[n % 10];
        n = n / 10;
    } while(n);

    return strlen(buffer);
}

// Unsigned int to charptr
// Bufferless variant: That's ideal!
size_t itou(size_t n, char *buffer) {
    char* p = buffer;

    size_t s = n;

    do {
        ++p;
        s = s / 10;
    } while(s);

    *p = '\0';

    do {
        *--p = '0' + (n % 10);
        n = n / 10;
    } while(n);

    return strlen(buffer);
}

size_t itoh(size_t i, char *buffer) {
	const unsigned char hex[16] = "0123456789ABCDEF";
    uint32_t n, d = 0x10000000;
    char* p = buffer;

    while ((i / d == 0) && (d >= 0x10)) {
        d /= 0x10;
    }
    n = i;

    while (d >= 0xF) {
         *p++ = hex[n / d];
        n = n % d;
        d /= 0x10;
    }

	*p++ = hex[n];
    *p = 0;

    return strlen(buffer);
}


int dcmpstr( const char *s1, const char *s2 )
{
    while ( *s1 && *s1 == *s2 ) ++s1, ++s2;

    return ( ( unsigned char )*s1 > ( unsigned char )*s2 ) -
           ( ( unsigned char )*s1 < ( unsigned char )*s2 );
}

char digit_count(size_t num) {
    if(num == 0)
        return 1;
    
    char count = 0;
    while(num > 0) {
        num /= 10;
        count++;
    }
    return count;
}

char hex_count(size_t num) {
    if(num == 0)
        return 1;
    
    char _ = 0;
    while(num > 0) {
        num /= 16;
        _++;
    }
    return _;
}

bool isnumberstr(char* a) {
    while(*a) {
        if(!isdigit(*a)) {
            return false;
        }
        
        a++;
    }

    return true;
}

/**
 * @brief Посчитать количество символов `character` в строке `string` 
 *
 * @param s1 - Строка
 * @param character - Символ
 *
 * @return size_t - количество найденных символов в строке
 */
size_t strcount(const char* string, char character) {
    size_t count = 0;

    while(*string++) {
        if(*string == character)
            count++;
    }

    return count;
}

char* strstr(const char* haystack, const char* needle) {
    if (*needle == '\0') {
        return (char*) haystack;
    }

    while (*haystack) {
        const char* h = haystack;
        const char* n = needle;

        while (*h && *n && (*h == *n)) {
            h++;
            n++;
        }

        if (*n == '\0') {
            return (char*) haystack;
        }

        haystack++;
    }

    return NULL;
}


double strtod(const char* str, char** endptr) {
    double result = 0.0;
    bool negative = false;
    int exponent = 0;
    int sign = 1;

    // Пропускаем начальные пробелы
    while (*str == ' ') {
        str++;
    }

    // Определяем знак числа
    if (*str == '-' || *str == '+') {
        sign = (*str++ == '-') ? -1 : 1;
    }

    // Парсим целую часть числа
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str++ - '0');
    }

    // Парсим дробную часть числа
    if (*str == '.') {
        double fraction = 1.0;
        str++;
        while (*str >= '0' && *str <= '9') {
            result = result + (double)(*str - '0') / (fraction *= 10.0);
            str++;
        }
    }

    // Парсим порядок числа
    if (*str == 'e' || *str == 'E') {
        str++;
        // Определяем знак порядка числа
        if (*str == '-' || *str == '+') {
            exponent = (*str++ == '-') ? -1 : 1;
        }
        // Парсим значение порядка
        int temp_exponent = 0;
        while (*str >= '0' && *str <= '9') {
            temp_exponent = temp_exponent * 10 + (*str++ - '0');
        }
        exponent *= temp_exponent;
    }

    result *= sign;

    if (endptr != NULL) {
        *endptr = (char*)str;
    }

    return result * pow(10, exponent);
}




unsigned long strtoul(const char* str, char** endptr, int base) {
    unsigned long result = 0;
    int i = 0;

    // Пропускаем начальные пробелы
    while (str[i] == ' ') {
        i++;
    }

    // Преобразуем основание base в случае необходимости
    if (base == 0) {
        if (str[i] == '0') {
            if (str[i + 1] == 'x' || str[i + 1] == 'X') {
                base = 16;
                i += 2;
            } else {
                base = 8;
                i++;
            }
        } else {
            base = 10;
        }
    }

    // Обрабатываем число
    while (str[i] != '\0') {
        int digit;
        if (str[i] >= '0' && str[i] <= '9') {
            digit = str[i] - '0';
        } else if (str[i] >= 'a' && str[i] <= 'z') {
            digit = str[i] - 'a' + 10;
        } else if (str[i] >= 'A' && str[i] <= 'Z') {
            digit = str[i] - 'A' + 10;
        } else {
            break;
        }
        if (digit < base) {
            result = result * base + digit;
            i++;
        } else {
            break;
        }
    }

    // Устанавливаем значение endptr
    if (endptr != NULL) {
        *endptr = (char*)(str + i);
    }

    return result;
}