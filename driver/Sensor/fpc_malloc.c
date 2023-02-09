#include "fpc_malloc.h"
#include "trace.h"

#include "os_mem.h"
#include "fpsapi.h"

#if 0
#define FPC_MALLOC_INFO0        APP_PRINT_INFO0
#define FPC_MALLOC_INFO1        APP_PRINT_INFO1
#define FPC_MALLOC_INFO2        APP_PRINT_INFO2
#define FPC_MALLOC_INFO3        APP_PRINT_INFO3
#define FPC_MALLOC_INFO4        APP_PRINT_INFO4
#else
#define FPC_MALLOC_INFO0(...)
#define FPC_MALLOC_INFO1(...)
#define FPC_MALLOC_INFO2(...)
#define FPC_MALLOC_INFO3(...)
#define FPC_MALLOC_INFO4(...)
#endif
/*
 * @brief Malloc wrapper.
 *
 * @param[in] size Size of allocation.
 *
 * @return Pointer to data or NULL if unsuccessful.
 *
 */

void *fpc_malloc(size_t size)
{
	void *addr = NULL;

	switch (size)
	{
		case 87:        //不能释放heap
		case 160:       //不能释放heap
			addr = malloc(size);           //os_mem_zalloc(RAM_TYPE_DATA_ON, size);   //addr = malloc(size);
            FPC_MALLOC_INFO2("malloc address:0x%x size:%d --- 1111", addr, size);
			break;

		default://用完释放
			addr = Ucas_malloc(size);
            FPC_MALLOC_INFO2("malloc 222 address:0x%x size:%d --- 2222", addr, size);
            break;
		
	}
    
	return addr;
}

/**
 * @brief Free wrapper.
 *
 * @param[in] data Pointer to data.
 *
 */

extern uint8_t test_umm_heap[];

void fpc_free(void *data)
{
    uint32_t address = (uint32_t)test_umm_heap;
    
    if(((unsigned int)data > address) && ((unsigned int)data < (address + 15360)))
    {
        FPC_MALLOC_INFO1("free address:0x%x --- 2222", data);
        Ucas_free(&data);
    }
    else
    {
        FPC_MALLOC_INFO1("free address:0x%x --- 1111", data);
        free(data);            //os_mem_free(data);
    }
    
}

