
#include <time.h>
#include "tsim.h"
#include "ocin_top.h"

string version( "OCIN TSIM Simulator - Version " TSIM_VERSION );

char*  progname = NULL;

int    trace_mode = 0;
int    exit_mode = 0;
tsim_u64 max_cycles = 10000000;
tsim_vmap vmap;
vector<string> configfiles;
string outfile = "./tsim_ocin";

time_t sim_start_time;
time_t sim_end_time;
tsim_u64 sim_elapsed_time;

void show_usage( int rc )
{
    // cout << "Usage: " << progname << " [options] appname [appargs]" << endl <<
    cout << "Usage: " << progname << " [options] " << endl <<
    endl <<
    "  -h, --help             show this usage information and quit" << endl <<
    "  -v, --version          just show the version information and quit" << endl <<
    "  -n <u64>               specifies the max number of sim cycles (default = " << tsim_u64tos(max_cycles) << ")" << endl <<
    "  -config <filename>     specifies a configuration input file" << endl <<
    "  -out <filename>        specifies an output file prefix (and path)" << endl <<
    "  -debug, -d             show verbose debug info in the log" << endl <<
    "  -debug_start <u64>     show verbose debug info, specify start cycle" << endl <<
    "  -debug_stop <u64>      show verbose debug info, specify stop cycle" << endl <<
    "  -parms, -p             create simulator parms file" << endl <<
    "  -stats, -s             create simulator stats file" << endl <<
    "  -nologfile             disables simulator log to a file" << endl <<
    "  -nologout              disables simulator log to stdout" << endl <<
    endl <<
    "  --<name>               initialize sim parameter <name> = 1" << endl <<
    "  --<name>=<value>       initialize sim parameter <name> = <value>" << endl <<
    endl << endl;

    exit( rc );
}

void process_args( int argc, char** argv )
{
    assert( argc > 0 );
    progname = argv[0];

    for (int i=1; i<argc; i++)
    {
        string s = argv[i];
        if ((s == "-h") || (s == "--help"))
        {
            cout << version << endl;
            show_usage(0);
        }
        else if ((s == "-v") || (s == "--version"))
        {
            cout << version << endl;
            exit(0);
        }
        else if (s == "-n")
        {
            if (++i >= argc) show_usage(-1);
            max_cycles = tsim_stou64( string(argv[i]) );
        }
        else if ((s == "-config") || (s == "-c"))
        {
            if (++i >= argc) show_usage(-1);
            configfiles.push_back( string(argv[i]) );
        }
        else if ((s == "-out") || (s == "-o"))
        {
            if (++i >= argc) show_usage(-1);
            outfile = string(argv[i]);
        }
        else if ((s == "-debug") || (s == "-d"))
        {
            tsim_enable_debugging = true;
        }
        else if (s == "-debug_start")
        {
            if (++i >= argc) show_usage(-1);
            tsim_debug_start = tsim_stou64( string(argv[i]) );
            tsim_enable_debugging = true;
        }
        else if (s == "-debug_stop")
        {
            if (++i >= argc) show_usage(-1);
            tsim_debug_stop = tsim_stou64( string(argv[i]) );
            tsim_enable_debugging = true;
        }
        else if ((s == "-parms") || (s == "-p"))
        {
            tsim_enable_parms_file = true;
        }
        else if (s == "-noparms")
        {
            tsim_enable_parms_file = false;
        }
        else if ((s == "-stats") || (s == "-s"))
        {
            tsim_enable_stats_file = true;
        }
        else if (s == "-nostats")
        {
            tsim_enable_stats_file = false;
        }
        else if (s == "-log")
        {
            tsim_enable_log_file = true;
        }
        else if (s == "-nolog")
        {
            tsim_enable_log_stdout = false;
        }
        // other ones
        else if ((s == "--logfile") || (s == "-logfile"))
        {
            tsim_enable_log_file = true;
        }
        else if ((s == "--nologfile") || (s == "-nologfile"))
        {
            tsim_enable_log_file = false;
        }
        else if ((s == "--logout") || (s == "-logout"))
        {
            tsim_enable_log_stdout = true;
        }
        else if ((s == "--nologout") || (s == "-nologout"))
        {
            tsim_enable_log_stdout = false;
        }
        else if ((s == "--logerr") || (s == "-logerr"))
        {
            tsim_enable_log_stderr = true;
        }
        else if ((s == "--nologerr") || (s == "-nologerr"))
        {
            tsim_enable_log_stderr = false;
        }
        else if (s[0] == '-')
        {
            if (s[1] == '-')
            {
                int x = s.find_first_of('=');
                if (x != s.npos)
                {
                    vmap.add( s.substr(2,x-2), s.substr(x+1) );
                }
                else
                {
                    vmap.add( s.substr(2), 1 );
                }
            }
            else
            {
                cout << "Illegal argument: " << s << endl;
                show_usage(-1);
            }
        }
        else
        {
            break;
        }
    }
}


// setup the static vars here:

// used for image color scaling
// int ocin_vis_node::fifo_max_val = 0;
// int ocin_vis_node::fifo_min_val = 0;
// int ocin_vis_node::link_max_val = 0;
// int ocin_vis_node::link_min_val = 0;

// current pid number
long long ocin_gen::pid = 0;
long long ocin_gen::temp_pid = 0;
// current pid number
bool ocin_gen::gen_done = false;

int main( int argc, char** argv, char** envp )
{
    tsim_enable_log_file = true;
    tsim_enable_parms_file = true;
    tsim_enable_stats_file = true;

    sim_start_time = time((time_t *)NULL);

    // process cmd line args
    process_args( argc, argv );

    // create simulator
    tsim_simulator sim( outfile );
    sim.print( version );
    sim.print( "" );

    // configure simulator
    sim.config( "sim.max_cycles", max_cycles );
    for (int i=0; i<configfiles.size(); i++)
    {
        sim.config( configfiles[i] );
    }
    sim.config( vmap );

    // initialize simulator
    sim.init();

    // create and init model
    ocin_top ocin;
    ocin.init( &sim, "ocin_top");

    // process sim commands
    sim.run( 0 );

skip_run:

    // end and summarize

    sim_end_time = time((time_t *)NULL);
    sim_elapsed_time = (tsim_u64) (sim_end_time - sim_start_time);

    sim.finalize();
    sim.print( "" );
    sim.print( "total elaped time    : " + tsim_u64tos(sim_elapsed_time));
    sim.print( "cycles               : " + tsim_u64tos(sim.cycles()));
    sim.print( "cycles per second    : " + ocin_ftos( (float)sim.cycles()/(float)sim_elapsed_time  ));
    sim.print( "warnings             : " + tsim_u64tos(sim.warnings()));
    sim.print( "fatals               : " + tsim_u64tos(sim.fatals()));

    return 0;
}
