optimargs=-O3 -ffast-math -march=native -std=c++0x
OS:=$(shell uname)
compiler=$(CXX)
USERNAME := $(shell whoami)

#Per platform configs
ifeq ($(OS),Linux) # for Ubuntu this seems to work for Open MPI
compiler=$(CXX)

ifeq ($(USERNAME),ranza)
# gslargs=-I/usr/include/x86_64-linux-gnu -L/usr/lib/x86_64-linux-gnu -lgsl -lgslcblas 
gslargs=-I/home/ranza/gsl/include -L/home/ranza/gsl/lib -lgsl -lgslcblas 
mpiargs=-I/usr/lib/x86_64-linux-gnu/openmpi/include -L/usr/lib/x86_64-linux-gnu -lmpi -lmpi_cxx -DHAVE_MPI

else 
#standard
gslargs=-I/usr/include -L/usr/lib -lgsl -lgslcblas
mpiargs=-I/usr/include -L/usr/lib -lmpi -lmpi_cxx -DHAVE_MPI
# for Ubuntu this seems to work for Open MPI
# mpiargs=-I/usr/include/openmpi -L/usr/lib -lmpi -lmpi -DHAVE_MPI
endif

endif

ifeq ($(OS),Darwin) 
#For Brew (if you've done 'brew install gcc' and you still get an error you may want to try 'brew link --overwrite gcc')
compiler=g++-10
gslargs= -I/usr/local/include -L/usr/local/lib -lgsl -lgslcblas
mpiargs= -I/usr/include/openmpi -L/usr/lib -lmpi -lmpi -DHAVE_MPI
#For macPorts
# gslargs= -I/opt/local/include -L/opt/local/lib -lgsl -lgslcblas
# mpiargs= -I/usr/local/Cellar/open-mpi/3.0.0_2/include -L/usr/local/opt/libevent/lib -L/usr/local/Cellar/open-mpi/3.0.0_2/lib -lmpi -DHAVE_MPI
endif

# for Windows provide your own includes, sorry :)
# To check compatibility aka C++ ISO compliance add -pedantic flag