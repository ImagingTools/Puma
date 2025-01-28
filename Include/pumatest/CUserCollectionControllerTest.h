#pragma once


// Qt includes
#include <QtTest/QTest>

// ImtCore includes
#include <pumatest/CTestBase.h>
#include <imtcrypt/CMD5HashCalculator.h>


class CUserCollectionControllerTest: public pumatest::CTestBase
{
	Q_OBJECT
private slots:
	void AddUserTest();
	void AddUserFailedTest();
	void RemoveUserTest();
	void RemoveUserFailedTest();
	void UpdateUserTest();
	void UpdateUserFailedTest();
	void GetUserListTest();

protected:
	virtual void SetData() const override;

private:
	imtcrypt::CMD5HashCalculator m_hashCalculator;
};


