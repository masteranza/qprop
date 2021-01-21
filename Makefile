include common-variables-for-make.mk
#Compile by running make proj=proj_folder_name action
#example: make proj=hydrogen im
#If no project is passed 'template' project gets built
ifeq ($(proj),)
	proj=template
endif

options=-DPROJNAME=$(proj) -I. -Iprojects/$(proj) -Ibase -lqprop -lm -Llib/x86_64

all: im re isurfv tsurff tsurff-mpi

im : libqprop.a im.cc
	$(compiler) $(optimargs) im.cc -o projects/$(proj)/im $(options)

re : libqprop.a re.cc
	$(compiler) $(optimargs) re.cc -o projects/$(proj)/re $(options) $(gslargs) 

tsurff : libqprop.a tsurff.cc
	$(compiler) $(optimargs) tsurff.cc -o projects/$(proj)/tsurff $(gslargs) $(options)

tsurff-mpi : libqprop.a tsurff.cc
	$(compiler) $(optimargs) tsurff.cc -o projects/$(proj)/tsurff-mpi $(mpiargs) $(gslargs) $(options)

isurfv : libqprop.a isurfv.cc
	$(compiler) $(optimargs) isurfv.cc -o projects/$(proj)/isurfv $(gslargs) $(options)

libqprop.a:
	cd base/ && make 

.PHONY: clean
clean:
	rm -f projects/$(proj)/im
	rm -f projects/$(proj)/re
	rm -f projects/$(proj)/isurfv
	rm -f projects/$(proj)/tsurff
	rm -f projects/$(proj)/tsurff-mpi
	rm -f projects/$(proj)/runner.log
	cd base/ && make clean
	rm -f lib/x86_64/libqprop.a