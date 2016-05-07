/*
 *Describe:
 *	Moudle test for mm_malloc(mm_free).
 */

#include <stdio.h>
#include <stdlib.h>
#include "mm.h"
#define SIZE 10
int main(){
	
	void* testp1;
	void* testp2;
	void* testp3;

	int res=mm_init();
	if(res==-1){
		fprintf(stderr,"Error: mm_init failed..\n");
		exit(EXIT_FAILURE);
	}
	testp1=mm_malloc(sizeof(char)*SIZE);
	if(testp1==NULL){
		fprintf(stderr,"Error: mm_malloc failed..\n");
		exit(EXIT_FAILURE);
	}/*
	for(int i=0;i<SIZE;i++)
		testp[i]='a'+rand()%26;
	for(int i=0;i<SIZE;i++)
		printf("%c",testp[i]);
		
	printf("\n");
	*/
	testp2=mm_malloc(4096);
	mm_free(testp1);
	
	testp3=mm_malloc(SIZE);

	mm_free(testp3);
	mm_free(testp2);
	exit(EXIT_SUCCESS);
}
