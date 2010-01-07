///////////////////////////////////////////////////////////////////////////////
// TSIM simulator functions
///////////////////////////////////////////////////////////////////////////////

#include "tsim.h"
#include "signal.h"


// global variables

bool tsim_enable_debugging   = false;
bool tsim_enable_log_file    = false;
bool tsim_enable_log_stdout  = true;
bool tsim_enable_log_stderr  = false;
bool tsim_enable_parms_file  = false;
bool tsim_enable_stats_file  = false;
bool tsim_enable_events_file = false;

tsim_u64 tsim_debug_start = 0;
tsim_u64 tsim_debug_stop  = 0xffffffffffffffffULL;
tsim_u64 tsim_events_start = 0;
tsim_u64 tsim_events_stop  = 0xffffffffffffffffULL;
bool tsim_debug_flag = false;
bool tsim_events_flag = false;

// ramdas: reporting individual opn hops for critical path analysis
bool tsim_opn_events_flag = false; 

// sibi: is power modeling needed
tsim_u64 enable_power_model = 0;

// sibi: the technology node being used
// TODO: validate the values for this parameter
tsim_u64 tech_node;

// sibi: the interconnect capacitance per unit length 
//       should depend on the tech node 
// TODO: remove this later
tsim_u64 interconn_cap_per_length;

// sibi: leakage current per unit area
//       this value is for normal logic
//       scale this for SRAM or dense structures
tsim_u64 leakage_current_per_unit_area;

// sibi: should I use Cacti to calculate the
//       optimal values for various caches
tsim_u64 enable_cacti_usage;

// sibi: how often should the power counters be updated
//       set this value to 1 to measure peak power accurately,
//       at the cost of simulation speed
tsim_u64 power_ctr_interval;

// sibi: what is the target clock frequency?
tsim_u64 processor_frequency;

// sibi: what is the voltage of the design?
// TODO: Add support for multiple voltage regions
double voltage;

// sibi: when set to 1, it means that
//       default values for voltage, frequency, etc will be
//       used based on the technology node.
tsim_u64 use_tech_defaults;

bool tsim_interrupted = false;
string tsim_flavor = "tsim";

void tsim_sigint(int ignore)
{
    // after the 1st time, fall back to default handler
    if (tsim_interrupted)
    {
        signal( SIGINT, SIG_DFL );
        raise( SIGINT );
        return;
    }

    tsim_interrupted = true;
}

// tsim_simulator functions

tsim_simulator::tsim_simulator( const string& simout ) :
    tsim_object( TSIM_OTYPE_SIMULATOR )
{
    _sim = this;
    _stopped = true;
    _simout = simout;

    // compute file names
    _logfn = _simout + ".log";
    _parmsfn = _simout + ".parms";
    _statsfn = _simout + ".stats";

    // open log file
    if (tsim_enable_log_file)
    {
        _logfs.open( _logfn.c_str() );
    }

    // init debug flag
    tsim_debug_flag = tsim_enable_debugging;
    tsim_events_flag = tsim_enable_events_file;

    _exit_value = (tsim_u64) -1;

    _max_cycles = 100000;
    _max_blocks = (tsim_u64) -1;
    
    _modules = 0;
    _fatals = 0;
    _warnings = 0;
    _insts = 0;
    _blocks = 0;
    _cycles = 0;
    _evaluates = 0;
    _updates = 0;
    // added by sibi
    _cycles_power = 0;
    _power_updates = 0;

    _mod_list.clear();
    _mod_list.reserve( 100 );
    _mod_update_list.clear();
    _mod_update_list.reserve( 100 );
    _mod_evaluate_list.clear();
    _mod_evaluate_list.reserve( 100 );

    _pruned_list = false;
}

tsim_simulator::~tsim_simulator()
{
    if (_logfs.is_open()) _logfs.close();
}

