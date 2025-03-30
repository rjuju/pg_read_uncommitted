/*
 * pg_read_uncommitted
 *
 * Enable read uncommitted visiblity, and deleted tuples visiblity.
 *
 * This program is open source, licensed under the PostgreSQL license.
 * For license terms, see the LICENSE file.
 *
 * Copyright (C) 2016-2025: Julien Rouhaud
 *
 */
#include "postgres.h"

#include "executor/executor.h"
#include "funcapi.h"
#include "miscadmin.h"
#include "utils/builtins.h"
#include "utils/guc.h"
#include "utils/snapshot.h"
#if PG_VERSION_NUM >= 120000
#include "utils/snapmgr.h"
#else
#include "utils/tqual.h"
#endif

PG_MODULE_MAGIC;

/* ExecutorStart hook */
#if PG_VERSION_NUM >= 180000
#define EXEC_START_RET	bool
#else
#define EXEC_START_RET	void
#endif
/* end of ExecutorStart hook */


/* saved hook address in case of unload */
static ExecutorStart_hook_type prev_ExecutorStart = NULL;

/*--- Functions --- */

void	_PG_init(void);

static EXEC_START_RET pgru_ExecutorStart(QueryDesc *queryDesc, int eflags);

static bool pgru_enabled;
static bool pgru_show_deleted;

void
_PG_init(void)
{
	/* Install hook */
	prev_ExecutorStart = ExecutorStart_hook;
	ExecutorStart_hook = pgru_ExecutorStart;

	DefineCustomBoolVariable("pg_read_uncommitted.enabled",
							 "Enable / Disable pg_read_uncommitted",
							 NULL,
							 &pgru_enabled,
							 false,
							 PGC_USERSET,
							 0,
							 NULL,
							 NULL,
							 NULL);

	DefineCustomBoolVariable("pg_read_uncommitted.show_deleted",
							 "Even read deleted rows",
							 NULL,
							 &pgru_show_deleted,
							 false,
							 PGC_USERSET,
							 0,
							 NULL,
							 NULL,
							 NULL);
}

static EXEC_START_RET
pgru_ExecutorStart (QueryDesc *queryDesc, int eflags)
{
	/* do not mess with non MVCC snapshots, like index build :) */
	if (pgru_enabled && IsMVCCSnapshot(queryDesc->snapshot))
	{
		if (pgru_show_deleted)
#if PG_VERSION_NUM >= 120000
			queryDesc->snapshot->snapshot_type = SNAPSHOT_ANY;
#else
			queryDesc->snapshot->satisfies = HeapTupleSatisfiesAny;
#endif
		else
#if PG_VERSION_NUM >= 120000
			queryDesc->snapshot->snapshot_type = SNAPSHOT_DIRTY;
#else
			queryDesc->snapshot->satisfies = HeapTupleSatisfiesDirty;
#endif
	}

	if (prev_ExecutorStart)
		return prev_ExecutorStart(queryDesc, eflags);
	else
		return standard_ExecutorStart(queryDesc, eflags);
}
