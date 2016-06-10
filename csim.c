/*Isaiah Genis IG596@NYU.EDU */
#include "cachelab.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define _GNU_SOURCE
#include <unistd.h>

#define MAX_WORD_SIZE 32
#define MAX_FILE_SIZE 256
typedef unsigned long long uint64_t;

unsigned long g_hits = 0;
unsigned long g_misses = 0;
unsigned long g_evicts = 0;
 extern char *optarg;
extern void printSummary(int hits, int misses, int evictions);
void usage()
{
	printf("./csim [-hv] -s <s> -E <E> -b <b> -t <tracefile>");
}
void parseCmdArguements(int argc,char *argv[] , int *setIndexBits,
		int *numberofLinesPerSet,int *numberOfBlockBits,char traceFileName[MAX_FILE_SIZE], int *verbose)
{
	 char ch;
	 while (-1 != (ch = getopt(argc, argv, "vh:s:E:t:b:")))
	 {
          switch(ch) {

                        case 's':
				*setIndexBits = atoi(optarg);
				if(*setIndexBits < 0 || *setIndexBits > MAX_WORD_SIZE)
				{
					fprintf(stderr,"Number of the set index bits should be between 0 to %d",  MAX_WORD_SIZE);
					exit(-1);
				}
                                break;
                        case 'E':
				*numberofLinesPerSet = atoi(optarg);
				if(*numberofLinesPerSet <= 0)
				{
					fprintf(stderr,"Number of lines per set should be less than or equal to zero");
					exit(-1);
				}
				break;
                        case 'b':
				*numberOfBlockBits = atoi(optarg);
				if(*numberOfBlockBits < 0 || *numberOfBlockBits > MAX_WORD_SIZE)
				{
					fprintf(stderr,"Number of the block size bits should be between 0 to %d", MAX_WORD_SIZE);
					exit(-1);
				}
                                break;
                        case 't':
                                strncpy(traceFileName,optarg,MAX_FILE_SIZE-1);
                                break;
                        case 'v':
                                *verbose = 1 ;
                                break;
			case 'h':
				usage();
				exit(0);
			default:
				printf("Invalid option");
				exit(-1);

	}
   }
}

unsigned long long getTwoPower(int numberofBits)
{
	return (1 << numberofBits);
}
typedef struct _Line
{
	int valid;
	uint64_t tag;
	int last;
}Line;
typedef struct _Set
{
	Line *addrLines;
	int count;
}Set;
void constructCache(Set **sets, int setSize, int numOfLines)
{
	*sets = calloc(setSize,sizeof(Set));
	if(sets == NULL)
	{
		fprintf(stderr,"Unable to allocate memory");
		exit(-1);
	}
	int index = 0;
	for(index = 0; index < setSize; index++)
	{
		(*sets)[index].addrLines = calloc(numOfLines,sizeof(Line));
		if((*sets)[index].addrLines == NULL)
		{
			fprintf(stderr,"Unable to allocate memory");
			exit(-1);
		}
	}
}
void deleteCache(Set *sets, int setSize)
{
	int count = 0;
	for(count = 0; count < setSize; count++)
	{
			free(sets[count].addrLines);
	}
	free(sets);	
}

