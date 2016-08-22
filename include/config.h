#pragma once
static const char PROGNAME[]    = "hashcat";
static const char VERSION_TAG[] = "HEAD -> fix-windows-build, origin/fix-windows-build v3.10-9-g23118f3";
static const unsigned int VERSION_BIN = 310;
#define COMPTIME 1471823896
#define WITH_HWMON
#define _POSIX

enum _HASHCAT_CONFIG{
  ETC_MAX             = 60*60*24*365*10,
  DEVICES_MAX         = 128,
  CL_PLATFORMS_MAX    = 16,
  BLOCK_SIZE          = 64,
  CHARSIZ             = 0x100,

  EXEC_CACHE          = 128,
  SPEED_CACHE         = 128,
  
  SPEED_MAXAGE        = 4096,
  
  HCBUFSIZ            = 0x50000,
  EXPECTED_ITERATIONS = 0x50000,
  CPT_BUF             = 0x20000,
  MAX_CUT_TRIES       = 4,
  MAX_DICTSTAT        = 10000,
  VERIFIER_CNT        = 1,
  STEPS_CNT           = 10,
};

static const char LOOPBACK_FILE[]     = "hashcat.loopback";
static const char DICTSTAT_FILENAME[] = "hashcat.dictstat";
static const char POTFILE_FILENAME[]  = "hashcat.pot";
static const char SP_HCSTAT[]         = "hashcat.hcstat";
static const char TUNING_DB_FILE[]    = "hashcat.hctune";

static const char INDUCT_DIR[]        = "induct";
static const char OUTFILES_DIR[]      = "outfiles";
static const char SESSIONS_FOLDER[]   = "sessions";
static const char DOT_HASHCAT[]       = ".hashcat";

#define LINUX
#ifdef LINUX
static const char INSTALL_FOLDER[]    = "/usr/local/bin";
static const char SHARED_FOLDER[]     = "/usr/local/share/";
static const char DOCUMENT_FOLDER[]   = "/usr/local/share/doc/";
#endif
