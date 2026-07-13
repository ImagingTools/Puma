// SPDX-License-Identifier: LicenseRef-Puma-Commercial
#include "CAuthServerLifecycleTest.h"


// ACF includes
#include <itest/CStandardTestExecutor.h>


void CAuthServerLifecycleTest::StartStopRestartTest()
{
	qDebug() << "=== [StartStopRestartTest] ===";

	AuthServerSdk::CAuthorizableServer server;

	AuthServerSdk::ServerConfig firstConfig;
	firstConfig.wsPort = 8891;
	firstConfig.httpPort = 7780;

	QVERIFY2(server.Start(firstConfig), "First Start() failed");
	QVERIFY2(server.Stop(), "Stop() after a successful Start() failed");

	// The same CAuthorizableServer instance must be reusable for a second
	// Start()/Stop() cycle after being stopped (a different port pair is
	// used here to avoid relying on the OS releasing the previous sockets
	// immediately, which is not guaranteed under TIME_WAIT).
	AuthServerSdk::ServerConfig secondConfig;
	secondConfig.wsPort = 8892;
	secondConfig.httpPort = 7781;

	QVERIFY2(server.Start(secondConfig), "Restart with a new configuration after Stop() failed");
	QVERIFY(server.Stop());
}


void CAuthServerLifecycleTest::SetFeaturesFilePathMissingFileTest()
{
	qDebug() << "=== [SetFeaturesFilePathMissingFileTest] ===";

	AuthServerSdk::CAuthorizableServer server;
	QVERIFY2(!server.SetFeaturesFilePath("Z:/definitely/does/not/exist/PumaFeatures.xml"),
		"SetFeaturesFilePath() unexpectedly succeeded for a nonexistent file");
}


I_ADD_TEST(CAuthServerLifecycleTest);
