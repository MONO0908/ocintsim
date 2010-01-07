//////////////////////////////////////////////////////////////////////////////
// TSIM Power Model implementations
//////////////////////////////////////////////////////////////////////////////

#include "tsim_power.h"


tsim_cache_power_model::tsim_cache_power_model(){
  bool single_ended;
  int num_rows, num_cols, num_read_ports, num_write_ports;
  double metal_cap, access_cell_diff_cap, access_cell_gate_cap,
    wordline_driver_diff_cap, precharge_diff_cap;
  double bitline_len, wordline_len;


  // default values
  // TODO: change these to appropriate values
  single_ended = false;
  use_cacti = false; // need not use cacti for the time being
  num_rows = 1024;
  num_cols = 32;
  num_read_ports = 1;
  num_write_ports = 0;
  
  // cap values
  // TODO: initialize to default values

  decoder_power = wordline_power = bitline_power = senseamp_power = 0.0;


}

tsim_cache_power_model::tsim_cache_power_model(int nrows, int ncols, 
                                               int rd_ports, int wr_ports, 
                                               bool single_ended){
  num_rows = nrows;
  num_cols = ncols;
  num_read_ports = rd_ports;
  num_write_ports = wr_ports;
  this->single_ended = single_ended;
  use_cacti = false;
  

}


tsim_cache_power_model::tsim_cache_power_model(int nrows, int ncols, 
                                               int rd_ports,
                                               int wr_ports, 
                                               bool single_ended,
                                               double bitline_len, 
                                               double wordline_len){

  num_rows = nrows;
  num_cols = ncols;
  num_read_ports = rd_ports;
  num_write_ports = wr_ports;
  this->single_ended = single_ended;
  //this->access_cell_diff_cap = access_cell_diff_cap;
  //this->access_cell_gate_cap = access_cell_gate_cap;
  this->wordline_len = wordline_len;
  this->bitline_len = bitline_len;
  use_cacti = false;

}


void tsim_cache_power_model::size_access_cell(){
  //TODO: complete this function

}

void tsim_cache_power_model::size_wordline_driver(){
  //TODO: complete this function
}

void tsim_cache_power_model::size_precharge_cell(){
  //TODO: complete this function
}
  

// these power numbers are actually energy needed to
// perform one access of the structure
double tsim_cache_power_model::get_decoder_power(){
  return decoder_power;

}

double tsim_cache_power_model::get_wordline_power(){
  return wordline_power;
}
double tsim_cache_power_model::get_bitline_power(){
  return bitline_power;

}

double tsim_cache_power_model::get_senseamp_power(){
  return senseamp_power;
}

double tsim_cache_power_model::get_total_power(){
  return decoder_power + wordline_power + bitline_power +
    senseamp_power;
}


void tsim_cache_power_model::calculate_decoder_power(){
  // based on Wattch's model
  double total_cap = 0.0, temp_cap = 0.0;
  int tot_ports = 0, decode_bits = 0;

  // total ports
  tot_ports = num_read_ports + num_write_ports;

  // number of decode bits needed
  decode_bits = (int)ceil(logtwo(num_rows));

  //TODO: Adjust this equation after figuring what
  //WATTCH is doing : add both drain and gate caps here
  //setting it to some random value for now
  temp_cap = 200e-15;

  total_cap += tot_ports * decode_bits * temp_cap;

  // TODO: ignoring the NOR/NAND part for the first cut
  // add this model later
  tot_dec_cap = total_cap;
  
  decoder_power = get_act_factor() * total_cap * get_power_factor(); 

}
void tsim_cache_power_model::calculate_wordline_power(){
  int num_bit_ends = (single_ended) ? 1 : 2;
  int tot_ports = num_read_ports + num_write_ports;
  double tot_cap = 0.0;

  // TODO: improve this model after carefully reading
  // WATTCH
  tot_cap += num_bit_ends * num_cols * access_cell_gate_cap;
  tot_cap += num_cols  * wordline_len * interconn_cap_per_length;

  tot_wordline_cap = tot_cap;

  // assuming a different wordline is being accessed every cycle
  wordline_power = 1.0 * tot_cap * get_power_factor();
  

}
void tsim_cache_power_model::calculate_bitline_power(){
  int num_bit_ends = (single_ended) ? 1 : 2;
  double tot_cap = 0;

  tot_cap += num_rows * access_cell_diff_cap;
  tot_cap += bitline_len * interconn_cap_per_length;
  tot_cap += precharge_diff_cap;
  tot_cap *= num_bit_ends;

  tot_bitline_cap = tot_cap;

  if(!single_ended)
    // double ended arrays are charged and discharged
    // every cycle
    bitline_power = 1.0 * tot_cap * get_bitline_power_factor();
  else
    bitline_power = act_factor * tot_cap * get_bitline_power_factor();

}
void tsim_cache_power_model::calculate_senseamp_power(){
  // TODO: Use WATTCH's model temporarily. Fix and refine later
  senseamp_power =  (num_cols * vdd/8.0 * 0.5e-3);
  
}

void tsim_cache_power_model::calculate_total_power(){
  // calculate power for individual components
  calculate_decoder_power();
  calculate_wordline_power();
  calculate_bitline_power();
  calculate_senseamp_power();
}


