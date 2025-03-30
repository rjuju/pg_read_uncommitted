REGRESS      = basic

MODULE_big = pg_read_uncommitted
OBJS = pg_read_uncommitted.o

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
