///////////////////////////////////////////////////////////////////////////////
// TSIM value map header
///////////////////////////////////////////////////////////////////////////////

#ifndef TSIM_VMAP_H
#define TSIM_VMAP_H

typedef map<string,string> tsim_vmap_map;

class tsim_vmap
{
public:

    tsim_vmap();
    tsim_vmap( const tsim_vmap& src );

    void copy( const tsim_vmap& src );
    void clear() { vmap.clear(); }

    bool add( const string& name, const string& value );
    bool add( const string& name, tsim_u64 value );

    bool set( const string& name, const string& value );
    bool set( const string& name, tsim_u64 value );

    bool get( const string& name, string& value );
    bool get( const string& name, tsim_u64& value );

    inline int size() const { return vmap.size(); }

    tsim_vmap_map vmap;
};

#endif
// TSIM_VLIST_H
