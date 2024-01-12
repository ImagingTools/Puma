import QtQuick 2.12
import Acf 1.0
import imtgui 1.0
import imtcontrols 1.0

Rectangle {
    id: window;

    anchors.fill: parent;

    color: Style.backgroundColor;

    property alias localSettings: preferenceDialog.settingsModel;
    signal settingsUpdate();

    onSettingsUpdate: {
        console.log("window onSettingsUpdate", localSettings.toJSON());
    }

    Component.onCompleted: {
        Style.setDecorators(decorators)
    }

    property Decorators decorators: decorators_
    Decorators {
        id: decorators_
    }

    MouseArea{
        anchors.fill: parent;

        onClicked: {
            preferenceDialog.visible = true;
        }
    }

    Preference {
        id: preferenceDialog;

        anchors.top: parent.top;
        anchors.left: parent.left;
        anchors.right: parent.right;
        anchors.bottom: buttons.top;

        onModelIsDirtyChanged: {
            buttons.setButtonState(Enums.ButtonType.Apply, preferenceDialog.modelIsDirty);
        }
    }

    ButtonsDialog {
        id: buttons;

        anchors.bottom: parent.bottom;
        anchors.right: parent.right;
        anchors.rightMargin: 20;
        anchors.bottomMargin: 10;

        Component.onCompleted: {
            buttons.addButton({"Id":Enums.ButtonType.Apply, "Name": qsTr("Apply"), "Enabled": false});
            buttons.addButton({"Id":Enums.ButtonType.Close, "Name": qsTr("Close"), "Enabled": true});
        }

        onButtonClicked: {
            if (buttonId == Enums.ButtonType.Apply){
                window.settingsUpdate();
                preferenceDialog.modelIsDirty = false;
            }
            else if (buttonId == Enums.ButtonType.Close){
                if (preferenceDialog.modelIsDirty){
                    modalDialogManager.openDialog(saveDialog, {"message": qsTr("Save all changes ?")});
                }
                else{
                    Qt.quit();
                }
            }
        }
    }

    ModalDialogManager {
        id: modalDialogManager;

        z: 30;

        anchors.fill: parent;
    }

//    Connections {
//        target: Qt.application;

//        onAboutToQuit: {
//            console.log("onAboutToQuit");

//            buttons.buttonClicked("Close");
//        }
//    }

    Component {
        id: saveDialog;

        MessageDialog {
            buttonsModel: ListModel{
                ListElement{Id: Enums.ButtonType.Yes; Name:qsTr("Yes"); Enabled: true}
                ListElement{Id: Enums.ButtonType.No; Name:qsTr("No"); Enabled: true}
                ListElement{Id: Enums.ButtonType.Cancel; Name:qsTr("Cancel"); Enabled: true}
            }

            onFinished: {
                if (buttonId == Enums.ButtonType.Yes){
                    window.settingsUpdate();

                    Qt.quit();
                }
                else if (buttonId == Enums.ButtonType.No){
                    Qt.quit();
                }
            }
        }
    }
}


