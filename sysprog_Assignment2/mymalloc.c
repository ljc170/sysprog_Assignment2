#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"

#define FREED 0
#define ALLOCATED 1
#define MEM_SIZE 4096

#define bool char
#define true 1
#define false 0

// structure used to indicate regions of memory
//		each mem_region in memory has:
//		(1) an int size (16bit to save space)
//			which tells how many bytes of memory, immediately
//			following the struct, it is responsible for, and 
//		(2) a bool status (8bit to save space)
//			indicating whether that region is currently ALLOCATED
//			or if it is FREED (see above #defines)
// mem_region structs are always immediately followed by a region of 
//		memory that is as large as ->size bytes that is used as user memory
// Every section of user memory is always immediately followed by another
//		mem_region struct and its region of user memory, and on and on through
//		the entire memory space
// At first, there is only one mem_region at memory[0] which has size (MEM_SIZE minus
//		the size of the one mem_region struct (itself)), that is FREED, and every call to
//		my_malloc subdivides this initial mem_region
// Any time my_free is called with a valid pointer, that pointer's mem_region is marked as FREED,
//		and is combined with other nearby mem_regions that are also marked FREED
typedef struct _ {
	uint16_t size;
	bool status;
} mem_region;


// When RAM is initialized, we establish the initial condition of memory
// i.e. set an initial mem_region struct at the start of RAM which has the size
//		the entire RAM (minus 3 bytes), and set it to freed
// This struct would be represented in memory as the bytes 0xfc, 0x0f, (the size) and 
//		0x00 (the FREED status)
// Any time an allocation happens, this initial mem_region will
//		be subdivided into other mem_regions, followed by user data
char memory[ MEM_SIZE ] = { 0xfc, 0x0f, 0x00 };

int main ()
{
	/*for (int i = 0; i < MEM_SIZE; i++)
	{
		void * ptr = my_malloc(1);
		
		if (ptr == NULL)
		{
			printf("NULL! at %d\n", i);
			return EXIT_SUCCE\nSS;
		}
		
	}*/

	struct point {
		int x;
		int y;
	};

	struct point3d {
		int x;
		int y;
		int z;
	};

	char * p = (char *)malloc( 200 );
	free( p + 10 );


	//malloc(5000);

	struct point * p1 = malloc(sizeof(struct point));
	struct point * p2 = malloc(sizeof(struct point));
	struct point * p3 = malloc(sizeof(struct point));
	struct point3d * p4 = malloc(sizeof(struct point3d));
	struct point * p5 = malloc(sizeof(struct point));
	free(p4 + 5);
	free(p2);
	free(p3);

	mem_region * r1 = (mem_region *) (&memory);
	mem_region * r2 = (mem_region *) (((int) &memory) + sizeof(mem_region) + sizeof(struct point));
	mem_region * r3 = (mem_region *) (((int) &memory) + (sizeof(mem_region) + sizeof(struct point)) * 2);
	mem_region * r4 = (mem_region *) (((int) &memory) + (sizeof(mem_region) + sizeof(struct point)) * 3);
	mem_region * r5 = (mem_region *) (((int) &memory) + (sizeof(mem_region) + sizeof(struct point)) * 3 + (sizeof(mem_region) + sizeof(struct point3d)));


	int i = 0;

	p5->x = 0x69;
	p5->y = 0x420;

	
	
	free(p5);
	
	
	return EXIT_SUCCESS;
}

