include main/common-variables-for-make.mk
#Compile by running make proj=proj_folder_name action
#example: make proj=hydrogen im
#If no project is passed 'template' project gets built
ifeq ($(proj),)
	proj=template
endif

options=-DPROJNAME=$(proj) -Imain -Iprojects/$(proj) -Ibase -lqprop -lm -Llib/x86_64

all: auto im re isurfv tsurff tsurff-mpi

auto : libqprop.a main/auto.cc
	$(compiler) $(optimargs) main/auto.cc -o projects/$(proj)/auto $(options)

im : auto libqprop.a main/im.cc
	$(compiler) $(optimargs) main/im.cc -o projects/$(proj)/im $(options)

re : libqprop.a main/re.cc
	$(compiler) $(optimargs) main/re.cc -o projects/$(proj)/re $(options) $(gslargs) 

tsurff : libqprop.a main/tsurff.cc
	$(compiler) $(optimargs) main/tsurff.cc -o projects/$(proj)/tsurff $(gslargs) $(options)

tsurff-mpi : libqprop.a main/tsurff.cc
	$(compiler) $(optimargs) main/tsurff.cc -o projects/$(proj)/tsurff-mpi $(mpiargs) $(gslargs) $(options)

isurfv : libqprop.a main/isurfv.cc
	$(compiler) $(optimargs) main/isurfv.cc -o projects/$(proj)/isurfv $(gslargs) $(options)

libqprop.a:
	cd base/ && make 

.PHONY: clean
clean:
	rm -f projects/$(proj)/im
	rm -f projects/$(proj)/re
	rm -f projects/$(proj)/isurfv
	rm -f projects/$(proj)/tsurff
	rm -f projects/$(proj)/tsurff-mpi
	cd base/ && make clean
	rm -f lib/x86_64/libqprop.a