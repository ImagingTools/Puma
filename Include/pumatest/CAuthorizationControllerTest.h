// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-Puma-Commercial
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


