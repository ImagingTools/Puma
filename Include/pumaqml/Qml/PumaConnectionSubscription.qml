import QtQuick 2.12
import Acf 1.0
import imtgui 1.0
import imtcontrols 1.0
import imtguigql 1.0

SubscriptionClient {
    id: pumaSub;

    property bool ok: window.subscriptionManager.status === 1;
    onOkChanged: {
        let subscriptionRequestId = "PumaWsConnection"
        var query = Gql.GqlRequest("subscription", subscriptionRequestId);
        var queryFields = Gql.GqlObject("notification");
        queryFields.InsertField("Id");
        query.AddField(queryFields);

        window.subscriptionManager.registerSubscription(query, pumaSub)
    }

    onStateChanged: {
        if (state === "Ready"){
            if (pumaSub.ContainsKey("data")){
                let localModel = pumaSub.GetData("data")

                if (localModel.ContainsKey("PumaWsConnection")){
                    localModel = localModel.GetData("PumaWsConnection")

                    if (localModel.ContainsKey("status")){
                        let status = localModel.GetData("status")
                        if (status === "Disconnected"){
                            window.pumaConnected = false;

                        }
                        else if (status === "Connected"){
                            window.pumaConnected = true;
                        }
                    }
                }
            }
        }
    }
}