void tsim_simulator::config( const string& filename )
{
    ifstream ifs;
    string name, value;

    print( "Loading configuration file " + filename );
    ifs.open( filename.c_str() );
    if (ifs.fail())
    {
        fatal( "Could not open configuration file!" );
        return;
    }
    while ( !ifs.eof() )
    {
        // any line beginning with # is assumed to be a comment line
        char c;
        ifs >> ws;
        ifs.get( c );
        if (c != '#')
        {
            // okay, this looks good
            ifs.putback( c );
            ifs >> name >> value;
            if (!ifs.fail())
            {
                config( name, value );
            }
        }
        // always ignore rest of line
        ifs.ignore( 1024, '\n' );
    }
    // close the file
    ifs.close();
}

void tsim_simulator::config( const string& name, const string& value )
{
    if (!_config_list.size()) print( "Configuring simulator" );
    TSIM_DEBUG( "configuring with " + name + " = " + value );
    _config_list.add( name, value );
}

void tsim_simulator::config( const string& name, tsim_u64 value )
{
    config( name, tsim_u64tos( value ) );
}

void tsim_simulator::config( const tsim_vmap& vmap )
{
    tsim_vmap_map::const_iterator it;
    for (it = vmap.vmap.begin(); it != vmap.vmap.end(); it++)
    {
        config( (*it).first, (*it).second );
    }
}

void tsim_simulator::init()
{
    tsim_object::init( NULL, "" );

    print( "Initializing simulator" );

    // warn about debug mode
    if (tsim_enable_debugging)
    {
        if (!TSIM_DEBUGGING)
        {
            warn( "Debug output is disabled in this version of the simulator" );
        }
    }

    // show some additional info
    TSIM_DEBUG( "tsim_debug_start = " + tsim_u64tos(tsim_debug_start) );
    TSIM_DEBUG( "tsim_debug_stop  = " + tsim_u64tos(tsim_debug_stop) );

    if (tsim_events_flag)
    {
        TSIM_DEBUG( "tsim_events_start = " + tsim_u64tos(tsim_events_start) );
        TSIM_DEBUG( "tsim_events_stop  = " + tsim_u64tos(tsim_events_stop) );
    }

    // initialize internal parameters
    define_parameter( "sim.max_cycles", _max_cycles );
    define_parameter( "sim.max_blocks", _max_blocks );

    // sibi: define power-related parameters
    define_parameter( "power_model_needed",  enable_power_model);
    define_parameter("tech",  tech_node);
    define_parameter("interconn_cap_per_length",  interconn_cap_per_length);
    define_parameter("leak_curr_per_area",  leakage_current_per_unit_area);
    define_parameter("cacti_enable",  enable_cacti_usage);
    define_parameter("power_counter_interval",  power_ctr_interval);
    define_parameter("freq", processor_frequency);
    //define_parameter("voltage", voltage);
    define_parameter("use_tech_defaults", use_tech_defaults);
}