void searchCache(char c, Set *sets, uint64_t memAddr, uint64_t setMask, uint64_t tagMask, int numOfBlockBits, int numberofLinesPerSet,int verbose,char *addr)
{
	uint64_t setId, tagId;
	setId = (memAddr & setMask) >> numOfBlockBits;
	tagId = memAddr & tagMask;
	int lurLine = 0;
	int lurLast = sets[setId].addrLines->last;
	Line *line;
	int index = 0;
	for( index = 0; index < numberofLinesPerSet ; index++)
	{
		line = sets[setId].addrLines + index;
		
		if(line->valid == 0)
		{
			g_misses++;
			line->tag = tagId;
			line->valid = 1;
			sets[setId].count++;
			line->last = sets[setId].count;
			if(verbose)
			{
				printf("%c %s, %d miss \n", c, addr,numberofLinesPerSet );
			}
			return;
		}
		else if(line->tag == tagId)
		{
			g_hits++;
			sets[setId].count++;
			line->last = sets[setId].count;
			
			if(verbose)
			{
				
				printf("%c %s, %d miss \n", c, addr,numberofLinesPerSet );
			}
			return;
		}
		else  if(line->last < lurLast)
		{
			lurLast = line->last;
			lurLine = index;
		}
	}
	
	if(verbose)
	{
		
				printf("%c %s, %d miss \n", c, addr,numberofLinesPerSet );
	}
	++g_evicts;
        ++g_misses;
        line = sets[setId].addrLines + lurLine;
        line->tag = tagId;
        sets[setId].count++;
        line->last = sets[setId].count;
        return;
	
}
void populateCache(char *traceFileName, Set *sets,  uint64_t setMask, uint64_t tagMask, int numOfBlockBits, int numberofLinesPerSet,int verbose)
{
	FILE *file = fopen(traceFileName,"r");
	if(file == NULL)
	{
		fprintf(stderr,"Unable to open the file %s", traceFileName);
		exit(-1);
	}
	char instructionLine[256];
	while(fgets(instructionLine, 255, file))
	{
		if(strcmp(instructionLine,"\n") == 0)
			continue;
		if(*instructionLine == 'I')
			continue;
		char c = *(instructionLine+1);
		
		char addr[256];
		strcpy(addr,instructionLine+3);
		addr[strlen(addr)-2] = '\0';
		char *pos = strchr(instructionLine,',');
		if(pos == NULL)
			continue;
		*pos = '\0';
		*(instructionLine + 1) = '0';
		*(instructionLine + 2)  = 'x';
		uint64_t memAddr = strtoull(instructionLine + 1, NULL, 0);
			//printf("addr=%s\n,%ld",addr,memAddr);
		//printf("instruction = %c " , c);
		if( c == 'M')
		{
			searchCache(c,sets,  memAddr,  setMask,  tagMask,  numOfBlockBits,  numberofLinesPerSet,verbose,addr);
			
			searchCache(c,sets,  memAddr,  setMask,  tagMask,  numOfBlockBits,  numberofLinesPerSet,verbose,addr);
		}
		else
		{
			
			searchCache(c,sets,  memAddr,  setMask,  tagMask,  numOfBlockBits,  numberofLinesPerSet,verbose,addr);
		}
	}
}
int main(int argc, char *argv[] )
{
        int setIndexBits, numberofLinesPerSet,numberOfBlockBits,verbose;
        char traceFileName[MAX_FILE_SIZE];
        verbose = 0;
	parseCmdArguements(argc, argv, &setIndexBits,
                &numberofLinesPerSet,&numberOfBlockBits, traceFileName, &verbose);
   
	int tagSize = MAX_WORD_SIZE - setIndexBits - numberOfBlockBits;
        if( 0 >= tagSize  ){
                printf("tagSize should larger than or equal to 0\n");
                exit(-1);

        }
	uint64_t setMask = (1L << (setIndexBits +numberOfBlockBits)) - ( 1L << numberOfBlockBits) ;	
	uint64_t tagMask;
	tagMask = (1L << 31) >> (tagSize -1);
	Set *set = NULL;
	uint64_t numOfSets = getTwoPower(setIndexBits);
	uint64_t numOfLines = getTwoPower(numberofLinesPerSet);
	constructCache(&set,numOfSets,numOfLines);
	populateCache(traceFileName, set, setMask, tagMask,  numberOfBlockBits,  numberofLinesPerSet,verbose);
 
	printSummary((unsigned int)g_hits, (unsigned int)g_misses,(unsigned int) g_evicts);
	deleteCache(set,numOfSets);
    return 0;
}
