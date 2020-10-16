#pragma once

void * mymalloc(int, const char *, int);
void myfree(void *, const char *, int);

#define malloc( x ) mymalloc(x, __FILE__, __LINE__)
#define free( x )   myfree(x,   __FILE__, __LINE__)

