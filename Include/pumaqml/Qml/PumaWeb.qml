// SPDX-License-Identifier: LicenseRef-Puma-Commercial
import QtQuick 2.0
import Acf 1.0
import com.imtcore.imtqml 1.0

Item {
	id: window

	anchors.fill: parent

	PumaMain {
		id: application

		anchors.fill: parent

		serverReady: true

		Component.onCompleted: {
			designProvider.setDesignSchema("Light")
		}

		function getServerUrl(){
			return context.location
		}
	}
}
