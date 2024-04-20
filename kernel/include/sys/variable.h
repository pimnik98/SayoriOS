#pragma once 

typedef struct {
	bool	Ready;
	char	Key[256];
	char	Value[1024];
} VARIABLE;

char* variable_read(char* Key);
int variable_write(char* Key, char* Value);
VARIABLE* variable_list(char* Search);