void * mymalloc(int size, const char * file, int line)
{
	// Offset into RAM
	uint16_t mem_offset = 0;
	// Start search for block of memory large enough for the current allocation
	//		at the start of RAM
	mem_region * cur_region = (mem_region *) (((int) &memory) + mem_offset);

	while ((cur_region->size < size || cur_region->status == ALLOCATED) && mem_offset < MEM_SIZE)
	{
		// We can determine if any mem_region is a valid place for the requested
		//		pointer if that mem_region’s size is is greater than or equal to the
		//		requested size AND if that mem_region’s status is FREED, so we
		//		continue iterating if the current mem_region does not fullfill both of
		//		these requirements
		// If the current region was not a valid place to store required memory,
		//		then we move the memory offset forward by the size of the current
		//		memory block + the size of a mem_region struct, so that
		//		mem_offset is effectively the offset into the next mem_region after
		//		cur_region
		mem_offset += (sizeof(mem_region) + cur_region->size);
		cur_region = (mem_region *) (((int) &memory) + mem_offset);
	}
	// Because the above while loop can be broken either: when the mem_offset has
	//		surpassed the size of the RAM, or when cur_region stores a pointer to
	//		a mem_region that is a valid place to store the required pointer, we need
	//		to first make sure it was broken because we found a good region:
	if (mem_offset < MEM_SIZE)
	{
		// If we have successfully found a block of memory large enough to store
		// the required pointer:

		// cur_region is a pointer to a mem_region that tracks the size of the entire
		//		region in front of it, even if it is much larger than the allocation size
		// We need to divide the memory region described by cur_region into two
		//		two areas, the region that we’re allocating now, and the remaining
		//		area in the region

		// Store the total size of the found region
		int reg_size = cur_region->size;

		// Now, we set cur_region to a mem_region that is appropriate for the
		//		requested memory size:
		// Setting the size of the current region to the size of the requested pointer
		cur_region->size = size;
		// Set the status of the region we’re allocating to ALLOCATED
		cur_region->status = ALLOCATED;

		// Finding the amount of memory between the region we just allocated and the 
		//		next mem_region we've allocated (or the end of memory) 
		// Rem size is NOT the total remaining bytes until the next mem_region, it is
		//		the remaining bytes until the next mem_region MINUS the amount bytes
		//		needed for a mem_region struct to account for the leftover data,
		//			-	If there is enough space to store some data, but not enough for that
		//					data as well as the mem_region struct (usually, 24bits) to account for it, 
		//					then we still cannot allow for it to be allocatable 
		//			-	It is effectively the size of the allowable user data between this allocation
		//					and the next mem_region (or the end of memory)
		int rem_size = reg_size - size - sizeof(mem_region);
		if (rem_size > 0)
		{
			// If there is enough bytes between the block of data we just allocated
			//		and the next mem_region for at least one more byte of data
			//		as well as the mem_region struct needed to account for it:
			//			-	 We create a new mem_region at the end of the user data region that cur_region
			//						accounts for (the region being requested in this my_malloc call), and
			//			-	 We give it a FREED ->status, and 
			//			-	 A ->size of of rem_size
			mem_region * rem_region = (mem_region *) ((int) &memory + mem_offset + sizeof(mem_region) + size);
			rem_region->size = rem_size;
			rem_region->status = FREED;
		}
		else
		{
			// If there is not enough room to store, and keep track of, even just one
			//		more byte of data, then we just set the size of the region we just 
			//		allocated to the size of the entire memory region between that allocation, 
			//		and the next mem_region (or the end or memory), which makes it
			//		so cur_region tracks the entire region, even if the returned
			//		pointer doesn’t necessarily need all those bytes
			cur_region->size = reg_size;
		}

		// Returning a pointer to the current region + the size of the mem_region
		//		struct (which means a pointer to the user memory that
		//		cur_region tracks)
		return (void *) (((int) cur_region) + sizeof(mem_region));
}
	else
	{
		// Error, not enough room to allocate

		printf (
			"%s: [%d]\n\tError.  Unable to allocate [%d] bytes.  No large enough area of memory.\n", 
			file, 
			line, 
			size
		);

		return NULL;
	}
}

