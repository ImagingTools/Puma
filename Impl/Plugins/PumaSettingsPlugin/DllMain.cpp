// SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-Puma-Commercial
// ACF includes
#include <icomp/export.h>

// ImtCore includes
#include <imtbase/PluginInterface.h>
#include <imtservice/TConnectionCollectionPluginComponentImpl.h>
#include <imtservice/TObjectCollectionPluginComponentImpl.h>

// Local includes
#include <GeneratedFiles/PumaSettingsPlugin/CPumaSettingsPlugin.h>


IMT_REGISTER_PLUGIN(
			imtservice::IConnectionCollectionPlugin,
			imtservice::TConnectionCollectionPluginComponentImpl<CPumaSettingsPlugin>,
			ServiceSettings,
			PumaServerSettings);

IMT_REGISTER_PLUGIN(
			imtservice::IObjectCollectionPlugin,
			imtservice::TObjectCollectionPluginComponentImpl<CPumaSettingsPlugin>,
			ServiceLog,
			PumaServerLog);


