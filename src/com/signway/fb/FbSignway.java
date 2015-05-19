package com.signway.fb;

import android.graphics.Bitmap;

public final class FbSignway {
	static {
		System.loadLibrary("fb_jni");
	}
	
	private static final String path = "/dev/graphics/fb4";
	
	public static final int PREPARE_FB = 0x8080;
	public static final int DISPLAY_FB = 0x8081;
	public static final int CLEAR_FB   = 0x8082;
		
	FbSignway()
	{
		FbOpen(path);
	}
	
	FbSignway(String device)
	{
		FbOpen(device);
	}
	
	public synchronized native int FbOpen(String device);
	
	public synchronized native int FbClose();

	public synchronized native int FbRead(byte[] buf, int len);

	public synchronized native int FbWrite(byte[] buf, int len);
	
	public synchronized native int FbIoctl(int cmd, Bitmap bp1, Bitmap bp2, Bitmap bp3);
	
}