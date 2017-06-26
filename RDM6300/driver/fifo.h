// FIFO.h
// Runs on any LM3Sxxx
// Provide functions that initialize a FIFO, put data in, get data out,
// and return the current size.  The file includes a transmit FIFO
// using index implementation and a receive FIFO using pointer
// implementation.  Other index or pointer implementation FIFOs can be
// created using the macros supplied at the end of the file.
// Daniel Valvano
// June 16, 2011

#ifndef _FIFO_H_
#define _FIFO_H_

// macro to create an index FIFO
#define AddIndexFifo(NAME, SIZE, TYPE, SUCCESS, FAIL)													\
uint32_t volatile NAME##PutI;																		\
uint32_t volatile NAME##GetI;																		\
TYPE static NAME##Fifo [SIZE];																		\
void NAME##Fifo_Init(void)																			\
{																									\
	NAME##PutI = NAME##GetI = 0;																	\
}																									\
int NAME##Fifo_Put (TYPE data)																		\
{																									\
	if(( NAME##PutI - NAME##GetI ) & ~(SIZE-1)) return(FAIL);										\
	NAME##Fifo[ NAME##PutI &(SIZE-1)] = data;														\
	NAME##PutI ## ++;																				\
	return (SUCCESS);																				\
}																									\
int NAME##Fifo_Get (TYPE *datapt)																	\
{																									\
	if( NAME##PutI == NAME##GetI )	return(FAIL);													\
	*datapt = NAME##Fifo[ NAME##GetI &(SIZE-1)];													\
	NAME##GetI ## ++;																				\
	return (SUCCESS);																				\
}																									\
unsigned short NAME##Fifo_Size (void)																\
{																									\
	return ((unsigned short)( NAME##PutI - NAME##GetI ));											\
}
// e.g.,
// AddIndexFifo(Tx,32,unsigned char, 1,0)
// SIZE must be a power of two
// creates TxFifo_Init() TxFifo_Get() and TxFifo_Put()

// macro to create a pointer FIFO
#define AddPointerFifo(NAME, SIZE, TYPE, SUCCESS, FAIL)												\
TYPE volatile *NAME##PutPt;																			\
TYPE volatile *NAME##GetPt;																			\
TYPE static NAME##Fifo [SIZE];																		\
void NAME##Fifo_Init(void)																			\
{																									\
	NAME##PutPt = NAME##GetPt = &NAME##Fifo[0];														\
}																									\
int NAME##Fifo_Put (TYPE data)																		\
{																									\
	TYPE volatile *nextPutPt;																		\
	nextPutPt = NAME##PutPt + 1;																	\
	if(nextPutPt == &NAME##Fifo[SIZE])																\
	{																								\
		nextPutPt = &NAME##Fifo[0];																	\
	}																								\
	if(nextPutPt == NAME##GetPt )																	\
	{																								\
		return(FAIL);																				\
	}																								\
	else																							\
	{																								\
		*( NAME##PutPt ) = data;																	\
		NAME##PutPt = nextPutPt;																	\
		return(SUCCESS);																			\
	}																								\
}																									\
int NAME##Fifo_Get (TYPE *datapt)																	\
{																									\
	if( NAME##PutPt == NAME##GetPt )																\
	{																								\
		return(FAIL);																				\
	}																								\
	*datapt = *( NAME##GetPt ## ++);																\
	if( NAME##GetPt == &NAME##Fifo[SIZE])															\
	{																								\
		NAME##GetPt = &NAME##Fifo[0];																\
	}																								\
	return(SUCCESS);																				\
}																									\
unsigned short NAME##Fifo_Size (void)																\
{																									\
	if( NAME##PutPt < NAME##GetPt )																	\
	{																								\
		return ((unsigned short)( NAME##PutPt - NAME##GetPt + (SIZE*sizeof(TYPE)))/sizeof(TYPE));	\
	}																								\
	return ((unsigned short)( NAME##PutPt - NAME##GetPt )/sizeof(TYPE));							\
}
// e.g.,
// AddPointerFifo(Rx,32,unsigned char, 1,0)
// SIZE can be any size
// creates RxFifo_Init() RxFifo_Get() and RxFifo_Put()
#endif /* _FIFO_H_ */
