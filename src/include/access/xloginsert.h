/*
 * xloginsert.h
 *
 * Functions for generating WAL records
 *
 * Portions Copyright (c) 1996-2021, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/access/xloginsert.h
 */
#ifndef XLOGINSERT_H
#define XLOGINSERT_H

#include "access/rmgr.h"
#include "access/xlogdefs.h"
#include "storage/block.h"
#include "storage/buf.h"
#include "storage/relfilenode.h"
#include "utils/relcache.h"

/*
 * The minimum size of the WAL construction working area. If you need to
 * register more than XLR_NORMAL_MAX_BLOCK_ID block references or have more
 * than XLR_NORMAL_RDATAS data chunks in a single WAL record, you must call
 * XLogEnsureRecordSpace() first to allocate more working memory.
 */
#define XLR_NORMAL_MAX_BLOCK_ID		4
#define XLR_NORMAL_RDATAS			20

/* flags for XLogRegisterBuffer */
#define REGBUF_FORCE_IMAGE	0x01	/* force a full-page image */
#define REGBUF_NO_IMAGE		0x02	/* don't take a full-page image */
#define REGBUF_WILL_INIT	(0x04 | 0x02)	/* page will be re-initialized at
											 * replay (implies NO_IMAGE) */
#define REGBUF_STANDARD		0x08	/* page follows "standard" page layout,
									 * (data between pd_lower and pd_upper
									 * will be skipped) */
#define REGBUF_KEEP_DATA	0x10	/* include data even if a full-page image
									 * is taken */

typedef void * (*RecordAssembleFunc)(RmgrId rmid, uint8 info,
                                XLogRecPtr RedoRecPtr, bool doPageWrites,
                                XLogRecPtr *fpw_lsn, TransactionId headerXid, int *num_fpi);
typedef XLogRecPtr (*XLogInsert_hook_type)(RmgrId rmid, uint8 info, TransactionId headerXid, uint8 curinsert_flags, RecordAssembleFunc recordAssembleFunc);
extern PGDLLIMPORT XLogInsert_hook_type XLogInsert_hook;

extern bool GetXLogRegisterBufferTagIfAny(RelFileNode *rnode, ForkNumber *forknum, BlockNumber *blkno);
extern int GetNumXLogRegisterBuffers(void);
extern bool GetXLogRegisterBuffer(int block_id, RelFileNode *rnode, ForkNumber *forknum, BlockNumber *blkno, char **page);
extern char *GetXLogRegisterRdata(int rdata_index);

/* prototypes for public functions in xloginsert.c: */
extern void XLogBeginInsert(void);
extern void XLogSetRecordFlags(uint8 flags);
extern XLogRecPtr XLogInsert(RmgrId rmid, uint8 info);
extern void XLogEnsureRecordSpace(int max_block_id, int ndatas);
extern void XLogRegisterData(char *data, int len);
extern void XLogRegisterBuffer(uint8 block_id, Buffer buffer, uint8 flags);
extern void XLogRegisterBlock(uint8 block_id, RelFileNode *rnode,
							  ForkNumber forknum, BlockNumber blknum, char *page,
							  uint8 flags);
extern void XLogRegisterBufData(uint8 block_id, char *data, int len);
extern void XLogResetInsertion(void);
extern bool XLogCheckBufferNeedsBackup(Buffer buffer);

extern XLogRecPtr log_newpage(RelFileNode *rnode, ForkNumber forkNum,
							  BlockNumber blk, char *page, bool page_std);
extern void log_newpages(RelFileNode *rnode, ForkNumber forkNum, int num_pages,
						 BlockNumber *blknos, char **pages, bool page_std);
extern XLogRecPtr log_newpage_buffer(Buffer buffer, bool page_std);
extern void log_newpage_range(Relation rel, ForkNumber forkNum,
							  BlockNumber startblk, BlockNumber endblk, bool page_std);
extern XLogRecPtr XLogSaveBufferForHint(Buffer buffer, bool buffer_std);

extern XLogRecPtr LSNForEncryption(bool use_wal_lsn);

extern void InitXLogInsert(void);

#endif							/* XLOGINSERT_H */
