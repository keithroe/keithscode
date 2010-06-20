include ../../Makerules

CCFILES =   Matrix.cc     \


OFILES  = $(addprefix $(OBJDIR)/,$(CCFILES:.cc=.o))
DESTDIR = ../../lib.$(UNAME)
#LIB     = $(DESTDIR)/libmath$(LDEXT)
LIB     = $(DESTDIR)/libmath.a

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

