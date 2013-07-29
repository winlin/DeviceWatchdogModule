$(APP_PROGNAME): ../src/$(LOG_MODULE_M_NAME) ../src/$(DATA_DEFINE_M_NAME) app_main.c create_feed_thread.c create_feed_thread.h 
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

install:
	mkdir -p $(INSTALLDIR)
	cp $(APP_PROGNAME) $(INSTALLDIR)

clean:
	$(rm) *.o *.gch *.d $(APP_PROGNAME)
