#include <stdlib.h>
#include <stdio.h>

#define bool  int
#define true  1
#define false 0


char memory[4096];

typedef struct _ {
	struct node {
		struct _ {
			void * pointer;
			int size;
		} data;
		struct node * next;
	} * front;
} ll_alloc;


int main ()
{

}




bool add_alloc (ll_alloc list, struct node * add_to, void * pointer, int size)
{	
	struct node * new_node = (struct node *) malloc(sizeof(struct node *));
	if (!new_node)
	{
		return false;
	}

	new_node->data.pointer = pointer;
	new_node->data.size    = size;

	if (!add_to)
	{
		new_node->next = list.front;
		list.front = new_node;
	}
	else
	{
		new_node->next = add_to->next;
		add_to->next = new_node;
	}
	
	return true;
}

bool remove_address (ll_alloc list, void * pointer)
{
	struct node * prev = NULL;
	for (struct node * ptr = list.front; ptr; prev = ptr, ptr = ptr->next)
	{
		if (ptr->data.pointer == pointer)
		{
			if (!prev)
			{
				list.front = ptr->next;
			}
			else
			{
				prev->next = ptr->next;
			}
			free(ptr);
			return true;
		}
	}
	return false;
}