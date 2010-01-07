///////////////////////////////////////////////////////////////////////////////
// TSIM value list functions
///////////////////////////////////////////////////////////////////////////////

#include "tsim.h"

tsim_vlist::tsim_vlist()
{
}

tsim_vlist::tsim_vlist( const tsim_vlist& src )
{
    copy( src );
}

void tsim_vlist::copy( const tsim_vlist& src )
{
    list = src.list;
}

bool tsim_vlist::add( const string& name, const string& value )
{
    if (name == "") return false;

    if (!set( name, value ))
    {
      list.push_back( tsim_vlist_pair( name, value ) );
    }

    return true;
}

bool tsim_vlist::add( const string& name, tsim_u64 value )
{
    return add( name, tsim_u64tos( value ) );
}

bool tsim_vlist::set( const string& name, const string& value )
{
    for (int i=list.size()-1; i>=0; i--)
    {
        if (list[i].first == name)
        {
            list[i].second = value;
            return true;
        }
    }
    return false;
}

bool tsim_vlist::set( const string& name, tsim_u64 value )
{
    return set( name, tsim_u64tos( value ) );
}

bool tsim_vlist::get( const string& name, string& value )
{
    for (int i=list.size()-1; i>=0; i--)
    {
        if (list[i].first == name)
        {
            value = list[i].second;
            return true;
        }
    }
    return false;
}

bool tsim_vlist::get( const string& name, tsim_u64& value )
{
    string s = "";
    bool rc = get( name, s );
    if (rc) value = tsim_stou64( s );
    return rc;
}
