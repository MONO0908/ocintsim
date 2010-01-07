///////////////////////////////////////////////////////////////////////////////
// TSIM memory interface header
///////////////////////////////////////////////////////////////////////////////

#ifndef TSIM_MEMORY_IF_H
#define TSIM_MEMORY_IF_H

// main class
class tsim_memory_if
{
public:

  virtual bool read( tsim_u64 addr, unsigned char& data )= 0;
  virtual bool read( tsim_u64 addr, unsigned short& data )= 0;
  virtual bool read( tsim_u64 addr, tsim_u32& data )= 0;
  virtual bool read( tsim_u64 addr, tsim_u64& data )= 0;
  virtual bool write( tsim_u64 addr, unsigned char& data )= 0;
  virtual bool write( tsim_u64 addr, unsigned short& data )= 0;
  virtual bool write( tsim_u64 addr, tsim_u32& data )= 0;
  virtual bool write( tsim_u64 addr, tsim_u64& data )= 0;
  virtual bool put_bytes( tsim_u64 addr, unsigned char* ptr, tsim_u64 n )= 0;
  virtual bool get_bytes( tsim_u64 addr, unsigned char* ptr, tsim_u64 n )= 0;  
  virtual void set_max_address( tsim_u64 addr )= 0;
  virtual tsim_u64 get_max_address() const= 0;
  virtual void set_big_endian() = 0;
  virtual void set_little_endian()= 0;
  virtual bool is_big_endian() const = 0;
  virtual bool is_little_endian() const= 0;
  virtual int get_page_count() const= 0;
  virtual ~tsim_memory_if() {};
  virtual void release_mem_pages(tsim_u64 start_addr, tsim_u64 end_addr) = 0;
};

#endif
// TSIM_MEMORY_IF_H
