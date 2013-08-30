
$(INSTALLER_PROGNAME): ../src/$(LOG_MODULE_M_NAME) ../src/$(DATA_DEFINE_M_NAME) installer_main.c installer_module.c installer_module.h 
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

install:
	mkdir -p $(INSTALLDIR)
	cp -f $(INSTALLER_PROGNAME) $(INSTALLDIR)

clean:
	$(rm) *.o *.gch *.d $(INSTALLER_PROGNAME) 
