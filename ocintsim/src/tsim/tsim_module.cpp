///////////////////////////////////////////////////////////////////////////////
// TSIM module functions
///////////////////////////////////////////////////////////////////////////////

#include "tsim.h"

tsim_module::tsim_module() :
    tsim_object( TSIM_OTYPE_MODULE )
{
   _clock_period = 1;
}

tsim_module::tsim_module(tsim_u64 clock_period) :
    tsim_object( TSIM_OTYPE_MODULE )
{
   _clock_period = clock_period;
}

tsim_module::~tsim_module()
{
}

void tsim_module::init( tsim_object* parent, const string& name )
{
    tsim_object::init( parent, name );

    // register with simulator
    tsim_simulator* s = sim();
    assert( s != NULL );
    if (s) s->add_module( *this );
}

void tsim_module::init( tsim_object* parent, const string& name, tsim_u64 clock_period )
{
    tsim_object::init( parent, name );
    _clock_period = clock_period;

    // register with simulator
    tsim_simulator* s = sim();
    assert( s != NULL );
    if (s) s->add_module( *this );
}

void tsim_module::set_clock( tsim_u64 clock_period )
{
    _clock_period = clock_period;
}

void tsim_module::update()
{
    // tell the simulator not to call me anymore
    tsim_simulator* s = sim();
    assert( s != NULL );
    if (s) s->disable_module_update( *this );
}

void tsim_module::update_power_counters()
{
    // Don't do anything here
    tsim_simulator* s = sim();
    assert( s != NULL );
}

void tsim_module::update_clock_period(tsim_u64 clock_period)
{
    _clock_period = clock_period;
}



void tsim_module::evaluate()
{
    // tell the simulator not to call me anymore
    tsim_simulator* s = sim();
    assert( s != NULL );
    if (s) s->disable_module_evaluate( *this );
}
