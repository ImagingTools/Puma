import QtQuick 2.12
import Acf 1.0
import imtgui 1.0

Rectangle {
    id: window;

    anchors.fill: parent;

    color: Style.backgroundColor;

    property alias localSettings: preferenceDialog.settingsModel;
    signal settingsUpdate();

    onSettingsUpdate: {
        console.log("window onSettingsUpdate", localSettings.toJSON());
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
            buttons.setButtonState("Apply", preferenceDialog.modelIsDirty);
        }
    }

    ButtonsDialog {
        id: buttons;

        anchors.bottom: parent.bottom;
        anchors.right: parent.right;
        anchors.rightMargin: 20;
        anchors.bottomMargin: 10;

        Component.onCompleted: {
            buttons.addButton({"Id":"Apply", "Name": qsTr("Apply"), "Enabled": false});
            buttons.addButton({"Id":"Close", "Name": qsTr("Close"), "Enabled": true});
        }

        onButtonClicked: {
            if (buttonId === "Apply"){
                window.settingsUpdate();
                preferenceDialog.modelIsDirty = false;

                //                buttons.setButtonState("Apply", false);
            }
            else if (buttonId === "Close"){
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
            Component.onCompleted: {
                console.log("saveDialog onCompleted");
                buttons.addButton({"Id":"Cancel", "Name":"Cancel", "Enabled": true});
            }

            onFinished: {
                console.log("saveDialog onFinished", buttonId);
                if (buttonId == "Yes"){
                    window.settingsUpdate();

                    Qt.quit();
                }
                else if (buttonId == "No"){
                    Qt.quit();
                }
            }
        }
    }
}


