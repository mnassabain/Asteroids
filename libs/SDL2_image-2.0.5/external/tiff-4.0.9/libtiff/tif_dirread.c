/* $Id: tif_dirread.c,v 1.218 2017-09-09 21:44:42 erouault Exp $ */

/*
 * Copyright (c) 1988-1997 Sam Leffler
 * Copyright (c) 1991-1997 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

/*
 * TIFF Library.
 *
 * Directory Read Support Routines.
 */

/* Suggested pending improvements:
 * - add a field 'ignore' to the TIFFDirEntry structure, to flag status,
 *   eliminating current use of the IGNORE value, and therefore eliminating
 *   current irrational behaviour on tags with tag id code 0
 * - add a field 'field_info' to the TIFFDirEntry structure, and set that with
 *   the pointer to the appropriate TIFFField structure early on in
 *   TIFFReadDirectory, so as to eliminate current possibly repetitive lookup.
 */

#include "tiffiop.h"
#include <float.h>
#include <stdlib.h>

#define IGNORE 0          /* tag placeholder used below */
#define FAILED_FII    ((uint32) -1)

#ifdef HAVE_IEEEFP
# define TIFFCvtIEEEFloatToNative(tif, n, fp)
# define TIFFCvtIEEEDoubleToNative(tif, n, dp)
#else
extern void TIFFCvtIEEEFloatToNative(TIFF*, uint32, float*);
extern void TIFFCvtIEEEDoubleToNative(TIFF*, uint32, double*);
#endif

enum TIFFReadDirEntryErr {
	TIFFReadDirEntryErrOk = 0,
	TIFFReadDirEntryErrCount = 1,
	TIFFReadDirEntryErrType = 2,
	TIFFReadDirEntryErrIo = 3,
	TIFFReadDirEntryErrRange = 4,
	TIFFReadDirEntryErrPsdif = 5,
	TIFFReadDirEntryErrSizesan = 6,
	TIFFReadDirEntryErrAlloc = 7,
};

