#include "array.h"

int main(){

	Array array = array_new(2);

for (int i = 0; i < 10000000; ++i)
{
	/* code */

	array_insertBack(&array,6);
	array_insertBack(&array,2);
	array_insertBack(&array,1);
	array_insertBack(&array,4);
	array_insertBack(&array,2);
	array_insertBack(&array,2);
}

	
	return 0;
}