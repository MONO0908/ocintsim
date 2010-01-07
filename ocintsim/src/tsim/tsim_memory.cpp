///////////////////////////////////////////////////////////////////////////////
// TSIM memory functions
///////////////////////////////////////////////////////////////////////////////

#include "tsim.h"

// tsim_memory_page functions

tsim_memory_page::tsim_memory_page()
{
    memset( data, 0, TSIM_MEMORY_PAGE_BYTES );
}

tsim_memory_page::tsim_memory_page( const tsim_memory_page& src )
{
    memcpy( data, src.data, TSIM_MEMORY_PAGE_BYTES );
}

// tsim_memory functions

tsim_memory::tsim_memory()
{
    _max_addr = 0xffffffffffffffffULL;
    _big_endian = true;
    _current_addr = 0;
    _current_page = &(_map[0]);
}

tsim_memory::~tsim_memory()
{
}

void tsim_memory::set_max_address( tsim_u64 addr )
{
    _max_addr = addr;
}

bool tsim_memory::read( tsim_u64 addr, unsigned char& data )
{
    if (addr > _max_addr) return false;

    tsim_memory_page* page = get_page( addr );

    tsim_u32 x = addr & TSIM_MEMORY_PAGE_MASK;
    data = page->data[x];
    return true;
}

bool tsim_memory::read( tsim_u64 addr, unsigned short& data )
{
    if (addr > _max_addr) return false;
    if ((addr & 0x1) != 0) return false;

    tsim_memory_page* page = get_page( addr );

    tsim_u32 x = addr & TSIM_MEMORY_PAGE_MASK;
    if (_big_endian)
    {
        data = (page->data[x+0] <<  8) |
               (page->data[x+1] <<  0) ;
    }
    else
    {
        data = (page->data[x+1] <<  8) |
               (page->data[x+0] <<  0) ;
    }
    return true;
}

bool tsim_memory::read( tsim_u64 addr, tsim_u32& data )
{
    if (addr > _max_addr) return false;
    if ((addr & 0x3) != 0) return false;

    tsim_memory_page* page = get_page( addr );

    tsim_u32 x = addr & TSIM_MEMORY_PAGE_MASK;
    if (_big_endian)
    {
        data = (page->data[x+0] << 24) |
               (page->data[x+1] << 16) |
               (page->data[x+2] <<  8) |
               (page->data[x+3] <<  0) ;
    }
    else
    {
        data = (page->data[x+3] << 24) |
               (page->data[x+2] << 16) |
               (page->data[x+1] <<  8) |
               (page->data[x+0] <<  0) ;
    }
    return true;
}

bool tsim_memory::read( tsim_u64 addr, tsim_u64& data )
{
    if (addr > _max_addr) return false;
    if ((addr & 0x7) != 0) return false;

    tsim_memory_page* page = get_page( addr );

    tsim_u32 x = addr & TSIM_MEMORY_PAGE_MASK;
    if (_big_endian)
    {
        data = ((tsim_u64) page->data[x+0] << 56) |
               ((tsim_u64) page->data[x+1] << 48) |
               ((tsim_u64) page->data[x+2] << 40) |
               ((tsim_u64) page->data[x+3] << 32) |
               ((tsim_u64) page->data[x+4] << 24) |
               ((tsim_u64) page->data[x+5] << 16) |
               ((tsim_u64) page->data[x+6] <<  8) |
               ((tsim_u64) page->data[x+7] <<  0) ;
    }
    else
    {
        data = ((tsim_u64) page->data[x+7] << 56) |
               ((tsim_u64) page->data[x+6] << 48) |
               ((tsim_u64) page->data[x+5] << 40) |
               ((tsim_u64) page->data[x+4] << 32) |
               ((tsim_u64) page->data[x+3] << 24) |
               ((tsim_u64) page->data[x+2] << 16) |
               ((tsim_u64) page->data[x+1] <<  8) |
               ((tsim_u64) page->data[x+0] <<  0) ;
    }
    return true;
}

bool tsim_memory::write( tsim_u64 addr, unsigned char& data )
{
    if (addr > _max_addr) return false;

    tsim_memory_page* page = get_page( addr );

    tsim_u32 x = addr & TSIM_MEMORY_PAGE_MASK;
    page->data[x] = data;
    return true;
}

