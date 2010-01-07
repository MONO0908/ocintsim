///////////////////////////////////////////////////////////////////////////////
// TSIM object header
///////////////////////////////////////////////////////////////////////////////

#ifndef TSIM_OBJECT_H
#define TSIM_OBJECT_H

// defines
#define TSIM_OTYPE_OBJECT       0
#define TSIM_OTYPE_SIMULATOR    1
#define TSIM_OTYPE_MODULE       2

// classes
class tsim_object
{
    friend class tsim_simulator;

public:

    tsim_object( int otype = TSIM_OTYPE_OBJECT );
    virtual ~tsim_object();

    void init( tsim_object* parent, const string& name );
    inline const string& name() const { return _name; }
    inline const string& fullname() const { return _fullname; }
    inline int initialized() const { return _initialized; }
    tsim_object* parent() { return _parent; }
    string otype_string();

protected:

    tsim_simulator* sim() { return _sim; }
    void print( const string& msg );
    void debug( const string& msg );
    void warn( const string& msg );
    void fatal( const string& msg );

    bool report_statistic( const string& name, const string& value );
    bool report_statistic( const string& name, tsim_u64 value );
    bool report_statistic_double( const string& name, double value );
    bool define_parameter( const string& name, string& value );
    bool define_parameter( const string& name, tsim_u64& value );
    bool get_parameter( const string& name, string& value );
    bool get_parameter( const string& name, tsim_u64& value );

    // added by sibi
    const string & get_name() { return _fullname;}

private:

    tsim_object* _parent;
    tsim_simulator* _sim;
    string _name;
    string _fullname;
    int _otype;
    int _initialized;
};

#endif
// TSIM_OBJECT_H
