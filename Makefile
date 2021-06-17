include main/common-variables-for-make.mk
#Compile by running make proj=proj_folder_name action
#example: make proj=hydrogen im
#If no project is passed 'template' project gets built
ifeq ($(proj),)
  $(info No project was passed using proj=projectname, template will be built!)
  proj=template
endif
srcp=projects/$(proj)

options=-DPROJNAME=$(proj) -Imain -I$(srcp) -Ibase -lqprop -lm -Llib/x86_64

all: auto im re isurfv tsurff tsurff-mpi

auto : libqprop.a main/auto.cc
	$(compiler) $(optimargs) main/auto.cc -o $(srcp)/auto $(options)

im : auto libqprop.a main/im.cc
	$(compiler) $(optimargs) main/im.cc -o $(srcp)/im $(options)

re : libqprop.a main/re.cc
	$(compiler) $(optimargs) main/re.cc -o $(srcp)/re $(options) $(gslargs) 

tsurff : libqprop.a main/tsurff.cc
	$(compiler) $(optimargs) main/tsurff.cc -o $(srcp)/tsurff $(gslargs) $(options)

tsurff-mpi : libqprop.a main/tsurff.cc
	$(compiler) $(optimargs) main/tsurff.cc -o $(srcp)/tsurff-mpi $(mpiargs) $(gslargs) $(options)

isurfv : libqprop.a main/isurfv.cc
	$(compiler) $(optimargs) main/isurfv.cc -o $(srcp)/isurfv $(gslargs) $(options)

libqprop.a:
	cd base/ && make 

.PHONY: clean
clean:
	rm -f $(srcp)/im
	rm -f $(srcp)/re
	rm -f $(srcp)/isurfv
	rm -f $(srcp)/tsurff
	rm -f $(srcp)/tsurff-mpi
	cd base/ && make clean
	rm -f lib/x86_64/libqprop.a