void myfree(void * pointer, const char * file, int line)
{
	if (pointer < (int) &memory || pointer >= ((int) &memory) + MEM_SIZE)
	{
		// Error, invalid pointer — memory not managed by this program
		printf (
			"%s: [%d]\n\tError.  Unable to free pointer to [0x%x].  This pointer is points to an invalid (out of range) region of memory.\n\tEither the parameter pointer is a local variable or it was not allocated by malloc.\n", 
			file, 
			line,
			(int) pointer
		);
	}

	int mem_offset = 0;
	mem_region * prev_region = NULL;
	while (mem_offset < MEM_SIZE)
	{
		mem_region * cur_region = (mem_region *) (((int) &memory) + mem_offset);
		void * ptr = ((int) cur_region) + sizeof(mem_region);
		if (ptr == pointer)
		{
			if (cur_region->status == FREED)
			{
				printf (
					"%s: [%d]\n\tError.  Unable to free pointer to [0x%x].  This pointer is points to an already-freed region of memory.\n", 
					file, 
					line,
					(int) pointer
				);
			}


			// If the current memory region is the region for the parameter pointer,
			//		then we set the status of the current memory region to FREED
			cur_region->status = FREED;

			// Now that the pointer region is freed, we check the next/prev regions to see
			//		if that also has the FREED ->status.  If either do, we need to
			//		combine the newly-freed region, and the other FREE regions

			// First, we check the previous region:
			if (prev_region != NULL && prev_region->status == FREED)
			{
				prev_region->size  += (cur_region->size + sizeof(mem_region));
				prev_region->status = FREED;

				// Now, we set cur_region pointer to the previous region... so in the
				//		case where both the previous and the next regions have FREED
				//		->status we use the now-resized previous region to combine
				//		with the next region that we are about to find:
				cur_region = prev_region;
			}
			// Finding the next region in memory:
			mem_region * next_region = (mem_region *) (
				((int) cur_region) + 
				sizeof(mem_region) +
				cur_region->size
			);
			if (((int) next_region) - ((int) &memory) < MEM_SIZE)
			{
				// If the next region is a valid location in RAM, then we check if it is a FREED region,
				//		if so, we combine the two regions
				if (next_region->status == FREED)
				{
					cur_region->size  += (next_region->size + sizeof(mem_region));
					cur_region->status = FREED;
				}
			}

			return;
		}
		if (pointer > ptr && pointer < (int) ptr + cur_region->size)
		{
			// Error, pointer was inside of an allocated region

			if (cur_region->status == FREED)
			{
				// If the region was freed, return the freed message
				printf (
					"%s: [%d]\n\tError.  Unable to free pointer to [0x%x].  This pointer is points to an already-freed region of memory.\n", 
					file, 
					line,
					(int) pointer
				);
			}
			else
			{
				// If the region was not free, then tell them it is inside of allocated space
				printf (
					"%s: [%d]\n\tError.  Unable to free pointer to [0x%x].  This pointer is points to an address that is inside of an already allocated region of memory.\n\tThe region that this pointer is in the middle of is [0x%x]\n", 
					file, 
					line,
					(int) pointer,
					(int) ptr
				);
			}

			return;
		}
		if (pointer > ((int)ptr + cur_region->size) && pointer < ((int)ptr + cur_region->size + sizeof(mem_region)))
		{
			// Error, pointer was inside of a meta data block (mem_region)

			printf (
				"%s: [%d]\n\tError.  Unable to free pointer to [0x%x].  This pointer is points to an address that is inside of used memory.\n", 
				file, 
				line,
				(int) pointer
			);

			return;
		}

		// Incrementing the offset to the next mem_region in RAM
		mem_offset += (sizeof(mem_region) + cur_region->size);
		// Setting the new previous region to the current region
		prev_region = cur_region;
	}

	// If the program was unable to return in any of the cases in the above loop, there is an
	//		error
	printf (
		"%s: [%d]\n\tError.  Unable to free pointer to [0x%x].  This pointer is points to an invalid address.\n", 
		file, 
		line,
		(int) pointer
	);

	return;

}