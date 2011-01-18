#ifndef _PGREPLAY_H
#define _PGREPLAY_H 1

#include "config.h"

#if defined(WIN32) || defined(WIN64)
#	ifndef WINDOWS
#		define WINDOWS
#	endif
#endif

#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>

/* types for replay items */
typedef enum {
	pg_connect = 0,
	pg_disconnect,
	pg_execute,
	pg_prepare,
	pg_exec_prepared,
	pg_cancel
} replay_type;

/* one "command" parsed from a log file to be replayed
   the definition is in replay_item.c */
typedef struct replay_item replay_item;

typedef int (replay_item_provider_init)(const char *, int, const char *, const char *);
typedef replay_item *(replay_item_provider)();
typedef void (replay_item_provider_finish)();

typedef int (replay_item_consumer_init)(const char *, const char *, int, const char *, double);
typedef int (replay_item_consumer)(replay_item *);
typedef void (replay_item_consumer_finish)();

/* hash value for session ID is computed as low byte of background PID */
#define hash_session(x) (unsigned char)(x & 0xFF);

/* printf/scanf formats for various data types */
#if SIZEOF_UNSIGNED_INT == 4
#	define UINT32_FORMAT "%x"
#else
#	define UINT32_FORMAT "%hx"
#endif

#ifdef WINDOWS
#	define UINT64_FORMAT "%I64x"
#else
#	if SIZEOF_UNSIGNED_LONG == 8
#		define UINT64_FORMAT "%lx"
#	else
#		define UINT64_FORMAT "%llx"
#	endif
#endif

/*********************/
/* defined in main.c */
/*********************/

extern int debug_level;

/* print debug messages */
#define debug(level, format, ...) { \
	if (level <= debug_level) { \
		fprintf (stderr, format, __VA_ARGS__); \
		fflush(stderr); \
	} \
}

/***************************/
/* defined in replayitem.c */
/***************************/

/* functions to create replay items */
extern replay_item *replay_create_connect(const struct timeval *time, uint64_t session_id, const char *user, const char *database);
extern replay_item *replay_create_disconnect(const struct timeval *time, uint64_t session_id);
extern replay_item *replay_create_execute(const struct timeval *time, uint64_t session_id, const char *statement);
extern replay_item *replay_create_prepare(const struct timeval *time, uint64_t session_id, const char *statement, const char *name);
extern replay_item *replay_create_exec_prepared(const struct timeval *time, uint64_t session_id, const char *name, uint16_t count, char * const *values);
extern replay_item *replay_create_cancel(const struct timeval *time, uint64_t session_id);

/* free mamory of a replay_item */
extern void replay_free(replay_item *r);

/* get attributes of a replay item */
extern replay_type replay_get_type(const replay_item *r);
extern uint64_t replay_get_session_id(const replay_item *r);
extern const struct timeval * replay_get_time(const replay_item *r);
extern const char * replay_get_statement(const replay_item *r);
extern const char * replay_get_name(const replay_item *r);
extern const char * replay_get_user(const replay_item *r);
extern const char * replay_get_database(const replay_item *r);
extern int replay_get_valuecount(const replay_item *r);
extern const char * const * replay_get_values(const replay_item *r);

/* dump a replay item at debug level 3 */
extern void replay_print_debug(const replay_item *r);

/* special replay_item that signals end-of-file */
extern replay_item * const end_item;

/**********************/
/* defined in parse.c */
/**********************/

/* set to 0 if backslashes in simple string literals do not escape the
   following single quote (e.g. if standard_conforming_strings=on) */
#define BACKSLASH_QUOTE 1

/* parse a timestamp (excluding time zone) */
extern const char * parse_time(const char *, struct timeval *);

extern replay_item_provider parse_provider;
extern replay_item_provider_init parse_provider_init;
extern replay_item_provider_finish parse_provider_finish;

/***************************/
/* defined in replayfile.c */
/***************************/

extern replay_item_provider file_provider;
extern replay_item_provider_init file_provider_init;
extern replay_item_provider_finish file_provider_finish;

extern replay_item_consumer file_consumer;
extern replay_item_consumer_init file_consumer_init;
extern replay_item_consumer_finish file_consumer_finish;

/*************************/
/* defined in database.c */
/*************************/

extern replay_item_consumer database_consumer;
extern replay_item_consumer_init database_consumer_init;
extern replay_item_consumer_finish database_consumer_finish;

#ifdef WINDOWS
/************************/
/* defined in windows.c */
/************************/

extern void win_perror(const char *prefix, int is_network_error);
#endif

#endif
