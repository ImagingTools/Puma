// SPDX-License-Identifier: LicenseRef-Puma-Commercial
#pragma once


// Puma includes
#include <pumatest/CTestBase.h>


class CAuthorizationControllerTest: public pumatest::CTestBase
{
	Q_OBJECT
private slots:
	void AuthorizationTest();
	void AuthorizationFailedTest();
};


