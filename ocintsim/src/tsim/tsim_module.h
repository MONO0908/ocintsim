///////////////////////////////////////////////////////////////////////////////
// TSIM module header
///////////////////////////////////////////////////////////////////////////////

#ifndef TSIM_MODULE_H
#define TSIM_MODULE_H

// classes
class tsim_module : public tsim_object
{
public:

    tsim_u64 _clock_period;
    tsim_module();
    tsim_module(tsim_u64 clock_period);
    virtual ~tsim_module();

    virtual void init( tsim_object* parent, const string& name );
    virtual void init( tsim_object* parent, const string& name, tsim_u64 clock_period );

    virtual void set_clock( tsim_u64 clock_period);
    virtual void update();
    virtual void evaluate();
    virtual void update_power_counters();
    virtual void update_clock_period(tsim_u64 clock_period);
    virtual void finalize() {};
};

#endif
// TSIM_MODULE_H
