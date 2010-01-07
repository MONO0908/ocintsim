//////////////////////////////////////////////////////////////////////////////
// TSIM Power Model Header file
//////////////////////////////////////////////////////////////////////////////


#ifndef TSIM_POWER_H
#define TSIM_POWER_H

#include <tsim.h>
#include <tsim_simulator.h>

/*********************************************
 * UNITS for various parameters              *
 *                                           *
 * capacitance - farad                       *
 * voltage     - volts                       *
 * Freq        - Hertz                       *
 * len         - um (micrometer)             *
 *                                           *
 *********************************************/

// all power model declarations go here


// power model for array structures

class tsim_cache_power_model{
 public:
  bool   single_ended, use_cacti;
  int    num_rows, num_cols, num_read_ports, num_write_ports;
  double metal_cap, access_cell_diff_cap, access_cell_gate_cap;
  double wordline_driver_diff_cap, precharge_diff_cap;
  double bitline_len, wordline_len;

  double vdd, freq, act_factor, vdd_swing;
  double tot_dec_cap, tot_wordline_cap, tot_bitline_cap;
  double tot_senseamp_cap;
  double decoder_power, wordline_power, bitline_power, senseamp_power;

  // default constructor
  tsim_cache_power_model();
  // constructor with the required values
  tsim_cache_power_model(int nrows, int ncols, int rd_ports,
                         int wr_ports, bool single_ended);
  // constructor with all the values;
  tsim_cache_power_model(int nrows, int ncols, int rd_ports,
                         int wr_ports, bool single_ended,
                         double bitline_len, double wordline_len);

  // functions to set Vdd and Frequency
  void set_freq_vdd(double vdd, double vdd_swing, double freq){
    this->vdd = vdd;
    this->freq = freq;
    this->vdd_swing = vdd_swing;
  }

  inline double get_power_factor(){
    return (vdd * vdd * freq);
  }
  
  inline double get_bitline_power_factor(){
    return (vdd_swing * vdd_swing * freq);
  }
  
  inline double get_act_factor(){
    return act_factor;
  }
  
  void set_act_factor(double alpha){
    if(alpha == 0.0){
      //TSIM_DEBUG("WARNING: activity factor can't be 0. Setting it to 0.5 \n");
      act_factor = 0.5;
    }
    else if(alpha > 1.0){
      act_factor = 1.0;
    }
    else if(alpha > 0.0)
      act_factor = alpha;
    else
      act_factor = 0.5;
    
  }

  // functions to auto-size the transistors
  void size_access_cell();
  void size_wordline_driver();
  void size_precharge_cell();

  // functions to calculate the power(energy)
  // needed for individual components of the cache
  void calculate_total_power();
  void calculate_decoder_power();
  void calculate_wordline_power();
  void calculate_bitline_power();
  void calculate_senseamp_power();


  // functions to access individual power values
  double get_decoder_power();
  double get_wordline_power();
  double get_bitline_power();
  double get_senseamp_power();
  double get_total_power();

  

  
 

};


// power model for complex combinational logic
class tsim_combinational_power_model{


};


// power model for clock tree 

class tsim_clock_power_model {



};




#endif
