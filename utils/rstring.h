#ifndef RSTRING_H
#define RSTRING_H

inline int rstrcmp(const char* a, const char* b)
{
	while (*a && *b)
		if (*a++ != *b++)
			return 1;
	return 0;
}

inline int rstrcmp(char* a, char* b)
{
	return rstrcmp((const char*)a, (const char*)b);
}

#endif
