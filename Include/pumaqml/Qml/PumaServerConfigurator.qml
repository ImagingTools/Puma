import QtQuick 2.12
import imtgui 1.0
import pumaqml 1.0

Configurator {
	id: configurator

	Component.onCompleted: {
		// The LDAP page shows the groups selected by the LDAP group filter as tree.
		configurator.settingsController.registerPageExtension("Ldap", ldapGroupTreeComp)
	}

	Component {
		id: ldapGroupTreeComp

		LdapGroupTreeView {}
	}
}
