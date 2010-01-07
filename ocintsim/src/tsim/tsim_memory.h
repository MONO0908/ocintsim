///////////////////////////////////////////////////////////////////////////////
// TSIM memory header
///////////////////////////////////////////////////////////////////////////////

#ifndef TSIM_MEMORY_H
#define TSIM_MEMORY_H

#include <string>
#define TSIM_MEMORY_PAGE_MASK   0x0FFFULL
#define TSIM_MEMORY_PAGE_BYTES  (1 << 12) // 4K bytes
#define TSIM_MEMORY_PAGE_LIMIT  (1 << 20) // 1M pages

// helper classes

class tsim_memory_page
{
public:
    tsim_memory_page();
    tsim_memory_page( const tsim_memory_page& src );

    unsigned char data[TSIM_MEMORY_PAGE_BYTES];
};

typedef map<tsim_u64,tsim_memory_page> tsim_memory_map;

// main class
class tsim_memory: public tsim_memory_if
{
public:

    tsim_memory();
    ~tsim_memory();

    // data type determines number of bytes read or written
    bool read( tsim_u64 addr, unsigned char& data );
    bool read( tsim_u64 addr, unsigned short& data );
    bool read( tsim_u64 addr, tsim_u32& data );
    bool read( tsim_u64 addr, tsim_u64& data );
    bool write( tsim_u64 addr, unsigned char& data );
    bool write( tsim_u64 addr, unsigned short& data );
    bool write( tsim_u64 addr, tsim_u32& data );
    bool write( tsim_u64 addr, tsim_u64& data );

    bool put_bytes( tsim_u64 addr, unsigned char* ptr, tsim_u64 n );
    bool get_bytes( tsim_u64 addr, unsigned char* ptr, tsim_u64 n );

    void set_max_address( tsim_u64 addr );
    tsim_u64 get_max_address() const { return _max_addr; }

    void set_big_endian() { _big_endian = true; }
    void set_little_endian() { _big_endian = false; }
    bool is_big_endian() const { return _big_endian; }
    bool is_little_endian() const { return !_big_endian; }

    int get_page_count() const { return _map.size(); }
    void release_mem_pages(tsim_u64 start_addr, tsim_u64 end_addr);

#ifdef TSIM_CKPT
    inline tsim_memory_map memmap() { return _map;}
#endif
    
private:

    tsim_u64 _max_addr;
    tsim_memory_map _map;
    bool _big_endian;

    tsim_memory_page* _current_page;
    tsim_u64          _current_addr;

    inline tsim_memory_page* get_page( tsim_u64 addr )
    {
        tsim_u64 upper = addr & ~TSIM_MEMORY_PAGE_MASK;
        if (_current_addr != upper)
        {
          _current_addr = upper;
          _current_page = &(_map[upper]);
        }
        return _current_page;
    }
};

#endif
// TSIM_MEMORY_H
