#
IDIR  = ../include
include $(IDIR)/$(DSX_ARCH)
LDIR  = ../lib
ODIR  = ../obj
BDIR  = ../bin

default:	$(BDIR)/dSpaceX $(LDIR)/sampleBE.so

$(BDIR)/dSpaceX:	$(ODIR)/dSpaceX.o $(ODIR)/dsxdyn.o $(ODIR)/render.o \
			$(LDIR)/libwsserver.a
	$(CXX) -o $(BDIR)/dSpaceX $(ODIR)/dSpaceX.o $(ODIR)/dsxdyn.o \
		$(ODIR)/render.o \
		-L$(LDIR) -lwsserver -lpthread -lz $(RPATH) -lm -ldl

$(ODIR)/dSpaceX.o:	dSpaceX.cpp $(IDIR)/wsserver.h
	$(CXX) -c $(CPPOPT) $(DEFINE) -I$(IDIR) dSpaceX.cpp -o $(ODIR)/dSpaceX.o

$(ODIR)/dsxdyn.o:	dsxdyn.c dsxdyn.h
	$(CC) -c $(COPTS) $(DEFINE) -I$(IDIR) dsxdyn.c -o $(ODIR)/dsxdyn.o

$(ODIR)/render.o:	render.c
	$(CC) -c $(COPTS) $(DEFINE) -I$(IDIR) render.c -o $(ODIR)/render.o

$(ODIR)/thumbs.o:	thumbs.c
	$(CC) -c $(COPTS) $(DEFINE) -I$(IDIR) thumbs.c -o $(ODIR)/thumbs.o

$(LDIR)/sampleBE.so:	$(ODIR)/sampleBE.o $(ODIR)/thumbs.o
	-rm $(LDIR)/sampleBE.so
	$(CC) $(SOFLGS) -o $(LDIR)/sampleBE.so $(ODIR)/sampleBE.o \
		$(ODIR)/thumbs.o -ljpeg -lpng -lm

$(ODIR)/sampleBE.o:	sampleBE.c
	$(CC) -c $(COPTS) $(DEFINE) -I$(IDIR) sampleBE.c -o $(ODIR)/sampleBE.o

clean:
	-rm $(ODIR)/dSpaceX.o $(ODIR)/dsxdyn.o $(ODIR)/render.o \
		$(ODIR)/thumbs.o $(ODIR)/sampleBE.o

cleanall:	clean
	-rm $(BDIR)/dSpaceX $(LDIR)/sampleBE.so
