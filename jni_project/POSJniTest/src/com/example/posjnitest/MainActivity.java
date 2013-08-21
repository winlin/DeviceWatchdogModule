package com.example.posjnitest;

import android.os.Bundle;
import android.app.Activity;
import android.util.Log;
import android.view.Menu;
import android.os.Process;
import com.ipaloma.posjniproject.jni.NativeUtilitiesClass;


public class MainActivity extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
  
        NativeUtilitiesClass nu = new NativeUtilitiesClass();
        String cmdline = "ls not 2>&1";
        int ret = nu.execCmdLine(cmdline);
        System.out.println("Get the return value:" + ret);
        
        String logTagStr = "posjnitest";
        int feedPeriod = 5;
        
        // save the app's info
        int funcRet = nu.saveAppInfoConfig(Process.myPid(), 
        									feedPeriod, 
        									"com.example.posjnitest", 
        									"am start -n com.example.posjnitest/com.example.posjnitest.MainActivity", 
        									"v1.0.1");
        if(funcRet == 0) {
        	// init the socket  
        	funcRet = nu.initUDPSocket();
        	if(funcRet == 0) {
        		// register to the watchdog
        		while((funcRet = nu.sendWDRegisterPackage()) != 0) {
        			Log.e(logTagStr, "sendWDRegisterPackage() failed, register package will be re-sent after 1 second RETURN VALUE:"+funcRet);
        			try {
						Thread.sleep(1000);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
        		}
        		// after register send feed package periodically
        		int i = 0;
        		while(i++ < 40) {
        			if((funcRet=nu.sendWDFeedPackage()) != 0) {
        				Log.e(logTagStr, "Send Feed Package failed, the register package will be re-sent after 1 second");
        				// register to the watchdog
                		while((funcRet = nu.sendWDRegisterPackage()) != 0) {
                			Log.e(logTagStr, "sendWDRegisterPackage() failed, register package will be re-sent after 1 second");
                			try {
        						Thread.sleep(1000);
        					} catch (InterruptedException e) {
        						// TODO Auto-generated catch block
        						e.printStackTrace();
        					}
                		}
        			}
        			Log.i(logTagStr, "Success sending feed package one time");
        			try {
						Thread.sleep((feedPeriod-1)*1000);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
        		}
        		Log.e(logTagStr, "Start to un-register to the watchdog...");
        		funcRet = nu.sendWDUnregisterPackage();
        		if(funcRet != 0) {
        			Log.e(logTagStr, "sendWDUnregisterPackage() failed"); 
        		}
        		funcRet = nu.closeUPDClient();
        		if(funcRet != 0) {
        			Log.e(logTagStr, "closeUPDClient() failed"); 
        		}
        	} else {
        		Log.e(logTagStr, "initUDPSocket() failed");
        	}
        } else {
        	Log.e(logTagStr, "saveAppInfoConfig() failed");
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }
    
}
