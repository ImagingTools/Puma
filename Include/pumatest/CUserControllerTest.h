// SPDX-License-Identifier: LicenseRef-Puma-Commercial
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


