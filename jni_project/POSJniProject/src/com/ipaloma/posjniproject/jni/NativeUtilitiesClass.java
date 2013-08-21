package com.ipaloma.posjniproject.jni;

/*
 * NativeUtilitiesClass watchdog related function return value please refer MITLogFuncRetValue:
 * typedef enum MITLogFuncRetValue {
 *   MITLOG_RETV_SUCCESS              = 0,
 *   MITLOG_RETV_FAIL                 = -1,
 *   MITLOG_RETV_PARAM_ERROR          = -100,
 *   MITLOG_RETV_OPEN_FILE_FAIL       = -101,
 *   MITLOG_RETV_ALLOC_MEM_FAIL       = -102,
 *   MITLOG_RETV_HAS_OPENED           = -103
 *	} MITLogFuncRetValue;
 */


public class NativeUtilitiesClass {
	// Declare the watchdog related function
	public native int saveAppInfoConfig(int pid, int feed_period, String appName, String cmdLine, String versionSre);
	public native int initUDPSocket();
	public native int sendWDRegisterPackage();
	public native int sendWDFeedPackage();
	public native int sendWDUnregisterPackage();
	public native int closeUPDClient();
	
    // Declare native method (and make it public to expose it directly)
	// @return On success 0 will be returned else means error.
    public native int execCmdLine(String cmdline);
 
    // Load library
    static {
        System.loadLibrary("native_utilities");
    }
}