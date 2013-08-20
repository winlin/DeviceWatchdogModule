$(APP_PROGNAME): ../src/$(LOG_MODULE_M_NAME) ../src/$(DATA_DEFINE_M_NAME) monitor_main.c watchdog_comm.c watchdog_comm.h 
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

install:
	mkdir -p $(INSTALLDIR)
	cp -f $(APP_PROGNAME) $(INSTALLDIR)

clean:
	$(rm) *.o *.gch *.d $(APP_PROGNAME)
