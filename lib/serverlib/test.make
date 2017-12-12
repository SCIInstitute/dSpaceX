#
include ../include/$(GEM_ARCH)
ODIR  = $(GEM_BLOC)/obj
LDIR  = $(GEM_BLOC)/lib
TDIR  = $(GEM_BLOC)/test

VPATH = $(ODIR)

$(TDIR)/ftest:	$(LDIR)/libwsserver.a $(ODIR)/test.o
	$(FCOMP) -o $(TDIR)/ftest $(ODIR)/test.o -L$(LDIR) -lwsserver\
		-lpthread -lz

$(ODIR)/test.o:	test.f 
	$(FCOMP) -c $(FOPTS) -fno-range-check test.f -I../include \
		-o $(ODIR)/test.o

clean:
	-rm $(ODIR)/test.o
