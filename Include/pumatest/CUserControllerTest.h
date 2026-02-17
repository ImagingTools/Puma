// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-Puma-Commercial
#pragma once


// ImtCore includes
#include <GeneratedFiles/imtauthsdl/SDL/1.0/CPP/Users.h>

// Puma includes
#include <pumatest/CTestBase.h>


class CUserControllerTest: public pumatest::CTestBase
{
	Q_OBJECT
private slots:
	void ChangePasswordTest();
	void ChangePasswordFailedTest();
	void RegisterUserTest();
	void RegisterUserFailedTest();
	void CheckEmailTest();
	void CheckEmailFailedTest();
	void CreateSuperuserTest();
	void CreateSuperuserFailedTest();
	void CheckSuperuserTest();
	void CheckSuperuserFailedTest();
};