tsim_u64 tsim_simulator::run( tsim_u64 n )
{
    // make sure things are ok
    if (!initialized()) return 0;
    if (_fatals > 0) return 0;

    if (n == 0) print( "Running simulator" );

    // interpret n==0 as unlimited
    if (n == 0) n = 0xffffffffffffffffULL;

    // initial check for max cycles
    tsim_u64 steps = 0;
    if (_cycles >= _max_cycles) return 0;

    define_parameter( "sim.max_blocks", _max_blocks );
    if (_blocks >= _max_blocks) return 0;

    // evaluation loop
    _stopped = false;
    while ( !_stopped )
    {
        int i, mc;

        // turn debugging off
        if (_cycles < tsim_debug_start)  tsim_debug_flag = false;
        if (_cycles > tsim_debug_stop)   tsim_debug_flag = false;
        if (_cycles < tsim_events_start) tsim_events_flag = false;
        if (_cycles > tsim_events_stop)  tsim_events_flag = false;

        // module evaluate phase (top down)
        mc = _mod_evaluate_list.size();
        for (i=0; i<mc; i++)
        {
            if (_fatals > 0) break;
            tsim_module* mptr = _mod_evaluate_list[i];
	    if (_cycles % mptr->_clock_period != 0)	break;
	    //print("i");
            mptr->evaluate();
            _evaluates++;
        }

        // module update phase (bottom up)
        mc = _mod_update_list.size();
        for (i=mc-1; i>=0; i--)
        {
            if (_fatals > 0) break;
            tsim_module* mptr = _mod_update_list[i];
	    if (_cycles % mptr->_clock_period != 0)	break;
            mptr->update();
            _updates++;
        }


        // list maintenance
        if (_pruned_list)
        {
           reform_lists();
           _pruned_list = false;
        }
        

        // turn debugging back on
        tsim_debug_flag = tsim_enable_debugging;
        tsim_events_flag = tsim_enable_events_file;

        // increment cycle count and check limit
        steps++;
        _cycles++;
        // added by sibi
        _cycles_power++;

        // if(_cycles_power == power_ctr_interval){
//           // update the power model
//           _cycles_power = 0;
//           // module evaluate phase (top down)
//           mc = _mod_evaluate_list.size();
//           for (i=0; i<mc; i++)
//             {
//               if (_fatals > 0) break;
//               tsim_module* mptr = _mod_evaluate_list[i];
//               mptr->update_power_counters();
//               _power_updates++;
//             }          
//         }


        if (_stopped)
        {
          //debug( "Stopping simulation (due to model stop request)" );
        }
        else if (steps >= n)
        {
          //debug( "Stopping simulation (due to step count reached)" );
            _stopped = true;
        }
        else if (_cycles >= _max_cycles)
        {
            print( "Stopping simulation (due to max_cycles limitation)" );
            _stopped = true;
        }
        else if (_blocks >= _max_blocks)
        {
            print( "Stopping simulation (due to max_blocks limitation)" );
            _stopped = true;
        }
        else if (_fatals > 0)
        {
            print( "Stopping simulation (due to fatal errors)" );
            _stopped = true;
        }
        else if (tsim_interrupted)
        {
            print( "Stopping simulation (due to user interrupt)" );
            _stopped = true;
        }
    }

    // return number of cycles simulated
    return steps;
}

void tsim_simulator::finalize()
{
    string ofn;
    ofstream ofs;
    string name;
    string value;

    print( "Finalizing simulator" );

    // call finalize for each module (top down)
    for (unsigned i=0; i<_mod_list.size(); i++)
    {
        tsim_module* mptr = _mod_list[i];
        mptr->finalize();
    }

    // report final statistics values
    report_statistic( "sim.modules", _modules );
    report_statistic( "sim.modules.update", _mod_update_list.size() );
    report_statistic( "sim.modules.evaluate", _mod_evaluate_list.size() );
    report_statistic( "sim.fatals", _fatals );
    report_statistic( "sim.warnings", _warnings );
    report_statistic( "sim.insts", _insts );
    report_statistic( "sim.blocks", _blocks );
    report_statistic( "sim.cycles", _cycles );
    report_statistic( "sim.updates", _updates );
    report_statistic( "sim.evaluates", _evaluates );
    // added by sibi 
    report_statistic( "sim.power_updates", _power_updates);

    // dump parameters
    if (tsim_enable_parms_file)
    {
        ofn = _parmsfn;
        ofs.open( ofn.c_str() );
        tsim_vmap_map::iterator it;
        for (it = _param_list.vmap.begin(); it != _param_list.vmap.end(); it++)
        // for (int i=0; i<_param_list.size(); i++)
        {
            name = (*it).first;
            value = (*it).second;
            // name = _param_list.list[i].first;
            // value = _param_list.list[i].second;
            ofs.setf( ios::left );
            ofs << setw(40) << name << " " << value << endl;
        }
        ofs.close();
    }

    // dump counters
    if (tsim_enable_stats_file)
    {
        ofn = _statsfn;
        ofs.open( ofn.c_str() );
        // tsim_vlist_map::iterator it;
        // for (it = _stats_list.map.begin(); it != _stats_list.map.end(); it++)
        for (int i=0; i<_stats_list.size(); i++)
        {
            // name = (*it).first;
            // value = (*it).second;
            name = _stats_list.list[i].first;
            value = _stats_list.list[i].second;
            ofs.setf( ios::left );
            ofs << setw(40) << name << " " << value << endl;
        }
        ofs.close();
    }

    // tend to exit value
    if (_fatals) _exit_value = (tsim_u64) -1;
}

// print and other output functions

void tsim_simulator::print( tsim_object& caller, const string& msg )
{
    print( msg );
}

