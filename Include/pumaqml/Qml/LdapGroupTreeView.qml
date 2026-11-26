import QtQuick 2.12
import Acf 1.0
import com.imtcore.imtqml 1.0
import com.imtcore.imtauth 1.0
import imtgui 1.0
import imtcontrols 1.0
import imtbaseImtBaseTypesSdl 1.0

// Preview of LDAP groups selected by the base DN and name filters.
Item {
	id: root

	height: content.height

	// JSON data of the LDAP settings page (set by the Preference page)
	property string pageData
	property string groupFilterParamId: "LdapGroupFilter"
	property string groupNameFilterParamId: "LdapGroupNameFilter"

	readonly property string baseDn: root.__groupFilterParam.m_text ? root.__groupFilterParam.m_text.trim() : ""
	readonly property string nameFilter: root.__groupNameFilterParam.m_text || ""
	property bool __isFilterInitialized: false

	property ParamsSet __pageParamsSet: ParamsSet {}
	property TextParam __groupFilterParam: TextParam {}
	property TextParam __groupNameFilterParam: TextParam {}

	onPageDataChanged: {
		root.__updateFilters()
	}
	onBaseDnChanged: {
		// The groups are read again after a change of the filter, not on each opening of the configurator.
		if (root.__isFilterInitialized){
			refreshTimer.restart()
		}
	}

	onNameFilterChanged: {
		if (root.__isFilterInitialized){
			refreshTimer.restart()
		}
	}

	Component.onCompleted: {
		root.__updateFilters()
		root.__isFilterInitialized = true
	}

	function refresh(){
		refreshTimer.stop()
		groupTreeController.refresh(root.baseDn, root.nameFilter)
	}

	function cancel(){
		refreshTimer.stop()
		groupTreeController.cancel()
	}

	function __updateFilters(){
		let text = ""
		let nameText = ""
		if (root.pageData !== "" && root.__pageParamsSet.createFromJson(root.pageData)){
			for (let i = 0; i < root.__pageParamsSet.m_parameters.count; ++i){
				let parameterInfo = root.__pageParamsSet.m_parameters.get(i).item
				if (parameterInfo.m_id === root.groupFilterParamId){
					if (root.__groupFilterParam.createFromJson(parameterInfo.m_data)){
						text = root.__groupFilterParam.m_text
					}

				}
				else if (parameterInfo.m_id === root.groupNameFilterParamId){
					if (root.__groupNameFilterParam.createFromJson(parameterInfo.m_data)){
						nameText = root.__groupNameFilterParam.m_text
					}
				}
			}
		}

		if (root.__groupFilterParam.m_text !== text){
			root.__groupFilterParam.m_text = text
		}
		if (root.__groupNameFilterParam.m_text !== nameText){
			root.__groupNameFilterParam.m_text = nameText
		}
	}

	LdapGroupTreeController {
		id: groupTreeController
	}

	Component {
		id: progressComp

		// The number of groups is unknown before the reading is finished.
		ProgressBar {
			indeterminate: true
			hasTitle: false
			hasText: false
			barHeight: Style.marginM
		}
	}

	Timer {
		id: refreshTimer
		interval: 500
		onTriggered: {
			root.refresh()
		}
	}

	Column {
		id: content
		width: root.width
		spacing: Style.marginXL

		GroupHeaderView {
			width: content.width
			title: qsTr("Filtered LDAP groups")
			groupView: group
		}

		GroupElementView {
			id: group
			width: content.width

			ButtonElementView {
				id: refreshButton
				width: group.width
				name: groupTreeController.busy ? qsTr("Reading LDAP groups...") : qsTr("LDAP groups below %1").arg(root.baseDn !== "" ? root.baseDn : qsTr("the domain of the server"))
				description: groupTreeController.busy ? qsTr("%1 groups read").arg(groupTreeController.readGroupCount)
						: groupTreeController.errorText !== "" ? groupTreeController.errorText
						: groupTreeController.cancelled ? qsTr("Reading of the LDAP groups was cancelled. Press 'Show groups' to read them again")
						: groupTreeController.groupCount > 0 ? qsTr("%1 groups match the current filters").arg(groupTreeController.groupCount)
						: qsTr("Press 'Show groups' to read the groups selected by the LDAP group filters")
				descriptionColor: !groupTreeController.busy && groupTreeController.errorText !== "" ? Style.errorTextColor : Style.inactiveTextColor
				// While reading, the button cancels the reading.
				text: groupTreeController.busy ? qsTr("Cancel") : qsTr("Show groups")
				onClicked: {
					if (groupTreeController.busy){
						root.cancel()
					}
					else{
						root.refresh()
					}
				}

				bottomComp: groupTreeController.busy ? progressComp : undefined
			}

			ElementView {
				id: treeElement
				width: group.width
				visible: groupTreeController.groups.length > 0

				bottomComp: Component {
					Item {
						height: Math.min(groupTreeView.contentHeight, Style.sizeHintL)

						ListView {
							id: groupTreeView
							anchors.fill: parent
							anchors.rightMargin: treeScrollbar.width + Style.marginS
							clip: true
							spacing: Style.marginS
							boundsBehavior: Flickable.StopAtBounds
							model: groupTreeController.groups

							delegate: Item {
								width: groupTreeView.width
								height: groupText.implicitHeight

								TextEdit {
									id: groupText
									anchors.left: parent.left
									anchors.leftMargin: Math.min(modelData.level * Style.marginXL, parent.width / 3)
									anchors.right: parent.right
									text: (modelData.level > 0 ? "\u2514 " : "") + modelData.name
											+ (modelData.description !== "" ? " — " + modelData.description : "")
									textFormat: TextEdit.PlainText
									readOnly: true
									selectByMouse: true
									wrapMode: TextEdit.Wrap
									color: Style.textColor
									font.family: modelData.level === 0 ? Style.fontFamilyBold : Style.fontFamily
									font.pixelSize: Style.fontSizeM
								}
							}
						}

						CustomScrollbar {
							id: treeScrollbar
							anchors.right: parent.right
							anchors.top: parent.top
							secondSize: Style.marginM
							targetItem: groupTreeView
						}
					}
				}
			}
		}
	}
}
