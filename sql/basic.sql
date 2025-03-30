CREATE TABLE read_uncommitted(id integer, val text);

INSERT INTO read_uncommitted SELECT 1, NULL;
UPDATE read_uncommitted SET val = 'first update';
UPDATE read_uncommitted SET id = 2, val = 'second update';
DELETE FROM read_uncommitted;

SELECT * FROM read_uncommitted ORDER BY id, val;

LOAD 'pg_read_uncommitted';
SET pg_read_uncommitted.enabled = true;
SET pg_read_uncommitted.show_deleted = true;

SELECT * FROM read_uncommitted ORDER BY id, val;
