///////////////////////////////////////////////////////////////////////////////
// TSIM value list header
///////////////////////////////////////////////////////////////////////////////

#ifndef TSIM_VLIST_H
#define TSIM_VLIST_H

typedef pair<string,string> tsim_vlist_pair;
typedef vector<tsim_vlist_pair> tsim_vlist_vector;

class tsim_vlist
{
public:

    tsim_vlist();
    tsim_vlist( const tsim_vlist& src );

    void copy( const tsim_vlist& src );
    void clear() { list.clear(); }

    bool add( const string& name, const string& value );
    bool add( const string& name, tsim_u64 value );

    bool set( const string& name, const string& value );
    bool set( const string& name, tsim_u64 value );

    bool get( const string& name, string& value );
    bool get( const string& name, tsim_u64& value );

    inline int size() const { return list.size(); }

    tsim_vlist_vector list;
};

#endif
// TSIM_VLIST_H
