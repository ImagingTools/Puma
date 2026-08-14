// SPDX-License-Identifier: LicenseRef-Puma-Commercial


// Qt includes
#include <QtCore/QRegularExpression>

// ImtCore includes
#include <imtcore/CApplicationRunner.h>
#include <imtcore/CImtCoreAuthorizableServerInitializer.h>

// Puma includes
#include <GeneratedFiles/PumaServerPg/CPumaServerPg.h>

static QtMessageHandler s_previousMessageHandler = nullptr;

static QString MaskSensitiveMessage(const QString& message)
{
	QString masked = message;

	static const QRegularExpression jsonRegex(
				R"((\"(password|passphrase|pwd)\"\s*:\s*)\"[^\"]*\")",
				QRegularExpression::CaseInsensitiveOption);
	masked.replace(jsonRegex, R"(\1"***")");

	static const QRegularExpression keyValueRegex(
				R"(((password|passphrase|pwd)\s*[:=]\s*)([^\s,&;]+))",
				QRegularExpression::CaseInsensitiveOption);
	masked.replace(keyValueRegex, "\\1***");

	return masked;
}

static void MaskingMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
	const QString masked = MaskSensitiveMessage(message);
	if (s_previousMessageHandler != nullptr) {
		s_previousMessageHandler(type, context, masked);
		return;
	}

	QByteArray localMsg = masked.toLocal8Bit();
	fprintf(stderr, "%s\n", localMsg.constData());
}


int main(int argc, char* argv[])
{
	s_previousMessageHandler = qInstallMessageHandler(MaskingMessageHandler);

	InitializeImtCoreAuthorizableServer();
	Q_INIT_RESOURCE(imtresthtml);
	Q_INIT_RESOURCE(PumaServerPg);

	CPumaServerPg instance;
	return imtcore::CApplicationRunner::Run(argc, argv, instance);
}
