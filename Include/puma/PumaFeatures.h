#pragma once


// Qt includes
#include <QtCore/qglobal.h>

// ImtCore includes
#include <imtlic/CProductInfo.h>
#include <imtlic/CFeatureInfo.h>


namespace puma
{


/**
	The permissions of the authorization server: the products give them to their roles as far as their tokens allow.
*/
static void FillProduct(imtlic::IProductInfo& productInfo){
	productInfo.SetProductId("Puma");
	productInfo.SetName(QT_TRANSLATE_NOOP("Product", "Puma"));
	productInfo.SetCategoryId("Software");

	istd::TDelPtr<imtlic::CIdentifiableFeatureInfo> administrationFeatureInfo;
	administrationFeatureInfo.SetPtr(new imtlic::CIdentifiableFeatureInfo);
	administrationFeatureInfo->SetObjectUuid("ce4a7f72-5303-4044-b25c-3a02020eebd3");
	administrationFeatureInfo->SetFeatureId("Administration");
	administrationFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Administration"));
	administrationFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Provides access to administration pages."));
	administrationFeatureInfo->SetOptional(false);
	administrationFeatureInfo->SetIsPermission(true);

	istd::TDelPtr<imtlic::CFeatureInfo> administrationUsersFeatureInfo;
	administrationUsersFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationUsersFeatureInfo->SetFeatureId("Users");
	administrationUsersFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Users"));
	administrationUsersFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Provides access to the users."));
	administrationUsersFeatureInfo->SetOptional(false);
	administrationUsersFeatureInfo->SetIsPermission(true);

	istd::TDelPtr<imtlic::CFeatureInfo> administrationUsersViewPageFeatureInfo;
	administrationUsersViewPageFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationUsersViewPageFeatureInfo->SetFeatureId("ViewPage");
	administrationUsersViewPageFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "View Users Page"));
	administrationUsersViewPageFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows opening the users page."));
	administrationUsersViewPageFeatureInfo->SetOptional(false);
	administrationUsersViewPageFeatureInfo->SetIsPermission(true);
	administrationUsersViewPageFeatureInfo->SetRequirements(QByteArray("/Administration/Users/View").split(';'));

	administrationUsersFeatureInfo->InsertSubFeature(administrationUsersViewPageFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationUsersViewFeatureInfo;
	administrationUsersViewFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationUsersViewFeatureInfo->SetFeatureId("View");
	administrationUsersViewFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "View Users"));
	administrationUsersViewFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows viewing Users."));
	administrationUsersViewFeatureInfo->SetOptional(false);
	administrationUsersViewFeatureInfo->SetIsPermission(true);

	administrationUsersFeatureInfo->InsertSubFeature(administrationUsersViewFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationUsersViewHistoryFeatureInfo;
	administrationUsersViewHistoryFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationUsersViewHistoryFeatureInfo->SetFeatureId("ViewHistory");
	administrationUsersViewHistoryFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "View User History"));
	administrationUsersViewHistoryFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows viewing User History."));
	administrationUsersViewHistoryFeatureInfo->SetOptional(false);
	administrationUsersViewHistoryFeatureInfo->SetIsPermission(true);
	administrationUsersViewHistoryFeatureInfo->SetRequirements(QByteArray("/Administration/Users/View").split(';'));

	administrationUsersFeatureInfo->InsertSubFeature(administrationUsersViewHistoryFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationUsersEditFeatureInfo;
	administrationUsersEditFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationUsersEditFeatureInfo->SetFeatureId("Edit");
	administrationUsersEditFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Edit User"));
	administrationUsersEditFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows editing User."));
	administrationUsersEditFeatureInfo->SetOptional(false);
	administrationUsersEditFeatureInfo->SetIsPermission(true);
	administrationUsersEditFeatureInfo->SetRequirements(QByteArray("/Administration/Users/View").split(';'));

	istd::TDelPtr<imtlic::CFeatureInfo> administrationUsersEditAddFeatureInfo;
	administrationUsersEditAddFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationUsersEditAddFeatureInfo->SetFeatureId("Add");
	administrationUsersEditAddFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Add User"));
	administrationUsersEditAddFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows adding User."));
	administrationUsersEditAddFeatureInfo->SetOptional(false);
	administrationUsersEditAddFeatureInfo->SetIsPermission(true);
	administrationUsersEditAddFeatureInfo->SetRequirements(QByteArray("/Administration/Roles/View;/Administration/Groups/View").split(';'));

	administrationUsersEditFeatureInfo->InsertSubFeature(administrationUsersEditAddFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationUsersEditChangeFeatureInfo;
	administrationUsersEditChangeFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationUsersEditChangeFeatureInfo->SetFeatureId("Change");
	administrationUsersEditChangeFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Change User"));
	administrationUsersEditChangeFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows changing User."));
	administrationUsersEditChangeFeatureInfo->SetOptional(false);
	administrationUsersEditChangeFeatureInfo->SetIsPermission(true);
	administrationUsersEditChangeFeatureInfo->SetRequirements(QByteArray("/Administration/Roles/View;/Administration/Groups/View").split(';'));

	administrationUsersEditFeatureInfo->InsertSubFeature(administrationUsersEditChangeFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationUsersEditRemoveFeatureInfo;
	administrationUsersEditRemoveFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationUsersEditRemoveFeatureInfo->SetFeatureId("Remove");
	administrationUsersEditRemoveFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Remove User"));
	administrationUsersEditRemoveFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows removing User."));
	administrationUsersEditRemoveFeatureInfo->SetOptional(false);
	administrationUsersEditRemoveFeatureInfo->SetIsPermission(true);

	administrationUsersEditFeatureInfo->InsertSubFeature(administrationUsersEditRemoveFeatureInfo.PopPtr());

	administrationUsersFeatureInfo->InsertSubFeature(administrationUsersEditFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationUsersRevisionsFeatureInfo;
	administrationUsersRevisionsFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationUsersRevisionsFeatureInfo->SetFeatureId("Revisions");
	administrationUsersRevisionsFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Users Revisions"));
	administrationUsersRevisionsFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Provides access to the revisions of users."));
	administrationUsersRevisionsFeatureInfo->SetOptional(false);
	administrationUsersRevisionsFeatureInfo->SetIsPermission(true);

	istd::TDelPtr<imtlic::CFeatureInfo> administrationUsersRevisionsViewFeatureInfo;
	administrationUsersRevisionsViewFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationUsersRevisionsViewFeatureInfo->SetFeatureId("View");
	administrationUsersRevisionsViewFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "View Users Revisions"));
	administrationUsersRevisionsViewFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows viewing the revisions of users."));
	administrationUsersRevisionsViewFeatureInfo->SetOptional(false);
	administrationUsersRevisionsViewFeatureInfo->SetIsPermission(true);
	administrationUsersRevisionsViewFeatureInfo->SetRequirements(QByteArray("/Administration/Users/View").split(';'));

	administrationUsersRevisionsFeatureInfo->InsertSubFeature(administrationUsersRevisionsViewFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationUsersRevisionsRestoreFeatureInfo;
	administrationUsersRevisionsRestoreFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationUsersRevisionsRestoreFeatureInfo->SetFeatureId("Restore");
	administrationUsersRevisionsRestoreFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Restore Users Revision"));
	administrationUsersRevisionsRestoreFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows restoring a revision of users."));
	administrationUsersRevisionsRestoreFeatureInfo->SetOptional(false);
	administrationUsersRevisionsRestoreFeatureInfo->SetIsPermission(true);
	administrationUsersRevisionsRestoreFeatureInfo->SetRequirements(QByteArray("/Administration/Users/Revisions/View;/Administration/Users/Edit/Change").split(';'));

	administrationUsersRevisionsFeatureInfo->InsertSubFeature(administrationUsersRevisionsRestoreFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationUsersRevisionsDeleteFeatureInfo;
	administrationUsersRevisionsDeleteFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationUsersRevisionsDeleteFeatureInfo->SetFeatureId("Delete");
	administrationUsersRevisionsDeleteFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Delete Users Revision"));
	administrationUsersRevisionsDeleteFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows deleting a revision of users."));
	administrationUsersRevisionsDeleteFeatureInfo->SetOptional(false);
	administrationUsersRevisionsDeleteFeatureInfo->SetIsPermission(true);
	administrationUsersRevisionsDeleteFeatureInfo->SetRequirements(QByteArray("/Administration/Users/Revisions/View").split(';'));

	administrationUsersRevisionsFeatureInfo->InsertSubFeature(administrationUsersRevisionsDeleteFeatureInfo.PopPtr());

	administrationUsersFeatureInfo->InsertSubFeature(administrationUsersRevisionsFeatureInfo.PopPtr());

	administrationFeatureInfo->InsertSubFeature(administrationUsersFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationRolesFeatureInfo;
	administrationRolesFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationRolesFeatureInfo->SetFeatureId("Roles");
	administrationRolesFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Roles"));
	administrationRolesFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Provides access to the roles."));
	administrationRolesFeatureInfo->SetOptional(false);
	administrationRolesFeatureInfo->SetIsPermission(true);

	istd::TDelPtr<imtlic::CFeatureInfo> administrationRolesViewPageFeatureInfo;
	administrationRolesViewPageFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationRolesViewPageFeatureInfo->SetFeatureId("ViewPage");
	administrationRolesViewPageFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "View Roles Page"));
	administrationRolesViewPageFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows opening the roles page."));
	administrationRolesViewPageFeatureInfo->SetOptional(false);
	administrationRolesViewPageFeatureInfo->SetIsPermission(true);
	administrationRolesViewPageFeatureInfo->SetRequirements(QByteArray("/Administration/Roles/View").split(';'));

	administrationRolesFeatureInfo->InsertSubFeature(administrationRolesViewPageFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationRolesViewFeatureInfo;
	administrationRolesViewFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationRolesViewFeatureInfo->SetFeatureId("View");
	administrationRolesViewFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "View Roles"));
	administrationRolesViewFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows viewing Roles."));
	administrationRolesViewFeatureInfo->SetOptional(false);
	administrationRolesViewFeatureInfo->SetIsPermission(true);

	administrationRolesFeatureInfo->InsertSubFeature(administrationRolesViewFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationRolesViewHistoryFeatureInfo;
	administrationRolesViewHistoryFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationRolesViewHistoryFeatureInfo->SetFeatureId("ViewHistory");
	administrationRolesViewHistoryFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "View Role History"));
	administrationRolesViewHistoryFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows viewing Role History."));
	administrationRolesViewHistoryFeatureInfo->SetOptional(false);
	administrationRolesViewHistoryFeatureInfo->SetIsPermission(true);
	administrationRolesViewHistoryFeatureInfo->SetRequirements(QByteArray("/Administration/Roles/View").split(';'));

	administrationRolesFeatureInfo->InsertSubFeature(administrationRolesViewHistoryFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationRolesEditFeatureInfo;
	administrationRolesEditFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationRolesEditFeatureInfo->SetFeatureId("Edit");
	administrationRolesEditFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Edit Role"));
	administrationRolesEditFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows editing Role."));
	administrationRolesEditFeatureInfo->SetOptional(false);
	administrationRolesEditFeatureInfo->SetIsPermission(true);
	administrationRolesEditFeatureInfo->SetRequirements(QByteArray("/Administration/Roles/View").split(';'));

	istd::TDelPtr<imtlic::CFeatureInfo> administrationRolesEditAddFeatureInfo;
	administrationRolesEditAddFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationRolesEditAddFeatureInfo->SetFeatureId("Add");
	administrationRolesEditAddFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Add Role"));
	administrationRolesEditAddFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows adding Role."));
	administrationRolesEditAddFeatureInfo->SetOptional(false);
	administrationRolesEditAddFeatureInfo->SetIsPermission(true);

	administrationRolesEditFeatureInfo->InsertSubFeature(administrationRolesEditAddFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationRolesEditChangeFeatureInfo;
	administrationRolesEditChangeFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationRolesEditChangeFeatureInfo->SetFeatureId("Change");
	administrationRolesEditChangeFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Change Role"));
	administrationRolesEditChangeFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows changing Role."));
	administrationRolesEditChangeFeatureInfo->SetOptional(false);
	administrationRolesEditChangeFeatureInfo->SetIsPermission(true);

	administrationRolesEditFeatureInfo->InsertSubFeature(administrationRolesEditChangeFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationRolesEditRemoveFeatureInfo;
	administrationRolesEditRemoveFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationRolesEditRemoveFeatureInfo->SetFeatureId("Remove");
	administrationRolesEditRemoveFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Remove Role"));
	administrationRolesEditRemoveFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows removing Role."));
	administrationRolesEditRemoveFeatureInfo->SetOptional(false);
	administrationRolesEditRemoveFeatureInfo->SetIsPermission(true);

	administrationRolesEditFeatureInfo->InsertSubFeature(administrationRolesEditRemoveFeatureInfo.PopPtr());

	administrationRolesFeatureInfo->InsertSubFeature(administrationRolesEditFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationRolesRevisionsFeatureInfo;
	administrationRolesRevisionsFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationRolesRevisionsFeatureInfo->SetFeatureId("Revisions");
	administrationRolesRevisionsFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Roles Revisions"));
	administrationRolesRevisionsFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Provides access to the revisions of roles."));
	administrationRolesRevisionsFeatureInfo->SetOptional(false);
	administrationRolesRevisionsFeatureInfo->SetIsPermission(true);

	istd::TDelPtr<imtlic::CFeatureInfo> administrationRolesRevisionsViewFeatureInfo;
	administrationRolesRevisionsViewFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationRolesRevisionsViewFeatureInfo->SetFeatureId("View");
	administrationRolesRevisionsViewFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "View Roles Revisions"));
	administrationRolesRevisionsViewFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows viewing the revisions of roles."));
	administrationRolesRevisionsViewFeatureInfo->SetOptional(false);
	administrationRolesRevisionsViewFeatureInfo->SetIsPermission(true);
	administrationRolesRevisionsViewFeatureInfo->SetRequirements(QByteArray("/Administration/Roles/View").split(';'));

	administrationRolesRevisionsFeatureInfo->InsertSubFeature(administrationRolesRevisionsViewFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationRolesRevisionsRestoreFeatureInfo;
	administrationRolesRevisionsRestoreFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationRolesRevisionsRestoreFeatureInfo->SetFeatureId("Restore");
	administrationRolesRevisionsRestoreFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Restore Roles Revision"));
	administrationRolesRevisionsRestoreFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows restoring a revision of roles."));
	administrationRolesRevisionsRestoreFeatureInfo->SetOptional(false);
	administrationRolesRevisionsRestoreFeatureInfo->SetIsPermission(true);
	administrationRolesRevisionsRestoreFeatureInfo->SetRequirements(QByteArray("/Administration/Roles/Revisions/View;/Administration/Roles/Edit/Change").split(';'));

	administrationRolesRevisionsFeatureInfo->InsertSubFeature(administrationRolesRevisionsRestoreFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationRolesRevisionsDeleteFeatureInfo;
	administrationRolesRevisionsDeleteFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationRolesRevisionsDeleteFeatureInfo->SetFeatureId("Delete");
	administrationRolesRevisionsDeleteFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Delete Roles Revision"));
	administrationRolesRevisionsDeleteFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows deleting a revision of roles."));
	administrationRolesRevisionsDeleteFeatureInfo->SetOptional(false);
	administrationRolesRevisionsDeleteFeatureInfo->SetIsPermission(true);
	administrationRolesRevisionsDeleteFeatureInfo->SetRequirements(QByteArray("/Administration/Roles/Revisions/View").split(';'));

	administrationRolesRevisionsFeatureInfo->InsertSubFeature(administrationRolesRevisionsDeleteFeatureInfo.PopPtr());

	administrationRolesFeatureInfo->InsertSubFeature(administrationRolesRevisionsFeatureInfo.PopPtr());

	administrationFeatureInfo->InsertSubFeature(administrationRolesFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationGroupsFeatureInfo;
	administrationGroupsFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationGroupsFeatureInfo->SetFeatureId("Groups");
	administrationGroupsFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Groups"));
	administrationGroupsFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Provides access to the groups."));
	administrationGroupsFeatureInfo->SetOptional(false);
	administrationGroupsFeatureInfo->SetIsPermission(true);

	istd::TDelPtr<imtlic::CFeatureInfo> administrationGroupsViewPageFeatureInfo;
	administrationGroupsViewPageFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationGroupsViewPageFeatureInfo->SetFeatureId("ViewPage");
	administrationGroupsViewPageFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "View Groups Page"));
	administrationGroupsViewPageFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows opening the groups page."));
	administrationGroupsViewPageFeatureInfo->SetOptional(false);
	administrationGroupsViewPageFeatureInfo->SetIsPermission(true);
	administrationGroupsViewPageFeatureInfo->SetRequirements(QByteArray("/Administration/Groups/View").split(';'));

	administrationGroupsFeatureInfo->InsertSubFeature(administrationGroupsViewPageFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationGroupsViewFeatureInfo;
	administrationGroupsViewFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationGroupsViewFeatureInfo->SetFeatureId("View");
	administrationGroupsViewFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "View Groups"));
	administrationGroupsViewFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows viewing Groups."));
	administrationGroupsViewFeatureInfo->SetOptional(false);
	administrationGroupsViewFeatureInfo->SetIsPermission(true);

	administrationGroupsFeatureInfo->InsertSubFeature(administrationGroupsViewFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationGroupsViewHistoryFeatureInfo;
	administrationGroupsViewHistoryFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationGroupsViewHistoryFeatureInfo->SetFeatureId("ViewHistory");
	administrationGroupsViewHistoryFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "View Group History"));
	administrationGroupsViewHistoryFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows viewing Group History."));
	administrationGroupsViewHistoryFeatureInfo->SetOptional(false);
	administrationGroupsViewHistoryFeatureInfo->SetIsPermission(true);
	administrationGroupsViewHistoryFeatureInfo->SetRequirements(QByteArray("/Administration/Groups/View").split(';'));

	administrationGroupsFeatureInfo->InsertSubFeature(administrationGroupsViewHistoryFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationGroupsEditFeatureInfo;
	administrationGroupsEditFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationGroupsEditFeatureInfo->SetFeatureId("Edit");
	administrationGroupsEditFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Edit Group"));
	administrationGroupsEditFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows editing Group."));
	administrationGroupsEditFeatureInfo->SetOptional(false);
	administrationGroupsEditFeatureInfo->SetIsPermission(true);
	administrationGroupsEditFeatureInfo->SetRequirements(QByteArray("/Administration/Groups/View").split(';'));

	istd::TDelPtr<imtlic::CFeatureInfo> administrationGroupsEditAddFeatureInfo;
	administrationGroupsEditAddFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationGroupsEditAddFeatureInfo->SetFeatureId("Add");
	administrationGroupsEditAddFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Add Group"));
	administrationGroupsEditAddFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows adding Group."));
	administrationGroupsEditAddFeatureInfo->SetOptional(false);
	administrationGroupsEditAddFeatureInfo->SetIsPermission(true);
	administrationGroupsEditAddFeatureInfo->SetRequirements(QByteArray("/Administration/Users/View;/Administration/Roles/View").split(';'));

	administrationGroupsEditFeatureInfo->InsertSubFeature(administrationGroupsEditAddFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationGroupsEditChangeFeatureInfo;
	administrationGroupsEditChangeFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationGroupsEditChangeFeatureInfo->SetFeatureId("Change");
	administrationGroupsEditChangeFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Change Group"));
	administrationGroupsEditChangeFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows changing Group."));
	administrationGroupsEditChangeFeatureInfo->SetOptional(false);
	administrationGroupsEditChangeFeatureInfo->SetIsPermission(true);
	administrationGroupsEditChangeFeatureInfo->SetRequirements(QByteArray("/Administration/Users/View;/Administration/Roles/View").split(';'));

	administrationGroupsEditFeatureInfo->InsertSubFeature(administrationGroupsEditChangeFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationGroupsEditRemoveFeatureInfo;
	administrationGroupsEditRemoveFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationGroupsEditRemoveFeatureInfo->SetFeatureId("Remove");
	administrationGroupsEditRemoveFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Remove Group"));
	administrationGroupsEditRemoveFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows removing Group."));
	administrationGroupsEditRemoveFeatureInfo->SetOptional(false);
	administrationGroupsEditRemoveFeatureInfo->SetIsPermission(true);

	administrationGroupsEditFeatureInfo->InsertSubFeature(administrationGroupsEditRemoveFeatureInfo.PopPtr());

	administrationGroupsFeatureInfo->InsertSubFeature(administrationGroupsEditFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationGroupsRevisionsFeatureInfo;
	administrationGroupsRevisionsFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationGroupsRevisionsFeatureInfo->SetFeatureId("Revisions");
	administrationGroupsRevisionsFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Groups Revisions"));
	administrationGroupsRevisionsFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Provides access to the revisions of groups."));
	administrationGroupsRevisionsFeatureInfo->SetOptional(false);
	administrationGroupsRevisionsFeatureInfo->SetIsPermission(true);

	istd::TDelPtr<imtlic::CFeatureInfo> administrationGroupsRevisionsViewFeatureInfo;
	administrationGroupsRevisionsViewFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationGroupsRevisionsViewFeatureInfo->SetFeatureId("View");
	administrationGroupsRevisionsViewFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "View Groups Revisions"));
	administrationGroupsRevisionsViewFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows viewing the revisions of groups."));
	administrationGroupsRevisionsViewFeatureInfo->SetOptional(false);
	administrationGroupsRevisionsViewFeatureInfo->SetIsPermission(true);
	administrationGroupsRevisionsViewFeatureInfo->SetRequirements(QByteArray("/Administration/Groups/View").split(';'));

	administrationGroupsRevisionsFeatureInfo->InsertSubFeature(administrationGroupsRevisionsViewFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationGroupsRevisionsRestoreFeatureInfo;
	administrationGroupsRevisionsRestoreFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationGroupsRevisionsRestoreFeatureInfo->SetFeatureId("Restore");
	administrationGroupsRevisionsRestoreFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Restore Groups Revision"));
	administrationGroupsRevisionsRestoreFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows restoring a revision of groups."));
	administrationGroupsRevisionsRestoreFeatureInfo->SetOptional(false);
	administrationGroupsRevisionsRestoreFeatureInfo->SetIsPermission(true);
	administrationGroupsRevisionsRestoreFeatureInfo->SetRequirements(QByteArray("/Administration/Groups/Revisions/View;/Administration/Groups/Edit/Change").split(';'));

	administrationGroupsRevisionsFeatureInfo->InsertSubFeature(administrationGroupsRevisionsRestoreFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> administrationGroupsRevisionsDeleteFeatureInfo;
	administrationGroupsRevisionsDeleteFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	administrationGroupsRevisionsDeleteFeatureInfo->SetFeatureId("Delete");
	administrationGroupsRevisionsDeleteFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Delete Groups Revision"));
	administrationGroupsRevisionsDeleteFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows deleting a revision of groups."));
	administrationGroupsRevisionsDeleteFeatureInfo->SetOptional(false);
	administrationGroupsRevisionsDeleteFeatureInfo->SetIsPermission(true);
	administrationGroupsRevisionsDeleteFeatureInfo->SetRequirements(QByteArray("/Administration/Groups/Revisions/View").split(';'));

	administrationGroupsRevisionsFeatureInfo->InsertSubFeature(administrationGroupsRevisionsDeleteFeatureInfo.PopPtr());

	administrationGroupsFeatureInfo->InsertSubFeature(administrationGroupsRevisionsFeatureInfo.PopPtr());

	administrationFeatureInfo->InsertSubFeature(administrationGroupsFeatureInfo.PopPtr());

	productInfo.AddFeature("ce4a7f72-5303-4044-b25c-3a02020eebd3", *administrationFeatureInfo.GetPtr());

	istd::TDelPtr<imtlic::CIdentifiableFeatureInfo> organizationManagementFeatureInfo;
	organizationManagementFeatureInfo.SetPtr(new imtlic::CIdentifiableFeatureInfo);
	organizationManagementFeatureInfo->SetObjectUuid("f1b9f9b0-7d54-4fd1-9d17-8d2f8c5f2cf0");
	organizationManagementFeatureInfo->SetFeatureId("OrganizationManagement");
	organizationManagementFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Organization Management"));
	organizationManagementFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Provides access to Organization-related operations."));
	organizationManagementFeatureInfo->SetOptional(false);
	organizationManagementFeatureInfo->SetIsPermission(true);

	istd::TDelPtr<imtlic::CFeatureInfo> organizationManagementViewPageFeatureInfo;
	organizationManagementViewPageFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	organizationManagementViewPageFeatureInfo->SetFeatureId("ViewPage");
	organizationManagementViewPageFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "View Organization Page"));
	organizationManagementViewPageFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows opening the organization page."));
	organizationManagementViewPageFeatureInfo->SetOptional(false);
	organizationManagementViewPageFeatureInfo->SetIsPermission(true);

	organizationManagementFeatureInfo->InsertSubFeature(organizationManagementViewPageFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> organizationManagementViewFeatureInfo;
	organizationManagementViewFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	organizationManagementViewFeatureInfo->SetFeatureId("View");
	organizationManagementViewFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "View Organizations"));
	organizationManagementViewFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows viewing Organizations."));
	organizationManagementViewFeatureInfo->SetOptional(false);
	organizationManagementViewFeatureInfo->SetIsPermission(true);

	organizationManagementFeatureInfo->InsertSubFeature(organizationManagementViewFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> organizationManagementAddFeatureInfo;
	organizationManagementAddFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	organizationManagementAddFeatureInfo->SetFeatureId("Add");
	organizationManagementAddFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Add Organization"));
	organizationManagementAddFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows adding Organization."));
	organizationManagementAddFeatureInfo->SetOptional(false);
	organizationManagementAddFeatureInfo->SetIsPermission(true);

	organizationManagementFeatureInfo->InsertSubFeature(organizationManagementAddFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> organizationManagementRemoveFeatureInfo;
	organizationManagementRemoveFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	organizationManagementRemoveFeatureInfo->SetFeatureId("Remove");
	organizationManagementRemoveFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Remove Organization"));
	organizationManagementRemoveFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows removing Organization."));
	organizationManagementRemoveFeatureInfo->SetOptional(false);
	organizationManagementRemoveFeatureInfo->SetIsPermission(true);

	organizationManagementFeatureInfo->InsertSubFeature(organizationManagementRemoveFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> organizationManagementRevisionsFeatureInfo;
	organizationManagementRevisionsFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	organizationManagementRevisionsFeatureInfo->SetFeatureId("Revisions");
	organizationManagementRevisionsFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Organization Revisions"));
	organizationManagementRevisionsFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Provides access to the revisions of organization."));
	organizationManagementRevisionsFeatureInfo->SetOptional(false);
	organizationManagementRevisionsFeatureInfo->SetIsPermission(true);

	istd::TDelPtr<imtlic::CFeatureInfo> organizationManagementRevisionsViewFeatureInfo;
	organizationManagementRevisionsViewFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	organizationManagementRevisionsViewFeatureInfo->SetFeatureId("View");
	organizationManagementRevisionsViewFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "View Organization Revisions"));
	organizationManagementRevisionsViewFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows viewing the revisions of organization."));
	organizationManagementRevisionsViewFeatureInfo->SetOptional(false);
	organizationManagementRevisionsViewFeatureInfo->SetIsPermission(true);
	organizationManagementRevisionsViewFeatureInfo->SetRequirements(QByteArray("/OrganizationManagement/View").split(';'));

	organizationManagementRevisionsFeatureInfo->InsertSubFeature(organizationManagementRevisionsViewFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> organizationManagementRevisionsRestoreFeatureInfo;
	organizationManagementRevisionsRestoreFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	organizationManagementRevisionsRestoreFeatureInfo->SetFeatureId("Restore");
	organizationManagementRevisionsRestoreFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Restore Organization Revision"));
	organizationManagementRevisionsRestoreFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows restoring a revision of organization."));
	organizationManagementRevisionsRestoreFeatureInfo->SetOptional(false);
	organizationManagementRevisionsRestoreFeatureInfo->SetIsPermission(true);
	organizationManagementRevisionsRestoreFeatureInfo->SetRequirements(QByteArray("/OrganizationManagement/Revisions/View").split(';'));

	organizationManagementRevisionsFeatureInfo->InsertSubFeature(organizationManagementRevisionsRestoreFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> organizationManagementRevisionsDeleteFeatureInfo;
	organizationManagementRevisionsDeleteFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	organizationManagementRevisionsDeleteFeatureInfo->SetFeatureId("Delete");
	organizationManagementRevisionsDeleteFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Delete Organization Revision"));
	organizationManagementRevisionsDeleteFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows deleting a revision of organization."));
	organizationManagementRevisionsDeleteFeatureInfo->SetOptional(false);
	organizationManagementRevisionsDeleteFeatureInfo->SetIsPermission(true);
	organizationManagementRevisionsDeleteFeatureInfo->SetRequirements(QByteArray("/OrganizationManagement/Revisions/View").split(';'));

	organizationManagementRevisionsFeatureInfo->InsertSubFeature(organizationManagementRevisionsDeleteFeatureInfo.PopPtr());

	organizationManagementFeatureInfo->InsertSubFeature(organizationManagementRevisionsFeatureInfo.PopPtr());

	istd::TDelPtr<imtlic::CFeatureInfo> organizationManagementSharesFeatureInfo;
	organizationManagementSharesFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	organizationManagementSharesFeatureInfo->SetFeatureId("Shares");
	organizationManagementSharesFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Document Shares"));
	organizationManagementSharesFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Provides access to the documents the organization shares and receives."));
	organizationManagementSharesFeatureInfo->SetOptional(false);
	organizationManagementSharesFeatureInfo->SetIsPermission(true);

	istd::TDelPtr<imtlic::CFeatureInfo> organizationManagementSharesEditFeatureInfo;
	organizationManagementSharesEditFeatureInfo.SetPtr(new imtlic::CFeatureInfo);
	organizationManagementSharesEditFeatureInfo->SetFeatureId("Edit");
	organizationManagementSharesEditFeatureInfo->SetFeatureName(QT_TRANSLATE_NOOP("Feature", "Manage Document Shares"));
	organizationManagementSharesEditFeatureInfo->SetFeatureDescription(QT_TRANSLATE_NOOP("Feature", "Allows restricting, accepting and removing the document shares of the organization."));
	organizationManagementSharesEditFeatureInfo->SetOptional(false);
	organizationManagementSharesEditFeatureInfo->SetIsPermission(true);
	organizationManagementSharesEditFeatureInfo->SetRequirements(QByteArray("/OrganizationManagement/View").split(';'));

	organizationManagementSharesFeatureInfo->InsertSubFeature(organizationManagementSharesEditFeatureInfo.PopPtr());

	organizationManagementFeatureInfo->InsertSubFeature(organizationManagementSharesFeatureInfo.PopPtr());

	productInfo.AddFeature("f1b9f9b0-7d54-4fd1-9d17-8d2f8c5f2cf0", *organizationManagementFeatureInfo.GetPtr());

}


};
