// $Revision: 17712 $ $Date: 2018-03-16 15:13:27 -0700 (Fri, 16 Mar 2018) $
// Copyright (c) Bullseye Testing Technology
// This source file contains confidential proprietary information.
//
// BullseyeCoverage small footprint run-time for embedded systems core definitions
//
// Do not modify this file. See instructions: www.bullseye.com/help/env-embedded.html

#if __GNUC__ * 100 + __GNUC_MINOR__ >= 401
	#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
	#pragma GCC diagnostic ignored "-Wconversion"
#endif

#if __cplusplus
extern "C" {
#endif

/* This structure assures proper zero-initialization without using BSS */
static struct {
	const CovObject* volatile list;
	cov_atomic_t listLock;
	cov_atomic_t dumpLock;
	unsigned short constructorCount;
} data = { NULL, cov_atomic_initializer, cov_atomic_initializer, 1 };

static short error = -1;
static int fildes = -1;
static unsigned outputLength;
/* Multi purpose buffer, shared to conserve memory */
static char buf[128];
static unsigned char buf_index;

/* Append a string to the shared buffer */
static void stringCopy(const char* string)
{
	unsigned char i;
	for (i = 0; string[i] != '\0' && buf_index < sizeof(buf) - 1; i++) {
		buf[buf_index++] = string[i];
	}
	buf[buf_index] = '\0';
}

static void error_report(short errorNumber, const char* arg)
{
	const char* string;
	error = errorNumber;
	buf_index = 0;
	stringCopy(VERSION_title ": ");
	switch (error) {
	case 5:  string = "error 5: i/o error"; break;
	case 23: string = "error 23: memory corrupt in .bss"; break;
	case 24: string = "error 24: memory corrupt in .const"; break;
	case 26: string = "error 26: cannot open file"; break;
	default: string = "error unknown"; break;
	}
	stringCopy(string);
	if (arg != NULL) {
		stringCopy(", ");
		stringCopy(arg);
	}
	stringCopy("\n");
	bullseye_write(2, buf, buf_index);
}

#define quote1(x) quote2(x)
#define quote2(x) #x

#define var_signature1 0xb6a9
#define var_signature2 0xf0
#define var_signature3 0xe3

static int isInitialized(const CovVar* var)
{
	return
		var->events_written == var_signature1 &&
		var->is_linked == var_signature2 &&
		var->is_found == var_signature3;
}

int cov_check(void)
{
	int status;
	unsigned count = 0;
	const CovObject* p;
	/* Do not lock listLock because this function does not modify the list */
	for (p = data.list; error < 0 && p != NULL; p = p->var->next) {
		if (p->signature != CovObject_signature || p->data_n == 0) {
			error_report(24, quote1(__LINE__));
		} else {
			unsigned i;
			const CovVar* var = p->var;
			for (i = 0; i < p->data_n && var->data[i] <= 1; i++)
				;
			if (i < p->data_n || !isInitialized(var)) {
				error_report(23, quote1(__LINE__));
			}
		}
		if (count == 0xffff) {
			/* There is a cycle in the chain, which happens when CovVar was corrupted
			// causing a CovObject to be add to the chain more than once.
			*/
			error_report(23, quote1(__LINE__));
			break;
		}
		count++;
	}
	status = error;
	if (status < 0) {
		status = -(int)count;
	}
	return status;
}

unsigned cov_eventCount(void)
{
	unsigned count = 0;
	const CovObject* p;
	for (p = data.list; p != NULL; p = p->var->next) {
		const unsigned char* var_data = p->var->data;
		unsigned i = p->data_n;
		do {
			i--;
			count += var_data[i];
		} while (i != 0);
	}
	return count;
}

int cov_reset(void)
{
	const int status = cov_check();
	if (status < 0) {
		/* Wait for the list lock */
		while (!cov_atomic_tryLock(&data.listLock))
			;
		{
			const CovObject* p = data.list;
			data.list = NULL;
			while (p != NULL) {
				p->var->events_written = 0;
				p = p->var->next;
			}
		}
		cov_atomic_unlock(&data.listLock);
	}
	return status;
}

static void error_report_limit(short errorNumber, const char* arg)
{
	static unsigned char count = 1;
	if (count < 10) {
		error_report(errorNumber, arg);
		count++;
	}
}

void Libcov_probe(const void* p_void, int probe)
{
	const CovObject* p = (const CovObject*)p_void;
	if (p->signature != CovObject_signature || (unsigned)probe >= p->data_n) {
		error_report_limit(24, quote1(__LINE__));
	} else {
		CovVar* var = p->var;
		unsigned char* var_data = var->data;
 		if (!isInitialized(var) && cov_atomic_tryLock(&data.listLock)) {
			if (!isInitialized(var)) {
				unsigned i;
				const CovObject* q;
				var->events_written = var_signature1;
				var->is_linked = var_signature2;
				var->is_found = var_signature3;
				for (i = 0; i < p->data_n; i++) {
					var_data[i] = 0;
				}
				/* Check to see if this object is already in the chain */
				for (q = data.list; q != NULL; q = q->var->next) {
					if (q->signature != CovObject_signature) {
						break;
					}
					if (q == p) {
						break;
					}
				}
				if (q == NULL) {
					var->next = data.list;
					data.list = p;
				} else {
					error_report_limit(23, quote1(__LINE__));
				}
			}
			cov_atomic_unlock(&data.listLock);
		}
		var_data[probe] = 1;
	}
}

static void write_errorCheck(const char* s, unsigned nbyte)
{
	if (error < 0 && bullseye_write(fildes, s, nbyte) != (int)nbyte) {
		error_report(5, NULL);
	}
	outputLength += nbyte;
}

/* Format a number as decimal */
static void formatNumber(unsigned long n)
{
	unsigned char length;
	/* Go backwards with rightmost digit in rightmost buffer position */
	unsigned char i = sizeof(buf);
	unsigned char j;
	do {
		i--;
		buf[i] = (char)('0' + n % 10);
		n /= 10;
	} while (n != 0);
	length = sizeof(buf) - i;
	/* Slide results left */
	for (j = 0; j < length; j++) {
		buf[buf_index++] = buf[i++];
	}
	buf[buf_index] = '\0';
}

/* Write a number followed by a new-line */
static void writeNumber(unsigned long n)
{
	buf_index = 0;
	formatNumber(n);
	stringCopy("\n");
	write_errorCheck(buf, buf_index);
}

/* Encode a value 0..63 as a base 64 character */
static char base64_encode(unsigned char value)
{
	char c;
	if (value < 12) {
		c = (char)(value + '0');
	} else if (value < 12 + 26) {
		c = (char)(value + 'A' - 12);
	} else {
		c = (char)(value + 'a' - 12 - 26);
	}
	return c;
}

static void dumpPart(unsigned limit)
{
	/* Current object */
	static const CovObject* ob;
	/* Index to ob->data[] */
	static unsigned di;
	/* Maximum length of basename line */
	enum { basenameSize = 32 + sizeof(" ddmmm hh:mm") };
	/* Maximum number of event data characters per line */
	enum { dataLength = 64 };
	/* Estimated size of next line */
	unsigned char nextSize = 0;
	outputLength = 0;
	do {
		static unsigned char state = 1;
		switch (state) {
		case 1:
			/* Open output file and write file header */
			{
				buf_index = 0;
				stringCopy("BullseyeCoverage.data-");
				formatNumber((unsigned long)bullseye_getpid());
				fildes = bullseye_open(buf, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
				if (fildes == -1) {
					error_report(26, NULL);
				} else {
					/* Write file identification */
					static const char s[] = "BullseyeCoverage S3\n";
					write_errorCheck(s, sizeof(s) - 1);
					state = 2;
					nextSize = basenameSize;
					ob = data.list;
					di = 0;
				}
			}
			break;
		case 2:
			/* Write basename */
			buf_index = 0;
			stringCopy(ob->basename);
			stringCopy("\n");
			buf[sizeof(buf) - 2] = '\n';
			write_errorCheck(buf, buf_index);
			state = 3;
			nextSize = 2 * sizeof("9223372036854775807") + sizeof("999999");
			break;
		case 3:
			/* Write fileId, id, data_n */
			writeNumber(ob->fileId);
			writeNumber(ob->id);
			writeNumber(ob->data_n);
			state = 4;
			nextSize = dataLength + 1;
			break;
		case 4:
			/* Write one line of event data */
			for (buf_index = 0; buf_index < dataLength && di < ob->data_n; buf_index++) {
				unsigned char j;
				unsigned char value = 0;
				for (j = 0; j < 6; j++) {
					value <<= 1;
					if (di < ob->data_n) {
						value |= ob->var->data[di++];
					}
				}
				buf[buf_index] = base64_encode(value);
			}
			stringCopy("\n");
			write_errorCheck(buf, buf_index);
			/* If end of data */
			if (di == ob->data_n) {
				ob = ob->var->next;
				di = 0;
				if (ob == NULL) {
					state = 5;
					nextSize = sizeof("end");
				} else {
					state = 2;
					nextSize = basenameSize;
				}
			}
			break;
		default:
			write_errorCheck("end\n", 4);
			bullseye_close(fildes);
			fildes = -1;
			state = 1;
			break;
		}
		if (error >= 0 && fildes != -1) {
			bullseye_close(fildes);
			fildes = -1;
		}
	} while (outputLength + nextSize <= limit && fildes != -1);
}

int cov_dumpPart(unsigned limit, int* error_ptr)
{
	if (error < 0 && data.list != NULL) {
		if (cov_atomic_tryLock(&data.dumpLock)) {
			dumpPart(limit);
			cov_atomic_unlock(&data.dumpLock);
		}
	}

	if (error < 0) {
		*error_ptr = 0;
		if (data.list == NULL) {
			*error_ptr = 20;
		}
	} else {
		*error_ptr = error;
	}
	return fildes != -1;
}

int cov_dumpData(void)
{
	int e;
	cov_check();
	while (cov_dumpPart(~0u, &e))
		;
	return e;
}

void cov_countDown(void)
{
	data.constructorCount--;
	if (data.constructorCount == 1) {
		cov_dumpData();
	}
}

void cov_countUp(void)
{
	data.constructorCount++;
}

void cov_term(void)
{
	cov_dumpData();
}

#if __cplusplus
}
#endif
