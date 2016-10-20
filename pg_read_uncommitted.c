/*
 * pg_stat_kcache (kcache)
 *
 * Provides basic statistics about real I/O done by the filesystem layer.
 * This way, calculating a real hit-ratio is doable.
 *
 * Large portions of code freely inspired by pg_stat_plans. Thanks to Peter
 * Geoghegan for this great extension.
 *
 * This program is open source, licensed under the PostgreSQL license.
 * For license terms, see the LICENSE file.
 *
 */
#include "postgres.h"

#include "executor/executor.h"
#include "funcapi.h"
#include "miscadmin.h"
//#include "storage/fd.h"
//#include "storage/ipc.h"
//#include "storage/spin.h"
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
bool HeapTupleSatisfiesAll(HeapTuple htup, Snapshot snapshot,
								   Buffer buffer);

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
	if (pgru_enabled)
	{
		if (pgru_show_deleted)
			queryDesc->snapshot->satisfies = HeapTupleSatisfiesAll;
		else
			queryDesc->snapshot->satisfies = HeapTupleSatisfiesDirty;
	}

	if (prev_ExecutorStart)
		prev_ExecutorStart(queryDesc, eflags);
	else
		standard_ExecutorStart(queryDesc, eflags);
}

bool
HeapTupleSatisfiesAll(HeapTuple htup, Snapshot snapshot,
								   Buffer buffer)
{
	/* don't care, return anything found */
	return true;
}
