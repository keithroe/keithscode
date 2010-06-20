include ../../Makerules


CCFILES =    main.cc     \
	     Camera.cc   \
	     Arcball.cc  \
             Viewer.cc  



OFILES  = $(addprefix $(OBJDIR)/,$(CCFILES:.cc=.o)) 
DESTDIR = ../../bin.$(UNAME)
PROG    = $(DESTDIR)/vmesh 


default: $(DESTDIR) $(PROG) 

$(DESTDIR) :
	-mkdir $@

$(PROG): $(OFILES)
	rm -f $@
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) $(GLLIBS) $(LIBS) -o $@

clean :
	-rm -f $(OFILES) $(OBJDIR)/Makedepend $(OBJDIR)/*.d 
	-rmdir $(OBJDIR)

spotless : clean
	-rm -f $(PROGS) 
	-rmdir $(DESTDIR)

