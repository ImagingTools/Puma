// SPDX-License-Identifier: LicenseRef-Puma-Commercial
import QtQuick 2.0
import Acf 1.0
import com.imtcore.imtqml 1.0
import imtgui 1.0
import imtauthgui 1.0
import imtguigql 1.0
import imtcontrols 1.0

/**
	The console of the authorization server: the products connected to it and their administration.
*/
ApplicationMain {
	id: window

	useWebSocketSubscription: true
	canRecoveryPassword: false
}
