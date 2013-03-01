package com.example.nittestandroid;

import nit.*;
import android.app.*;
import android.content.*;
import android.os.*;
import android.view.*;

public class MainActivity extends NitActivity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		return true;
	}
	
	@Override
	public void onBackPressed()
	{
		AlertDialog.Builder confirmDlgBuilder = new AlertDialog.Builder(this);
		confirmDlgBuilder.setMessage("Are you sure to quit NitActivity?");
		confirmDlgBuilder.setCancelable(false);
		confirmDlgBuilder.setNegativeButton(android.R.string.no, null);
		confirmDlgBuilder.setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
			@Override public void onClick(DialogInterface dialog, int which) {
				MainActivity.this.finish();
			}
		});
		confirmDlgBuilder.create().show();

		// no super call
	}
}
