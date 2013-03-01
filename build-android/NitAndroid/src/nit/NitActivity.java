package nit;

import android.app.*;
import android.os.*;
import android.content.*;

public class NitActivity extends NativeActivity 
{
    private Handler m_Handler;
    
    public Handler getHandler() { return m_Handler; }
    
    @Override 
    protected void onCreate(Bundle savedInstanceState)
    {
    	m_Handler = new Handler();
    	
    	super.onCreate(savedInstanceState);
    }

    @Override
    protected void onStop()
    {
    	m_Handler = null;
    	super.onStop();
    }
    
    @Override
    protected void onPause()
    {
    	super.onPause();
    }
    
    /////////////////////////////
    
    void setupPathCallback()
    {
    	String externFilesDir = "";
    	String externCacheDir = "";
    	
    	try { externFilesDir = getExternalFilesDir(null).getAbsolutePath(); } catch (Exception ex) { ex.printStackTrace(); }
    	try { externCacheDir = getExternalCacheDir().getAbsolutePath(); } catch (Exception ex) { ex.printStackTrace(); }
    	
    	nativeSetupPath(
    			"", // obb dir
    			getFilesDir().getAbsolutePath(),
    			getCacheDir().getAbsolutePath(),
    			externFilesDir,
    			externCacheDir
    			);
    }
    
    private static native void nativeSetupPath(String obbDir, String filesDir, String cacheDir, String externFileDir, String externCacheDir);

    void showMessageBox(final String title, final String message)
    {
    	if (m_Handler == null) return;
    	
		Runnable dlgRun = new Runnable() {
			@Override public void run()
			{
		    	Dialog dialog = new AlertDialog.Builder(NitActivity.this)
				    .setTitle(title)
				    .setMessage(message)
				    .setPositiveButton("Ok", new DialogInterface.OnClickListener()
				    	{
				    		public void onClick(DialogInterface dialog, int whichButton) { }
				    	})
				    .create();

			    dialog.show();
			}
		};

		m_Handler.post(dlgRun);
    }
    
    static
    {
    	System.loadLibrary("nitandroid");
    }
}
