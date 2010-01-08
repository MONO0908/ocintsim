#################################################
# TSIM Makefile
#################################################

# assume gcc compiler
#CC      = /lusr/opt/gcc-3.4.3/bin/gcc
#CXX     = /lusr/opt/gcc-3.4.3/bin/g++
CC      = gcc 
#CC      = gcc-4.1 
##CXX     = g++ -pg
CXX     = g++ 
#CXX     = g++-4.1 

# customize according to MODEL
ifeq ($(MODEL),tsim)
	THE_LIB = lib/libtsim.a
	VPATH   = src/tsim
	CFLAGS += -Wall
	IFLAGS += -Isrc/tsim
	CPPS   += $(wildcard src/tsim/*.cpp)
	HDRS   += $(wildcard src/tsim/*.h)
endif
ifeq ($(MODEL),ocin)
	THE_EXE = bin/tsim_ocin(CONDOR_EXE)
#	VPATH   = src/ocin:src/ocin/vis_modules:src/ocin/ej_modules:src/ocin/gen_modules:src/ocin/prtr_modules:src/ocin/io_modules:src/ocin/vc_modules:src/ocin/rt_modules:src/ocin/xbar_modules:src/ocin/monitors:src/ocin/sel_modules:src/tsim
	VPATH   = src/ocin:src/ocin/vis_modules:src/ocin/ej_modules:src/ocin/gen_modules:src/ocin/prtr_modules:src/ocin/io_modules:src/ocin/vc_modules:src/ocin/rt_modules:src/ocin/xbar_modules:src/ocin/monitors:src/ocin/sel_modules:src/tsim
	IFLAGS += -Isrc/tsim
	IFLAGS += -Isrc/ocin
	IFLAGS += -Isrc/ocin/io_modules
	IFLAGS += -Isrc/ocin/vc_modules
	IFLAGS += -Isrc/ocin/rt_modules
	IFLAGS += -Isrc/ocin/xbar_modules
	IFLAGS += -Isrc/ocin/gen_modules
	IFLAGS += -Isrc/ocin/prtr_modules
	IFLAGS += -Isrc/ocin/ej_modules
	IFLAGS += -Isrc/ocin/monitors
	IFLAGS += -Isrc/ocin/sel_modules
#	IFLAGS += -Isrc/ocin/vis_modules
	LIBS   += lib/libtsim.a
#	LFLAGS += -lgd -lpng -lz -ljpeg -lm -lgsl -lgslcblas 
	LFLAGS += -lz -lm -lgsl -lgslcblas 
	CPPS   += $(wildcard src/tsim/*.cpp)
	CPPS   += $(wildcard src/ocin/*.cpp)
	CPPS   += $(wildcard src/ocin/io_modules/*.cpp)
	CPPS   += $(wildcard src/ocin/vc_modules/*.cpp)
	CPPS   += $(wildcard src/ocin/rt_modules/*.cpp)
	CPPS   += $(wildcard src/ocin/xbar_modules/*.cpp)
	CPPS   += $(wildcard src/ocin/gen_modules/*.cpp)
	CPPS   += $(wildcard src/ocin/prtr_modules/*.cpp)
	CPPS   += $(wildcard src/ocin/ej_modules/*.cpp)
	CPPS   += $(wildcard src/ocin/monitors/*.cpp)
	CPPS   += $(wildcard src/ocin/sel_modules/*.cpp)
#	CPPS   += $(wildcard src/ocin/vis_modules/*.cpp)
	HDRS   += $(wildcard src/tsim/*.h)
	HDRS   += $(wildcard src/ocin/*.h)
	HDRS   += $(wildcard src/ocin/io_modules/*.h)
	HDRS   += $(wildcard src/ocin/vc_modules/*.h)
	HDRS   += $(wildcard src/ocin/rt_modules/*.h)
	HDRS   += $(wildcard src/ocin/xbar_modules/*.h)
	HDRS   += $(wildcard src/ocin/gen_modules/*.h)
	HDRS   += $(wildcard src/ocin/prtr_modules/*.h)
	HDRS   += $(wildcard src/ocin/ej_modules/*.h)
	HDRS   += $(wildcard src/ocin/monitors/*.h)
	HDRS   += $(wildcard src/ocin/sel_modules/*.h)
#	HDRS   += $(wildcard src/ocin/vis_modules/*.h)
endif

# common, derived variables
OPPS    = $(patsubst src/%.cpp,lib/%.opp,$(CPPS))
OBJS    = $(patsubst src/%.c,lib/%.o,$(CSRC))


# adjust for debug builds
ifeq ($(DEBUG),true)
	CFLAGS += -g -DDEBUG -DSIM_ARCH_BPRED_PROTO -DSIM_ARCH_BPRED 
else
	CFLAGS += -O2 -DSIM_ARCH_BPRED_PROTO -DSIM_ARCH_BPRED
	LFLAGS += -O2
endif

ifeq ($(NDEBUG),true)
	CFLAGS += -DNDEBUG
endif

ifeq ($(OCIN_MON),true)
	CFLAGS += -DOCIN_MON
endif

ifeq ($(OCIN_RCA),true)
	CFLAGS += -DOCIN_RCA
endif

ifeq ($(BEC),true)
	CFLAGS += -DBEC
endif

ifeq ($(RTL_VERIF_ET),true)
	CFLAGS += -DRTL_VERIF_ET
endif

# adjust for profiling builds
ifeq ($(PROFILE),true)
	CFLAGS += -pg
	LFLAGS += -pg
endif

ifeq ($(RELEASE),true)
	CFLAGS += -DTSIM_RELEASE
endif

# a cleaner way to pass misc defines
ifdef DEFINES
	CFLAGS += $(patsubst %,-D%,$(DEFINES))
endif

# other temporary definitions
ifeq ($(TEST_ICACHE),true)
	CFLAGS += -DTEST_ICACHE
endif

ifeq ($(TEST_ALLOCATE),true)
	CFLAGS += -DTEST_ALLOCATE
endif

ifeq ($(TEST_GT),true)
	CFLAGS += -DTEST_GT
endif


#
# default target
#

.PHONY: help
help:
	@echo "This makefile builds ocin_tsim simulator.  The following" ; \
	echo "make targets are supported:" ; \
	echo "  all      - builds all" ; \
	echo "  help     - shows this information" ; \
	echo "  ocin   - builds ocin_tsim" ; \
	echo "  clean    - removes all compiled object files (in lib)" ; \
	echo "  clobber  - removes all compiled executables (in bin)"


#
# primary targets
# - add new targets as necessary
#

.PHONY: all
all: arch proc ocn sys critical

.PHONY: tsim
tsim:
	@echo "Building tsim library"
	@$(MAKE) MODEL=tsim build

ocin: tsim
	@echo "Building tsim_ocin executable"
	@$(MAKE) MODEL=ocin build


#
# install support (crude, i know)
#

.PHONY: install
install:
	@if [ -d $(INSTALLDIR) ]; then \
	  echo "Installing at $(INSTALLDIR)"; \
	  mkdir -p $(INSTALLDIR)/tsim ; \
	  cp README $(INSTALLDIR)/tsim/ ; \
	  cp -r bin $(INSTALLDIR)/tsim/ ; \
	  rm -r $(INSTALLDIR)/tsim/bin/CVS ; \
	  cp -r config $(INSTALLDIR)/tsim/ ; \
	  rm -r $(INSTALLDIR)/tsim/config/CVS ; \
	else \
	  echo "Install directory missing or unspecified!"; \
	fi

#
# generic rules
# - try to keep these generic
#

build: $(THE_EXE) $(THE_LIB)

.PHONY: clean clobber
clean:
	rm -f lib/*/*.opp
	rm -f lib/*/*.o

clobber: clean
	rm -f bin/tsim_*
	rm -f bin/tracetool
	rm -f lib/*.a

$(THE_LIB): $(OPPS) $(OBJS)
	ar -csr $@ $(OPPS) $(OBJS)

$(THE_EXE): $(OPPS) $(OBJS) $(LIBS)
ifeq ($(RTL_VERIF_ET),true)
	ar -r lib/libartl.a $(OPPS) $(OBJS)
else
	$(CXX) $(LFLAGS) -o $@ $(OPPS) $(OBJS) $(LIBS)
endif

lib/%.opp : src/%.cpp
	$(CXX) $(CFLAGS) $(IFLAGS) -o $@ -c $<

lib/%.o : src/%.c
	$(CC) $(CFLAGS) $(IFLAGS) -o $@ -c $<

# enforce hdr dependences (crude but easy to maintain)
$(OPPS) $(OBJS): $(HDRS)

# enforce Makefile dependency (do we want this?)
#$(OPPS) $(OBJS): Makefile