void tsim_simulator::debug( tsim_object& caller, const string& msg )
{
    debug( msg );
}

void tsim_simulator::warn( tsim_object& caller, const string& msg )
{
    _warnings++;
    string omsg = "tsim_" + caller.otype_string() + " " + caller.fullname();
    print( "WARNING: " + omsg );
    print( "WARNING: " + msg );
}

void tsim_simulator::fatal( tsim_object& caller, const string& msg )
{
    _fatals++;
    string omsg = "tsim_" + caller.otype_string() + " " + caller.fullname();
    print( "FATAL: " + omsg );
    print( "FATAL: " + msg );
}

void tsim_simulator::print( const string& msg )
{
    if (tsim_enable_log_file) _logfs << msg << endl;
    if (tsim_enable_log_stdout) cout << msg << endl;
    if (tsim_enable_log_stderr) cerr << msg << endl;
}

void tsim_simulator::debug( const string& msg )
{
    if (tsim_debugging) print( "DEBUG " + tsim_u64tos(_cycles) + ": " + msg );
}

void tsim_simulator::warn( const string& msg )
{
    _warnings++;
    string omsg = "tsim_" + otype_string();
    print( "WARNING: " + omsg );
    print( "WARNING: " + msg );
}

void tsim_simulator::fatal( const string& msg )
{
    _fatals++;
    string omsg = "tsim_" + otype_string();
    print( "FATAL: " + omsg );
    print( "FATAL: " + msg );
}

bool tsim_simulator::report_statistic( const string& name, const string& value )
{
    return _stats_list.add( name, value );
}

bool tsim_simulator::report_statistic( const string& name, tsim_u64 value )
{
    return _stats_list.add( name, value );
}

bool tsim_simulator::report_statistic_double( const string& name, double value )
{
    char buff[128];
    sprintf( buff, "%.4f", value );
    return _stats_list.add( name, string( buff ) );
}

bool tsim_simulator::define_parameter( const string& name, string& value )
{
    // check for matching configuration info (replaces value)
    _config_list.get( name, value );
    // add to param list
    return _param_list.add( name, value );
}

bool tsim_simulator::define_parameter( const string& name, tsim_u64& value )
{
    // check for matching configuration info (replaces value)
    _config_list.get( name, value );
    // add to param list
    return _param_list.add( name, value );
}

bool tsim_simulator::get_parameter( const string& name, string& value )
{
    return _param_list.get( name, value );
}

bool tsim_simulator::get_parameter( const string& name, tsim_u64& value )
{
    return _param_list.get( name, value );
}

void tsim_simulator::add_module( tsim_module& obj )
{
    // error checking
    if (!initialized()) return;

    _modules++;
    _mod_list.push_back( &obj );
    _mod_update_list.push_back( &obj );
    _mod_evaluate_list.push_back( &obj );
}

void tsim_simulator::disable_module_update( tsim_module& obj )
{
    for (unsigned i=0; i<_mod_update_list.size(); i++)
    {
        if (_mod_update_list[i] == &obj)
        {
            _mod_update_list[i] = NULL;
            _pruned_list = true;
            return;
        }
    }
}

void tsim_simulator::disable_module_evaluate( tsim_module& obj )
{
    for (unsigned i=0; i<_mod_evaluate_list.size(); i++)
    {
        if (_mod_evaluate_list[i] == &obj)
        {
            _mod_evaluate_list[i] = NULL;
            _pruned_list = true;
            return;
        }
    }
}

void tsim_simulator::reform_lists()
{
    // this should happen only very rarely (once)

    unsigned i;
    vector<tsim_module*> temp_list;

    for (i=0; i<_mod_update_list.size(); i++)
    {
        if (_mod_update_list[i]) temp_list.push_back( _mod_update_list[i] );
    }
    _mod_update_list.clear();
    _mod_update_list = temp_list;  // implied copy all

    temp_list.clear();
    for (i=0; i<_mod_evaluate_list.size(); i++)
    {
        if (_mod_evaluate_list[i]) temp_list.push_back( _mod_evaluate_list[i] );
    }
    _mod_evaluate_list.clear();
    _mod_evaluate_list = temp_list;  // implied copy all
}
