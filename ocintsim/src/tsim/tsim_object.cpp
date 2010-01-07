///////////////////////////////////////////////////////////////////////////////
// TSIM object functions
///////////////////////////////////////////////////////////////////////////////

#include "tsim.h"

tsim_object::tsim_object( int otype )
{
    _initialized = 0;
    _otype       = otype;
    _parent      = NULL;
    _sim         = NULL;
    _name        = "";
    _fullname    = "";
}

tsim_object::~tsim_object()
{
}

void tsim_object::init( tsim_object* parent, const string& name )
{
    // copy the name
    _name = name;
    _fullname = name;

    // formulate a fullname
    _parent = parent;
    if (_parent)
    {
        // save ptr to simulator
        _sim = _parent->_sim;
        // generate fullname
        if (_parent->_name != "")
        {
            _fullname = _parent->_fullname + "." + _name;
        }
    }
    else
    {
        if (_otype != TSIM_OTYPE_SIMULATOR)
        {
            // all objects that are simulators must have parents!
            assert( _parent != NULL );
        }
    }

    // mark as initialized
    _initialized = 1;
}

void tsim_object::print( const string& msg )
{
    if (_sim)
    {
        _sim->print( *this, msg );
    }
}

void tsim_object::debug( const string& msg )
{
    if (_sim)
    {
        _sim->debug( *this, msg );
    }
}

void tsim_object::warn( const string& msg )
{
    if (_sim)
    {
        _sim->warn( *this, msg );
    }
}

void tsim_object::fatal( const string& msg )
{
    if (_sim)
    {
        _sim->fatal( *this, msg );
    }
}

bool tsim_object::report_statistic( const string& name, const string& value )
{
    if (_sim) return _sim->report_statistic( name, value );
    else return false;
}

bool tsim_object::report_statistic( const string& name, tsim_u64 value )
{
    if (_sim) return _sim->report_statistic( name, value );
    else return false;
}

bool tsim_object::report_statistic_double( const string& name, double value )
{
    if (_sim) return _sim->report_statistic_double( name, value );
    else return false;
}

bool tsim_object::define_parameter( const string& name, string& value )
{
    if (_sim) return _sim->define_parameter( name, value );
    else return false;
}

bool tsim_object::define_parameter( const string& name, tsim_u64& value )
{
    if (_sim) return _sim->define_parameter( name, value );
    else return false;
}

bool tsim_object::get_parameter( const string& name, string& value )
{
    if (_sim) return _sim->get_parameter( name, value );
    else return false;
}

bool tsim_object::get_parameter( const string& name, tsim_u64& value )
{
    if (_sim) return _sim->get_parameter( name, value );
    else return false;
}

string tsim_object::otype_string()
{
    switch (_otype)
    {
    case TSIM_OTYPE_SIMULATOR:
        return string( "simulator" );
    case TSIM_OTYPE_MODULE:
        return string( "module" );
    case TSIM_OTYPE_OBJECT:
    default:
        return string( "object" );
    }
}
