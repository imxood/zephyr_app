/* $Revision: 17674 $ $Date: 2018-03-09 17:23:42 -0800 (Fri, 09 Mar 2018) $
 * Copyright (c) Bullseye Testing Technology
 * This source file contains confidential proprietary information.
 */

#if defined(__GNUC__) && __GNUC__ >= 4
	#pragma GCC diagnostic ignored "-Wredundant-decls"
#endif

/* Run-time library probe interface version */
#define Libcov_probe cov_probe_v12
#define Libcov_version 808014u	/* 8.8.14 */

#if __cplusplus
extern "C" {
#endif

/* Data for one translation unit */
/*   Variable data (bss section) */
typedef struct {
	const struct CovObject_struct* next;
	unsigned events_written;
	unsigned char is_linked;
	unsigned char is_found;
	unsigned char data[1];
} CovVar;
/*   Constant data (const section)*/
#if __CYGWIN__ || __MINGW32__
	typedef unsigned cov_uint32;
#else
	typedef unsigned long cov_uint32;
#endif
typedef struct CovObject_struct {
	cov_uint32 signature;
	cov_uint32 id;
	CovVar* var;
	cov_uint32 fileId;
	unsigned data_n;
	char basename[1];
} CovObject;
#define CovObject_signature 0x5a6b7c8dL

/* Interface to covgetkernel */
typedef struct {
	/* Libcov_version */
	unsigned version;
	unsigned arch;
	const CovObject* volatile* array;
} CovKernel;

#if defined(_WIN32)
	#define Libcov_cdecl __cdecl
#else
	#define Libcov_cdecl
#endif

/* Might already be defined for DLL export */
#if !defined(Libcov_export)
	#define Libcov_export
#endif

Libcov_export int Libcov_cdecl cov_check(void);
void cov_dumpFile(void);
Libcov_export unsigned Libcov_cdecl cov_eventCount(void);
Libcov_export int Libcov_cdecl cov_file(const char* path);
Libcov_export int Libcov_cdecl cov_reset(void);
void Libcov_cdecl cov_term(void);
Libcov_export int Libcov_cdecl cov_write(void);

#if __cplusplus
}
#endif
