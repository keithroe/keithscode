include ../../Makerules

CCFILES =   HalfEdge.cc     \
            Face.cc         \
            Vertex.cc       \
            Mesh.cc         


OFILES  = $(addprefix $(OBJDIR)/,$(CCFILES:.cc=.o))
DESTDIR = ../../lib.$(UNAME)
#LIB     = $(DESTDIR)/libmesh$(LDEXT)
LIB     = $(DESTDIR)/libmesh.a

default: $(DESTDIR) $(LIB)

$(DESTDIR):
	-mkdir $@

$(LIB): $(OFILES)
	rm -f $@
	@#$(LD) -o $@ $^
	$(AR) $@ $^

clean:
	-rm -f $(OFILES) $(OBJDIR)/Makedepend $(OBJDIR)/*.d
	-rmdir $(OBJDIR)

spotless: clean
	-rm -f $(LIB)
	-rmdir $(DESTDIR)

