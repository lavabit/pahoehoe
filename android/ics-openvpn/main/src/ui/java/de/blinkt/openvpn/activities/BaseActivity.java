/*
 * Copyright (c) 2012-2015 Arne Schwabe
 * Distributed under the GNU GPL v2 with additional terms. For full terms see the file doc/LICENSE.txt
 */

package de.blinkt.openvpn.activities;

import androidx.appcompat.app.AppCompatActivity;
import android.app.UiModeManager;
import android.content.res.Configuration;
import android.os.Bundle;
import android.view.Window;

public class BaseActivity extends AppCompatActivity {
    private boolean isAndroidTV() {
        final UiModeManager uiModeManager = (UiModeManager) getSystemService(Activity.UI_MODE_SERVICE);
        if (uiModeManager == null)
            return false;
        return uiModeManager.getCurrentModeType() == Configuration.UI_MODE_TYPE_TELEVISION;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        if (isAndroidTV()) {
            requestWindowFeature(Window.FEATURE_OPTIONS_PANEL);
        }
        super.onCreate(savedInstanceState);
    }
}