static enum TIFFReadDirEntryErr TIFFReadDirEntryByte(TIFF* tif, TIFFDirEntry* direntry, uint8* value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryShort(TIFF* tif, TIFFDirEntry* direntry, uint16* value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryLong(TIFF* tif, TIFFDirEntry* direntry, uint32* value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryLong8(TIFF* tif, TIFFDirEntry* direntry, uint64* value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryFloat(TIFF* tif, TIFFDirEntry* direntry, float* value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryDouble(TIFF* tif, TIFFDirEntry* direntry, double* value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryIfd8(TIFF* tif, TIFFDirEntry* direntry, uint64* value);

static enum TIFFReadDirEntryErr TIFFReadDirEntryArray(TIFF* tif, TIFFDirEntry* direntry, uint32* count, uint32 desttypesize, void** value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryByteArray(TIFF* tif, TIFFDirEntry* direntry, uint8** value);
static enum TIFFReadDirEntryErr TIFFReadDirEntrySbyteArray(TIFF* tif, TIFFDirEntry* direntry, int8** value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryShortArray(TIFF* tif, TIFFDirEntry* direntry, uint16** value);
static enum TIFFReadDirEntryErr TIFFReadDirEntrySshortArray(TIFF* tif, TIFFDirEntry* direntry, int16** value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryLongArray(TIFF* tif, TIFFDirEntry* direntry, uint32** value);
static enum TIFFReadDirEntryErr TIFFReadDirEntrySlongArray(TIFF* tif, TIFFDirEntry* direntry, int32** value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryLong8Array(TIFF* tif, TIFFDirEntry* direntry, uint64** value);
static enum TIFFReadDirEntryErr TIFFReadDirEntrySlong8Array(TIFF* tif, TIFFDirEntry* direntry, int64** value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryFloatArray(TIFF* tif, TIFFDirEntry* direntry, float** value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryDoubleArray(TIFF* tif, TIFFDirEntry* direntry, double** value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryIfd8Array(TIFF* tif, TIFFDirEntry* direntry, uint64** value);

static enum TIFFReadDirEntryErr TIFFReadDirEntryPersampleShort(TIFF* tif, TIFFDirEntry* direntry, uint16* value);
#if 0
static enum TIFFReadDirEntryErr TIFFReadDirEntryPersampleDouble(TIFF* tif, TIFFDirEntry* direntry, double* value);
#endif

static void TIFFReadDirEntryCheckedByte(TIFF* tif, TIFFDirEntry* direntry, uint8* value);
static void TIFFReadDirEntryCheckedSbyte(TIFF* tif, TIFFDirEntry* direntry, int8* value);
static void TIFFReadDirEntryCheckedShort(TIFF* tif, TIFFDirEntry* direntry, uint16* value);
static void TIFFReadDirEntryCheckedSshort(TIFF* tif, TIFFDirEntry* direntry, int16* value);
static void TIFFReadDirEntryCheckedLong(TIFF* tif, TIFFDirEntry* direntry, uint32* value);
static void TIFFReadDirEntryCheckedSlong(TIFF* tif, TIFFDirEntry* direntry, int32* value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckedLong8(TIFF* tif, TIFFDirEntry* direntry, uint64* value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckedSlong8(TIFF* tif, TIFFDirEntry* direntry, int64* value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckedRational(TIFF* tif, TIFFDirEntry* direntry, double* value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckedSrational(TIFF* tif, TIFFDirEntry* direntry, double* value);
static void TIFFReadDirEntryCheckedFloat(TIFF* tif, TIFFDirEntry* direntry, float* value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckedDouble(TIFF* tif, TIFFDirEntry* direntry, double* value);

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeByteSbyte(int8 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeByteShort(uint16 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeByteSshort(int16 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeByteLong(uint32 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeByteSlong(int32 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeByteLong8(uint64 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeByteSlong8(int64 value);

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSbyteByte(uint8 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSbyteShort(uint16 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSbyteSshort(int16 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSbyteLong(uint32 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSbyteSlong(int32 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSbyteLong8(uint64 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSbyteSlong8(int64 value);

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeShortSbyte(int8 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeShortSshort(int16 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeShortLong(uint32 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeShortSlong(int32 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeShortLong8(uint64 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeShortSlong8(int64 value);

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSshortShort(uint16 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSshortLong(uint32 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSshortSlong(int32 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSshortLong8(uint64 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSshortSlong8(int64 value);

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeLongSbyte(int8 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeLongSshort(int16 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeLongSlong(int32 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeLongLong8(uint64 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeLongSlong8(int64 value);

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSlongLong(uint32 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSlongLong8(uint64 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSlongSlong8(int64 value);

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeLong8Sbyte(int8 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeLong8Sshort(int16 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeLong8Slong(int32 value);
static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeLong8Slong8(int64 value);

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSlong8Long8(uint64 value);

static enum TIFFReadDirEntryErr TIFFReadDirEntryData(TIFF* tif, uint64 offset, tmsize_t size, void* dest);
static void TIFFReadDirEntryOutputErr(TIFF* tif, enum TIFFReadDirEntryErr err, const char* module, const char* tagname, int recover);

static void TIFFReadDirectoryCheckOrder(TIFF* tif, TIFFDirEntry* dir, uint16 dircount);
static TIFFDirEntry* TIFFReadDirectoryFindEntry(TIFF* tif, TIFFDirEntry* dir, uint16 dircount, uint16 tagid);
static void TIFFReadDirectoryFindFieldInfo(TIFF* tif, uint16 tagid, uint32* fii);

static int EstimateStripByteCounts(TIFF* tif, TIFFDirEntry* dir, uint16 dircount);
static void MissingRequired(TIFF*, const char*);
static int TIFFCheckDirOffset(TIFF* tif, uint64 diroff);
static int CheckDirCount(TIFF*, TIFFDirEntry*, uint32);
static uint16 TIFFFetchDirectory(TIFF* tif, uint64 diroff, TIFFDirEntry** pdir, uint64* nextdiroff);
static int TIFFFetchNormalTag(TIFF*, TIFFDirEntry*, int recover);
static int TIFFFetchStripThing(TIFF* tif, TIFFDirEntry* dir, uint32 nstrips, uint64** lpp);
static int TIFFFetchSubjectDistance(TIFF*, TIFFDirEntry*);
static void ChopUpSingleUncompressedStrip(TIFF*);
static uint64 TIFFReadUInt64(const uint8 *value);
static int _TIFFGetMaxColorChannels(uint16 photometric);

static int _TIFFFillStrilesInternal( TIFF *tif, int loadStripByteCount );

typedef union _UInt64Aligned_t
{
        double d;
	uint64 l;
	uint32 i[2];
	uint16 s[4];
	uint8  c[8];
} UInt64Aligned_t;

/*
  Unaligned safe copy of a uint64 value from an octet array.
*/
static uint64 TIFFReadUInt64(const uint8 *value)
{
	UInt64Aligned_t result;

	result.c[0]=value[0];
	result.c[1]=value[1];
	result.c[2]=value[2];
	result.c[3]=value[3];
	result.c[4]=value[4];
	result.c[5]=value[5];
	result.c[6]=value[6];
	result.c[7]=value[7];

	return result.l;
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryByte(TIFF* tif, TIFFDirEntry* direntry, uint8* value)
{
	enum TIFFReadDirEntryErr err;
	if (direntry->tdir_count!=1)
		return(TIFFReadDirEntryErrCount);
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
			TIFFReadDirEntryCheckedByte(tif,direntry,value);
			return(TIFFReadDirEntryErrOk);
		case TIFF_SBYTE:
			{
				int8 m;
				TIFFReadDirEntryCheckedSbyte(tif,direntry,&m);
				err=TIFFReadDirEntryCheckRangeByteSbyte(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint8)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SHORT:
			{
				uint16 m;
				TIFFReadDirEntryCheckedShort(tif,direntry,&m);
				err=TIFFReadDirEntryCheckRangeByteShort(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint8)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SSHORT:
			{
				int16 m;
				TIFFReadDirEntryCheckedSshort(tif,direntry,&m);
				err=TIFFReadDirEntryCheckRangeByteSshort(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint8)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_LONG:
			{
				uint32 m;
				TIFFReadDirEntryCheckedLong(tif,direntry,&m);
				err=TIFFReadDirEntryCheckRangeByteLong(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint8)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SLONG:
			{
				int32 m;
				TIFFReadDirEntryCheckedSlong(tif,direntry,&m);
				err=TIFFReadDirEntryCheckRangeByteSlong(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint8)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_LONG8:
			{
				uint64 m;
				err=TIFFReadDirEntryCheckedLong8(tif,direntry,&m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				err=TIFFReadDirEntryCheckRangeByteLong8(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint8)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SLONG8:
			{
				int64 m;
				err=TIFFReadDirEntryCheckedSlong8(tif,direntry,&m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				err=TIFFReadDirEntryCheckRangeByteSlong8(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint8)m;
				return(TIFFReadDirEntryErrOk);
			}
		default:
			return(TIFFReadDirEntryErrType);
	}
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryShort(TIFF* tif, TIFFDirEntry* direntry, uint16* value)
{
	enum TIFFReadDirEntryErr err;
	if (direntry->tdir_count!=1)
		return(TIFFReadDirEntryErrCount);
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
			{
				uint8 m;
				TIFFReadDirEntryCheckedByte(tif,direntry,&m);
				*value=(uint16)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SBYTE:
			{
				int8 m;
				TIFFReadDirEntryCheckedSbyte(tif,direntry,&m);
				err=TIFFReadDirEntryCheckRangeShortSbyte(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint16)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SHORT:
			TIFFReadDirEntryCheckedShort(tif,direntry,value);
			return(TIFFReadDirEntryErrOk);
		case TIFF_SSHORT:
			{
				int16 m;
				TIFFReadDirEntryCheckedSshort(tif,direntry,&m);
				err=TIFFReadDirEntryCheckRangeShortSshort(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint16)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_LONG:
			{
				uint32 m;
				TIFFReadDirEntryCheckedLong(tif,direntry,&m);
				err=TIFFReadDirEntryCheckRangeShortLong(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint16)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SLONG:
			{
				int32 m;
				TIFFReadDirEntryCheckedSlong(tif,direntry,&m);
				err=TIFFReadDirEntryCheckRangeShortSlong(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint16)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_LONG8:
			{
				uint64 m;
				err=TIFFReadDirEntryCheckedLong8(tif,direntry,&m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				err=TIFFReadDirEntryCheckRangeShortLong8(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint16)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SLONG8:
			{
				int64 m;
				err=TIFFReadDirEntryCheckedSlong8(tif,direntry,&m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				err=TIFFReadDirEntryCheckRangeShortSlong8(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint16)m;
				return(TIFFReadDirEntryErrOk);
			}
		default:
			return(TIFFReadDirEntryErrType);
	}
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryLong(TIFF* tif, TIFFDirEntry* direntry, uint32* value)
{
	enum TIFFReadDirEntryErr err;
	if (direntry->tdir_count!=1)
		return(TIFFReadDirEntryErrCount);
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
			{
				uint8 m;
				TIFFReadDirEntryCheckedByte(tif,direntry,&m);
				*value=(uint32)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SBYTE:
			{
				int8 m;
				TIFFReadDirEntryCheckedSbyte(tif,direntry,&m);
				err=TIFFReadDirEntryCheckRangeLongSbyte(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint32)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SHORT:
			{
				uint16 m;
				TIFFReadDirEntryCheckedShort(tif,direntry,&m);
				*value=(uint32)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SSHORT:
			{
				int16 m;
				TIFFReadDirEntryCheckedSshort(tif,direntry,&m);
				err=TIFFReadDirEntryCheckRangeLongSshort(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint32)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_LONG:
			TIFFReadDirEntryCheckedLong(tif,direntry,value);
			return(TIFFReadDirEntryErrOk);
		case TIFF_SLONG:
			{
				int32 m;
				TIFFReadDirEntryCheckedSlong(tif,direntry,&m);
				err=TIFFReadDirEntryCheckRangeLongSlong(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint32)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_LONG8:
			{
				uint64 m;
				err=TIFFReadDirEntryCheckedLong8(tif,direntry,&m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				err=TIFFReadDirEntryCheckRangeLongLong8(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint32)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SLONG8:
			{
				int64 m;
				err=TIFFReadDirEntryCheckedSlong8(tif,direntry,&m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				err=TIFFReadDirEntryCheckRangeLongSlong8(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint32)m;
				return(TIFFReadDirEntryErrOk);
			}
		default:
			return(TIFFReadDirEntryErrType);
	}
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryLong8(TIFF* tif, TIFFDirEntry* direntry, uint64* value)
{
	enum TIFFReadDirEntryErr err;
	if (direntry->tdir_count!=1)
		return(TIFFReadDirEntryErrCount);
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
			{
				uint8 m;
				TIFFReadDirEntryCheckedByte(tif,direntry,&m);
				*value=(uint64)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SBYTE:
			{
				int8 m;
				TIFFReadDirEntryCheckedSbyte(tif,direntry,&m);
				err=TIFFReadDirEntryCheckRangeLong8Sbyte(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint64)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SHORT:
			{
				uint16 m;
				TIFFReadDirEntryCheckedShort(tif,direntry,&m);
				*value=(uint64)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SSHORT:
			{
				int16 m;
				TIFFReadDirEntryCheckedSshort(tif,direntry,&m);
				err=TIFFReadDirEntryCheckRangeLong8Sshort(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint64)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_LONG:
			{
				uint32 m;
				TIFFReadDirEntryCheckedLong(tif,direntry,&m);
				*value=(uint64)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SLONG:
			{
				int32 m;
				TIFFReadDirEntryCheckedSlong(tif,direntry,&m);
				err=TIFFReadDirEntryCheckRangeLong8Slong(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint64)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_LONG8:
			err=TIFFReadDirEntryCheckedLong8(tif,direntry,value);
			return(err);
		case TIFF_SLONG8:
			{
				int64 m;
				err=TIFFReadDirEntryCheckedSlong8(tif,direntry,&m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				err=TIFFReadDirEntryCheckRangeLong8Slong8(m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(uint64)m;
				return(TIFFReadDirEntryErrOk);
			}
		default:
			return(TIFFReadDirEntryErrType);
	}
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryFloat(TIFF* tif, TIFFDirEntry* direntry, float* value)
{
	enum TIFFReadDirEntryErr err;
	if (direntry->tdir_count!=1)
		return(TIFFReadDirEntryErrCount);
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
			{
				uint8 m;
				TIFFReadDirEntryCheckedByte(tif,direntry,&m);
				*value=(float)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SBYTE:
			{
				int8 m;
				TIFFReadDirEntryCheckedSbyte(tif,direntry,&m);
				*value=(float)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SHORT:
			{
				uint16 m;
				TIFFReadDirEntryCheckedShort(tif,direntry,&m);
				*value=(float)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SSHORT:
			{
				int16 m;
				TIFFReadDirEntryCheckedSshort(tif,direntry,&m);
				*value=(float)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_LONG:
			{
				uint32 m;
				TIFFReadDirEntryCheckedLong(tif,direntry,&m);
				*value=(float)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SLONG:
			{
				int32 m;
				TIFFReadDirEntryCheckedSlong(tif,direntry,&m);
				*value=(float)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_LONG8:
			{
				uint64 m;
				err=TIFFReadDirEntryCheckedLong8(tif,direntry,&m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
#if defined(__WIN32__) && (_MSC_VER < 1500)
				/*
				 * XXX: MSVC 6.0 does not support conversion
				 * of 64-bit integers into floating point
				 * values.
				 */
				*value = _TIFFUInt64ToFloat(m);
#else
				*value=(float)m;
#endif
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SLONG8:
			{
				int64 m;
				err=TIFFReadDirEntryCheckedSlong8(tif,direntry,&m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(float)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_RATIONAL:
			{
				double m;
				err=TIFFReadDirEntryCheckedRational(tif,direntry,&m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(float)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SRATIONAL:
			{
				double m;
				err=TIFFReadDirEntryCheckedSrational(tif,direntry,&m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(float)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_FLOAT:
			TIFFReadDirEntryCheckedFloat(tif,direntry,value);
			return(TIFFReadDirEntryErrOk);
		case TIFF_DOUBLE:
			{
				double m;
				err=TIFFReadDirEntryCheckedDouble(tif,direntry,&m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				if ((m > FLT_MAX) || (m < FLT_MIN))
					return(TIFFReadDirEntryErrRange);
				*value=(float)m;
				return(TIFFReadDirEntryErrOk);
			}
		default:
			return(TIFFReadDirEntryErrType);
	}
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryDouble(TIFF* tif, TIFFDirEntry* direntry, double* value)
{
	enum TIFFReadDirEntryErr err;
	if (direntry->tdir_count!=1)
		return(TIFFReadDirEntryErrCount);
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
			{
				uint8 m;
				TIFFReadDirEntryCheckedByte(tif,direntry,&m);
				*value=(double)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SBYTE:
			{
				int8 m;
				TIFFReadDirEntryCheckedSbyte(tif,direntry,&m);
				*value=(double)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SHORT:
			{
				uint16 m;
				TIFFReadDirEntryCheckedShort(tif,direntry,&m);
				*value=(double)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SSHORT:
			{
				int16 m;
				TIFFReadDirEntryCheckedSshort(tif,direntry,&m);
				*value=(double)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_LONG:
			{
				uint32 m;
				TIFFReadDirEntryCheckedLong(tif,direntry,&m);
				*value=(double)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SLONG:
			{
				int32 m;
				TIFFReadDirEntryCheckedSlong(tif,direntry,&m);
				*value=(double)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_LONG8:
			{
				uint64 m;
				err=TIFFReadDirEntryCheckedLong8(tif,direntry,&m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
#if defined(__WIN32__) && (_MSC_VER < 1500)
				/*
				 * XXX: MSVC 6.0 does not support conversion
				 * of 64-bit integers into floating point
				 * values.
				 */
				*value = _TIFFUInt64ToDouble(m);
#else
				*value = (double)m;
#endif
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SLONG8:
			{
				int64 m;
				err=TIFFReadDirEntryCheckedSlong8(tif,direntry,&m);
				if (err!=TIFFReadDirEntryErrOk)
					return(err);
				*value=(double)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_RATIONAL:
			err=TIFFReadDirEntryCheckedRational(tif,direntry,value);
			return(err);
		case TIFF_SRATIONAL:
			err=TIFFReadDirEntryCheckedSrational(tif,direntry,value);
			return(err);
		case TIFF_FLOAT:
			{
				float m;
				TIFFReadDirEntryCheckedFloat(tif,direntry,&m);
				*value=(double)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_DOUBLE:
			err=TIFFReadDirEntryCheckedDouble(tif,direntry,value);
			return(err);
		default:
			return(TIFFReadDirEntryErrType);
	}
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryIfd8(TIFF* tif, TIFFDirEntry* direntry, uint64* value)
{
	enum TIFFReadDirEntryErr err;
	if (direntry->tdir_count!=1)
		return(TIFFReadDirEntryErrCount);
	switch (direntry->tdir_type)
	{
		case TIFF_LONG:
		case TIFF_IFD:
			{
				uint32 m;
				TIFFReadDirEntryCheckedLong(tif,direntry,&m);
				*value=(uint64)m;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_LONG8:
		case TIFF_IFD8:
			err=TIFFReadDirEntryCheckedLong8(tif,direntry,value);
			return(err);
		default:
			return(TIFFReadDirEntryErrType);
	}
}


#define INITIAL_THRESHOLD (1024 * 1024)
#define THRESHOLD_MULTIPLIER 10
#define MAX_THRESHOLD (THRESHOLD_MULTIPLIER * THRESHOLD_MULTIPLIER * THRESHOLD_MULTIPLIER * INITIAL_THRESHOLD)

static enum TIFFReadDirEntryErr TIFFReadDirEntryDataAndRealloc(
                    TIFF* tif, uint64 offset, tmsize_t size, void** pdest)
{
#if SIZEOF_VOIDP == 8 || SIZEOF_SIZE_T == 8
        tmsize_t threshold = INITIAL_THRESHOLD;
#endif
        tmsize_t already_read = 0;

        assert( !isMapped(tif) );

        if (!SeekOK(tif,offset))
                return(TIFFReadDirEntryErrIo);

        /* On 64 bit processes, read first a maximum of 1 MB, then 10 MB, etc */
        /* so as to avoid allocating too much memory in case the file is too */
        /* short. We could ask for the file size, but this might be */
        /* expensive with some I/O layers (think of reading a gzipped file) */
        /* Restrict to 64 bit processes, so as to avoid reallocs() */
        /* on 32 bit processes where virtual memory is scarce.  */
        while( already_read < size )
        {
            void* new_dest;
            tmsize_t bytes_read;
            tmsize_t to_read = size - already_read;
#if SIZEOF_VOIDP == 8 || SIZEOF_SIZE_T == 8
            if( to_read >= threshold && threshold < MAX_THRESHOLD )
            {
                to_read = threshold;
                threshold *= THRESHOLD_MULTIPLIER;
            }
#endif

            new_dest = (uint8*) _TIFFrealloc(
                            *pdest, already_read + to_read);
            if( new_dest == NULL )
            {
                TIFFErrorExt(tif->tif_clientdata, tif->tif_name,
                            "Failed to allocate memory for %s "
                            "(%ld elements of %ld bytes each)",
                            "TIFFReadDirEntryArray",
                             (long) 1, (long) (already_read + to_read));
                return TIFFReadDirEntryErrAlloc;
            }
            *pdest = new_dest;

            bytes_read = TIFFReadFile(tif,
                (char*)*pdest + already_read, to_read);
            already_read += bytes_read;
            if (bytes_read != to_read) {
                return TIFFReadDirEntryErrIo;
            }
        }
        return TIFFReadDirEntryErrOk;
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryArrayWithLimit(
    TIFF* tif, TIFFDirEntry* direntry, uint32* count, uint32 desttypesize,
    void** value, uint64 maxcount)
{
	int typesize;
	uint32 datasize;
	void* data;
        uint64 target_count64;
	typesize=TIFFDataWidth(direntry->tdir_type);

        target_count64 = (direntry->tdir_count > maxcount) ?
                maxcount : direntry->tdir_count;

	if ((target_count64==0)||(typesize==0))
	{
		*value=0;
		return(TIFFReadDirEntryErrOk);
	}
        (void) desttypesize;

        /* 
         * As a sanity check, make sure we have no more than a 2GB tag array 
         * in either the current data type or the dest data type.  This also
         * avoids problems with overflow of tmsize_t on 32bit systems.
         */
	if ((uint64)(2147483647/typesize)<target_count64)
		return(TIFFReadDirEntryErrSizesan);
	if ((uint64)(2147483647/desttypesize)<target_count64)
		return(TIFFReadDirEntryErrSizesan);

	*count=(uint32)target_count64;
	datasize=(*count)*typesize;
	assert((tmsize_t)datasize>0);

	if( isMapped(tif) && datasize > (uint32)tif->tif_size )
		return TIFFReadDirEntryErrIo;

	if( !isMapped(tif) &&
		(((tif->tif_flags&TIFF_BIGTIFF) && datasize > 8) ||
		(!(tif->tif_flags&TIFF_BIGTIFF) && datasize > 4)) )
	{
		data = NULL;
	}
	else
	{
		data=_TIFFCheckMalloc(tif, *count, typesize, "ReadDirEntryArray");
		if (data==0)
			return(TIFFReadDirEntryErrAlloc);
	}
	if (!(tif->tif_flags&TIFF_BIGTIFF))
	{
		if (datasize<=4)
			_TIFFmemcpy(data,&direntry->tdir_offset,datasize);
		else
		{
			enum TIFFReadDirEntryErr err;
			uint32 offset = direntry->tdir_offset.toff_long;
			if (tif->tif_flags&TIFF_SWAB)
				TIFFSwabLong(&offset);
			if( isMapped(tif) )
				err=TIFFReadDirEntryData(tif,(uint64)offset,(tmsize_t)datasize,data);
			else
				err=TIFFReadDirEntryDataAndRealloc(tif,(uint64)offset,(tmsize_t)datasize,&data);
			if (err!=TIFFReadDirEntryErrOk)
			{
				_TIFFfree(data);
				return(err);
			}
		}
	}
	else
	{
		if (datasize<=8)
			_TIFFmemcpy(data,&direntry->tdir_offset,datasize);
		else
		{
			enum TIFFReadDirEntryErr err;
			uint64 offset = direntry->tdir_offset.toff_long8;
			if (tif->tif_flags&TIFF_SWAB)
				TIFFSwabLong8(&offset);
			if( isMapped(tif) )
				err=TIFFReadDirEntryData(tif,(uint64)offset,(tmsize_t)datasize,data);
			else
				err=TIFFReadDirEntryDataAndRealloc(tif,(uint64)offset,(tmsize_t)datasize,&data);
			if (err!=TIFFReadDirEntryErrOk)
			{
				_TIFFfree(data);
				return(err);
			}
		}
	}
	*value=data;
	return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryArray(TIFF* tif, TIFFDirEntry* direntry, uint32* count, uint32 desttypesize, void** value)
{
    return TIFFReadDirEntryArrayWithLimit(tif, direntry, count,
                                          desttypesize, value, ~((uint64)0));
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryByteArray(TIFF* tif, TIFFDirEntry* direntry, uint8** value)
{
	enum TIFFReadDirEntryErr err;
	uint32 count;
	void* origdata;
	uint8* data;
	switch (direntry->tdir_type)
	{
		case TIFF_ASCII:
		case TIFF_UNDEFINED:
		case TIFF_BYTE:
		case TIFF_SBYTE:
		case TIFF_SHORT:
		case TIFF_SSHORT:
		case TIFF_LONG:
		case TIFF_SLONG:
		case TIFF_LONG8:
		case TIFF_SLONG8:
			break;
		default:
			return(TIFFReadDirEntryErrType);
	}
	err=TIFFReadDirEntryArray(tif,direntry,&count,1,&origdata);
	if ((err!=TIFFReadDirEntryErrOk)||(origdata==0))
	{
		*value=0;
		return(err);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_ASCII:
		case TIFF_UNDEFINED:
		case TIFF_BYTE:
			*value=(uint8*)origdata;
			return(TIFFReadDirEntryErrOk);
		case TIFF_SBYTE:
			{
				int8* m;
				uint32 n;
				m=(int8*)origdata;
				for (n=0; n<count; n++)
				{
					err=TIFFReadDirEntryCheckRangeByteSbyte(*m);
					if (err!=TIFFReadDirEntryErrOk)
					{
						_TIFFfree(origdata);
						return(err);
					}
					m++;
				}
				*value=(uint8*)origdata;
				return(TIFFReadDirEntryErrOk);
			}
	}
	data=(uint8*)_TIFFmalloc(count);
	if (data==0)
	{
		_TIFFfree(origdata);
		return(TIFFReadDirEntryErrAlloc);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_SHORT:
			{
				uint16* ma;
				uint8* mb;
				uint32 n;
				ma=(uint16*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabShort(ma);
					err=TIFFReadDirEntryCheckRangeByteShort(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(uint8)(*ma++);
				}
			}
			break;
		case TIFF_SSHORT:
			{
				int16* ma;
				uint8* mb;
				uint32 n;
				ma=(int16*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabShort((uint16*)ma);
					err=TIFFReadDirEntryCheckRangeByteSshort(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(uint8)(*ma++);
				}
			}
			break;
		case TIFF_LONG:
			{
				uint32* ma;
				uint8* mb;
				uint32 n;
				ma=(uint32*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong(ma);
					err=TIFFReadDirEntryCheckRangeByteLong(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(uint8)(*ma++);
				}
			}
			break;
		case TIFF_SLONG:
			{
				int32* ma;
				uint8* mb;
				uint32 n;
				ma=(int32*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong((uint32*)ma);
					err=TIFFReadDirEntryCheckRangeByteSlong(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(uint8)(*ma++);
				}
			}
			break;
		case TIFF_LONG8:
			{
				uint64* ma;
				uint8* mb;
				uint32 n;
				ma=(uint64*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong8(ma);
					err=TIFFReadDirEntryCheckRangeByteLong8(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(uint8)(*ma++);
				}
			}
			break;
		case TIFF_SLONG8:
			{
				int64* ma;
				uint8* mb;
				uint32 n;
				ma=(int64*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong8((uint64*)ma);
					err=TIFFReadDirEntryCheckRangeByteSlong8(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(uint8)(*ma++);
				}
			}
			break;
	}
	_TIFFfree(origdata);
	if (err!=TIFFReadDirEntryErrOk)
	{
		_TIFFfree(data);
		return(err);
	}
	*value=data;
	return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntrySbyteArray(TIFF* tif, TIFFDirEntry* direntry, int8** value)
{
	enum TIFFReadDirEntryErr err;
	uint32 count;
	void* origdata;
	int8* data;
	switch (direntry->tdir_type)
	{
		case TIFF_UNDEFINED:
		case TIFF_BYTE:
		case TIFF_SBYTE:
		case TIFF_SHORT:
		case TIFF_SSHORT:
		case TIFF_LONG:
		case TIFF_SLONG:
		case TIFF_LONG8:
		case TIFF_SLONG8:
			break;
		default:
			return(TIFFReadDirEntryErrType);
	}
	err=TIFFReadDirEntryArray(tif,direntry,&count,1,&origdata);
	if ((err!=TIFFReadDirEntryErrOk)||(origdata==0))
	{
		*value=0;
		return(err);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_UNDEFINED:
		case TIFF_BYTE:
			{
				uint8* m;
				uint32 n;
				m=(uint8*)origdata;
				for (n=0; n<count; n++)
				{
					err=TIFFReadDirEntryCheckRangeSbyteByte(*m);
					if (err!=TIFFReadDirEntryErrOk)
					{
						_TIFFfree(origdata);
						return(err);
					}
					m++;
				}
				*value=(int8*)origdata;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SBYTE:
			*value=(int8*)origdata;
			return(TIFFReadDirEntryErrOk);
	}
	data=(int8*)_TIFFmalloc(count);
	if (data==0)
	{
		_TIFFfree(origdata);
		return(TIFFReadDirEntryErrAlloc);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_SHORT:
			{
				uint16* ma;
				int8* mb;
				uint32 n;
				ma=(uint16*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabShort(ma);
					err=TIFFReadDirEntryCheckRangeSbyteShort(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(int8)(*ma++);
				}
			}
			break;
		case TIFF_SSHORT:
			{
				int16* ma;
				int8* mb;
				uint32 n;
				ma=(int16*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabShort((uint16*)ma);
					err=TIFFReadDirEntryCheckRangeSbyteSshort(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(int8)(*ma++);
				}
			}
			break;
		case TIFF_LONG:
			{
				uint32* ma;
				int8* mb;
				uint32 n;
				ma=(uint32*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong(ma);
					err=TIFFReadDirEntryCheckRangeSbyteLong(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(int8)(*ma++);
				}
			}
			break;
		case TIFF_SLONG:
			{
				int32* ma;
				int8* mb;
				uint32 n;
				ma=(int32*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong((uint32*)ma);
					err=TIFFReadDirEntryCheckRangeSbyteSlong(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(int8)(*ma++);
				}
			}
			break;
		case TIFF_LONG8:
			{
				uint64* ma;
				int8* mb;
				uint32 n;
				ma=(uint64*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong8(ma);
					err=TIFFReadDirEntryCheckRangeSbyteLong8(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(int8)(*ma++);
				}
			}
			break;
		case TIFF_SLONG8:
			{
				int64* ma;
				int8* mb;
				uint32 n;
				ma=(int64*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong8((uint64*)ma);
					err=TIFFReadDirEntryCheckRangeSbyteSlong8(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(int8)(*ma++);
				}
			}
			break;
	}
	_TIFFfree(origdata);
	if (err!=TIFFReadDirEntryErrOk)
	{
		_TIFFfree(data);
		return(err);
	}
	*value=data;
	return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryShortArray(TIFF* tif, TIFFDirEntry* direntry, uint16** value)
{
	enum TIFFReadDirEntryErr err;
	uint32 count;
	void* origdata;
	uint16* data;
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
		case TIFF_SBYTE:
		case TIFF_SHORT:
		case TIFF_SSHORT:
		case TIFF_LONG:
		case TIFF_SLONG:
		case TIFF_LONG8:
		case TIFF_SLONG8:
			break;
		default:
			return(TIFFReadDirEntryErrType);
	}
	err=TIFFReadDirEntryArray(tif,direntry,&count,2,&origdata);
	if ((err!=TIFFReadDirEntryErrOk)||(origdata==0))
	{
		*value=0;
		return(err);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_SHORT:
			*value=(uint16*)origdata;
			if (tif->tif_flags&TIFF_SWAB)
				TIFFSwabArrayOfShort(*value,count);  
			return(TIFFReadDirEntryErrOk);
		case TIFF_SSHORT:
			{
				int16* m;
				uint32 n;
				m=(int16*)origdata;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabShort((uint16*)m);
					err=TIFFReadDirEntryCheckRangeShortSshort(*m);
					if (err!=TIFFReadDirEntryErrOk)
					{
						_TIFFfree(origdata);
						return(err);
					}
					m++;
				}
				*value=(uint16*)origdata;
				return(TIFFReadDirEntryErrOk);
			}
	}
	data=(uint16*)_TIFFmalloc(count*2);
	if (data==0)
	{
		_TIFFfree(origdata);
		return(TIFFReadDirEntryErrAlloc);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
			{
				uint8* ma;
				uint16* mb;
				uint32 n;
				ma=(uint8*)origdata;
				mb=data;
				for (n=0; n<count; n++)
					*mb++=(uint16)(*ma++);
			}
			break;
		case TIFF_SBYTE:
			{
				int8* ma;
				uint16* mb;
				uint32 n;
				ma=(int8*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					err=TIFFReadDirEntryCheckRangeShortSbyte(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(uint16)(*ma++);
				}
			}
			break;
		case TIFF_LONG:
			{
				uint32* ma;
				uint16* mb;
				uint32 n;
				ma=(uint32*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong(ma);
					err=TIFFReadDirEntryCheckRangeShortLong(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(uint16)(*ma++);
				}
			}
			break;
		case TIFF_SLONG:
			{
				int32* ma;
				uint16* mb;
				uint32 n;
				ma=(int32*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong((uint32*)ma);
					err=TIFFReadDirEntryCheckRangeShortSlong(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(uint16)(*ma++);
				}
			}
			break;
		case TIFF_LONG8:
			{
				uint64* ma;
				uint16* mb;
				uint32 n;
				ma=(uint64*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong8(ma);
					err=TIFFReadDirEntryCheckRangeShortLong8(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(uint16)(*ma++);
				}
			}
			break;
		case TIFF_SLONG8:
			{
				int64* ma;
				uint16* mb;
				uint32 n;
				ma=(int64*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong8((uint64*)ma);
					err=TIFFReadDirEntryCheckRangeShortSlong8(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(uint16)(*ma++);
				}
			}
			break;
	}
	_TIFFfree(origdata);
	if (err!=TIFFReadDirEntryErrOk)
	{
		_TIFFfree(data);
		return(err);
	}
	*value=data;
	return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntrySshortArray(TIFF* tif, TIFFDirEntry* direntry, int16** value)
{
	enum TIFFReadDirEntryErr err;
	uint32 count;
	void* origdata;
	int16* data;
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
		case TIFF_SBYTE:
		case TIFF_SHORT:
		case TIFF_SSHORT:
		case TIFF_LONG:
		case TIFF_SLONG:
		case TIFF_LONG8:
		case TIFF_SLONG8:
			break;
		default:
			return(TIFFReadDirEntryErrType);
	}
	err=TIFFReadDirEntryArray(tif,direntry,&count,2,&origdata);
	if ((err!=TIFFReadDirEntryErrOk)||(origdata==0))
	{
		*value=0;
		return(err);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_SHORT:
			{
				uint16* m;
				uint32 n;
				m=(uint16*)origdata;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabShort(m);
					err=TIFFReadDirEntryCheckRangeSshortShort(*m);
					if (err!=TIFFReadDirEntryErrOk)
					{
						_TIFFfree(origdata);
						return(err);
					}
					m++;
				}
				*value=(int16*)origdata;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SSHORT:
			*value=(int16*)origdata;
			if (tif->tif_flags&TIFF_SWAB)
				TIFFSwabArrayOfShort((uint16*)(*value),count);
			return(TIFFReadDirEntryErrOk);
	}
	data=(int16*)_TIFFmalloc(count*2);
	if (data==0)
	{
		_TIFFfree(origdata);
		return(TIFFReadDirEntryErrAlloc);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
			{
				uint8* ma;
				int16* mb;
				uint32 n;
				ma=(uint8*)origdata;
				mb=data;
				for (n=0; n<count; n++)
					*mb++=(int16)(*ma++);
			}
			break;
		case TIFF_SBYTE:
			{
				int8* ma;
				int16* mb;
				uint32 n;
				ma=(int8*)origdata;
				mb=data;
				for (n=0; n<count; n++)
					*mb++=(int16)(*ma++);
			}
			break;
		case TIFF_LONG:
			{
				uint32* ma;
				int16* mb;
				uint32 n;
				ma=(uint32*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong(ma);
					err=TIFFReadDirEntryCheckRangeSshortLong(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(int16)(*ma++);
				}
			}
			break;
		case TIFF_SLONG:
			{
				int32* ma;
				int16* mb;
				uint32 n;
				ma=(int32*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong((uint32*)ma);
					err=TIFFReadDirEntryCheckRangeSshortSlong(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(int16)(*ma++);
				}
			}
			break;
		case TIFF_LONG8:
			{
				uint64* ma;
				int16* mb;
				uint32 n;
				ma=(uint64*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong8(ma);
					err=TIFFReadDirEntryCheckRangeSshortLong8(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(int16)(*ma++);
				}
			}
			break;
		case TIFF_SLONG8:
			{
				int64* ma;
				int16* mb;
				uint32 n;
				ma=(int64*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong8((uint64*)ma);
					err=TIFFReadDirEntryCheckRangeSshortSlong8(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(int16)(*ma++);
				}
			}
			break;
	}
	_TIFFfree(origdata);
	if (err!=TIFFReadDirEntryErrOk)
	{
		_TIFFfree(data);
		return(err);
	}
	*value=data;
	return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryLongArray(TIFF* tif, TIFFDirEntry* direntry, uint32** value)
{
	enum TIFFReadDirEntryErr err;
	uint32 count;
	void* origdata;
	uint32* data;
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
		case TIFF_SBYTE:
		case TIFF_SHORT:
		case TIFF_SSHORT:
		case TIFF_LONG:
		case TIFF_SLONG:
		case TIFF_LONG8:
		case TIFF_SLONG8:
			break;
		default:
			return(TIFFReadDirEntryErrType);
	}
	err=TIFFReadDirEntryArray(tif,direntry,&count,4,&origdata);
	if ((err!=TIFFReadDirEntryErrOk)||(origdata==0))
	{
		*value=0;
		return(err);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_LONG:
			*value=(uint32*)origdata;
			if (tif->tif_flags&TIFF_SWAB)
				TIFFSwabArrayOfLong(*value,count);
			return(TIFFReadDirEntryErrOk);
		case TIFF_SLONG:
			{
				int32* m;
				uint32 n;
				m=(int32*)origdata;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong((uint32*)m);
					err=TIFFReadDirEntryCheckRangeLongSlong(*m);
					if (err!=TIFFReadDirEntryErrOk)
					{
						_TIFFfree(origdata);
						return(err);
					}
					m++;
				}
				*value=(uint32*)origdata;
				return(TIFFReadDirEntryErrOk);
			}
	}
	data=(uint32*)_TIFFmalloc(count*4);
	if (data==0)
	{
		_TIFFfree(origdata);
		return(TIFFReadDirEntryErrAlloc);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
			{
				uint8* ma;
				uint32* mb;
				uint32 n;
				ma=(uint8*)origdata;
				mb=data;
				for (n=0; n<count; n++)
					*mb++=(uint32)(*ma++);
			}
			break;
		case TIFF_SBYTE:
			{
				int8* ma;
				uint32* mb;
				uint32 n;
				ma=(int8*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					err=TIFFReadDirEntryCheckRangeLongSbyte(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(uint32)(*ma++);
				}
			}
			break;
		case TIFF_SHORT:
			{
				uint16* ma;
				uint32* mb;
				uint32 n;
				ma=(uint16*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabShort(ma);
					*mb++=(uint32)(*ma++);
				}
			}
			break;
		case TIFF_SSHORT:
			{
				int16* ma;
				uint32* mb;
				uint32 n;
				ma=(int16*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabShort((uint16*)ma);
					err=TIFFReadDirEntryCheckRangeLongSshort(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(uint32)(*ma++);
				}
			}
			break;
		case TIFF_LONG8:
			{
				uint64* ma;
				uint32* mb;
				uint32 n;
				ma=(uint64*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong8(ma);
					err=TIFFReadDirEntryCheckRangeLongLong8(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(uint32)(*ma++);
				}
			}
			break;
		case TIFF_SLONG8:
			{
				int64* ma;
				uint32* mb;
				uint32 n;
				ma=(int64*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong8((uint64*)ma);
					err=TIFFReadDirEntryCheckRangeLongSlong8(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(uint32)(*ma++);
				}
			}
			break;
	}
	_TIFFfree(origdata);
	if (err!=TIFFReadDirEntryErrOk)
	{
		_TIFFfree(data);
		return(err);
	}
	*value=data;
	return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntrySlongArray(TIFF* tif, TIFFDirEntry* direntry, int32** value)
{
	enum TIFFReadDirEntryErr err;
	uint32 count;
	void* origdata;
	int32* data;
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
		case TIFF_SBYTE:
		case TIFF_SHORT:
		case TIFF_SSHORT:
		case TIFF_LONG:
		case TIFF_SLONG:
		case TIFF_LONG8:
		case TIFF_SLONG8:
			break;
		default:
			return(TIFFReadDirEntryErrType);
	}
	err=TIFFReadDirEntryArray(tif,direntry,&count,4,&origdata);
	if ((err!=TIFFReadDirEntryErrOk)||(origdata==0))
	{
		*value=0;
		return(err);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_LONG:
			{
				uint32* m;
				uint32 n;
				m=(uint32*)origdata;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong((uint32*)m);
					err=TIFFReadDirEntryCheckRangeSlongLong(*m);
					if (err!=TIFFReadDirEntryErrOk)
					{
						_TIFFfree(origdata);
						return(err);
					}
					m++;
				}
				*value=(int32*)origdata;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SLONG:
			*value=(int32*)origdata;
			if (tif->tif_flags&TIFF_SWAB)
				TIFFSwabArrayOfLong((uint32*)(*value),count);
			return(TIFFReadDirEntryErrOk);
	}
	data=(int32*)_TIFFmalloc(count*4);
	if (data==0)
	{
		_TIFFfree(origdata);
		return(TIFFReadDirEntryErrAlloc);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
			{
				uint8* ma;
				int32* mb;
				uint32 n;
				ma=(uint8*)origdata;
				mb=data;
				for (n=0; n<count; n++)
					*mb++=(int32)(*ma++);
			}
			break;
		case TIFF_SBYTE:
			{
				int8* ma;
				int32* mb;
				uint32 n;
				ma=(int8*)origdata;
				mb=data;
				for (n=0; n<count; n++)
					*mb++=(int32)(*ma++);
			}
			break;
		case TIFF_SHORT:
			{
				uint16* ma;
				int32* mb;
				uint32 n;
				ma=(uint16*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabShort(ma);
					*mb++=(int32)(*ma++);
				}
			}
			break;
		case TIFF_SSHORT:
			{
				int16* ma;
				int32* mb;
				uint32 n;
				ma=(int16*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabShort((uint16*)ma);
					*mb++=(int32)(*ma++);
				}
			}
			break;
		case TIFF_LONG8:
			{
				uint64* ma;
				int32* mb;
				uint32 n;
				ma=(uint64*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong8(ma);
					err=TIFFReadDirEntryCheckRangeSlongLong8(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(int32)(*ma++);
				}
			}
			break;
		case TIFF_SLONG8:
			{
				int64* ma;
				int32* mb;
				uint32 n;
				ma=(int64*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong8((uint64*)ma);
					err=TIFFReadDirEntryCheckRangeSlongSlong8(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(int32)(*ma++);
				}
			}
			break;
	}
	_TIFFfree(origdata);
	if (err!=TIFFReadDirEntryErrOk)
	{
		_TIFFfree(data);
		return(err);
	}
	*value=data;
	return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryLong8ArrayWithLimit(
        TIFF* tif, TIFFDirEntry* direntry, uint64** value, uint64 maxcount)
{
	enum TIFFReadDirEntryErr err;
	uint32 count;
	void* origdata;
	uint64* data;
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
		case TIFF_SBYTE:
		case TIFF_SHORT:
		case TIFF_SSHORT:
		case TIFF_LONG:
		case TIFF_SLONG:
		case TIFF_LONG8:
		case TIFF_SLONG8:
			break;
		default:
			return(TIFFReadDirEntryErrType);
	}
	err=TIFFReadDirEntryArrayWithLimit(tif,direntry,&count,8,&origdata,maxcount);
	if ((err!=TIFFReadDirEntryErrOk)||(origdata==0))
	{
		*value=0;
		return(err);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_LONG8:
			*value=(uint64*)origdata;
			if (tif->tif_flags&TIFF_SWAB)
				TIFFSwabArrayOfLong8(*value,count);
			return(TIFFReadDirEntryErrOk);
		case TIFF_SLONG8:
			{
				int64* m;
				uint32 n;
				m=(int64*)origdata;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong8((uint64*)m);
					err=TIFFReadDirEntryCheckRangeLong8Slong8(*m);
					if (err!=TIFFReadDirEntryErrOk)
					{
						_TIFFfree(origdata);
						return(err);
					}
					m++;
				}
				*value=(uint64*)origdata;
				return(TIFFReadDirEntryErrOk);
			}
	}
	data=(uint64*)_TIFFmalloc(count*8);
	if (data==0)
	{
		_TIFFfree(origdata);
		return(TIFFReadDirEntryErrAlloc);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
			{
				uint8* ma;
				uint64* mb;
				uint32 n;
				ma=(uint8*)origdata;
				mb=data;
				for (n=0; n<count; n++)
					*mb++=(uint64)(*ma++);
			}
			break;
		case TIFF_SBYTE:
			{
				int8* ma;
				uint64* mb;
				uint32 n;
				ma=(int8*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					err=TIFFReadDirEntryCheckRangeLong8Sbyte(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(uint64)(*ma++);
				}
			}
			break;
		case TIFF_SHORT:
			{
				uint16* ma;
				uint64* mb;
				uint32 n;
				ma=(uint16*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabShort(ma);
					*mb++=(uint64)(*ma++);
				}
			}
			break;
		case TIFF_SSHORT:
			{
				int16* ma;
				uint64* mb;
				uint32 n;
				ma=(int16*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabShort((uint16*)ma);
					err=TIFFReadDirEntryCheckRangeLong8Sshort(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(uint64)(*ma++);
				}
			}
			break;
		case TIFF_LONG:
			{
				uint32* ma;
				uint64* mb;
				uint32 n;
				ma=(uint32*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong(ma);
					*mb++=(uint64)(*ma++);
				}
			}
			break;
		case TIFF_SLONG:
			{
				int32* ma;
				uint64* mb;
				uint32 n;
				ma=(int32*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong((uint32*)ma);
					err=TIFFReadDirEntryCheckRangeLong8Slong(*ma);
					if (err!=TIFFReadDirEntryErrOk)
						break;
					*mb++=(uint64)(*ma++);
				}
			}
			break;
	}
	_TIFFfree(origdata);
	if (err!=TIFFReadDirEntryErrOk)
	{
		_TIFFfree(data);
		return(err);
	}
	*value=data;
	return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryLong8Array(TIFF* tif, TIFFDirEntry* direntry, uint64** value)
{
    return TIFFReadDirEntryLong8ArrayWithLimit(tif, direntry, value, ~((uint64)0));
}

static enum TIFFReadDirEntryErr TIFFReadDirEntrySlong8Array(TIFF* tif, TIFFDirEntry* direntry, int64** value)
{
	enum TIFFReadDirEntryErr err;
	uint32 count;
	void* origdata;
	int64* data;
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
		case TIFF_SBYTE:
		case TIFF_SHORT:
		case TIFF_SSHORT:
		case TIFF_LONG:
		case TIFF_SLONG:
		case TIFF_LONG8:
		case TIFF_SLONG8:
			break;
		default:
			return(TIFFReadDirEntryErrType);
	}
	err=TIFFReadDirEntryArray(tif,direntry,&count,8,&origdata);
	if ((err!=TIFFReadDirEntryErrOk)||(origdata==0))
	{
		*value=0;
		return(err);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_LONG8:
			{
				uint64* m;
				uint32 n;
				m=(uint64*)origdata;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong8(m);
					err=TIFFReadDirEntryCheckRangeSlong8Long8(*m);
					if (err!=TIFFReadDirEntryErrOk)
					{
						_TIFFfree(origdata);
						return(err);
					}
					m++;
				}
				*value=(int64*)origdata;
				return(TIFFReadDirEntryErrOk);
			}
		case TIFF_SLONG8:
			*value=(int64*)origdata;
			if (tif->tif_flags&TIFF_SWAB)
				TIFFSwabArrayOfLong8((uint64*)(*value),count);
			return(TIFFReadDirEntryErrOk);
	}
	data=(int64*)_TIFFmalloc(count*8);
	if (data==0)
	{
		_TIFFfree(origdata);
		return(TIFFReadDirEntryErrAlloc);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
			{
				uint8* ma;
				int64* mb;
				uint32 n;
				ma=(uint8*)origdata;
				mb=data;
				for (n=0; n<count; n++)
					*mb++=(int64)(*ma++);
			}
			break;
		case TIFF_SBYTE:
			{
				int8* ma;
				int64* mb;
				uint32 n;
				ma=(int8*)origdata;
				mb=data;
				for (n=0; n<count; n++)
					*mb++=(int64)(*ma++);
			}
			break;
		case TIFF_SHORT:
			{
				uint16* ma;
				int64* mb;
				uint32 n;
				ma=(uint16*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabShort(ma);
					*mb++=(int64)(*ma++);
				}
			}
			break;
		case TIFF_SSHORT:
			{
				int16* ma;
				int64* mb;
				uint32 n;
				ma=(int16*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabShort((uint16*)ma);
					*mb++=(int64)(*ma++);
				}
			}
			break;
		case TIFF_LONG:
			{
				uint32* ma;
				int64* mb;
				uint32 n;
				ma=(uint32*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong(ma);
					*mb++=(int64)(*ma++);
				}
			}
			break;
		case TIFF_SLONG:
			{
				int32* ma;
				int64* mb;
				uint32 n;
				ma=(int32*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong((uint32*)ma);
					*mb++=(int64)(*ma++);
				}
			}
			break;
	}
	_TIFFfree(origdata);
	*value=data;
	return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryFloatArray(TIFF* tif, TIFFDirEntry* direntry, float** value)
{
	enum TIFFReadDirEntryErr err;
	uint32 count;
	void* origdata;
	float* data;
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
		case TIFF_SBYTE:
		case TIFF_SHORT:
		case TIFF_SSHORT:
		case TIFF_LONG:
		case TIFF_SLONG:
		case TIFF_LONG8:
		case TIFF_SLONG8:
		case TIFF_RATIONAL:
		case TIFF_SRATIONAL:
		case TIFF_FLOAT:
		case TIFF_DOUBLE:
			break;
		default:
			return(TIFFReadDirEntryErrType);
	}
	err=TIFFReadDirEntryArray(tif,direntry,&count,4,&origdata);
	if ((err!=TIFFReadDirEntryErrOk)||(origdata==0))
	{
		*value=0;
		return(err);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_FLOAT:
			if (tif->tif_flags&TIFF_SWAB)
				TIFFSwabArrayOfLong((uint32*)origdata,count);  
			TIFFCvtIEEEDoubleToNative(tif,count,(float*)origdata);
			*value=(float*)origdata;
			return(TIFFReadDirEntryErrOk);
	}
	data=(float*)_TIFFmalloc(count*sizeof(float));
	if (data==0)
	{
		_TIFFfree(origdata);
		return(TIFFReadDirEntryErrAlloc);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
			{
				uint8* ma;
				float* mb;
				uint32 n;
				ma=(uint8*)origdata;
				mb=data;
				for (n=0; n<count; n++)
					*mb++=(float)(*ma++);
			}
			break;
		case TIFF_SBYTE:
			{
				int8* ma;
				float* mb;
				uint32 n;
				ma=(int8*)origdata;
				mb=data;
				for (n=0; n<count; n++)
					*mb++=(float)(*ma++);
			}
			break;
		case TIFF_SHORT:
			{
				uint16* ma;
				float* mb;
				uint32 n;
				ma=(uint16*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabShort(ma);
					*mb++=(float)(*ma++);
				}
			}
			break;
		case TIFF_SSHORT:
			{
				int16* ma;
				float* mb;
				uint32 n;
				ma=(int16*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabShort((uint16*)ma);
					*mb++=(float)(*ma++);
				}
			}
			break;
		case TIFF_LONG:
			{
				uint32* ma;
				float* mb;
				uint32 n;
				ma=(uint32*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong(ma);
					*mb++=(float)(*ma++);
				}
			}
			break;
		case TIFF_SLONG:
			{
				int32* ma;
				float* mb;
				uint32 n;
				ma=(int32*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong((uint32*)ma);
					*mb++=(float)(*ma++);
				}
			}
			break;
		case TIFF_LONG8:
			{
				uint64* ma;
				float* mb;
				uint32 n;
				ma=(uint64*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong8(ma);
#if defined(__WIN32__) && (_MSC_VER < 1500)
					/*
					 * XXX: MSVC 6.0 does not support
					 * conversion of 64-bit integers into
					 * floating point values.
					 */
					*mb++ = _TIFFUInt64ToFloat(*ma++);
#else
					*mb++ = (float)(*ma++);
#endif
				}
			}
			break;
		case TIFF_SLONG8:
			{
				int64* ma;
				float* mb;
				uint32 n;
				ma=(int64*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong8((uint64*)ma);
					*mb++=(float)(*ma++);
				}
			}
			break;
		case TIFF_RATIONAL:
			{
				uint32* ma;
				uint32 maa;
				uint32 mab;
				float* mb;
				uint32 n;
				ma=(uint32*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong(ma);
					maa=*ma++;
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong(ma);
					mab=*ma++;
					if (mab==0)
						*mb++=0.0;
					else
						*mb++=(float)maa/(float)mab;
				}
			}
			break;
		case TIFF_SRATIONAL:
			{
				uint32* ma;
				int32 maa;
				uint32 mab;
				float* mb;
				uint32 n;
				ma=(uint32*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong(ma);
					maa=*(int32*)ma;
					ma++;
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong(ma);
					mab=*ma++;
					if (mab==0)
						*mb++=0.0;
					else
						*mb++=(float)maa/(float)mab;
				}
			}
			break;
		case TIFF_DOUBLE:
			{
				double* ma;
				float* mb;
				uint32 n;
				if (tif->tif_flags&TIFF_SWAB)
					TIFFSwabArrayOfLong8((uint64*)origdata,count);
				TIFFCvtIEEEDoubleToNative(tif,count,(double*)origdata);
				ma=(double*)origdata;
				mb=data;
				for (n=0; n<count; n++)
                                {
                                    double val = *ma++;
                                    if( val > FLT_MAX )
                                        val = FLT_MAX;
                                    else if( val < -FLT_MAX )
                                        val = -FLT_MAX;
                                    *mb++=(float)val;
                                }
			}
			break;
	}
	_TIFFfree(origdata);
	*value=data;
	return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr
TIFFReadDirEntryDoubleArray(TIFF* tif, TIFFDirEntry* direntry, double** value)
{
	enum TIFFReadDirEntryErr err;
	uint32 count;
	void* origdata;
	double* data;
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
		case TIFF_SBYTE:
		case TIFF_SHORT:
		case TIFF_SSHORT:
		case TIFF_LONG:
		case TIFF_SLONG:
		case TIFF_LONG8:
		case TIFF_SLONG8:
		case TIFF_RATIONAL:
		case TIFF_SRATIONAL:
		case TIFF_FLOAT:
		case TIFF_DOUBLE:
			break;
		default:
			return(TIFFReadDirEntryErrType);
	}
	err=TIFFReadDirEntryArray(tif,direntry,&count,8,&origdata);
	if ((err!=TIFFReadDirEntryErrOk)||(origdata==0))
	{
		*value=0;
		return(err);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_DOUBLE:
			if (tif->tif_flags&TIFF_SWAB)
				TIFFSwabArrayOfLong8((uint64*)origdata,count);
			TIFFCvtIEEEDoubleToNative(tif,count,(double*)origdata);
			*value=(double*)origdata;
			return(TIFFReadDirEntryErrOk);
	}
	data=(double*)_TIFFmalloc(count*sizeof(double));
	if (data==0)
	{
		_TIFFfree(origdata);
		return(TIFFReadDirEntryErrAlloc);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_BYTE:
			{
				uint8* ma;
				double* mb;
				uint32 n;
				ma=(uint8*)origdata;
				mb=data;
				for (n=0; n<count; n++)
					*mb++=(double)(*ma++);
			}
			break;
		case TIFF_SBYTE:
			{
				int8* ma;
				double* mb;
				uint32 n;
				ma=(int8*)origdata;
				mb=data;
				for (n=0; n<count; n++)
					*mb++=(double)(*ma++);
			}
			break;
		case TIFF_SHORT:
			{
				uint16* ma;
				double* mb;
				uint32 n;
				ma=(uint16*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabShort(ma);
					*mb++=(double)(*ma++);
				}
			}
			break;
		case TIFF_SSHORT:
			{
				int16* ma;
				double* mb;
				uint32 n;
				ma=(int16*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabShort((uint16*)ma);
					*mb++=(double)(*ma++);
				}
			}
			break;
		case TIFF_LONG:
			{
				uint32* ma;
				double* mb;
				uint32 n;
				ma=(uint32*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong(ma);
					*mb++=(double)(*ma++);
				}
			}
			break;
		case TIFF_SLONG:
			{
				int32* ma;
				double* mb;
				uint32 n;
				ma=(int32*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong((uint32*)ma);
					*mb++=(double)(*ma++);
				}
			}
			break;
		case TIFF_LONG8:
			{
				uint64* ma;
				double* mb;
				uint32 n;
				ma=(uint64*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong8(ma);
#if defined(__WIN32__) && (_MSC_VER < 1500)
					/*
					 * XXX: MSVC 6.0 does not support
					 * conversion of 64-bit integers into
					 * floating point values.
					 */
					*mb++ = _TIFFUInt64ToDouble(*ma++);
#else
					*mb++ = (double)(*ma++);
#endif
				}
			}
			break;
		case TIFF_SLONG8:
			{
				int64* ma;
				double* mb;
				uint32 n;
				ma=(int64*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong8((uint64*)ma);
					*mb++=(double)(*ma++);
				}
			}
			break;
		case TIFF_RATIONAL:
			{
				uint32* ma;
				uint32 maa;
				uint32 mab;
				double* mb;
				uint32 n;
				ma=(uint32*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong(ma);
					maa=*ma++;
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong(ma);
					mab=*ma++;
					if (mab==0)
						*mb++=0.0;
					else
						*mb++=(double)maa/(double)mab;
				}
			}
			break;
		case TIFF_SRATIONAL:
			{
				uint32* ma;
				int32 maa;
				uint32 mab;
				double* mb;
				uint32 n;
				ma=(uint32*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong(ma);
					maa=*(int32*)ma;
					ma++;
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong(ma);
					mab=*ma++;
					if (mab==0)
						*mb++=0.0;
					else
						*mb++=(double)maa/(double)mab;
				}
			}
			break;
		case TIFF_FLOAT:
			{
				float* ma;
				double* mb;
				uint32 n;
				if (tif->tif_flags&TIFF_SWAB)
					TIFFSwabArrayOfLong((uint32*)origdata,count);  
				TIFFCvtIEEEFloatToNative(tif,count,(float*)origdata);
				ma=(float*)origdata;
				mb=data;
				for (n=0; n<count; n++)
					*mb++=(double)(*ma++);
			}
			break;
	}
	_TIFFfree(origdata);
	*value=data;
	return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryIfd8Array(TIFF* tif, TIFFDirEntry* direntry, uint64** value)
{
	enum TIFFReadDirEntryErr err;
	uint32 count;
	void* origdata;
	uint64* data;
	switch (direntry->tdir_type)
	{
		case TIFF_LONG:
		case TIFF_LONG8:
		case TIFF_IFD:
		case TIFF_IFD8:
			break;
		default:
			return(TIFFReadDirEntryErrType);
	}
	err=TIFFReadDirEntryArray(tif,direntry,&count,8,&origdata);
	if ((err!=TIFFReadDirEntryErrOk)||(origdata==0))
	{
		*value=0;
		return(err);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_LONG8:
		case TIFF_IFD8:
			*value=(uint64*)origdata;
			if (tif->tif_flags&TIFF_SWAB)
				TIFFSwabArrayOfLong8(*value,count);
			return(TIFFReadDirEntryErrOk);
	}
	data=(uint64*)_TIFFmalloc(count*8);
	if (data==0)
	{
		_TIFFfree(origdata);
		return(TIFFReadDirEntryErrAlloc);
	}
	switch (direntry->tdir_type)
	{
		case TIFF_LONG:
		case TIFF_IFD:
			{
				uint32* ma;
				uint64* mb;
				uint32 n;
				ma=(uint32*)origdata;
				mb=data;
				for (n=0; n<count; n++)
				{
					if (tif->tif_flags&TIFF_SWAB)
						TIFFSwabLong(ma);
					*mb++=(uint64)(*ma++);
				}
			}
			break;
	}
	_TIFFfree(origdata);
	*value=data;
	return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryPersampleShort(TIFF* tif, TIFFDirEntry* direntry, uint16* value)
{
	enum TIFFReadDirEntryErr err;
	uint16* m;
	uint16* na;
	uint16 nb;
	if (direntry->tdir_count<(uint64)tif->tif_dir.td_samplesperpixel)
		return(TIFFReadDirEntryErrCount);
	err=TIFFReadDirEntryShortArray(tif,direntry,&m);
	if (err!=TIFFReadDirEntryErrOk || m == NULL)
		return(err);
	na=m;
	nb=tif->tif_dir.td_samplesperpixel;
	*value=*na++;
	nb--;
	while (nb>0)
	{
		if (*na++!=*value)
		{
			err=TIFFReadDirEntryErrPsdif;
			break;
		}
		nb--;
	}
	_TIFFfree(m);
	return(err);
}

#if 0
static enum TIFFReadDirEntryErr TIFFReadDirEntryPersampleDouble(TIFF* tif, TIFFDirEntry* direntry, double* value)
{
	enum TIFFReadDirEntryErr err;
	double* m;
	double* na;
	uint16 nb;
	if (direntry->tdir_count<(uint64)tif->tif_dir.td_samplesperpixel)
		return(TIFFReadDirEntryErrCount);
	err=TIFFReadDirEntryDoubleArray(tif,direntry,&m);
	if (err!=TIFFReadDirEntryErrOk)
		return(err);
	na=m;
	nb=tif->tif_dir.td_samplesperpixel;
	*value=*na++;
	nb--;
	while (nb>0)
	{
		if (*na++!=*value)
		{
			err=TIFFReadDirEntryErrPsdif;
			break;
		}
		nb--;
	}
	_TIFFfree(m);
	return(err);
}
#endif

static void TIFFReadDirEntryCheckedByte(TIFF* tif, TIFFDirEntry* direntry, uint8* value)
{
	(void) tif;
	*value=*(uint8*)(&direntry->tdir_offset);
}

static void TIFFReadDirEntryCheckedSbyte(TIFF* tif, TIFFDirEntry* direntry, int8* value)
{
	(void) tif;
	*value=*(int8*)(&direntry->tdir_offset);
}

static void TIFFReadDirEntryCheckedShort(TIFF* tif, TIFFDirEntry* direntry, uint16* value)
{
	*value = direntry->tdir_offset.toff_short;
	/* *value=*(uint16*)(&direntry->tdir_offset); */
	if (tif->tif_flags&TIFF_SWAB)
		TIFFSwabShort(value);
}

static void TIFFReadDirEntryCheckedSshort(TIFF* tif, TIFFDirEntry* direntry, int16* value)
{
	*value=*(int16*)(&direntry->tdir_offset);
	if (tif->tif_flags&TIFF_SWAB)
		TIFFSwabShort((uint16*)value);
}

static void TIFFReadDirEntryCheckedLong(TIFF* tif, TIFFDirEntry* direntry, uint32* value)
{
	*value=*(uint32*)(&direntry->tdir_offset);
	if (tif->tif_flags&TIFF_SWAB)
		TIFFSwabLong(value);
}

static void TIFFReadDirEntryCheckedSlong(TIFF* tif, TIFFDirEntry* direntry, int32* value)
{
	*value=*(int32*)(&direntry->tdir_offset);
	if (tif->tif_flags&TIFF_SWAB)
		TIFFSwabLong((uint32*)value);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckedLong8(TIFF* tif, TIFFDirEntry* direntry, uint64* value)
{
	if (!(tif->tif_flags&TIFF_BIGTIFF))
	{
		enum TIFFReadDirEntryErr err;
		uint32 offset = direntry->tdir_offset.toff_long;
		if (tif->tif_flags&TIFF_SWAB)
			TIFFSwabLong(&offset);
		err=TIFFReadDirEntryData(tif,offset,8,value);
		if (err!=TIFFReadDirEntryErrOk)
			return(err);
	}
	else
		*value = direntry->tdir_offset.toff_long8;
	if (tif->tif_flags&TIFF_SWAB)
		TIFFSwabLong8(value);
	return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckedSlong8(TIFF* tif, TIFFDirEntry* direntry, int64* value)
{
	if (!(tif->tif_flags&TIFF_BIGTIFF))
	{
		enum TIFFReadDirEntryErr err;
		uint32 offset = direntry->tdir_offset.toff_long;
		if (tif->tif_flags&TIFF_SWAB)
			TIFFSwabLong(&offset);
		err=TIFFReadDirEntryData(tif,offset,8,value);
		if (err!=TIFFReadDirEntryErrOk)
			return(err);
	}
	else
		*value=*(int64*)(&direntry->tdir_offset);
	if (tif->tif_flags&TIFF_SWAB)
		TIFFSwabLong8((uint64*)value);
	return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckedRational(TIFF* tif, TIFFDirEntry* direntry, double* value)
{
	UInt64Aligned_t m;

	assert(sizeof(double)==8);
	assert(sizeof(uint64)==8);
	assert(sizeof(uint32)==4);
	if (!(tif->tif_flags&TIFF_BIGTIFF))
	{
		enum TIFFReadDirEntryErr err;
		uint32 offset = direntry->tdir_offset.toff_long;
		if (tif->tif_flags&TIFF_SWAB)
			TIFFSwabLong(&offset);
		err=TIFFReadDirEntryData(tif,offset,8,m.i);
		if (err!=TIFFReadDirEntryErrOk)
			return(err);
	}
	else
		m.l = direntry->tdir_offset.toff_long8;
	if (tif->tif_flags&TIFF_SWAB)
		TIFFSwabArrayOfLong(m.i,2);
        /* Not completely sure what we should do when m.i[1]==0, but some */
        /* sanitizers do not like division by 0.0: */
        /* http://bugzilla.maptools.org/show_bug.cgi?id=2644 */
	if (m.i[0]==0 || m.i[1]==0)
		*value=0.0;
	else
		*value=(double)m.i[0]/(double)m.i[1];
	return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckedSrational(TIFF* tif, TIFFDirEntry* direntry, double* value)
{
	UInt64Aligned_t m;
	assert(sizeof(double)==8);
	assert(sizeof(uint64)==8);
	assert(sizeof(int32)==4);
	assert(sizeof(uint32)==4);
	if (!(tif->tif_flags&TIFF_BIGTIFF))
	{
		enum TIFFReadDirEntryErr err;
		uint32 offset = direntry->tdir_offset.toff_long;
		if (tif->tif_flags&TIFF_SWAB)
			TIFFSwabLong(&offset);
		err=TIFFReadDirEntryData(tif,offset,8,m.i);
		if (err!=TIFFReadDirEntryErrOk)
			return(err);
	}
	else
		m.l=direntry->tdir_offset.toff_long8;
	if (tif->tif_flags&TIFF_SWAB)
		TIFFSwabArrayOfLong(m.i,2);
        /* Not completely sure what we should do when m.i[1]==0, but some */
        /* sanitizers do not like division by 0.0: */
        /* http://bugzilla.maptools.org/show_bug.cgi?id=2644 */
	if ((int32)m.i[0]==0 || m.i[1]==0)
		*value=0.0;
	else
		*value=(double)((int32)m.i[0])/(double)m.i[1];
	return(TIFFReadDirEntryErrOk);
}

static void TIFFReadDirEntryCheckedFloat(TIFF* tif, TIFFDirEntry* direntry, float* value)
{
         union
	 {
	   float  f;
	   uint32 i;
	 } float_union;
	assert(sizeof(float)==4);
	assert(sizeof(uint32)==4);
	assert(sizeof(float_union)==4);
	float_union.i=*(uint32*)(&direntry->tdir_offset);
	*value=float_union.f;
	if (tif->tif_flags&TIFF_SWAB)
		TIFFSwabLong((uint32*)value);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckedDouble(TIFF* tif, TIFFDirEntry* direntry, double* value)
{
	assert(sizeof(double)==8);
	assert(sizeof(uint64)==8);
	assert(sizeof(UInt64Aligned_t)==8);
	if (!(tif->tif_flags&TIFF_BIGTIFF))
	{
		enum TIFFReadDirEntryErr err;
		uint32 offset = direntry->tdir_offset.toff_long;
		if (tif->tif_flags&TIFF_SWAB)
			TIFFSwabLong(&offset);
		err=TIFFReadDirEntryData(tif,offset,8,value);
		if (err!=TIFFReadDirEntryErrOk)
			return(err);
	}
	else
	{
	       UInt64Aligned_t uint64_union;
	       uint64_union.l=direntry->tdir_offset.toff_long8;
	       *value=uint64_union.d;
	}
	if (tif->tif_flags&TIFF_SWAB)
		TIFFSwabLong8((uint64*)value);
	return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeByteSbyte(int8 value)
{
	if (value<0)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeByteShort(uint16 value)
{
	if (value>0xFF)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeByteSshort(int16 value)
{
	if ((value<0)||(value>0xFF))
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeByteLong(uint32 value)
{
	if (value>0xFF)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeByteSlong(int32 value)
{
	if ((value<0)||(value>0xFF))
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeByteLong8(uint64 value)
{
	if (value>0xFF)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeByteSlong8(int64 value)
{
	if ((value<0)||(value>0xFF))
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSbyteByte(uint8 value)
{
	if (value>0x7F)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSbyteShort(uint16 value)
{
	if (value>0x7F)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSbyteSshort(int16 value)
{
	if ((value<-0x80)||(value>0x7F))
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSbyteLong(uint32 value)
{
	if (value>0x7F)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSbyteSlong(int32 value)
{
	if ((value<-0x80)||(value>0x7F))
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSbyteLong8(uint64 value)
{
	if (value>0x7F)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSbyteSlong8(int64 value)
{
	if ((value<-0x80)||(value>0x7F))
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeShortSbyte(int8 value)
{
	if (value<0)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeShortSshort(int16 value)
{
	if (value<0)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeShortLong(uint32 value)
{
	if (value>0xFFFF)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeShortSlong(int32 value)
{
	if ((value<0)||(value>0xFFFF))
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeShortLong8(uint64 value)
{
	if (value>0xFFFF)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeShortSlong8(int64 value)
{
	if ((value<0)||(value>0xFFFF))
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSshortShort(uint16 value)
{
	if (value>0x7FFF)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSshortLong(uint32 value)
{
	if (value>0x7FFF)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSshortSlong(int32 value)
{
	if ((value<-0x8000)||(value>0x7FFF))
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSshortLong8(uint64 value)
{
	if (value>0x7FFF)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeSshortSlong8(int64 value)
{
	if ((value<-0x8000)||(value>0x7FFF))
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeLongSbyte(int8 value)
{
	if (value<0)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeLongSshort(int16 value)
{
	if (value<0)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr TIFFReadDirEntryCheckRangeLongSlong(int32 value)
{
	if (value<0)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

/*
 * Largest 32-bit unsigned integer value.
 */
#define TIFF_UINT32_MAX 0xFFFFFFFFU

static enum TIFFReadDirEntryErr
TIFFReadDirEntryCheckRangeLongLong8(uint64 value)
{
	if (value > TIFF_UINT32_MAX)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr
TIFFReadDirEntryCheckRangeLongSlong8(int64 value)
{
	if ((value < 0) || (value > (int64) TIFF_UINT32_MAX))
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

#undef TIFF_UINT32_MAX

static enum TIFFReadDirEntryErr
TIFFReadDirEntryCheckRangeSlongLong(uint32 value)
{
	if (value > 0x7FFFFFFFUL)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

/* Check that the 8-byte unsigned value can fit in a 4-byte unsigned range */
static enum TIFFReadDirEntryErr
TIFFReadDirEntryCheckRangeSlongLong8(uint64 value)
{
	if (value > 0x7FFFFFFF)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

/* Check that the 8-byte signed value can fit in a 4-byte signed range */
static enum TIFFReadDirEntryErr
TIFFReadDirEntryCheckRangeSlongSlong8(int64 value)
{
        if ((value < 0-((int64) 0x7FFFFFFF+1)) || (value > 0x7FFFFFFF))
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr
TIFFReadDirEntryCheckRangeLong8Sbyte(int8 value)
{
	if (value < 0)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr
TIFFReadDirEntryCheckRangeLong8Sshort(int16 value)
{
	if (value < 0)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr
TIFFReadDirEntryCheckRangeLong8Slong(int32 value)
{
	if (value < 0)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

static enum TIFFReadDirEntryErr
TIFFReadDirEntryCheckRangeLong8Slong8(int64 value)
{
	if (value < 0)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

/*
 * Largest 64-bit signed integer value.
 */
#define TIFF_INT64_MAX ((int64)(((uint64) ~0) >> 1))

static enum TIFFReadDirEntryErr
TIFFReadDirEntryCheckRangeSlong8Long8(uint64 value)
{
	if (value > TIFF_INT64_MAX)
		return(TIFFReadDirEntryErrRange);
	else
		return(TIFFReadDirEntryErrOk);
}

#undef TIFF_INT64_MAX

static enum TIFFReadDirEntryErr
TIFFReadDirEntryData(TIFF* tif, uint64 offset, tmsize_t size, void* dest)
{
	assert(size>0);
	if (!isMapped(tif)) {
		if (!SeekOK(tif,offset))
			return(TIFFReadDirEntryErrIo);
		if (!ReadOK(tif,dest,size))
			return(TIFFReadDirEntryErrIo);
	} else {
		size_t ma,mb;
		ma=(size_t)offset;
		mb=ma+size;
		if (((uint64)ma!=offset)
		    || (mb < ma)
		    || (mb - ma != (size_t) size)
		    || (mb < (size_t)size)
		    || (mb > (size_t)tif->tif_size)
		    )
			return(TIFFReadDirEntryErrIo);
		_TIFFmemcpy(dest,tif->tif_base+ma,size);
	}
	return(TIFFReadDirEntryErrOk);
}

static void TIFFReadDirEntryOutputErr(TIFF* tif, enum TIFFReadDirEntryErr err, const char* module, const char* tagname, int recover)
{
	if (!recover) {
		switch (err) {
			case TIFFReadDirEntryErrCount:
				TIFFErrorExt(tif->tif_clientdata, module,
					     "Incorrect count for \"%s\"",
					     tagname);
				break;
			case TIFFReadDirEntryErrType:
				TIFFErrorExt(tif->tif_clientdata, module,
					     "Incompatible type for \"%s\"",
					     tagname);
				break;
			case TIFFReadDirEntryErrIo:
				TIFFErrorExt(tif->tif_clientdata, module,
					     "IO error during reading of \"%s\"",
					     tagname);
				break;
			case TIFFReadDirEntryErrRange:
				TIFFErrorExt(tif->tif_clientdata, module,
					     "Incorrect value for \"%s\"",
					     tagname);
				break;
			case TIFFReadDirEntryErrPsdif:
				TIFFErrorExt(tif->tif_clientdata, module,
			"Cannot handle different values per sample for \"%s\"",
					     tagname);
				break;
			case TIFFReadDirEntryErrSizesan:
				TIFFErrorExt(tif->tif_clientdata, module,
				"Sanity check on size of \"%s\" value failed",
					     tagname);
				break;
			case TIFFReadDirEntryErrAlloc:
				TIFFErrorExt(tif->tif_clientdata, module,
					     "Out of memory reading of \"%s\"",
					     tagname);
				break;
			default:
				assert(0);   /* we should never get here */
				break;
		}
	} else {
		switch (err) {
			case TIFFReadDirEntryErrCount:
				TIFFWarningExt(tif->tif_clientdata, module,
				"Incorrect count for \"%s\"; tag ignored",
					     tagname);
				break;
			case TIFFReadDirEntryErrType:
				TIFFWarningExt(tif->tif_clientdata, module,
				"Incompatible type for \"%s\"; tag ignored",
					       tagname);
				break;
			case TIFFReadDirEntryErrIo:
				TIFFWarningExt(tif->tif_clientdata, module,
			"IO error during reading of \"%s\"; tag ignored",
					       tagname);
				break;
			case TIFFReadDirEntryErrRange:
				TIFFWarningExt(tif->tif_clientdata, module,
				"Incorrect value for \"%s\"; tag ignored",
					       tagname);
				break;
			case TIFFReadDirEntryErrPsdif:
				TIFFWarningExt(tif->tif_clientdata, module,
	"Cannot handle different values per sample for \"%s\"; tag ignored",
					       tagname);
				break;
			case TIFFReadDirEntryErrSizesan:
				TIFFWarningExt(tif->tif_clientdata, module,
		"Sanity check on size of \"%s\" value failed; tag ignored",
					       tagname);
				break;
			case TIFFReadDirEntryErrAlloc:
				TIFFWarningExt(tif->tif_clientdata, module,
				"Out of memory reading of \"%s\"; tag ignored",
					       tagname);
				break;
			default:
				assert(0);   /* we should never get here */
				break;
		}
	}
}

/*
 * Return the maximum number of color channels specified for a given photometric
 * type. 0 is returned if photometric type isn't supported or no default value
 * is defined by the specification.
 */
static int _TIFFGetMaxColorChannels( uint16 photometric )
{
    switch (photometric) {
	case PHOTOMETRIC_PALETTE:
	case PHOTOMETRIC_MINISWHITE:
	case PHOTOMETRIC_MINISBLACK:
            return 1;
	case PHOTOMETRIC_YCBCR:
	case PHOTOMETRIC_RGB:
	case PHOTOMETRIC_CIELAB:
            return 3;
	case PHOTOMETRIC_SEPARATED:
	case PHOTOMETRIC_MASK:
            return 4;
	case PHOTOMETRIC_LOGL:
	case PHOTOMETRIC_LOGLUV:
	case PHOTOMETRIC_CFA:
	case PHOTOMETRIC_ITULAB:
	case PHOTOMETRIC_ICCLAB:
	default:
            return 0;
    }
}

/*
 * Read the next TIFF directory from a file and convert it to the internal
 * format. We read directories sequentially.
 */
int
TIFFReadDirectory(TIFF* tif)
{
	static const char module[] = "TIFFReadDirectory";
	TIFFDirEntry* dir;
	uint16 dircount;
	TIFFDirEntry* dp;
	uint16 di;
	const TIFFField* fip;
	uint32 fii=FAILED_FII;
        toff_t nextdiroff;
    int bitspersample_read = FALSE;
        int color_channels;

	tif->tif_diroff=tif->tif_nextdiroff;
	if (!TIFFCheckDirOffset(tif,tif->tif_nextdiroff))
		return 0;           /* last offset or bad offset (IFD looping) */
	(*tif->tif_cleanup)(tif);   /* cleanup any previous compression state */
	tif->tif_curdir++;
        nextdiroff = tif->tif_nextdiroff;
	dircount=TIFFFetchDirectory(tif,nextdiroff,&dir,&tif->tif_nextdiroff);
	if (!dircount)
	{
		TIFFErrorExt(tif->tif_clientdata,module,
		    "Failed to read directory at offset " TIFF_UINT64_FORMAT,nextdiroff);
		return 0;
	}
	TIFFReadDirectoryCheckOrder(tif,dir,dircount);

        /*
         * Mark duplicates of any tag to be ignored (bugzilla 1994)
         * to avoid certain pathological problems.
         */
	{
		TIFFDirEntry* ma;
		uint16 mb;
		for (ma=dir, mb=0; mb<dircount; ma++, mb++)
		{
			TIFFDirEntry* na;
			uint16 nb;
			for (na=ma+1, nb=mb+1; nb<dircount; na++, nb++)
			{
				if (ma->tdir_tag==na->tdir_tag)
					na->tdir_tag=IGNORE;
			}
		}
	}
        
	tif->tif_flags &= ~TIFF_BEENWRITING;    /* reset before new dir */
	tif->tif_flags &= ~TIFF_BUF4WRITE;      /* reset before new dir */
	/* free any old stuff and reinit */
	TIFFFreeDirectory(tif);
	TIFFDefaultDirectory(tif);
	/*
	 * Electronic Arts writes gray-scale TIFF files
	 * without a PlanarConfiguration directory entry.
	 * Thus we setup a default value here, even though
	 * the TIFF spec says there is no default value.
	 */
	TIFFSetField(tif,TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
	/*
	 * Setup default value and then make a pass over
	 * the fields to check type and tag information,
	 * and to extract info required to size data
	 * structures.  A second pass is made afterwards
	 * to read in everything not taken in the first pass.
	 * But we must process the Compression tag first
	 * in order to merge in codec-private tag definitions (otherwise
	 * we may get complaints about unknown tags).  However, the
	 * Compression tag may be dependent on the SamplesPerPixel
	 * tag value because older TIFF specs permitted Compression
	 * to be written as a SamplesPerPixel-count tag entry.
	 * Thus if we don't first figure out the correct SamplesPerPixel
	 * tag value then we may end up ignoring the Compression tag
	 * value because it has an incorrect count value (if the
	 * true value of SamplesPerPixel is not 1).
	 */
	dp=TIFFReadDirectoryFindEntry(tif,dir,dircount,TIFFTAG_SAMPLESPERPIXEL);
	if (dp)
	{
		if (!TIFFFetchNormalTag(tif,dp,0))
			goto bad;
		dp->tdir_tag=IGNORE;
	}
	dp=TIFFReadDirectoryFindEntry(tif,dir,dircount,TIFFTAG_COMPRESSION);
	if (dp)
	{
		/*
		 * The 5.0 spec says the Compression tag has one value, while
		 * earlier specs say it has one value per sample.  Because of
		 * this, we accept the tag if one value is supplied with either
		 * count.
		 */
		uint16 value;
		enum TIFFReadDirEntryErr err;
		err=TIFFReadDirEntryShort(tif,dp,&value);
		if (err==TIFFReadDirEntryErrCount)
			err=TIFFReadDirEntryPersampleShort(tif,dp,&value);
		if (err!=TIFFReadDirEntryErrOk)
		{
			TIFFReadDirEntryOutputErr(tif,err,module,"Compression",0);
			goto bad;
		}
		if (!TIFFSetField(tif,TIFFTAG_COMPRESSION,value))
			goto bad;
		dp->tdir_tag=IGNORE;
	}
	else
	{
		if (!TIFFSetField(tif,TIFFTAG_COMPRESSION,COMPRESSION_NONE))
			goto bad;
	}
	/*
	 * First real pass over the directory.
	 */
	for (di=0, dp=dir; di<dircount; di++, dp++)
	{
		if (dp->tdir_tag!=IGNORE)
		{
			TIFFReadDirectoryFindFieldInfo(tif,dp->tdir_tag,&fii);
			if (fii == FAILED_FII)
			{
				TIFFWarningExt(tif->tif_clientdata, module,
				    "Unknown field with tag %d (0x%x) encountered",
				    dp->tdir_tag,dp->tdir_tag);
                                /* the following knowingly leaks the 
                                   anonymous field structure */
				if (!_TIFFMergeFields(tif,
					_TIFFCreateAnonField(tif,
						dp->tdir_tag,
						(TIFFDataType) dp->tdir_type),
					1)) {
					TIFFWarningExt(tif->tif_clientdata,
					    module,
					    "Registering anonymous field with tag %d (0x%x) failed",
					    dp->tdir_tag,
					    dp->tdir_tag);
					dp->tdir_tag=IGNORE;
				} else {
					TIFFReadDirectoryFindFieldInfo(tif,dp->tdir_tag,&fii);
					assert(fii != FAILED_FII);
				}
			}
		}
		if (dp->tdir_tag!=IGNORE)
		{
			fip=tif->tif_fields[fii];
			if (fip->field_bit==FIELD_IGNORE)
				dp->tdir_tag=IGNORE;
			else
			{
				switch (dp->tdir_tag)
				{
					case TIFFTAG_STRIPOFFSETS:
					case TIFFTAG_STRIPBYTECOUNTS:
					case TIFFTAG_TILEOFFSETS:
					case TIFFTAG_TILEBYTECOUNTS:
						TIFFSetFieldBit(tif,fip->field_bit);
						break;
					case TIFFTAG_IMAGEWIDTH:
					case TIFFTAG_IMAGELENGTH:
					case TIFFTAG_IMAGEDEPTH:
					case TIFFTAG_TILELENGTH:
					case TIFFTAG_TILEWIDTH:
					case TIFFTAG_TILEDEPTH:
					case TIFFTAG_PLANARCONFIG:
					case TIFFTAG_ROWSPERSTRIP:
					case TIFFTAG_EXTRASAMPLES:
						if (!TIFFFetchNormalTag(tif,dp,0))
							goto bad;
						dp->tdir_tag=IGNORE;
						break;
                                        default:
                                            if( !_TIFFCheckFieldIsValidForCodec(tif, dp->tdir_tag) )
                                                dp->tdir_tag=IGNORE;
                                            break;
				}
			}
		}
	}
	/*
	 * XXX: OJPEG hack.
	 * If a) compression is OJPEG, b) planarconfig tag says it's separate,
	 * c) strip offsets/bytecounts tag are both present and
	 * d) both contain exactly one value, then we consistently find
	 * that the buggy implementation of the buggy compression scheme
	 * matches contig planarconfig best. So we 'fix-up' the tag here
	 */
	if ((tif->tif_dir.td_compression==COMPRESSION_OJPEG)&&
	    (tif->tif_dir.td_planarconfig==PLANARCONFIG_SEPARATE))
	{
        if (!_TIFFFillStriles(tif))
            goto bad;
		dp=TIFFReadDirectoryFindEntry(tif,dir,dircount,TIFFTAG_STRIPOFFSETS);
		if ((dp!=0)&&(dp->tdir_count==1))
		{
			dp=TIFFReadDirectoryFindEntry(tif,dir,dircount,
			    TIFFTAG_STRIPBYTECOUNTS);
			if ((dp!=0)&&(dp->tdir_count==1))
			{
				tif->tif_dir.td_planarconfig=PLANARCONFIG_CONTIG;
				TIFFWarningExt(tif->tif_clientdata,module,
				    "Planarconfig tag value assumed incorrect, "
				    "assuming data is contig instead of chunky");
			}
		}
	}
	/*
	 * Allocate directory structure and setup defaults.
	 */
	if (!TIFFFieldSet(tif,FIELD_IMAGEDIMENSIONS))
	{
		MissingRequired(tif,"ImageLength");
		goto bad;
	}
	/*
	 * Setup appropriate structures (by strip or by tile)
	 */
	if (!TIFFFieldSet(tif, FIELD_TILEDIMENSIONS)) {
		tif->tif_dir.td_nstrips = TIFFNumberOfStrips(tif);  
		tif->tif_dir.td_tilewidth = tif->tif_dir.td_imagewidth;
		tif->tif_dir.td_tilelength = tif->tif_dir.td_rowsperstrip;
		tif->tif_dir.td_tiledepth = tif->tif_dir.td_imagedepth;
		tif->tif_flags &= ~TIFF_ISTILED;
	} else {
		tif->tif_dir.td_nstrips = TIFFNumberOfTiles(tif);
		tif->tif_flags |= TIFF_ISTILED;
	}
	if (!tif->tif_dir.td_nstrips) {
		TIFFErrorExt(tif->tif_clientdata, module,
		    "Cannot handle zero number of %s",
		    isTiled(tif) ? "tiles" : "strips");
		goto bad;
	}
	tif->tif_dir.td_stripsperimage = tif->tif_dir.td_nstrips;
	if (tif->tif_dir.td_planarconfig == PLANARCONFIG_SEPARATE)
		tif->tif_dir.td_stripsperimage /= tif->tif_dir.td_samplesperpixel;
	if (!TIFFFieldSet(tif, FIELD_STRIPOFFSETS)) {
#ifdef OJPEG_SUPPORT
		if ((tif->tif_dir.td_compression==COMPRESSION_OJPEG) &&
		    (isTiled(tif)==0) &&
		    (tif->tif_dir.td_nstrips==1)) {
			/*
			 * XXX: OJPEG hack.
			 * If a) compression is OJPEG, b) it's not a tiled TIFF,
			 * and c) the number of strips is 1,
			 * then we tolerate the absence of stripoffsets tag,
			 * because, presumably, all required data is in the
			 * JpegInterchangeFormat stream.
			 */
			TIFFSetFieldBit(tif, FIELD_STRIPOFFSETS);
		} else
#endif
        {
			MissingRequired(tif,
				isTiled(tif) ? "TileOffsets" : "StripOffsets");
			goto bad;
		}
	}
	/*
	 * Second pass: extract other information.
	 */
	for (di=0, dp=dir; di<dircount; di++, dp++)
	{
		switch (dp->tdir_tag)
		{
			case IGNORE:
				break;
			case TIFFTAG_MINSAMPLEVALUE:
			case TIFFTAG_MAXSAMPLEVALUE:
			case TIFFTAG_BITSPERSAMPLE:
			case TIFFTAG_DATATYPE:
			case TIFFTAG_SAMPLEFORMAT:
				/*
				 * The MinSampleValue, MaxSampleValue, BitsPerSample
				 * DataType and SampleFormat tags are supposed to be
				 * written as one value/sample, but some vendors
				 * incorrectly write one value only -- so we accept
				 * that as well (yuck). Other vendors write correct
				 * value for NumberOfSamples, but incorrect one for
				 * BitsPerSample and friends, and we will read this
				 * too.
				 */
				{
					uint16 value;
					enum TIFFReadDirEntryErr err;
					err=TIFFReadDirEntryShort(tif,dp,&value);
					if (err==TIFFReadDirEntryErrCount)
						err=TIFFReadDirEntryPersampleShort(tif,dp,&value);
					if (err!=TIFFReadDirEntryErrOk)
					{
						fip = TIFFFieldWithTag(tif,dp->tdir_tag);
						TIFFReadDirEntryOutputErr(tif,err,module,fip ? fip->field_name : "unknown tagname",0);
						goto bad;
					}
					if (!TIFFSetField(tif,dp->tdir_tag,value))
						goto bad;
                    if( dp->tdir_tag == TIFFTAG_BITSPERSAMPLE )
                        bitspersample_read = TRUE;
				}
				break;
			case TIFFTAG_SMINSAMPLEVALUE:
			case TIFFTAG_SMAXSAMPLEVALUE:
				{

					double *data = NULL;
					enum TIFFReadDirEntryErr err;
					uint32 saved_flags;
					int m;
					if (dp->tdir_count != (uint64)tif->tif_dir.td_samplesperpixel)
						err = TIFFReadDirEntryErrCount;
					else
						err = TIFFReadDirEntryDoubleArray(tif, dp, &data);
					if (err!=TIFFReadDirEntryErrOk)
					{
						fip = TIFFFieldWithTag(tif,dp->tdir_tag);
						TIFFReadDirEntryOutputErr(tif,err,module,fip ? fip->field_name : "unknown tagname",0);
						goto bad;
					}
					saved_flags = tif->tif_flags;
					tif->tif_flags |= TIFF_PERSAMPLE;
					m = TIFFSetField(tif,dp->tdir_tag,data);
					tif->tif_flags = saved_flags;
					_TIFFfree(data);
					if (!m)
						goto bad;
				}
				break;
			case TIFFTAG_STRIPOFFSETS:
			case TIFFTAG_TILEOFFSETS:
#if defined(DEFER_STRILE_LOAD)
                                _TIFFmemcpy( &(tif->tif_dir.td_stripoffset_entry),
                                             dp, sizeof(TIFFDirEntry) );
#else                          
                                if( tif->tif_dir.td_stripoffset != NULL )
                                {
                                    TIFFErrorExt(tif->tif_clientdata, module,
                                        "tif->tif_dir.td_stripoffset is "
                                        "already allocated. Likely duplicated "
                                        "StripOffsets/TileOffsets tag");
                                    goto bad;
                                }
				if (!TIFFFetchStripThing(tif,dp,tif->tif_dir.td_nstrips,&tif->tif_dir.td_stripoffset))  
					goto bad;
#endif                                
				break;
			case TIFFTAG_STRIPBYTECOUNTS:
			case TIFFTAG_TILEBYTECOUNTS:
#if defined(DEFER_STRILE_LOAD)
                                _TIFFmemcpy( &(tif->tif_dir.td_stripbytecount_entry),
                                             dp, sizeof(TIFFDirEntry) );
#else                          
                                if( tif->tif_dir.td_stripbytecount != NULL )
                                {
                                    TIFFErrorExt(tif->tif_clientdata, module,
                                        "tif->tif_dir.td_stripbytecount is "
                                        "already allocated. Likely duplicated "
                                        "StripByteCounts/TileByteCounts tag");
                                    goto bad;
                                }
                                if (!TIFFFetchStripThing(tif,dp,tif->tif_dir.td_nstrips,&tif->tif_dir.td_stripbytecount))  
					goto bad;
#endif                                
				break;
			case TIFFTAG_COLORMAP:
			case TIFFTAG_TRANSFERFUNCTION:
				{
					enum TIFFReadDirEntryErr err;
					uint32 countpersample;
					uint32 countrequired;
					uint32 incrementpersample;
					uint16* value=NULL;
                    /* It would be dangerous to instantiate those tag values */
                    /* since if td_bitspersample has not yet been read (due to */
                    /* unordered tags), it could be read afterwards with a */
                    /* values greater than the default one (1), which may cause */
                    /* crashes in user code */
                    if( !bitspersample_read )
                    {
                        fip = TIFFFieldWithTag(tif,dp->tdir_tag);
                        TIFFWarningExt(tif->tif_clientdata,module,
                                       "Ignoring %s since BitsPerSample tag not found",
                                       fip ? fip->field_name : "unknown tagname");
                        continue;
                    }
					/* ColorMap or TransferFunction for high bit */
					/* depths do not make much sense and could be */
					/* used as a denial of service vector */
					if (tif->tif_dir.td_bitspersample > 24)
					{
					    fip = TIFFFieldWithTag(tif,dp->tdir_tag);
					    TIFFWarningExt(tif->tif_clientdata,module,
						"Ignoring %s because BitsPerSample=%d>24",
						fip ? fip->field_name : "unknown tagname",
						tif->tif_dir.td_bitspersample);
					    continue;
					}
					countpersample=(1U<<tif->tif_dir.td_bitspersample);
					if ((dp->tdir_tag==TIFFTAG_TRANSFERFUNCTION)&&(dp->tdir_count==(uint64)countpersample))
					{
						countrequired=countpersample;
						incrementpersample=0;
					}
					else
					{
						countrequired=3*countpersample;
						incrementpersample=countpersample;
					}
					if (dp->tdir_count!=(uint64)countrequired)
						err=TIFFReadDirEntryErrCount;
					else
						err=TIFFReadDirEntryShortArray(tif,dp,&value);
					if (err!=TIFFReadDirEntryErrOk)
                    {
						fip = TIFFFieldWithTag(tif,dp->tdir_tag);
						TIFFReadDirEntryOutputErr(tif,err,module,fip ? fip->field_name : "unknown tagname",1);
                    }
					else
					{
						TIFFSetField(tif,dp->tdir_tag,value,value+incrementpersample,value+2*incrementpersample);
						_TIFFfree(value);
					}
				}
				break;
/* BEGIN REV 4.0 COMPATIBILITY */
			case TIFFTAG_OSUBFILETYPE:
				{
					uint16 valueo;
					uint32 value;
					if (TIFFReadDirEntryShort(tif,dp,&valueo)==TIFFReadDirEntryErrOk)
					{
						switch (valueo)
						{
							case OFILETYPE_REDUCEDIMAGE: value=FILETYPE_REDUCEDIMAGE; break;
							case OFILETYPE_PAGE: value=FILETYPE_PAGE; break;
							default: value=0; break;
						}
						if (value!=0)
							TIFFSetField(tif,TIFFTAG_SUBFILETYPE,value);
					}
				}
				break;
/* END REV 4.0 COMPATIBILITY */
			default:
				(void) TIFFFetchNormalTag(tif, dp, TRUE);
				break;
		}
	}
	/*
	 * OJPEG hack:
	 * - If a) compression is OJPEG, and b) photometric tag is missing,
	 * then we consistently find that photometric should be YCbCr
	 * - If a) compression is OJPEG, and b) photometric tag says it's RGB,
	 * then we consistently find that the buggy implementation of the
	 * buggy compression scheme matches photometric YCbCr instead.
	 * - If a) compression is OJPEG, and b) bitspersample tag is missing,
	 * then we consistently find bitspersample should be 8.
	 * - If a) compression is OJPEG, b) samplesperpixel tag is missing,
	 * and c) photometric is RGB or YCbCr, then we consistently find
	 * samplesperpixel should be 3
	 * - If a) compression is OJPEG, b) samplesperpixel tag is missing,
	 * and c) photometric is MINISWHITE or MINISBLACK, then we consistently
	 * find samplesperpixel should be 3
	 */
	if (tif->tif_dir.td_compression==COMPRESSION_OJPEG)
	{
		if (!TIFFFieldSet(tif,FIELD_PHOTOMETRIC))
		{
			TIFFWarningExt(tif->tif_clientdata, module,
			    "Photometric tag is missing, assuming data is YCbCr");
			if (!TIFFSetField(tif,TIFFTAG_PHOTOMETRIC,PHOTOMETRIC_YCBCR))
				goto bad;
		}
		else if (tif->tif_dir.td_photometric==PHOTOMETRIC_RGB)
		{
			tif->tif_dir.td_photometric=PHOTOMETRIC_YCBCR;
			TIFFWarningExt(tif->tif_clientdata, module,
			    "Photometric tag value assumed incorrect, "
			    "assuming data is YCbCr instead of RGB");
		}
		if (!TIFFFieldSet(tif,FIELD_BITSPERSAMPLE))
		{
			TIFFWarningExt(tif->tif_clientdata,module,
			    "BitsPerSample tag is missing, assuming 8 bits per sample");
			if (!TIFFSetField(tif,TIFFTAG_BITSPERSAMPLE,8))
				goto bad;
		}
		if (!TIFFFieldSet(tif,FIELD_SAMPLESPERPIXEL))
		{
			if (tif->tif_dir.td_photometric==PHOTOMETRIC_RGB)
			{
				TIFFWarningExt(tif->tif_clientdata,module,
				    "SamplesPerPixel tag is missing, "
				    "assuming correct SamplesPerPixel value is 3");
				if (!TIFFSetField(tif,TIFFTAG_SAMPLESPERPIXEL,3))
					goto bad;
			}
			if (tif->tif_dir.td_photometric==PHOTOMETRIC_YCBCR)
			{
				TIFFWarningExt(tif->tif_clientdata,module,
				    "SamplesPerPixel tag is missing, "
				    "applying correct SamplesPerPixel value of 3");
				if (!TIFFSetField(tif,TIFFTAG_SAMPLESPERPIXEL,3))
					goto bad;
			}
			else if ((tif->tif_dir.td_photometric==PHOTOMETRIC_MINISWHITE)
				 || (tif->tif_dir.td_photometric==PHOTOMETRIC_MINISBLACK))
			{
				/*
				 * SamplesPerPixel tag is missing, but is not required
				 * by spec.  Assume correct SamplesPerPixel value of 1.
				 */
				if (!TIFFSetField(tif,TIFFTAG_SAMPLESPERPIXEL,1))
					goto bad;
			}
		}
	}

	/*
	 * Make sure all non-color channels are extrasamples.
	 * If it's not the case, define them as such.
	 */
        color_channels = _TIFFGetMaxColorChannels(tif->tif_dir.td_photometric);
        if (color_channels && tif->tif_dir.td_samplesperpixel - tif->tif_dir.td_extrasamples > color_channels) {
                uint16 old_extrasamples;
                uint16 *new_sampleinfo;

                TIFFWarningExt(tif->tif_clientdata,module, "Sum of Photometric type-related "
                    "color channels and ExtraSamples doesn't match SamplesPerPixel. "
                    "Defining non-color channels as ExtraSamples.");

                old_extrasamples = tif->tif_dir.td_extrasamples;
                tif->tif_dir.td_extrasamples = (tif->tif_dir.td_samplesperpixel - color_channels);

                // sampleinfo should contain information relative to these new extra samples
                new_sampleinfo = (uint16*) _TIFFcalloc(tif->tif_dir.td_extrasamples, sizeof(uint16));
                if (!new_sampleinfo) {
                    TIFFErrorExt(tif->tif_clientdata, module, "Failed to allocate memory for "
                                "temporary new sampleinfo array (%d 16 bit elements)",
                                tif->tif_dir.td_extrasamples);
                    goto bad;
                }

                memcpy(new_sampleinfo, tif->tif_dir.td_sampleinfo, old_extrasamples * sizeof(uint16));
                _TIFFsetShortArray(&tif->tif_dir.td_sampleinfo, new_sampleinfo, tif->tif_dir.td_extrasamples);
                _TIFFfree(new_sampleinfo);
        }

	/*
	 * Verify Palette image has a Colormap.
	 */
	if (tif->tif_dir.td_photometric == PHOTOMETRIC_PALETTE &&
	    !TIFFFieldSet(tif, FIELD_COLORMAP)) {
		if ( tif->tif_dir.td_bitspersample>=8 && tif->tif_dir.td_samplesperpixel==3)
			tif->tif_dir.td_photometric = PHOTOMETRIC_RGB;
		else if (tif->tif_dir.td_bitspersample>=8)
			tif->tif_dir.td_photometric = PHOTOMETRIC_MINISBLACK;
		else {
			MissingRequired(tif, "Colormap");
			goto bad;
		}
	}
	/*
	 * OJPEG hack:
	 * We do no further messing with strip/tile offsets/bytecounts in OJPEG
	 * TIFFs
	 */
	if (tif->tif_dir.td_compression!=COMPRESSION_OJPEG)
	{
		/*
		 * Attempt to deal with a missing StripByteCounts tag.
		 */
		if (!TIFFFieldSet(tif, FIELD_STRIPBYTECOUNTS)) {
			/*
			 * Some manufacturers violate the spec by not giving
			 * the size of the strips.  In this case, assume there
			 * is one uncompressed strip of data.
			 */
			if ((tif->tif_dir.td_planarconfig == PLANARCONFIG_CONTIG &&
			    tif->tif_dir.td_nstrips > 1) ||
			    (tif->tif_dir.td_planarconfig == PLANARCONFIG_SEPARATE &&
			     tif->tif_dir.td_nstrips != (uint32)tif->tif_dir.td_samplesperpixel)) {
			    MissingRequired(tif, "StripByteCounts");
			    goto bad;
			}
			TIFFWarningExt(tif->tif_clientdata, module,
				"TIFF directory is missing required "
				"\"StripByteCounts\" field, calculating from imagelength");
			if (EstimateStripByteCounts(tif, dir, dircount) < 0)
			    goto bad;
		/*
		 * Assume we have wrong StripByteCount value (in case
		 * of single strip) in following cases:
		 *   - it is equal to zero along with StripOffset;
		 *   - it is larger than file itself (in case of uncompressed
		 *     image);
		 *   - it is smaller than the size of the bytes per row
		 *     multiplied on the number of rows.  The last case should
		 *     not be checked in the case of writing new image,
		 *     because we may do not know the exact strip size
		 *     until the whole image will be written and directory
		 *     dumped out.
		 */
		#define	BYTECOUNTLOOKSBAD \
		    ( (tif->tif_dir.td_stripbytecount[0] == 0 && tif->tif_dir.td_stripoffset[0] != 0) || \
		      (tif->tif_dir.td_compression == COMPRESSION_NONE && \
		       (tif->tif_dir.td_stripoffset[0] <= TIFFGetFileSize(tif) && \
		        tif->tif_dir.td_stripbytecount[0] > TIFFGetFileSize(tif) - tif->tif_dir.td_stripoffset[0])) || \
		      (tif->tif_mode == O_RDONLY && \
		       tif->tif_dir.td_compression == COMPRESSION_NONE && \
		       tif->tif_dir.td_stripbytecount[0] < TIFFScanlineSize64(tif) * tif->tif_dir.td_imagelength) )

		} else if (tif->tif_dir.td_nstrips == 1
                           && !(tif->tif_flags&TIFF_ISTILED)
                           && _TIFFFillStriles(tif)
			   && tif->tif_dir.td_stripoffset[0] != 0
			   && BYTECOUNTLOOKSBAD) {
			/*
			 * XXX: Plexus (and others) sometimes give a value of
			 * zero for a tag when they don't know what the
			 * correct value is!  Try and handle the simple case
			 * of estimating the size of a one strip image.
			 */
			TIFFWarningExt(tif->tif_clientdata, module,
			    "Bogus \"StripByteCounts\" field, ignoring and calculating from imagelength");
			if(EstimateStripByteCounts(tif, dir, dircount) < 0)
			    goto bad;

#if !defined(DEFER_STRILE_LOAD)
		} else if (tif->tif_dir.td_planarconfig == PLANARCONFIG_CONTIG
			   && tif->tif_dir.td_nstrips > 2
			   && tif->tif_dir.td_compression == COMPRESSION_NONE
			   && tif->tif_dir.td_stripbytecount[0] != tif->tif_dir.td_stripbytecount[1]
			   && tif->tif_dir.td_stripbytecount[0] != 0
			   && tif->tif_dir.td_stripbytecount[1] != 0 ) {
			/*
			 * XXX: Some vendors fill StripByteCount array with
			 * absolutely wrong values (it can be equal to
			 * StripOffset array, for example). Catch this case
			 * here.
                         *
                         * We avoid this check if deferring strile loading
                         * as it would always force us to load the strip/tile
                         * information.
			 */
			TIFFWarningExt(tif->tif_clientdata, module,
			    "Wrong \"StripByteCounts\" field, ignoring and calculating from imagelength");
			if (EstimateStripByteCounts(tif, dir, dircount) < 0)
			    goto bad;
#endif /* !defined(DEFER_STRILE_LOAD) */                        
		}
	}
	if (dir)
	{
		_TIFFfree(dir);
		dir=NULL;
	}
	if (!TIFFFieldSet(tif, FIELD_MAXSAMPLEVALUE))
	{
		if (tif->tif_dir.td_bitspersample>=16)
			tif->tif_dir.td_maxsamplevalue=0xFFFF;
		else
			tif->tif_dir.td_maxsamplevalue = (uint16)((1L<<tif->tif_dir.td_bitspersample)-1);
	}
	/*
	 * XXX: We can optimize checking for the strip bounds using the sorted
	 * bytecounts array. See also comments for TIFFAppendToStrip()
	 * function in tif_write.c.
	 */
#if !defined(DEFER_STRILE_LOAD)        
	if (tif->tif_dir.td_nstrips > 1) {
		uint32 strip;

		tif->tif_dir.td_stripbytecountsorted = 1;
		for (strip = 1; strip < tif->tif_dir.td_nstrips; strip++) {
			if (tif->tif_dir.td_stripoffset[strip - 1] >
			    tif->tif_dir.td_stripoffset[strip]) {
				tif->tif_dir.td_stripbytecountsorted = 0;
				break;
			}
		}
	}
#endif /* !defined(DEFER_STRILE_LOAD) */
        
	/*
	 * An opportunity for compression mode dependent tag fixup
	 */
	(*tif->tif_fixuptags)(tif);

	/*
	 * Some manufacturers make life difficult by writing
	 * large amounts of uncompressed data as a single strip.
	 * This is contrary to the recommendations of the spec.
	 * The following makes an attempt at breaking such images
	 * into strips closer to the recommended 8k bytes.  A
	 * side effect, however, is that the RowsPerStrip tag
	 * value may be changed.
	 */
	if ((tif->tif_dir.td_planarconfig==PLANARCONFIG_CONTIG)&&
	    (tif->tif_dir.td_nstrips==1)&&
	    (tif->tif_dir.td_compression==COMPRESSION_NONE)&&  
	    ((tif->tif_flags&(TIFF_STRIPCHOP|TIFF_ISTILED))==TIFF_STRIPCHOP))
    {
        if ( !_TIFFFillStriles(tif) || !tif->tif_dir.td_stripbytecount )
            return 0;
		ChopUpSingleUncompressedStrip(tif);
    }

        /*
         * Clear the dirty directory flag. 
         */
	tif->tif_flags &= ~TIFF_DIRTYDIRECT;
	tif->tif_flags &= ~TIFF_DIRTYSTRIP;

	/*
	 * Reinitialize i/o since we are starting on a new directory.
	 */
	tif->tif_row = (uint32) -1;
	tif->tif_curstrip = (uint32) -1;
	tif->tif_col = (uint32) -1;
	tif->tif_curtile = (uint32) -1;
	tif->tif_tilesize = (tmsize_t) -1;

	tif->tif_scanlinesize = TIFFScanlineSize(tif);
	if (!tif->tif_scanlinesize) {
		TIFFErrorExt(tif->tif_clientdata, module,
		    "Cannot handle zero scanline size");
		return (0);
	}

	if (isTiled(tif)) {
		tif->tif_tilesize = TIFFTileSize(tif);
		if (!tif->tif_tilesize) {
			TIFFErrorExt(tif->tif_clientdata, module,
			     "Cannot handle zero tile size");
			return (0);
		}
	} else {
		if (!TIFFStripSize(tif)) {
			TIFFErrorExt(tif->tif_clientdata, module,
			    "Cannot handle zero strip size");
			return (0);
		}
	}
	return (1);
bad:
	if (dir)
		_TIFFfree(dir);
	return (0);
}

static void
TIFFReadDirectoryCheckOrder(TIFF* tif, TIFFDirEntry* dir, uint16 dircount)
{
	static const char module[] = "TIFFReadDirectoryCheckOrder";
	uint16 m;
	uint16 n;
	TIFFDirEntry* o;
	m=0;
	for (n=0, o=dir; n<dircount; n++, o++)
	{
		if (o->tdir_tag<m)
		{
			TIFFWarningExt(tif->tif_clientdata,module,
			    "Invalid TIFF directory; tags are not sorted in ascending order");
			break;
		}
		m=o->tdir_tag+1;
	}
}

static TIFFDirEntry*
TIFFReadDirectoryFindEntry(TIFF* tif, TIFFDirEntry* dir, uint16 dircount, uint16 tagid)
{
	TIFFDirEntry* m;
	uint16 n;
	(void) tif;
	for (m=dir, n=0; n<dircount; m++, n++)
	{
		if (m->tdir_tag==tagid)
			return(m);
	}
	return(0);
}

static void
TIFFReadDirectoryFindFieldInfo(TIFF* tif, uint16 tagid, uint32* fii)
{
	int32 ma,mb,mc;
	ma=-1;
	mc=(int32)tif->tif_nfields;
	while (1)
	{
		if (ma+1==mc)
		{
			*fii = FAILED_FII;
			return;
		}
		mb=(ma+mc)/2;
		if (tif->tif_fields[mb]->field_tag==(uint32)tagid)
			break;
		if (tif->tif_fields[mb]->field_tag<(uint32)tagid)
			ma=mb;
		else
			mc=mb;
	}
	while (1)
	{
		if (mb==0)
			break;
		if (tif->tif_fields[mb-1]->field_tag!=(uint32)tagid)
			break;
		mb--;
	}
	*fii=mb;
}

/*
 * Read custom directory from the arbitrary offset.
 * The code is very similar to TIFFReadDirectory().
 */
int
TIFFReadCustomDirectory(TIFF* tif, toff_t diroff,
			const TIFFFieldArray* infoarray)
{
	static const char module[] = "TIFFReadCustomDirectory";
	TIFFDirEntry* dir;
	uint16 dircount;
	TIFFDirEntry* dp;
	uint16 di;
	const TIFFField* fip;
	uint32 fii;
	_TIFFSetupFields(tif, infoarray);
	dircount=TIFFFetchDirectory(tif,diroff,&dir,NULL);
	if (!dircount)
	{
		TIFFErrorExt(tif->tif_clientdata,module,
		    "Failed to read custom directory at offset " TIFF_UINT64_FORMAT,diroff);
		return 0;
	}
	TIFFFreeDirectory(tif);
	_TIFFmemset(&tif->tif_dir, 0, sizeof(TIFFDirectory));
	TIFFReadDirectoryCheckOrder(tif,dir,dircount);
	for (di=0, dp=dir; di<dircount; di++, dp++)
	{
		TIFFReadDirectoryFindFieldInfo(tif,dp->tdir_tag,&fii);
		if (fii == FAILED_FII)
		{
			TIFFWarningExt(tif->tif_clientdata, module,
			    "Unknown field with tag %d (0x%x) encountered",
			    dp->tdir_tag, dp->tdir_tag);
			if (!_TIFFMergeFields(tif, _TIFFCreateAnonField(tif,
						dp->tdir_tag,
						(TIFFDataType) dp->tdir_type),
					     1)) {
				TIFFWarningExt(tif->tif_clientdata, module,
				    "Registering anonymous field with tag %d (0x%x) failed",
				    dp->tdir_tag, dp->tdir_tag);
				dp->tdir_tag=IGNORE;
			} else {
				TIFFReadDirectoryFindFieldInfo(tif,dp->tdir_tag,&fii);
				assert( fii != FAILED_FII );
			}
		}
		if (dp->tdir_tag!=IGNORE)
		{
			fip=tif->tif_fields[fii];
			if (fip->field_bit==FIELD_IGNORE)
				dp->tdir_tag=IGNORE;
			else
			{
				/* check data type */
				while ((fip->field_type!=TIFF_ANY)&&(fip->field_type!=dp->tdir_type))
				{
					fii++;
					if ((fii==tif->tif_nfields)||
					    (tif->tif_fields[fii]->field_tag!=(uint32)dp->tdir_tag))
					{
						fii=0xFFFF;
						break;
					}
					fip=tif->tif_fields[fii];
				}
				if (fii==0xFFFF)
				{
					TIFFWarningExt(tif->tif_clientdata, module,
					    "Wrong data type %d for \"%s\"; tag ignored",
					    dp->tdir_type,fip->field_name);
					dp->tdir_tag=IGNORE;
				}
				else
				{
					/* check count if known in advance */
					if ((fip->field_readcount!=TIFF_VARIABLE)&&
					    (fip->field_readcount!=TIFF_VARIABLE2))
					{
						uint32 expected;
						if (fip->field_readcount==TIFF_SPP)
							expected=(uint32)tif->tif_dir.td_samplesperpixel;
						else
							expected=(uint32)fip->field_readcount;
						if (!CheckDirCount(tif,dp,expected))
							dp->tdir_tag=IGNORE;
					}
				}
			}
			switch (dp->tdir_tag)
			{
				case IGNORE:
					break;
				case EXIFTAG_SUBJECTDISTANCE:
					(void) TIFFFetchSubjectDistance(tif,dp);
					break;
				default:
					(void) TIFFFetchNormalTag(tif, dp, TRUE);
					break;
			}
		}
	}
	if (dir)
		_TIFFfree(dir);
	return 1;
}

/*
 * EXIF is important special case of custom IFD, so we have a special
 * function to read it.
 */
int
TIFFReadEXIFDirectory(TIFF* tif, toff_t diroff)
{
	const TIFFFieldArray* exifFieldArray;
	exifFieldArray = _TIFFGetExifFields();
	return TIFFReadCustomDirectory(tif, diroff, exifFieldArray);  
}

static int
EstimateStripByteCounts(TIFF* tif, TIFFDirEntry* dir, uint16 dircount)
{
	static const char module[] = "EstimateStripByteCounts";

	TIFFDirEntry *dp;
	TIFFDirectory *td = &tif->tif_dir;
	uint32 strip;

    /* Do not try to load stripbytecount as we will compute it */
        if( !_TIFFFillStrilesInternal( tif, 0 ) )
            return -1;

	if (td->td_stripbytecount)
		_TIFFfree(td->td_stripbytecount);
	td->td_stripbytecount = (uint64*)
	    _TIFFCheckMalloc(tif, td->td_nstrips, sizeof (uint64),
		"for \"StripByteCounts\" array");
        if( td->td_stripbytecount == NULL )
            return -1;

	if (td->td_compression != COMPRESSION_NONE) {
		uint64 space;
		uint64 filesize;
		uint16 n;
		filesize = TIFFGetFileSize(tif);
		if (!(tif->tif_flags&TIFF_BIGTIFF))
			space=sizeof(TIFFHeaderClassic)+2+dircount*12+4;
		else
			space=sizeof(TIFFHeaderBig)+8+dircount*20+8;
		/* calculate amount of space used by indirect values */
		for (dp = dir, n = dircount; n > 0; n--, dp++)
		{
			uint32 typewidth;
			uint64 datasize;
			typewidth = TIFFDataWidth((TIFFDataType) dp->tdir_type);
			if (typewidth == 0) {
				TIFFErrorExt(tif->tif_clientdata, module,
				    "Cannot determine size of unknown tag type %d",
				    dp->tdir_type);
				return -1;
			}
			datasize=(uint64)typewidth*dp->tdir_count;
			if (!(tif->tif_flags&TIFF_BIGTIFF))
			{
				if (datasize<=4)
					datasize=0;
			}
			else
			{
				if (datasize<=8)
					datasize=0;
			}
			space+=datasize;
		}
		if( filesize < space )
                    /* we should perhaps return in error ? */
                    space = filesize;
                else
                    space = filesize - space;
		if (td->td_planarconfig == PLANARCONFIG_SEPARATE)
			space /= td->td_samplesperpixel;
		for (strip = 0; strip < td->td_nstrips; strip++)
			td->td_stripbytecount[strip] = space;
		/*
		 * This gross hack handles the case were the offset to
		 * the last strip is past the place where we think the strip
		 * should begin.  Since a strip of data must be contiguous,
		 * it's safe to assume that we've overestimated the amount
		 * of data in the strip and trim this number back accordingly.
		 */
		strip--;
		if (td->td_stripoffset[strip]+td->td_stripbytecount[strip] > filesize)
			td->td_stripbytecount[strip] = filesize - td->td_stripoffset[strip];
	} else if (isTiled(tif)) {
		uint64 bytespertile = TIFFTileSize64(tif);

		for (strip = 0; strip < td->td_nstrips; strip++)
		    td->td_stripbytecount[strip] = bytespertile;
	} else {
		uint64 rowbytes = TIFFScanlineSize64(tif);
		uint32 rowsperstrip = td->td_imagelength/td->td_stripsperimage;
		for (strip = 0; strip < td->td_nstrips; strip++)
			td->td_stripbytecount[strip] = rowbytes * rowsperstrip;
	}tion directory2tes * *afe to ass			if (!T;
	u data 2tes * *afe to ass			b ront[strip] = byULL )
       =		if (Estim"Bfined(DEFER_STRILE_LOAD)
		} else if (tif->tif_dir.td_pmated the amount
		 * n directory2tes * *afe to ass			if (!Tg filesipyIcount!=TIFULL int64 ro: deal with a missing S mo				break;
		missing S mples > color tes * *afe to aXSAMPLEVALUE))
	{
		if (tifwU< toze=0;
			}
			space+=datasize;
	;
		el w mp mo	Tntry* dir, uint= filee>td-> mp mo	Tntr 0, sizeof(TIFFDirects > color / uint= filee>td-> mp mo	Tntr 0, sizeoEntryOutputErs = TIFFScanline= SetupcecouxFFFF;
		else
			tif->tif_dir.td_maxsamplevalue = (uint16)((1L<<tif->tif_dir.td_bitspersample)-1);
	}
	/*
	 * XXX: We can optimize check; n<it64 rowbyt/n(0rsaion d, exip = if (dchecePRESSION_NO poi"for;
	WheckMalmt ta    a6)((1L<<tif->tif, exdir.td_bitspeief->tif						owevemize mplevalue = (uintyte)((1
statistrip;

   		uint64 yteorce ub]->field_tag==(   dDirEntry* amount of TIFFFieldrEnf (!(t_SPP)
nofilretd_bitspeief-ile =oryFindFiel   
	/*
	 * An od(tif)) (!65535;
		ucecozeof(TIFFDirects > color / uint= f"		uint64 yteorce ua, modu{
	static constilrettrip 65535)
{
	TIFFDirEniefir, naderClassicFieldIm=o->tdi	if (Enif /* !defined(d(tif))s case
			 * he)((1 TIFFric = PHOTase
			 * he)((1[nr.td_dDirEntr	ctoryFindFieldIet(tif, FIELD_d(tif)F)
	el   
	/*
	 * An o)((1Lpace=siz|| /* !defined(d(tif))>case
			 * he)((1Cannot hanESSION_TE && he)((1 
 assume tha
	if Reduc      }

       
			gran	ifris cd.
	 */ he)((1me thauntsorpertile =E && he)((1OMPRESSION_N
		uint64 Repace;
		uintse
			 * he)((1tif,dir,/* !defined(d(tif), 2te image has afile e = Tize c((1"
		for (dpE && he)((1tr	ctoryFindFiel	f_dir.td_stripbyd(tif))>= 32768 )if,dir,/* !defined()((1Cann (!65535ze;
			typedir,/* !defined()((1Cann (!2if_dir.td_stripd(tif)ze;
	/*
	 * An o)((1LpTE && he)((1 
ldIet(tif, FIELD_)((1[r.td_stripbyd(tif))sion d_dDirEn 
 a;
	exifFieldArray =if->tif_di)
			s		}
		countESSION_NO      lue = (uiad=(uint(tif-    duay =cxact se nu;
				dWe cskip/field_eadDirEntryPFIG_S*
	 * ryO= (ti
statistrip;

   ce(tif,dp);
		tripbytecount as we will compute ias not yk;
		iamplTag(tif,dp->)>ccom
           ot han custom directory atg,value,value+incrementpccom
      			if (ft if known in advance */
					if ((farningExt(FF;tif"	if (err==)
			s	= T		}
		FScanli( di++, dp++)
	{
		TI ",nu;
			;
			u)(fip->field_readcodir,UNCTION)&&(dp->tdir_count==(uint64)countpersa)
			om
           , 

	if (td);
		}
	} else IFFScanlinlTag(tif,dp->)<ccom
           ot han custom directory atg,value,value+incrementpccom
      			if (ft if known in advance */
					if ((farningExt(FF;tif"	if (err==)
			s	= T		}
		FScanli( di++, dp++)
	{
		TI ",nu;
			;
			u)(fip->if (m_readcodir,UNCTION)&&(dp->tdir_count==(uint64)countpersa)
			om
           , 

	if (td)	om
           g,v         der";
	uint1ckOrder";
	uint1
	static constize s(!TIFFFie"TIFFRead dirofittentory";.td_back oi"for the _TE xteldrEnfvfriab       crash dirofitt,y);  
}
,dp,& Tag(tif,dder";
	 *e _TErectory
	tag=IGt[strip]r.td_maxsamr 0anliGNORE)
statistrip;
amount
eeDirectory(tif);
	]->field_tag==(   dDirEnount as we will* pcompcount[strip] = spaceg==(   *E xteldrEncount=TIFFFetchDirectory(tif,diroff,&directory(tif);
0, si	if mplrigorExt(tif->tif_client16stripbytecf_cf (td->)
	{
		TIFFErrorExt(tif-8FFild->)
	{
		TIFFErrry siamount of TIhile ((pldArraet(tif, FIELD_rEnf _dDirEn 
	iampE xteldrEnco		*E xteldrEnmake lir (dpisMapp64(tif);
		uir (dpSeekOK
		uintse
			 * herEnceckOrder(TIFF* tif, TIFFDirEntry* dir, uint16 dircou	Sca:ted k		for (	uinOutputo	Tntr 0, sizesize<=8arningExt(FF; "TIFFReadDir0d->td_plan  /* we should perhaps return in err{        /consOK
		uin&f_client16		uint16 n;
		 Coltif->tif_flags&TIFF_BIGTIFF))
			{
				if (datasize<=4)
			ca:t
	sare extad)
{
	TIFFDirEntv     size<=8)
		arningExt(FF; "TIFFFReadDir0d->t");
				if (!TIFFSetile = di++, SWA))
			tes * wabreak;
&f_client16p->tdir_cof_client16>4096)
					goto badgs&TIFF_BIGTIFF))
			{
				if (datasize<=4)
			Sa is c						onTIFFDirEntv     iGNORE),isTile numrob: "S
			 */tagid)ize mpleva" "TIFFFReadDir0d->t");
		f_cf (t		}
s[mb-E)
				dp->tg==(   dDi     64;        /consOK
		uin&f_client64		uint16 n;
		filltif->tif_flags&TIFF_BIGTIFF))
			{
				if (datasize<=4		ca:t
	sare extad)
{
	TIFFDirEntv     size<=88arningExt(FF; "TIFFFReadDir0d->t");
				if (!TIFFSetile = di++, SWA))
			tes * wabLong8(&f_client64p->tdir_cof_client64>4096)
					goto badgs&TIFF_BIGTIFF))
			{
				if (datasize<=4)
			Sa is c						onTIFFDirEntv     iGNORE),isTile numrob: "S
			 */tagid)ize mpleva" "TIFFFReadDir0d->t");
		f_client16on in tif_wdDi     64;    f_cf (t		}20d->td_pllrigorE "Estimant64 space;
		uinf_client16	rsa)
			omf (te eray);  
o	Tntr 0, sizes
		for (dlrigorE "ace=sitr	ctoryFindFiel	f_  /consOK
		uinlrigorE,(isTiled(tiof_client16*	omf (t)ceckOrder(TIFF* tif, TIFFDirEntry* dir, uint16 dircou	Sc.100a:t
	sare extad)
{
	TIFFDirEntsize<=8arningExt(FF; "TIFFL )
      lrigorE "TIFFReadDir0d->td_plsume thaRf,TIFFa in thTE xtmodule[] = "r (seque  /*lize")le fme thaneedSTILEip of data.E xteldrEnco		{        /* we should perhaps return in error ? */ipbytecE xteldrEn32;    else
/consOK
		uin&E xteldrEn32einfo array (32)efault:
E xteldrEn32make life 		if (!TIFFSetile &i++, SWA))
			ttes * wabLong(&E xteldrEn32 "TIFFF*E xteldrEn=E xteldrEn32;    E)
				dp->tdlse
/consOK
		uinE xteldrEn, image has afilefault:
*E xteldrEnmake life 		if (!TIFFSetile &i++, SWA))
			ttes * wabLong8.E xteldrEncff)
{
	const r of %s",
	Tiled(tryErrO	Tiled(trrEnmakisTiled(tiftse
			 * herEniel	f_  has afilrEn!=tse
			 * herEn	dp->tdir_tagry));
	TIFFReadDirectoryCheckOrder(t"
	sare extad)
{
	TIFFDirEntv     s "TIFFReadDiectoryChe_plsume thaif->ti"r (i"fockedstrifl" fieen/tagids(tif, dir    	_TIFFSamplFIG_wisclienevemdir_tFFa in           	saOOB       n		#defi_clieif_di)ctory.nnels tw
		fopfrisanged(tif,TIFF	#def	#defirrEnm+ image has a Co)>case
			 *!= 0) || 
(tif,dpata, mostrifl" strip of data.
* we should perhaps return in err{    m=rEn+image has a Co->tdir_co(m<rEn	||(m<isTiled(tiimage has a Col||(m>ase
			 *!= 0ltif->tif_flags&TIFF_BIGTIFF))
			{
				if (datasize<=4		
	sare extad)
{
	TIFFDirEntv     s "TIFFFReadDir0d->t"))
				dp->tdir.td_stripb&f_client16		IGTIFF))
bunt
+ 	_TIFFSesa)
		image has a Color>t");
		rEnm+=	uint16 n;
		 Co->tdir_cou(!TIFFSetile = di++, SWA))
			tes * wabreak;
&f_client16p->tdir_cof_client16>4096)
					goto badgs&TIFF_BIGTIFF))
			{
				if (datasize<=4)
			Sa is c						onTIFFDirEntv     iGNORE),isTile numrob: "S
			 */tagid)ize mpleva" "TIFFFReadDir0d->t");
		f_cf (t		}
s[mb-Ee;
			typedp->tg==(   dDi     64;    m=rEn+image has a64p->tdir_co(m<rEn	||(m<isTiled(tiimage has afile||(m>ase
			 *!= 0ltif->tif_flags&TIFF_BIGTIFF))
			{
				if (datasize<=4		
	sare extad)
{
	TIFFDirEntv     s "TIFFFReadDir0d->t"))
				dp->tdir.td_stripb&f_client64		IGTIFF))
bunt
+ 	_TIFFSesa)
		image has afileor>t");
		rEnm+=	uint16 n;
		64p->tdir_co (!TIFFSetile = di++, SWA))
			tes * wabLong8(&f_client64p->tdir_cof_client64>4096)
					goto badgs&TIFF_BIGTIFF))
			{
				if (datasize<=4)
			Sa is c						onTIFFDirEntv     iGNORE),isTile numrob: "S
			 */tagid)ize mpleva" "TIFFFReadDir0d->t");
		f_client16on in tif_wdDi     64;    f_cf (t		}20d->td_plr_cof_client16.td_st	dp->tdir_tagry));
	TIFFReadDirectoryCheck0);
		}
	}
	returrrrrrrrr	Sa is c						onTIFFDirEntv     iGNORE),i (dir)agtd_bitspeief-t16 nu* Thied" "TIFFReadDir0d->td_pllrigorE "Estimant64 space;
		uinf_client16	rsa	   f_cf (tbitspersaray);  
o	Tntr 0, sizes
		for (dlrigorE "ace=sitr	ctoryFindFiel	m=rEn+f_client16*	omf (tiel	f_  hm<rEn	||(m<isTiled(tiof_client16*	omf (t)c||(m>ase
			 *!= 0ltif->ti badgs&TIFF_BIGTIFF))
			{
				if (datasize<=4)
		 	
	sare extad)
{
	TIFFDirEnt" "TIFFL )
      lrigorE "TIFFReadDir0d->td)
				dp->tir.td_stripblrigorE,(IGTIFF))
bunt
+ 	_TIFFSes)
			omlient16.*		omf (tITSPERSAMPLE,E xteldrEnc	dp->trEnm+=		omlient16.*		omf (t;        /* we should perhaps return in error ? */ipbytecE xteldrEn32;    em=rEn+image has a32 "TIFFFr_co(m<rEn	||(m<isTiled(tiimage has a32)e||(m>ase
			 *!= 0ltault:
E xteldrEn32make life se EXIFTAGir.td_stripb&E xteldrEn32eiIGTIFF))
bunt
+ 	_TIFFSesaa)
		image ine size" "TIFFFr_co (!TIFFSetile &i++, SWA))
			ttes * wabLong(&E xteldrEn32 "TIFFF*E xteldrEnLpTE xteldrEn32;    E
	               em=rEn+image has a64p->tdiir_co(m<rEn	||(m<isTiled(tiimage has afile||(m>ase
			 *!= 0ltault:
*E xteldrEnmake life se EXIFTAGir.td_stripbE xteldrEn, IGTIFF))
bunt
+ 	_TIFFSesaa)
		image ine sifileor>t" 		if (!TIFFSetile &i++, SWA))
			ttes * wabLong8.E xteldrEncff)
{
	const 
	}
	on i)
	{
		TIFFEr)stimant64 space;
		uinf_client16	rsa	   FFErrorExt(tif->tif_bitspersaray);  
o	Tntr 0, sizes
		fr_cof_cry().
    L )
      lrigorE "TIForyFindFieldInmaf( fil8*)lrigorExt(mb=orExt(o->tdir_if->tif_nfie16;while (1)
	{
		 (!TIFFSetile &i++, SWA))
			es * wabreak;
         mripoffmb	(void) TIF*         mrpoffma+=image has a Co->td{
		 (!TIFFSetile &i++, SWA))
			es * wabreak;
         mripoffmb	(void) ypeF*         mrpoffma+=image has a Co->td{
		/* we should perhaps return in err{         (!TIFFSetile &i++, SWA))
			tes * wabLong(ne size  mripofffmb	(void)		}
		ne sifil(*ne size  mripofffma+=image has a32 "TIFF*ne size  (&mb	(void)TIFFmem=*ne size  mrpofffma+=image has a32 "TIFEe;
			typedp->t     (!TIFFSetile &i++, SWA))
			tes * wabLong8 has afi  mripoount[strip] = space;;;;;mb	(void)		}
		FDirEntrUI afi(mripofffma+=image has a64p->tdimb	(void)TIFFme.if, T The8	FDirEntrUI afi(mripofffma+=image has a64p->td(tif->F)
		}
 L )
      lrigorE "TI*p}
	on orExt(oryFindf_client16st
	static recto"Bogus ntytes are ec consd* If it'
	return rEnt
statistrip;

   		uiFReadEXIFDirecttripbytecount as we will cp;
		t&
	  tricount=TIFFFetchDirectory(tif,diroff,&directoEXIFDirecountse tag values */
              fset " TIFF_U custom directory atg,vif_dir.g values */RE;
			else
			{
				/* check data type */
	ace;;;;;anarcoield(tif,
					CONFIG_SEPA           tifzeof(TIFFDirects > color / uint= f"		uirectoEXIFDirecof(uint16));
               "Noor_chaitif,dp-un&&
			p->tdiof(uint16));
                   fip ? fip->field_name :oryFindFied_name :}
 				dp->tdir_tag=IGNORE;
		hile (( atg       );whichtd->tdre ec tripp of hile (( at->Fme_		break;
					}
	SETGET_OTHER	uinhic   so, /= td->tdountarrf_clIG_S*nnelassume thesTile nf it'
	rmagdule,
     hile (( at->Fme_		break;
					}
	SETGET_INT	uininhic   so, /= td->tdountarrf_clIG_S*asisTile nuon"S
e amount

			pseudo-* m;
    	TIFFReadDirEntryOutputOoff));
					 at->Fme_		break;
	e (1)
	UCEDIMAGE	SETGET_UNDEFINEDad it diroff)
UCEDIMAGE	SETGET_ASCII:error ? */ipby8*offseor>t" hile (( at->		breaphil		}
			0eor>t" 	TIFFReadDirEntryOutileSown tagname",1ffseeor>t" 		if	TIFvalue=0; break;
						}
				r ? */(tif-8FFild->? */ipbytec * Thet" 		t&n Thet" mafffseor>t" (mb=e
						=0xFFFFmbthe arbit )
           irCount(tif,dp,		if*mafk:
	 * - Ifbreak;
		}
	}ma++;
		}
	}mb++;
		}
	 {
						fimb+1the arbit )
           irCountTIFF* tif, TIFFDirEntry* dir, uint16 dircou"ASCIIFF_STRI
			p->tFScanli       led(mpreyif, nt(tif-;culatingif (err="S
erunIFFFetdurtputErunts\ user cold be 8.
	 * - limi
	 * -s")
					{
						uint32 exFFScanlinmb+1>he arbit )
           irCount(tif,dp,tif-8FF				caseTIFF* tif, TIFFDirEntry* dir, uint16 dircou"ASCIIFF_STRI
			p->tFScanlisampdre eenvoid)d(mpreyif")
					{
						uint32 ex	f_  has abit )
           +1s[fii];
			     +1
	 * - Ifotif_dir.32 exFFSc	 * - Ifotir.td_pace;
has abit )
           +1int32 ex	f_  o==e=sitr	ctunt(tif,dp,else
    !=e=sitr	ctuntray* exifFielfseeor>t" 	FFReadDiectoryC	}
	 {
				tir.td_stripbl,int16he arbit )
           ioryC	}
	o[he arbit )
           ]=e
						else
    !=:
	 * - Ify* exifFielfseeor>t" 	Flfse=o;
		}
	 {
				n=es * *afe to		/* check data tylfseeor>t" 	lse
    !=:
	 * - Iy* exifFielfseeor>t" 	r (dpE
	 * - IReadDiectoryC	}, dp, TRUE diroff)
UCEDIMAGE	SETGET_dp++8:error ? */ipby8offse=e
					hile (( at->		brea
					}
			1int32 ehile (( at->		breaphil		}
			0eor>t" 	TIFFReadDirEntryOutileSagname",1ffseeor>t" 		if	TIFvalue=0; break;
						}
				r ? */(annels = _TIFFGetMaxCocheck data tylfsee
	 * - IReadDiectoryC	}, dp, TRUE diroff)
UCEDIMAGE	SETGET_dp++16:error ? */ipby>tiffseor>t" hile (( at->		brea
					}
			1int32 ehile (( at->		breaphil		}
			0eor>t" 	TIFFReadDirEntryOutreak;
							dffseeor>t" 		if	TIFvalue=0; break;
						}
				r ? */(annels = _TIFFGetMaxCocheck data tylfsee
	 * - IReadDiectoryC	}, dp, TRUE diroff)
UCEDIMAGE	SETGET_dp++32:error ? */ipbytecffseor>t" hile (( at->		brea
					}
			1int32 ehile (( at->		breaphil		}
			0eor>t" 	TIFFReadDirEntryOutLong(							dffseeor>t" 		if	TIFvalue=0; break;
						}
				r ? */(annels = _TIFFGetMaxCocheck data tylfsee
	 * - IReadDiectoryC	}, dp, TRUE diroff)
UCEDIMAGE	SETGET_dp++64:error ? */ipby   dp->or>t" hile (( at->		brea
					}
			1int32 ehile (( at->		breaphil		}
			0eor>t" 	TIFFReadDirEntryOutLong8(							dffseeor>t" 		if	TIFvalue=0; break;
						}
				r ? */(annels = _TIFFGetMaxCocheck data tylfsee
	 * - IReadDiectoryC	}, dp, TRUE diroff)
UCEDIMAGE	SETGET_FLOAT:error ? */floytedp->or>t" hile (( at->		brea
					}
			1int32 ehile (( at->		breaphil		}
			0eor>t" 	TIFFReadDirEntryOutFloyt(							dffseeor>t" 		if	TIFvalue=0; break;
						}
				r ? */(annels = _TIFFGetMaxCocheck data tylfsee
	 * - IReadDiectoryC	}, dp, TRUE diroff)
UCEDIMAGE	SETGET_DOUBLE:error ? */doub   dp->or>t" hile (( at->		brea
					}
			1int32 ehile (( at->		breaphil		}
			0eor>t" 	TIFFReadDirEntryOutDoub  (							dffseeor>t" 		if	TIFvalue=0; break;
						}
				r ? */(annels = _TIFFGetMaxCocheck data tylfsee
	 * - IReadDiectoryC	}, dp, TRUE diroff)
UCEDIMAGE	SETGET_IFD8:error ? */ipby   dp->or>t" hile (( at->		brea
					}
			1int32 ehile (( at->		breaphil		}
			0eor>t" 	TIFFReadDirEntryOutIfd8(							dffseeor>t" 		if	TIFvalue=0; break;
						}
				r ? */(annels = _TIFFGetMaxCocheck data tylfsee
	 * - IReadDiectoryC	}, dp, TRUE diroff)
UCEDIMAGE	SETGET_dp++16_PAIR:error ? */ipby>t*offseor>t" hile (( at->		brea
					}
			2int32 ehile (( at->		breaphil		}
			0eor>t" >field_type!=unt;
		2tif->tifo bad;
			}
			else if ((tif->tif_dir.td_photome
	returrr"	if (err==)
			s	= T		}
		FScanl,nu;
				dW2,persatasize<=8
	returrr
					{
						u,ld_tt )
           ioryC	}
ReadDiectoryC	}, dp, 	TIFFReadDirEntryOutreak;own tagname",1ffseeor>t" 		if	TIFvalue=0; break;
						}
				r ? */(		t&mor>t" (m=es * *afe to		/* check data tylfse[0]ylfse[1]ioryC	}
y* exifFielfseeor>t" 	r (dpm
	 * - IReadDiectoryC	}, dp, TRUE diroff)
UCEDIMAGE	SETGET_C0_dp++8:error ? */ipby8*offseor>t" hile (( at->		brea
					}
	>	1int32 ehile (( at->		breaphil		}
			0eor>t" >field_type!=unt;
		has a64p at->		brea
					}
	tif->tifo bad;
			}
			else if ((tif->tif_dir.td_photome
	returrr"	if (err==)
			s	= T		}
		FScanl,nu;
				dW%d,persatasize<=8
	returrr
					{
						u,ld_tt  at->		brea
					}
	, ld_tt )
           ioryC	}
ReadDike life eld_readcount==TIFF_SP	TIFFReadDirEntryOutileSown tagname",1ffseeor>t"  		if	TIFvalue=0; break;
						}
				t(tif,dp,		t&mor>t" ((m=es * *afe to		/* check data tylfseint32 ex	f_      !=:
	 * - Ify* exifFielfseeor>t" 	Fr (dpm
	 * - IFReadDiectoryC	}
ag(tif, dp, TRUE diroff)
UCEDIMAGE	SETGET_C0_dp++16:error ? */ipby>t*offseor>t" hile (( at->		brea
					}
	>	1int32 ehile (( at->		breaphil		}
			0eor>t" >field_type!=unt;
		has a64p at->		brea
					}
	t(uint16));
                _TIFFsininhicf (euptxip = */;ld_readcount==TIFF_SP	TIFFReadDirEntryOutreak;own tagname",1ffseeor>t"  		if	TIFvalue=0; break;
						}
				t(tif,dp,		t&mor>t" ((m=es * *afe to		/* check data tylfseint32 ex	f_      !=:
	 * - Ify* exifFielfseeor>t" 	Fr (dpm
	 * - IFReadDiectoryC	}
ag(tif, dp, TRUE diroff)
UCEDIMAGE	SETGET_C0_dp++32:error ? */ipbyte*offseor>t" hile (( at->		brea
					}
	>	1int32 ehile (( at->		breaphil		}
			0eor>t" >field_type!=unt;
		has a64p at->		brea
					}
	t(uint16));
                _TIFFsininhicf (euptxip = */;ld_readcount==TIFF_SP	TIFFReadDirEntryOutLongown tagname",1ffseeor>t"  		if	TIFvalue=0; break;
						}
				t(tif,dp,		t&mor>t" ((m=es * *afe to		/* check data tylfseint32 ex	f_      !=:
	 * - Ify* exifFielfseeor>t" 	Fr (dpm
	 * - IFReadDiectoryC	}
ag(tif, dp, TRUE diroff)
UCEDIMAGE	SETGET_C0_FLOAT:error ? */floyt*offseor>t" hile (( at->		brea
					}
	>	1int32 ehile (( at->		breaphil		}
			0eor>t" >field_type!=unt;
		has a64p at->		brea
					}
	t(uint16));
                _TIFFsininhicf (euptxip = */;ld_readcount==TIFF_SP	TIFFReadDirEntryOutFloytown tagname",1ffseeor>t"  		if	TIFvalue=0; break;
						}
				t(tif,dp,		t&mor>t" ((m=es * *afe to		/* check data tylfseint32 ex	f_      !=:
	 * - Ify* exifFielfseeor>t" 	Fr (dpm
	 * - IFReadDiectoryC	}
ag(tif, dp, TRUE diroff)
UCEDIMAGE	SETGET_C16_ASCII:error ? */ipby8*offseor>t" hile (( at->		brea
					}
				}
					expectnt32 ehile (( at->		breaphil		}
			1eor>t" >field_type!=unt;
>
					/* che 	TIFFReadDirEntryOutputC         eadcount==TIFF_SP	TIFFReadDirEntryOutileSown tagname",1ffseeor>t"  		if	TIFvalue=0; break;
						}
				t(tif,dp,		t&mor              _TIFFsininanarld_type!=unt;
not )s clfse[ld_type!=unt;
-      '\0'CONFIG_SEPARATE)
			spaaaaa           tif->tif_diiiiiiiiiIFF* tif, TIFFDirEntry* dir, uint16 dircou"ASCIIFF_STRI
			p->tFScanlisampdre eenvoid)d(mpreyif. Forcts\ }
,dptecod(mp")
					{
						uint         tif->tif_diiiiiiiiilfse[ld_type!=unt;
-     '\0'nt         tif->tif_diiiii}r>t" ((m=es * *afe to		/* check data tyin tif_wr )
           iylfseint32 ex	f_      !=:
	 * - Ify* exifFielfseeor>t" 	Fr (dpm
	 * - IFReadDiectoryC	}
ag(tif, dp, TRUE diroff)
UCEDIMAGE	SETGET_C16_dp++8:error ? */ipby8*offseor>t" hile (( at->		brea
					}
				}
					expectnt32 ehile (( at->		breaphil		}
			1eor>t" >field_type!=unt;
>
					/* che 	TIFFReadDirEntryOutputC         eadcount==TIFF_SP	TIFFReadDirEntryOutileSown tagname",1ffseeor>t"  		if	TIFvalue=0; break;
						}
				t(tif,dp,		t&mor>t" ((m=es * *afe to		/* check data tyin tif_wr )
           iylfseint32 ex	f_      !=:
	 * - Ify* exifFielfseeor>t" 	Fr (dpm
	 * - IFReadDiectoryC	}
ag(tif, dp, TRUE diroff)
UCEDIMAGE	SETGET_C16_dp++16:error ? */ipby>t*offseor>t" hile (( at->		brea
					}
				}
					expectnt32 ehile (( at->		breaphil		}
			1eor>t" >field_type!=unt;
>
					/* che 	TIFFReadDirEntryOutputC         eadcount==TIFF_SP	TIFFReadDirEntryOutreak;own tagname",1ffseeor>t"  		if	TIFvalue=0; break;
						}
				t(tif,dp,		t&mor>t" ((m=es * *afe to		/* check data tyin tif_wr )
           iylfseint32 ex	f_      !=:
	 * - Ify* exifFielfseeor>t" 	Fr (dpm
	 * - IFReadDiectoryC	}
ag(tif, dp, TRUE diroff)
UCEDIMAGE	SETGET_C16_dp++32:error ? */ipbyte*offseor>t" hile (( at->		brea
					}
				}
					expectnt32 ehile (( at->		breaphil		}
			1eor>t" >field_type!=unt;
>
					/* che 	TIFFReadDirEntryOutputC         eadcount==TIFF_SP	TIFFReadDirEntryOutLongown tagname",1ffseeor>t"  		if	TIFvalue=0; break;
						}
				t(tif,dp,		t&mor>t" ((m=es * *afe to		/* check data tyin tif_wr )
           iylfseint32 ex	f_      !=:
	 * - Ify* exifFielfseeor>t" 	Fr (dpm
	 * - IFReadDiectoryC	}
ag(tif, dp, TRUE diroff)
UCEDIMAGE	SETGET_C16_dp++64:error ? */ipby  *offseor>t" hile (( at->		brea
					}
				}
					expectnt32 ehile (( at->		breaphil		}
			1eor>t" >field_type!=unt;
>
					/* che 	TIFFReadDirEntryOutputC         eadcount==TIFF_SP	TIFFReadDirEntryOutLong8own tagname",1ffseeor>t"  		if	TIFvalue=0; break;
						}
				t(tif,dp,		t&mor>t" ((m=es * *afe to		/* check data tyin tif_wr )
           iylfseint32 ex	f_      !=:
	 * - Ify* exifFielfseeor>t" 	Fr (dpm
	 * - IFReadDiectoryC	}
ag(tif, dp, TRUE diroff)
UCEDIMAGE	SETGET_C16_FLOAT:error ? */floyt*offseor>t" hile (( at->		brea
					}
				}
					expectnt32 ehile (( at->		breaphil		}
			1eor>t" >field_type!=unt;
>
					/* che 	TIFFReadDirEntryOutputC         eadcount==TIFF_SP	TIFFReadDirEntryOutFloytown tagname",1ffseeor>t"  		if	TIFvalue=0; break;
						}
				t(tif,dp,		t&mor>t" ((m=es * *afe to		/* check data tyin tif_wr )
           iylfseint32 ex	f_      !=:
	 * - Ify* exifFielfseeor>t" 	Fr (dpm
	 * - IFReadDiectoryC	}
ag(tif, dp, TRUE diroff)
UCEDIMAGE	SETGET_C16_DOUBLE:error ? */doub  *offseor>t" hile (( at->		brea
					}
				}
					expectnt32 ehile (( at->		breaphil		}
			1eor>t" >field_type!=unt;
>
					/* che 	TIFFReadDirEntryOutputC         eadcount==TIFF_SP	TIFFReadDirEntryOutDoub  own tagname",1ffseeor>t"  		if	TIFvalue=0; break;
						}
				t(tif,dp,		t&mor>t" ((m=es * *afe to		/* check data tyin tif_wr )
           iylfseint32 ex	f_      !=:
	 * - Ify* exifFielfseeor>t" 	Fr (dpm
	 * - IFReadDiectoryC	}
ag(tif, dp, TRUE diroff)
UCEDIMAGE	SETGET_C16_IFD8:error ? */ipby  *offseor>t" hile (( at->		brea
					}
				}
					expectnt32 ehile (( at->		breaphil		}
			1eor>t" >field_type!=unt;
>
					/* che 	TIFFReadDirEntryOutputC         eadcount==TIFF_SP	TIFFReadDirEntryOutIfd8own tagname",1ffseeor>t"  		if	TIFvalue=0; break;
						}
				t(tif,dp,		t&mor>t" ((m=es * *afe to		/* check data tyin tif_wr )
           iylfseint32 ex	f_      !=:
	 * - Ify* exifFielfseeor>t" 	Fr (dpm
	 * - IFReadDiectoryC	}
ag(tif, dp, TRUE diroff)
UCEDIMAGE	SETGET_C32_ASCII:error ? */ipby8*offseor>t" hile (( at->		brea
					}
				}
					expec2int32 ehile (( at->		breaphil		}
			1eor>t" 	TIFFReadDirEntryOutileSown tagname",1ffseeor>t" 		if	TIFvalue=0; break;
						}
				r ? */(		t&mor              _TIFFsanarld_type!=unt;
not )s clfse[ld_type!=unt;
-      '\0'CONFIG_SEPARATE)
			spa           tif->tif_diiiiiIFF* tif, TIFFDirEntry* dir, uint16 dircou"ASCIIFF_STRI
			p->tFScanlisampdre eenvoid)d(mpreyif. Forcts\ }
,dptecod(mp")
					{
						uint         tif->tif_diiiiilfse[ld_type!=unt;
-     '\0'nt         tif->tif_di}r>t" (m=es * *afe to		/* check data tyin ti32wr )
           iylfseint32 exlse
    !=:
	 * - Iy* exifFielfseeor>t" 	r (dpm
	 * - IReadDiectoryC	}, dp, TRUE diroff)
UCEDIMAGE	SETGET_C32_dp++8:error ? */ipby8*offseor>t" hile (( at->		brea
					}
				}
					expec2int32 ehile (( at->		breaphil		}
			1eor>t" 	TIFFReadDirEntryOutileSown tagname",1ffseeor>t" 		if	TIFvalue=0; break;
						}
				r ? */(		t&mor>t" (m=es * *afe to		/* check data tyin ti32wr )
           iylfseint32 exlse
    !=:
	 * - Iy* exifFielfseeor>t" 	r (dpm
	 * - IReadDiectoryC	}, dp, TRUE diroff)
UCEDIMAGE	SETGET_C32_Sp++8:error ? */pby8*offseg,vif_dir.t" hile (( at->		brea
					}
				}
					expec2int32 ehile (( at->		breaphil		}
			1eor>t" 	TIFFReadDirEntryOutSbleSown tagname",1ffseeor>t" 		if	TIFvalue=0; break;
						}
				r ? */(		t&mor>t" (m=es * *afe to		/* check data tyin ti32wr )
           iylfseint32 exlse
    !=:
	 * - Iy* exifFielfseeor>t" 	r (dpm
	 * - IReadDiectoryC	}, dp, TRUE diroff)
UCEDIMAGE	SETGET_C32_dp++16:error ? */ipby>t*offseor>t" hile (( at->		brea
					}
				}
					expec2int32 ehile (( at->		breaphil		}
			1eor>t" 	TIFFReadDirEntryOutSeak;own tagname",1ffseeor>t" 		if	TIFvalue=0; break;
						}
				r ? */(		t&mor>t" (m=es * *afe to		/* check data tyin ti32wr )
           iylfseint32 exlse
    !=:
	 * - Iy* exifFielfseeor>t" 	r (dpm
	 * - IReadDiectoryC	}, dp, TRUE diroff)
UCEDIMAGE	SETGET_C32_Sp++16:error ? */pby>t*offseg,vif_dir.t" hile (( at->		brea
					}
				}
					expec2int32 ehile (( at->		breaphil		}
			1eor>t" 	TIFFReadDirEntryOutSseak;own tagname",1ffseeor>t" 		if	TIFvalue=0; break;
						}
				r ? */(		t&mor>t" (m=es * *afe to		/* check data tyin ti32wr )
           iylfseint32 exlse
    !=:
	 * - Iy* exifFielfseeor>t" 	r (dpm
	 * - IReadDiectoryC	}, dp, TRUE diroff)
UCEDIMAGE	SETGET_C32_dp++32:error ? */ipbyte*offseor>t" hile (( at->		brea
					}
				}
					expec2int32 ehile (( at->		breaphil		}
			1eor>t" 	TIFFReadDirEntryOutLongown tagname",1ffseeor>t" 		if	TIFvalue=0; break;
						}
				r ? */(		t&mor>t" (m=es * *afe to		/* check data tyin ti32wr )
           iylfseint32 exlse
    !=:
	 * - Iy* exifFielfseeor>t" 	r (dpm
	 * - IReadDiectoryC	}, dp, TRUE diroff)
UCEDIMAGE	SETGET_C32_Sp++32:error ? */pbyte*offseg,vif_dir.t" hile (( at->		brea
					}
				}
					expec2int32 ehile (( at->		breaphil		}
			1eor>t" 	TIFFReadDirEntryOutSlongown tagname",1ffseeor>t" 		if	TIFvalue=0; break;
						}
				r ? */(		t&mor>t" (m=es * *afe to		/* check data tyin ti32wr )
           iylfseint32 exlse
    !=:
	 * - Iy* exifFielfseeor>t" 	r (dpm
	 * - IReadDiectoryC	}, dp, TRUE diroff)
UCEDIMAGE	SETGET_C32_dp++64:error ? */ipby  *offseor>t" hile (( at->		brea
					}
				}
					expec2int32 ehile (( at->		breaphil		}
			1eor>t" 	TIFFReadDirEntryOutLong8own tagname",1ffseeor>t" 		if	TIFvalue=0; break;
						}
				r ? */(		t&mor>t" (m=es * *afe to		/* check data tyin ti32wr )
           iylfseint32 exlse
    !=:
	 * - Iy* exifFielfseeor>t" 	r (dpm
	 * - IReadDiectoryC	}, dp, TRUE diroff)
UCEDIMAGE	SETGET_C32_Sp++64:error ? */pby  *offseg,vif_dir.t" hile (( at->		brea
					}
				}
					expec2int32 ehile (( at->		breaphil		}
			1eor>t" 	TIFFReadDirEntryOutSlong8own tagname",1ffseeor>t" 		if	TIFvalue=0; break;
						}
				r ? */(		t&mor>t" (m=es * *afe to		/* check data tyin ti32wr )
           iylfseint32 exlse
    !=:
	 * - Iy* exifFielfseeor>t" 	r (dpm
	 * - IReadDiectoryC	}, dp, TRUE diroff)
UCEDIMAGE	SETGET_C32_FLOAT:error ? */floyt*offseor>t" hile (( at->		brea
					}
				}
					expec2int32 ehile (( at->		breaphil		}
			1eor>t" 	TIFFReadDirEntryOutFloytown tagname",1ffseeor>t" 		if	TIFvalue=0; break;
						}
				r ? */(		t&mor>t" (m=es * *afe to		/* check data tyin ti32wr )
           iylfseint32 exlse
    !=:
	 * - Iy* exifFielfseeor>t" 	r (dpm
	 * - IReadDiectoryC	}, dp, TRUE diroff)
UCEDIMAGE	SETGET_C32_DOUBLE:error ? */doub  *offseor>t" hile (( at->		brea
					}
				}
					expec2int32 ehile (( at->		breaphil		}
			1eor>t" 	TIFFReadDirEntryOutDoub  own tagname",1ffseeor>t" 		if	TIFvalue=0; break;
						}
				r ? */(		t&mor>t" (m=es * *afe to		/* check data tyin ti32wr )
           iylfseint32 exlse
    !=:
	 * - Iy* exifFielfseeor>t" 	r (dpm
	 * - IReadDiectoryC	}, dp, TRUE diroff)
UCEDIMAGE	SETGET_C32_IFD8:error ? */ipby  *offseor>t" hile (( at->		brea
					}
				}
					expec2int32 ehile (( at->		breaphil		}
			1eor>t" 	TIFFReadDirEntryOutIfd8own tagname",1ffseeor>t" 		if	TIFvalue=0; break;
						}
				r ? */(		t&mor>t" (m=es * *afe to		/* check data tyin ti32wr )
           iylfseint32 exlse
    !=:
	 * - Iy* exifFielfseeor>t" 	r (dpm
	 * - IReadDiectoryC	}, dp, TRUE diroff)
n to read ithile ((0	uininhic/= td->td_nowev getlIG_S*_dir.t diroff) TIFFFi	TI!value=0; break;
						}
	r ? alue=0; break;
	Output					/* 	TI6 dircou
					{
						u,
	  tricff)
ReadDiectorykOrder";
uint1
	static recto"Bo in ofple)-1);amr  *afe s;
	TIW0xFFFsTilerout
			}ays "R_STRI";
		 factesize)also{
				
			perti
statistrip;

   		uiFReadFGetFThincttripbytecount as we will compute ias esize;
		ipby  ** lppcount=TIFFFetchDirectory(tif,diroff,&directoFGetFThincountse tag values */
              fset  *offseor>	TIFFReadDirEntryOutLong8own te+inLimi
		/* comp1ffse,esize;
toryFFFi	TI!value=0; break;
						}
	r ?  custom directory atg,value,value+incrementpdom
      			if  ? alue=0; break;
	Output					/* 	TI6 dircou
		TION)&&(dp->tdir_count==(uint64)countpe0cff)
ReadDiectorykOrr_cof_c
           <has a64pesize;
t
	r ? fset  *ortile;dffseor>t custom directory atg,value,value+incrementpdom
      			ifr>t custoasize;pszMaxg,vgetenv("LIBMAGE	STRILE_ARRAY_MAX_RESIZE_count" "TIFipbytec axFER_STRI		}
000000"TIFanarpszMaxg
	 *  axFER_STRI		}in ti32w atoi(pszMax "TIFalue=0; break;
	Output					/* FReadDirEntryOutputC    	rsa)
										r.td_photo)
										UNCTION)&&(dp->tdir_count==(uint64)countpersa)
										( esize;
 <=c axFER_STRI	)IFF_TIFanarER_STRI	>c axFER_STRI	)
		r ? *y* exifFielfseeor>t"ReadDiectoryChe_plrtile;dffse	has a64r)stimant64 space;
		uiesize;
	image has afilee = T;
	} el		if (!( 	r (drtile;dffse	=(tif->tiy* exifFielfseeor>t"ReadDiectoryChe     tif->tif_diir.td_stripbrtile;dffse,int16he arbit _c
           *image has afileor     tif->tif_diir.td_ste ubrtile;dffse+he arbit _c
           ,0,(esize;
-he arbit _c
           )*image has afileortiy* exifFielfseeor>tffse	rtile;dffseor>kOr*lppfffseor>der";
uint1
	static recto"Bndo in e amstom IFD, so we= _TIF4)c
statistrip;

   		uiFReadFtom IFD, so we tripbytecount as we will comcount=TIFFFetchDirectory(tif,diroff,&directoFtom IFD, so weountse tag values */
              UI afiAligned_t&mor    m.l=e
		hile ((image hdoub  )==8)
		hile ((image has afil==8)
		hile ((image has abit==4toryFFFi _c
           !=1
	 *	TIFFReadDirEntryOutputC       FFScanlindom
      	;
					}
	RATIONAL
	 *	TIFFReadDirEntryOutput(uin   FFSc (1)
	{
		/* we should perhaps return in err{    ipbytecle)-1)or>t"le)-1)=*ne size  (&dom
      TIFFmemor>t"     (!TIFFSetile &i++, SWA))
			tes * wabLong(&TIFFmemor>t"	TIFFReadDirEntryOutDfse
		uiTIFFme,8,m.itoryChe;
			typedp->tm.l=dom
      TIFFme.if, T The8or>t"	TIFFReadDirEntryOutEutOoff)onst TIFFFi	TIFvalue=0; break;
						}
	dp->doub   n!( 	r (d (!TIFFSetile &i++, SWA))
			es * wabown tOfLong(m.i,2(!( 	r (dm.i[0]fk:
	 * n=0.0"TIFFFScanlinm.i[0]fk:									/* chsume  tha
	if Numerator :									 me")lepbytecoory(tiinchaiteme  thad, so we. IndiIFFFepbytec thea_nogIFF(tifloytts\  oi"fme  thastom IFD, so we=(tif- me  th/	 * n=-1.0"TIFFFSc	 * n=hdoub  )m.i[0]/hdoub  )m.i[1]ff)
ReadDiees * *afe to		/* com
      			,n)torykOrFFSc (1)
	alue=0; break;
	Output					/* 	TI6 dircou"Ftom IFD, so weo, tif, tofReadDiectorykO
	static co accorBo  bytp];
	} e(pert)	else
;
		elseFetdfsegby m reiptp];
	} stic (perts), eato"Bpprox= filly mo			_SIZE_DEFAULTecount
	}
	/*
	 
		ful	
		tic assuts\ if (dcargtiies *;amr  = Tassuts\ if (dmato
		sec thea_limi
edtic ypewidt    }

statistrip;
	if (ChopUpS bytpU
;
		elseFeFGetF tripbyteccountreg, sev nt as wcount)
		_TIFFfree(td->td_stripby rowspec       ipby role)-1)or>ipbytecthe ce;k   ipby rothe ce;kcount   ipby ro(tif->tifs;tripbytecount);
	te ias esize;
or>ipbytecthe* n directorfset  *onewc    storfset  *onewle)-1);F_TIsion != COMPlse if (tif->tif_dir.t0]or     tifhicOnea_nowly n(0rsadxip =, jpbytre-open	dWe cbexip RE),iwetrip = 0; sthicdoountwanrim thischopWe c thgckedas }
,we'votif, o       issf->tif_ = 0; sthicif (r	( FGetFOe)-1);aBndoFGetFileSize(tiold roperly ip RE)) .d->td_nstrips, sIFF))
			spac0)s case
			 *me,
 != O_RDONLYzeof(TIFFHeaderClassitorFFa in =} else {
		uint64 ro0;
		hile ((es the case were the offset to
		 * o
	TIGtoryFFFi(es the chotomeXXX:e offHOTOMEo		C_YCBCR)&& naderdpisUpSce wh4(tif);/* cthe ce;k =} else {ycbcrsubace wtif[1]ff)FFSc	 *the ce;k =}+=dathe ce;kcountg,valueV_imagelength/t,othe ce;ktorysumeic nels t amthe* hold yte)eber  rowsrectory,*nnelip Rd dirofitte- td->tdstripoffset[f  ossib  .
 th/	 r (drhe ce;kcountg> mo			_SIZE_DEFAULTtif->t(tif->tifsed the ce;kcount   	ssing S mples >the ce;k    IFFScanlinrhe ce;kcountg> 0 ot hanESSItecthe ce;k* n director	the ce;k* n direc		}in ti32w (mo			_SIZE_DEFAULTe/ the ce;kcount)   	ssing S mples >the ce;kng S mples*>the ce;k   t(tif->tifsed the ce;kng S mples*>the ce;kcount   kOrFFSc (aderClassitorysumeic nowev inn(0rss t amErectory
	the* g Srip--;
	th/	 r (drheng S mples>=} else {rheng S mple)tofReadDior     tifER_STRI		}aluehowmany_32( color tes * *afe ,cthe* n direc)
	ace;;;;;anarER_STRI		d_st	df(TIFFHeaderClassito_ = 0; sthicIfiwetare'votif, o        orBolon ofp    }
,dmals surFepbytet amif_ = 0; sthicip = 
	 *s bthe*s needSTd->td_nstrips, sase
			 *me,
 == O_RDONLYz&& f(TIFFHeaderER_STRI	>c
000000z&& f(TIFFHeader(FFa in >	}alueG*afemagele(tif) ||r     tif->tif(tif->tifse> i)
	{G*afemagele(tif) -ole)-1))e/ (esize;
)sio))CONFIG_SEPA           tifReadDior     tifhe_pnewc    sOMPRESSION_NEstimant64 space;
		uinesize;
		image ine sifilircou	S = TchoppTIFFSFGetFileSize(ti\"el		if (!( newle)-1);OMPRESSION_NEstimant64 space;
		uinesize;
		image ine sifilircou	S = TchoppTIFFSFGetFOe)-1);\"el		if (!( r (dnewc    sOMace=siz|| newle)-1);OM      )t hansume thaUnab    o        ornewim this nfXIFD * -, gF(tiu else u	typed*et amlriginal  rows this nfXIFD * -ILEip of data.E wc    sO!ace=sitr	cty* exifFieE wc    s(!( 	r (dnewle)-1);O!ace=sitr	cty* exifFieE wle)-1);, tofReadDi   kOrsumeic Fp Rdrip];
	} e nfXIFD * -el		ifsec thenewiIFF))
			;aBndole)-1);
ed*et aytreflrr==rip]broken-u efXIFD .
 th/	 
       =		if (Estim"BfiER_STRILE_LOAD)
	
		uir (d(tif->tifse> IFF))
			tr	ct(tif->tifsed wspec       pnewc    std_pmated (tif->tifs;tr newle)-1);td_pmated (tif->tifs ?tFFa in :ke lifFFa in +d (tif->tifs;tr IFF))
			s-d (tif->tifs;trkOrsumeic co accorold   bytp];
	} e nfX if (dm rei-;
	} e nfX.
 th/	 		b ront[strip] = byU =} else {ER_STRI		}esize;
or>es * *afe to		/* }alueTAGssing S mo			,cthe* n direc)
	
 L )
      lse if (tif->tif_dir., toL )
      lse if (tif-TIFFmemor>lse if (tif->tif_dir.LpTE &c    storlse if (tif-TIFFmeLpTE &le)-1);F_>lse if (tif->tif_dir.sThied =}+=dhe_		t&_alue,vllFGetrts(
o	Tnt*	/* coun tifReadDi&_alue,vllFGetrtsI"fornal(ytecou1 )VALUE))
	{
			t&_alue,vllFGetrtsI"fornal(yo	Tnt*	/*;
		t&loydFGetFileSize(t coun#/* r_chah4(DEFER	STRILE_LOADONFIG_SEPAreg, sev nt as wcount)
		_TIFFfree(td->td_std_nstrips	t&
	adDi_F_STRI=}+=dtd_nstrips, sase if (tif-TIFFmeL!ace=siCONFIG_SEPARATE)
		;
	exifFietd_nstrips, sase if (tif-TIFFme_     .          g,d_st	df(TIFFHeader)
		;
	exif0ietd_nstrips,nels = FReadFGetFThinct	/*;& lse if (tif-TIFFme_     )f(uint16));
                        else {ER_STRI,&lse if (tif-TIFFmemONFIG_SEPA           tif)
		;
	exi_F_STRI=}Fied_name :}
td_nstrips,neloydFGetFileSize(t && f(TIFFHeaderls = FReadFGetFThinct	/*;& lse if (tif->tif_dir._     )f(uint16));
                        else {ER_STRI,&lse if (tif->tif_dir.,ONFIG_SEPA           tif)
		;
	exi_F_STRI=}Fied_name :}
td_nstripir.td_ste ub & lse if (tif-TIFFme_     )f 0, image hnt as we willeor     tifir.td_ste ub & lse if (tif->tif_dir._     )f 0, image hnt as we willeorel   
	/*
	 * An o.e {ER_STRI	>c
)s c;
	exi_F_STRI==u1 )t hanESSItecdirecto
;
	/*
	 * An o.if (tif->tif_dir.sThied =}+=d	 
       =		if1(Estim"Bfi	/*
	 * An o.e {ER_STRILE_LOAD)
	
		uil   
	/*
	 * An o.e {
		uint64 rostim"Bsion >me  t  i	/*
	 * An o.e {
		uint64 rostim"]tif->tif	/*
	 * An o.if (tif->tif_dir.sThied =}e life break;
		}
	const 

 tif)
		;
	exic;
	exi_F_STR;
#FFScahic!r_chah4(DEFER	STRILE_LOADOd->td_nstrip(	if iftse;td_nstrip(	if ifloydFGetFileSize(t;
 tif)
		;
	exic1;
#Fnd   ALUE
hicvim:o in es=8E_Ls=8E_w=8Enoet:d->ttatic L   l Vfriab  s:tic me,
: cay =c-basic-nt64 r: 8ay =ip R-column: 78ay =End:stati