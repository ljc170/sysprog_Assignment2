
#include <stdlib.h>
#include <stdio.h>
#include "mymalloc.h"
#include <stdint.h>

int main ()
{
	for (int outer = 0; outer < 50; outer++)
	{
		// Test A: malloc() 1 byte and immediately free -- do this 120 times
		for (int inner = 0; inner < 120; inner++)
		{
			char * ptr = malloc(sizeof(char));
			free(ptr);
		}

		// Test B: malloc() 1 byte, store the pointer in an array - do this 120 times.
		//		Once you've malloc()ed 120 byte chunks, then free() the 120 1 byte pointers 
		//		one by one.
		char * array[120];

		// Storing pointers into array:
		for (int index = 0; index < 120; index++)
		{
			array[index] = (char *) malloc(sizeof(char));
		}
		// Freeing those pointers
		for (int index = 0; index < 120; index++)
		{
			free(array[index]);
		}

		// 240 times, randomly choose between a 1 byte malloc() or free()ing one of the malloc()ed
		//		 pointers
		//		-	 Keep track of each operation so that you eventually malloc() 120 bytes, in total
		//		-	 Keep track of each operation so that you eventually free() all pointers
		//				(don't allow a free() if you have no pointers to free())
		
		int mallocs = 0;
		int mallocs_index = 0;
		
		for (int inner = 0; inner < 240; inner++)
		{
			if (mallocs_index > 0)
			{
				int random = rand();

				if (mallocs == 120)
				{
					// If there have been 120 mallocs, just free for the rest of the iterations
					goto FREE;
				}



				if (random > RAND_MAX / 2)
				{
					// 50% of the time, malloc
				MALLOC:
					array[mallocs_index] = (char *) malloc(sizeof(char));
					mallocs_index++;
					mallocs++;
				}
				else
				{
				FREE:
					// 50% of the time, free
					mallocs_index--;
					free(array[mallocs_index]);
				}
			}
			else
			{
				// If there are 0 mallocs, malloc:
				goto MALLOC;
			}
		}
	}

	
	
	return EXIT_SUCCESS;
}
