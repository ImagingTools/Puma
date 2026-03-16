// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-Puma-Commercial
// Qt includes
#include <QtCore/QDir>
#include <QtCore/QCoreApplication>
#include <QtCore/QRegularExpression>
#include <cstdio>

// ACF includes
#include <ibase/IApplication.h>

// ImtCore includes
#include <GeneratedFiles/PumaServerSl/CPumaServerSl.h>

static QtMessageHandler s_previousMessageHandler = nullptr;

static QString MaskSensitiveMessage(const QString& message)
{
	QString masked = message;

	static const QRegularExpression jsonRegex(
		R"((\"(password|passphrase|pwd)\"\s*:\s*)\"[^\"]*\")",
		QRegularExpression::CaseInsensitiveOption);
	masked.replace(jsonRegex, "\\1\"***\"");

	static const QRegularExpression keyValueRegex(
		R"(((password|passphrase|pwd)\s*[:=]\s*)([^\s,&;]+))",
		QRegularExpression::CaseInsensitiveOption);
	masked.replace(keyValueRegex, "\\1***");

	return masked;
}

static void MaskingMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
	const QString masked = MaskSensitiveMessage(message);
	if (s_previousMessageHandler != nullptr){
		s_previousMessageHandler(type, context, masked);
		return;
	}

	QByteArray localMsg = masked.toLocal8Bit();
	fprintf(stderr, "%s\n", localMsg.constData());
}


int main(int argc, char *argv[])
{
	s_previousMessageHandler = qInstallMessageHandler(MaskingMessageHandler);

	Q_INIT_RESOURCE(imtresthtml);
	Q_INIT_RESOURCE(imtdb);
	Q_INIT_RESOURCE(imtauthdb);
	Q_INIT_RESOURCE(imtbase);

	CPumaServerSl instance;

	ibase::IApplication* applicationPtr = instance.GetInterface<ibase::IApplication>();
	if (applicationPtr != nullptr){
		return applicationPtr->Execute(argc, argv);
	}

	return -1;
}
