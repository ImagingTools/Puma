// SPDX-License-Identifier: LicenseRef-Puma-Commercial
#include "OidcTypes.h"

#include <QtCore/QJsonArray>


namespace Oidc
{


QJsonObject TokenClaims::ToJson() const
{
	QJsonObject json;

	if (!iss.isEmpty()){
		json["iss"] = iss;
	}
	if (!sub.isEmpty()){
		json["sub"] = sub;
	}
	if (!aud.isEmpty()){
		json["aud"] = aud;
	}
	if (iat != 0){
		json["iat"] = iat;
	}
	if (exp != 0){
		json["exp"] = exp;
	}
	if (auth_time != 0){
		json["auth_time"] = auth_time;
	}
	if (!nonce.isEmpty()){
		json["nonce"] = nonce;
	}

	// Profile claims
	if (!name.isEmpty()){
		json["name"] = name;
	}
	if (!email.isEmpty()){
		json["email"] = email;
	}
	if (!preferred_username.isEmpty()){
		json["preferred_username"] = preferred_username;
	}

	// Puma-specific claims
	if (!roles.isEmpty()){
		QJsonArray rolesArray;
		for (const QString& role : roles){
			rolesArray.append(role);
		}
		json["roles"] = rolesArray;
	}
	if (!groups.isEmpty()){
		QJsonArray groupsArray;
		for (const QString& group : groups){
			groupsArray.append(group);
		}
		json["groups"] = groupsArray;
	}
	if (!permissions.isEmpty()){
		QJsonArray permsArray;
		for (const QByteArray& perm : permissions){
			permsArray.append(QString::fromUtf8(perm));
		}
		json["permissions"] = permsArray;
	}

	return json;
}


void TokenClaims::FromJson(const QJsonObject& json)
{
	iss = json["iss"].toString();
	sub = json["sub"].toString();
	aud = json["aud"].toString();
	iat = static_cast<qint64>(json["iat"].toDouble());
	exp = static_cast<qint64>(json["exp"].toDouble());
	auth_time = static_cast<qint64>(json["auth_time"].toDouble());
	nonce = json["nonce"].toString();

	name = json["name"].toString();
	email = json["email"].toString();
	preferred_username = json["preferred_username"].toString();

	roles.clear();
	const QJsonArray rolesArray = json["roles"].toArray();
	for (const QJsonValue& val : rolesArray){
		roles.append(val.toString());
	}

	groups.clear();
	const QJsonArray groupsArray = json["groups"].toArray();
	for (const QJsonValue& val : groupsArray){
		groups.append(val.toString());
	}

	permissions.clear();
	const QJsonArray permsArray = json["permissions"].toArray();
	for (const QJsonValue& val : permsArray){
		permissions.append(val.toString().toUtf8());
	}
}


QJsonObject TokenResponse::ToJson() const
{
	QJsonObject json;
	json["access_token"] = accessToken;
	json["token_type"] = tokenType;
	json["expires_in"] = expiresIn;

	if (!refreshToken.isEmpty()){
		json["refresh_token"] = refreshToken;
	}
	if (!idToken.isEmpty()){
		json["id_token"] = idToken;
	}
	if (!scope.isEmpty()){
		json["scope"] = scope;
	}

	return json;
}


QJsonObject IntrospectionResponse::ToJson() const
{
	QJsonObject json;
	json["active"] = active;

	if (active){
		if (!scope.isEmpty()){
			json["scope"] = scope;
		}
		if (!clientId.isEmpty()){
			json["client_id"] = clientId;
		}
		if (!username.isEmpty()){
			json["username"] = username;
		}
		if (!tokenType.isEmpty()){
			json["token_type"] = tokenType;
		}
		if (exp != 0){
			json["exp"] = exp;
		}
		if (iat != 0){
			json["iat"] = iat;
		}
		if (!sub.isEmpty()){
			json["sub"] = sub;
		}
		if (!iss.isEmpty()){
			json["iss"] = iss;
		}
	}

	return json;
}


} // namespace Oidc
