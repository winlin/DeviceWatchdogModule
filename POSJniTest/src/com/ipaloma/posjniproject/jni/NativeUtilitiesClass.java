package com.ipaloma.posjniproject.jni;

public class NativeUtilitiesClass {
	// Declare the watchdog related function
	public native int saveAppInfoConfig(int pid, int feed_period, String appName, String cmdLine, String versionSre);
	public native int initUDPSocket();
	public native int sendWDRegisterPackage();
	public native int sendWDFeedPackage();
	public native int sendWDUnregisterPackage();
	public native int closeUPDClient();
	
    // Declare native method (and make it public to expose it directly)
    public native int execCmdLine(String cmdline);
 
    // Load library
    static {
        System.loadLibrary("native_utilities");
    }
}