-- Tenant isolation check of the Puma cross-tenant tables, executed as the application role.
\set ON_ERROR_STOP on
SET client_min_messages = notice;

DO $$
DECLARE
	spec text[];
	tbl text;
	columnA text;
	columnB text;
	tenantId text;
	otherTenantId text;
	foreignId text;
	total int; visible int; own int; leaked int; changed int;
	isSuper bool; isBypass bool;
	ownerName text; rlsEnabled bool; rlsForced bool; policyCount int;
	ownSql text;
BEGIN
	SELECT rolsuper, rolbypassrls INTO isSuper, isBypass FROM pg_roles WHERE rolname = current_user;
	RAISE NOTICE '% role %: superuser=%, bypassrls=%', CASE WHEN NOT isSuper AND NOT isBypass THEN 'PASS' ELSE 'FAIL' END, current_user, isSuper, isBypass;

	FOREACH spec SLICE 1 IN ARRAY ARRAY[
			ARRAY['TenantConnections', 'TenantAId', 'TenantBId'],
			ARRAY['TenantConnectionRequests', 'SourceTenantId', 'TargetTenantId'],
			ARRAY['TenantRelationships', 'SourceTenantId', 'TargetTenantId'],
			ARRAY['TenantRelationshipProposals', 'InitiatorTenantId', 'CounterpartyTenantId'],
			ARRAY['CrossOrgGrants', 'SourceTenantId', 'TargetTenantId']] LOOP
		tbl := spec[1]; columnA := spec[2]; columnB := spec[3];

		SELECT pg_get_userbyid(c.relowner), c.relrowsecurity, c.relforcerowsecurity INTO ownerName, rlsEnabled, rlsForced
			FROM pg_class c JOIN pg_namespace n ON n.oid = c.relnamespace WHERE n.nspname = 'public' AND c.relname = tbl;
		SELECT count(*) INTO policyCount FROM pg_policies WHERE schemaname = 'public' AND tablename = tbl;
		RAISE NOTICE '% %: owner=%, rls=%, force=%, policies=%',
			CASE WHEN ownerName = current_user AND rlsEnabled AND rlsForced AND policyCount > 0 THEN 'PASS' ELSE 'FAIL' END,
			tbl, ownerName, rlsEnabled, rlsForced, policyCount;

		ownSql := format('(%I::text = $1 OR %I::text = $1)', columnA, columnB);

		PERFORM set_config('imt.tenant_id', '', false);
		PERFORM set_config('imt.rls_bypass', 'on', false);
		EXECUTE format('SELECT count(*) FROM %I', tbl) INTO total;

		PERFORM set_config('imt.rls_bypass', 'off', false);
		EXECUTE format('SELECT count(*) FROM %I', tbl) INTO visible;
		RAISE NOTICE '% % without context: visible % of % rows', CASE WHEN visible = 0 THEN 'PASS' ELSE 'FAIL' END, tbl, visible, total;

		FOR tenantId IN EXECUTE format('SELECT DISTINCT t FROM (SELECT %I::text AS t FROM %I UNION SELECT %I::text FROM %I) x WHERE COALESCE(t, '''') <> '''' ORDER BY 1', columnA, tbl, columnB, tbl) LOOP
			PERFORM set_config('imt.rls_bypass', 'on', false);
			PERFORM set_config('imt.tenant_id', '', false);
			EXECUTE format('SELECT count(*) FROM %I WHERE ', tbl) || ownSql INTO own USING tenantId;
			EXECUTE format('SELECT "Id"::text FROM %I WHERE NOT ', tbl) || ownSql || ' LIMIT 1' INTO foreignId USING tenantId;
			EXECUTE format('SELECT t FROM (SELECT %I::text AS t FROM %I UNION SELECT %I::text FROM %I) x WHERE COALESCE(t, '''') NOT IN ('''', $1) LIMIT 1', columnA, tbl, columnB, tbl)
				INTO otherTenantId USING tenantId;

			PERFORM set_config('imt.rls_bypass', 'off', false);
			PERFORM set_config('imt.tenant_id', tenantId, false);
			EXECUTE format('SELECT count(*) FROM %I', tbl) INTO visible;
			EXECUTE format('SELECT count(*) FROM %I WHERE NOT ', tbl) || ownSql INTO leaked USING tenantId;
			RAISE NOTICE '% % tenant %: visible %, expected %, foreign visible %',
				CASE WHEN visible = own AND leaked = 0 THEN 'PASS' ELSE 'FAIL' END, tbl, tenantId, visible, own, leaked;

			IF foreignId IS NOT NULL THEN
				EXECUTE format('UPDATE %I SET %I = %I WHERE "Id"::text = %L', tbl, columnA, columnA, foreignId);
				GET DIAGNOSTICS changed = ROW_COUNT;
				RAISE NOTICE '% % tenant %: update of foreign row changed % rows', CASE WHEN changed = 0 THEN 'PASS' ELSE 'FAIL' END, tbl, tenantId, changed;
			END IF;

			IF otherTenantId IS NOT NULL THEN
				BEGIN
					-- Moving an own row to foreign tenants must be rejected by WITH CHECK
					EXECUTE format('UPDATE %I SET %I = %L, %I = %L WHERE ', tbl, columnA, otherTenantId, columnB, otherTenantId) || ownSql USING tenantId;
					GET DIAGNOSTICS changed = ROW_COUNT;
					RAISE NOTICE '% % tenant %: own rows moved to tenant %: % rows', CASE WHEN changed = 0 THEN 'PASS' ELSE 'FAIL' END, tbl, tenantId, otherTenantId, changed;
					RAISE EXCEPTION USING ERRCODE = 'P0001', MESSAGE = 'rollback';
				EXCEPTION
					WHEN insufficient_privilege THEN
						RAISE NOTICE 'PASS % tenant %: moving own rows to tenant % rejected (%)', tbl, tenantId, otherTenantId, SQLSTATE;
					WHEN raise_exception THEN
						NULL;
				END;
			END IF;
		END LOOP;
	END LOOP;

	PERFORM set_config('imt.tenant_id', '', false);
	PERFORM set_config('imt.rls_bypass', 'off', false);
END
$$;
