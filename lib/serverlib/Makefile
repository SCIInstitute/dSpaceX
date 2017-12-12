#
IDIR  = ../include
include $(IDIR)/$(DSX_ARCH)
LDIR  = ../lib
ODIR  = ../obj
BDIR  = ../bin


VPATH = $(ODIR)

OBJS = base64-decode.o handshake.o client-handshake.o libwebsockets.o \
       extension-deflate-stream.o md5.o extension-x-google-mux.o parsers.o \
       extension.o sha-1.o server.o wv.o fwv.o

$(BDIR)/servertest:	$(LDIR)/libwsserver.a $(ODIR)/servertest.o
	$(CC) -o $(BDIR)/servertest $(ODIR)/servertest.o -L$(LDIR) -lwsserver \
		-lpthread -lz -lm

$(ODIR)/servertest.o:	server.c 
	$(CC) -c $(COPTS) -DLWS_NO_FORK -DSTANDALONE server.c \
		-I$(IDIR) -I. -o $(ODIR)/servertest.o

$(LDIR)/libwsserver.a:	$(OBJS)
	touch $(LDIR)/libwsserver.a
	rm $(LDIR)/libwsserver.a
	(cd $(ODIR); ar $(LOPTS) $(LDIR)/libwsserver.a $(OBJS); $(RANLB) )

$(OBJS):	extension-deflate-stream.h libwebsockets.h \
		extension-x-google-mux.h private-libwebsockets.h \
		$(IDIR)/wsserver.h $(IDIR)/wsss.h
.c.o:
	$(CC) -c $(COPTS) -DLWS_NO_FORK -I$(IDIR) $< -o $(ODIR)/$@

clean:
	-(cd $(ODIR); rm $(OBJS) servertest.o )

cleanall:	clean
	-rm $(BDIR)/servertest $(LDIR)/libwsserver.a
