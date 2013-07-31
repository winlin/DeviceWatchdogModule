
$(WD_PROGNAME): ../src/$(LOG_MODULE_M_NAME) ../src/$(DATA_DEFINE_M_NAME) watchdog_main.c wd_configure.c wd_configure.h 
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

install:
	mkdir -p $(INSTALLDIR)
	cp -f $(WD_PROGNAME) $(INSTALLDIR)

clean:
	$(rm) *.o *.gch *.d $(WD_PROGNAME) 
