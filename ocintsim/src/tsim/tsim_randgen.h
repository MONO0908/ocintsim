#ifndef TSIM_RANDGEN_H
#define TSIM_RANDGEN_H

enum tsim_randgen_method {TSIM_RANDGEN_FIXED,
                          TSIM_RANDGEN_UNIFORM,
                          TSIM_RANDGEN_BOOLEAN,
                          TSIM_RANDGEN_NORMAL,
                          TSIM_RANDGEN_EXP};

#define tsim_i64 long long

class tsim_randgen 
{
 public:
 
    tsim_randgen();

    tsim_randgen(enum tsim_randgen_method m, float false_bias);
    tsim_randgen(enum tsim_randgen_method m, tsim_i64 value);
    tsim_randgen(enum tsim_randgen_method m, tsim_i64 max, tsim_i64 min);
    tsim_randgen(enum tsim_randgen_method m, tsim_i64 max, tsim_i64 min, tsim_i64 mean);

    ~tsim_randgen();

    void init(enum tsim_randgen_method m, float false_bias);
    void init(enum tsim_randgen_method m, tsim_i64 value);
    void init(enum tsim_randgen_method m, tsim_i64 max, tsim_i64 min);

    void init(enum tsim_randgen_method m, tsim_i64 max, tsim_i64 min, tsim_i64 mean);

    bool next(tsim_i64 &);
    tsim_i64 next();

 private:
    tsim_i64 new_number();
    float randf();
    tsim_i64 fixed_random();
    tsim_i64 uniform_random();
    tsim_i64 normal_random();
    tsim_i64 exp_random();
    tsim_i64 boolean_random();


    float gasdev();
    float scale_gasdev(float );

    float max;
    float min;
    float mean;
    float max_a;
    float min_a;

    tsim_i64 i_max;
    tsim_i64 i_min;


    tsim_i64 fixed_value;
    float false_bias;

    bool legal_ranges;
    enum tsim_randgen_method method;

    float EXP_MAX;

    float GAUSS_MIN;
    float GAUSS_MAX;
    bool bounds_check;
};

#endif

