///////////////////////////////////////////////////////////////////////////////
// TSIM simulator header
///////////////////////////////////////////////////////////////////////////////

#ifndef TSIM_SIMULATOR_H
#define TSIM_SIMULATOR_H

extern bool tsim_enable_debugging;
extern bool tsim_enable_log_file;
extern bool tsim_enable_log_stdout;
extern bool tsim_enable_log_stderr;
extern bool tsim_enable_parms_file;
extern bool tsim_enable_stats_file;
extern bool tsim_enable_events_file;

extern tsim_u64 tsim_debug_start;
extern tsim_u64 tsim_debug_stop;
extern tsim_u64 tsim_events_start;
extern tsim_u64 tsim_events_stop;

// added by sibi
extern tsim_u64 enable_power_model;
extern tsim_u64 tech_node;
extern tsim_u64 interconn_cap_per_length;
extern tsim_u64 leakage_current_per_unit_area;
extern tsim_u64 enable_cacti_usage;
extern tsim_u64 power_ctr_interval;
extern tsim_u64 processor_frequency;
extern double voltage;
extern tsim_u64 use_tech_defaults;


#define TSIM_DEBUG_NONE   0x0
#define TSIM_DEBUG_ALL    0x1

extern bool tsim_debug_flag;   // simulator-managed flag
extern bool tsim_events_flag;  // simulator-managed flag
extern bool tsim_opn_events_flag; // ramdas: used for critical path analysis
extern bool tsim_interrupted;  // used with signals
extern void tsim_sigint( int ignore );

extern string tsim_flavor;

#ifdef NDEBUG
#define TSIM_DEBUGGING 0
#define tsim_debugging TSIM_DEBUGGING
#define TSIM_DEBUG(x)
#else
#define TSIM_DEBUGGING (tsim_debug_flag)
#define tsim_debugging TSIM_DEBUGGING
#define TSIM_DEBUG(x) if (tsim_debug_flag) debug(x)
#endif

// classes
class tsim_simulator : public tsim_object
{
    friend class tsim_object;
    friend class tsim_module;

public:

    tsim_simulator( const string& simout = "tsim" );
    ~tsim_simulator();

    void config( const string& filename );
    void config( const string& name, const string& value );
    void config( const string& name, tsim_u64 value );
    void config( const tsim_vmap& vmap );

    void init();
    tsim_u64 run( tsim_u64 n );
    tsim_u64 run() { return run( 0 ); }
    void stop() { _stopped = true; }
    void finalize();

    inline const string& simout() const { return _simout; }

    inline tsim_u64 exit_value() const { return _exit_value; }
    void set_exit_value( tsim_u64 v ) { _exit_value = v; }

    inline tsim_u64 fatals() const { return _fatals; }
    inline tsim_u64 warnings() const { return _warnings; }
    inline tsim_u64 insts() const { return _insts; }
    inline tsim_u64 blocks() const { return _blocks; }
    inline tsim_u64 cycles() const { return _cycles; }
    inline tsim_u64 modules() const { return _modules; }

    void increment_insts( tsim_u64 insts ) { _insts += insts; }
    void increment_blocks( tsim_u64 blocks ) { _blocks += blocks; }

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

#ifdef RTL_VERIF_ET
    // methods for etile verif
    vector<tsim_module*> get_mod_update_list () { return _mod_update_list; }
    vector<tsim_module*> get_mod_evaluate_list () { return _mod_evaluate_list; }
    bool get_pruned_list () { return _pruned_list; }
    void set_pruned_list (bool pl) { _pruned_list = pl; }
    void reform_lists_ext () { reform_lists (); }
    void increment_cycles () { _cycles++; }
#endif

    inline bool stopped() { return _stopped; }
    inline void unstop() { _stopped = false; }
    inline void increment_cycles(tsim_u64 n) { _cycles += n; }
    inline tsim_u64 max_cycles() { return _max_cycles; }
    inline tsim_u64 max_blocks() { return _max_blocks; }
    
protected:

    void print( tsim_object& caller, const string& msg );
    void debug( tsim_object& caller, const string& msg );
    void warn( tsim_object& caller, const string& msg );
    void fatal( tsim_object& caller, const string& msg );

    void add_module( tsim_module& obj );
    void disable_module_update( tsim_module& obj );
    void disable_module_evaluate( tsim_module& obj );

private:

    void reform_lists();

    bool _stopped;
    string _simout;
    string _logfn;
    string _parmsfn;
    string _statsfn;
    ofstream _logfs;
    tsim_u64 _exit_value;

    // parameters
    tsim_u64 _max_cycles;
    tsim_u64 _max_blocks;
    
    // counters
    tsim_u64 _modules;
    tsim_u64 _fatals;
    tsim_u64 _warnings;
    tsim_u64 _insts;
    tsim_u64 _blocks;
    tsim_u64 _cycles;
    tsim_u64 _evaluates;
    tsim_u64 _updates;
    // added by sibi
    // counter that keeps track
    // of number of cycles elapsed from
    // last power update event
    tsim_u64 _cycles_power;
    tsim_u64 _power_updates;

    // lists
    tsim_vmap  _config_list;
    tsim_vmap  _param_list;
    tsim_vlist _stats_list;
    vector<tsim_module*> _mod_list;
    vector<tsim_module*> _mod_update_list;
    vector<tsim_module*> _mod_evaluate_list;
    bool _pruned_list;
};

#endif
// TSIM_SIMULATOR_H
