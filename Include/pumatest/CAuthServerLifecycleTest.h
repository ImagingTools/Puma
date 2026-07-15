// SPDX-License-Identifier: LicenseRef-Puma-Commercial
#pragma once


// Qt includes
#include <QtTest/QtTest>

// Puma includes
#include <AuthServerSdk/AuthServerSdk.h>


/**
	\brief Test class for AuthServerSdk lifecycle and configuration methods
	       that are not exercised by CAuthClientSdkTest/CPersonalAccessTokenTest.

	Those two classes keep a single AuthorizableServer running for their
	entire lifetime (the process never destroys I_ADD_TEST-registered test
	objects until exit) to serve as the backend for their AuthClientSdk
	calls, so they never call Stop() or exercise SetFeaturesFilePath().
	This class uses local, stack-scoped CAuthorizableServer instances so
	each test cleanly starts and tears down its own server.

	\note Unlike CAuthClientSdkTest/CPersonalAccessTokenTest, these tests
	      do not need a reachable Puma backend: Start()/Stop() only bind
	      local sockets and SetFeaturesFilePath() only touches the local
	      filesystem.

	\ingroup AuthServerSdk
*/
class CAuthServerLifecycleTest: public QObject
{
	Q_OBJECT
private slots:
	void StartStopRestartTest();
	void SetFeaturesFilePathMissingFileTest();
};
