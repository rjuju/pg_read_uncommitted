/*
 * pg_read_uncommitted
 *
 * Enable read uncommitted visiblity, and deleted tuples visiblity.
 *
 * This program is open source, licensed under the PostgreSQL license.
 * For license terms, see the LICENSE file.
 *
 */
#include "postgres.h"

#include "executor/executor.h"
#include "funcapi.h"
#include "miscadmin.h"
#include "utils/builtins.h"
#include "utils/guc.h"
#include "utils/snapshot.h"
#include "utils/tqual.h"


PG_MODULE_MAGIC;


/* saved hook address in case of unload */
static ExecutorStart_hook_type prev_ExecutorStart = NULL;

/*--- Functions --- */

void	_PG_init(void);
void	_PG_fini(void);

static void pgru_ExecutorStart(QueryDesc *queryDesc, int eflags);

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

void
_PG_fini(void)
{
	/* uninstall hook */
	ExecutorStart_hook = prev_ExecutorStart;
}

static void
pgru_ExecutorStart (QueryDesc *queryDesc, int eflags)
{
	/* do not mess with non MVCC snapshots, like index build :) */
	if (pgru_enabled && IsMVCCSnapshot(queryDesc->snapshot))
	{
		if (pgru_show_deleted)
			queryDesc->snapshot->satisfies = HeapTupleSatisfiesAny;
		else
			queryDesc->snapshot->satisfies = HeapTupleSatisfiesDirty;
	}

	if (prev_ExecutorStart)
		prev_ExecutorStart(queryDesc, eflags);
	else
		standard_ExecutorStart(queryDesc, eflags);
}
