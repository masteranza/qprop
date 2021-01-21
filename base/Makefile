include ../main/common-variables-for-make.mk
# The option -march=native may allow the compiler to use fancy instructions (SSE, AVX and the like).
# It also means you have to recompile on each CPU architecture you are using.
# gxxoptions = $(optimargs) -g -Wno-deprecated  -funroll-all-loops -finline-functions -fexpensive-optimizations  -Xlinker  -defsym -Xlinker MAIN__=main -I.
gxxoptions = $(optimargs) -g  -finline-functions -fexpensive-optimizations  -Xlinker  -defsym -Xlinker MAIN__=main -I.
libpath = ../lib/x86_64
# $@ - means the target
# $^ - means all prerequisites
# $< - means just the first prerequisite

all: install

bar.o: bar.cc
	$(compiler) $(gxxoptions) -c -o $@ $<

ylm.o: ylm.cc
	$(compiler) $(gxxoptions) -c -o $@ $<

wavefunction.o: wavefunction.cc
	$(compiler) $(gxxoptions) -c -o $@ $<

hamop.o: hamop.cc
	$(compiler) $(gxxoptions) -c -o $@ $<

grid.o: grid.cc
	$(compiler) $(gxxoptions) -c -o $@ $<

fluid.o: fluid.cc
	$(compiler) $(gxxoptions) -c -o $@ $<

factorial.o: factorial.cc
	$(compiler) $(gxxoptions) -c -o $@ $<

gfunc.o: gfunc.cc
	$(compiler) $(gxxoptions) -c -o $@ $<

# winop.o: winop.cc
	# $(compiler) $(gxxoptions) -c -o $@ $<

parameter.o: parameter.cc
	$(compiler) $(gxxoptions) -c -o $@ $<

# ar rvs ---> replace files, be verbose, Write an object-file index into the archive
libqprop.a: bar.o ylm.o wavefunction.o hamop.o grid.o fluid.o factorial.o gfunc.o parameter.o #winop.o
	ar rvs $@ $^

clean:
	rm -f *.o *.a

install: $(libpath)/libqprop.a

# solution with mkdir -p is ugly but it works
$(libpath)/libqprop.a: libqprop.a
	mkdir -p $(libpath); cp libqprop.a $(libpath)
