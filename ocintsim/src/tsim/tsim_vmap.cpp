///////////////////////////////////////////////////////////////////////////////
// TSIM value map functions
///////////////////////////////////////////////////////////////////////////////

#include "tsim.h"

tsim_vmap::tsim_vmap()
{
}

tsim_vmap::tsim_vmap( const tsim_vmap& src )
{
    copy( src );
}

void tsim_vmap::copy( const tsim_vmap& src )
{
    vmap = src.vmap;
}

bool tsim_vmap::add( const string& name, const string& value )
{
    if (name == "") return false;

    vmap[name] = value;

    return true;
}

bool tsim_vmap::add( const string& name, tsim_u64 value )
{
    return add( name, tsim_u64tos( value ) );
}

bool tsim_vmap::set( const string& name, const string& value )
{
    tsim_vmap_map::iterator i;
    i = vmap.find( name );
    if (i != vmap.end())
    {
        (*i).second = value;
        return true;
    }

    return false;
}

bool tsim_vmap::set( const string& name, tsim_u64 value )
{
    return set( name, tsim_u64tos( value ) );
}

bool tsim_vmap::get( const string& name, string& value )
{
    tsim_vmap_map::iterator i;
    i = vmap.find( name );
    if (i != vmap.end())
    {
        value = (*i).second;
        return true;
    }

    return false;
}

bool tsim_vmap::get( const string& name, tsim_u64& value )
{
    string s = "";
    bool rc = get( name, s );
    if (rc) value = tsim_stou64( s );
    return rc;
}