bool tsim_memory::write( tsim_u64 addr, unsigned short& data )
{
    if (addr > _max_addr) return false;
    if ((addr & 0x1) != 0) return false;

    tsim_memory_page* page = get_page( addr );

    tsim_u32 x = addr & TSIM_MEMORY_PAGE_MASK;
    if (_big_endian)
    {
        page->data[x+0] = (unsigned char) (data >> 8);
        page->data[x+1] = (unsigned char) (data >> 0);
    }
    else
    {
        page->data[x+1] = (unsigned char) (data >> 8);
        page->data[x+0] = (unsigned char) (data >> 0);
    }
    return true;
}

bool tsim_memory::write( tsim_u64 addr, tsim_u32& data )
{
    if (addr > _max_addr) return false;
    if ((addr & 0x3) != 0) return false;

    tsim_memory_page* page = get_page( addr );

    tsim_u32 x = addr & TSIM_MEMORY_PAGE_MASK;
    if (_big_endian)
    {
        page->data[x+0] = (unsigned char) (data >> 24);
        page->data[x+1] = (unsigned char) (data >> 16);
        page->data[x+2] = (unsigned char) (data >>  8);
        page->data[x+3] = (unsigned char) (data >>  0);
    }
    else
    {
        page->data[x+3] = (unsigned char) (data >> 24);
        page->data[x+2] = (unsigned char) (data >> 16);
        page->data[x+1] = (unsigned char) (data >>  8);
        page->data[x+0] = (unsigned char) (data >>  0);
    }
    return true;
}

bool tsim_memory::write( tsim_u64 addr, tsim_u64& data )
{
    if (addr > _max_addr) return false;
    if ((addr & 0x7) != 0) return false;

    tsim_memory_page* page = get_page( addr );

    tsim_u32 x = addr & TSIM_MEMORY_PAGE_MASK;
    if (_big_endian)
    {
        page->data[x+0] = (unsigned char) (data >> 56);
        page->data[x+1] = (unsigned char) (data >> 48);
        page->data[x+2] = (unsigned char) (data >> 40);
        page->data[x+3] = (unsigned char) (data >> 32);
        page->data[x+4] = (unsigned char) (data >> 24);
        page->data[x+5] = (unsigned char) (data >> 16);
        page->data[x+6] = (unsigned char) (data >>  8);
        page->data[x+7] = (unsigned char) (data >>  0);
    }
    else
    {
        page->data[x+7] = (unsigned char) (data >> 56);
        page->data[x+6] = (unsigned char) (data >> 48);
        page->data[x+5] = (unsigned char) (data >> 40);
        page->data[x+4] = (unsigned char) (data >> 32);
        page->data[x+3] = (unsigned char) (data >> 24);
        page->data[x+2] = (unsigned char) (data >> 16);
        page->data[x+1] = (unsigned char) (data >>  8);
        page->data[x+0] = (unsigned char) (data >>  0);
    }
    return true;
}

bool tsim_memory::put_bytes( tsim_u64 addr, unsigned char* ptr, tsim_u64 n )
{
    if (addr > _max_addr) return false;
    if ((addr + n) > _max_addr) return false;
    if ((addr + n) < addr) return false;  // wrapped
    if (!ptr) return false;

    // need to do efficient mem copies... (but just cop out for now)
    for (tsim_u64 i=0; i<n; i++)
    {
        if (!write( addr + i, ptr[i] )) return false;
    }

    return true;
}

bool tsim_memory::get_bytes( tsim_u64 addr, unsigned char* ptr, tsim_u64 n )
{
    if (addr > _max_addr) return false;
    if ((addr + n) > _max_addr) return false;
    if ((addr + n) < addr) return false;  // wrapped
    if (!ptr) return false;

    // need to do efficient mem copies... (but just cop out for now)
    for (tsim_u64 i=0; i<n; i++)
    {
        if (!read( addr + i, ptr[i] )) return false;
    }

    return true;
}

//Release memory pages in the range specified by the start_ and
//end_addr. 
void tsim_memory::release_mem_pages(tsim_u64 start_addr, tsim_u64 end_addr)
{
  tsim_memory_map::iterator iter = _map.begin();
  tsim_memory_map::iterator erase_iter = iter;

  while (iter != _map.end())
    {
      erase_iter = iter;
      iter++;
      
      tsim_u64 page_addr = (*erase_iter).first;
      
      if (page_addr >= start_addr && page_addr <= end_addr)
	{
	  _map.erase(erase_iter);
	}
    }
}
