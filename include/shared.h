/**
 * Authors.....: Jens Steube <jens.steube@gmail.com>
 *               Gabriele Gristina <matrix@hashcat.net>
 *               magnum <john.magnum@hushmail.com>
 *
 * License.....: MIT
 */

#ifndef SHARED_H
#define SHARED_H

#include "common.h"
#include "inc_hash_constants.h"

 /**
  * thread management
  */

#ifdef _WIN
#define hc_timer_get(a,r)           { hc_timer_t hr_freq; QueryPerformanceFrequency (&hr_freq); hc_timer_t hr_tmp; hc_timer_set (&hr_tmp); (r) = (double) ((double) (hr_tmp.QuadPart - (a).QuadPart) / (double) (hr_freq.QuadPart / 1000)); }
#define hc_timer_set(a)             { QueryPerformanceCounter ((a)); }
#elif _POSIX
#define hc_timer_get(a,r)           { hc_timer_t hr_tmp; hc_timer_set (&hr_tmp); (r) = (double) (((hr_tmp.tv_sec - (a).tv_sec) * 1000) + ((double) (hr_tmp.tv_usec - (a).tv_usec) / 1000)); }
#define hc_timer_set(a)             { gettimeofday ((a), NULL); }
#endif

#ifdef _WIN
#define hc_thread_create(t,f,a)     t = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) &f, a, 0, NULL)
#define hc_thread_wait(n,a)         for (uint i = 0; i < n; i++) WaitForSingleObject ((a)[i], INFINITE)
#define hc_thread_exit(t)           ExitThread (t)

#define hc_thread_mutex_lock(m)     EnterCriticalSection      (&m)
#define hc_thread_mutex_unlock(m)   LeaveCriticalSection      (&m)
#define hc_thread_mutex_init(m)     InitializeCriticalSection (&m)
#define hc_thread_mutex_delete(m)   DeleteCriticalSection     (&m)

#elif _POSIX

#define hc_thread_create(t,f,a)     pthread_create (&t, NULL, f, a)
#define hc_thread_wait(n,a)         for (uint i = 0; i < n; i++) pthread_join ((a)[i], NULL)
#define hc_thread_exit(t)           pthread_exit (&t)

#define hc_thread_mutex_lock(m)     pthread_mutex_lock     (&m)
#define hc_thread_mutex_unlock(m)   pthread_mutex_unlock   (&m)
#define hc_thread_mutex_init(m)     pthread_mutex_init     (&m, NULL)
#define hc_thread_mutex_delete(m)   pthread_mutex_destroy  (&m)
#endif

#ifdef __APPLE__
typedef struct cpu_set
{
  uint32_t count;

} cpu_set_t;

static inline void CPU_ZERO(cpu_set_t *cs) { cs->count = 0; }
static inline void CPU_SET(int num, cpu_set_t *cs) { cs->count |= (1 << num); }
static inline int  CPU_ISSET(int num, cpu_set_t *cs) { return (cs->count & (1 << num)); }
#endif

/**
 * libraries stuff
 */

#ifdef _WIN
#define hc_dlopen LoadLibrary
#define hc_dlclose FreeLibrary
#define hc_dlsym GetProcAddress
#else
#define hc_dlopen dlopen
#define hc_dlclose dlclose
#define hc_dlsym dlsym
#endif

#define HC_LOAD_FUNC2(ptr,name,type,var,libname,noerr) \
  ptr->name = (type) hc_dlsym (ptr->var, #name); \
  if (noerr != -1) { \
    if (!ptr->name) { \
      if (noerr == 1) { \
        log_error ("ERROR: %s is missing from %s shared library.", #name, #libname); \
        exit (-1); \
      } else { \
        log_info ("WARNING: %s is missing from %s shared library.", #name, #libname); \
        return -1; \
      } \
    } \
  }

#define HC_LOAD_FUNC(ptr,name,type,libname,noerr) \
  ptr->name = (type) hc_dlsym (ptr->lib, #name); \
  if (noerr != -1) { \
    if (!ptr->name) { \
      if (noerr == 1) { \
        log_error ("ERROR: %s is missing from %s shared library.", #name, #libname); \
        exit (-1); \
      } else { \
        log_info ("WARNING: %s is missing from %s shared library.", #name, #libname); \
        return -1; \
      } \
    } \
  }

#define HC_LOAD_ADDR(ptr,name,type,func,addr,libname,noerr) \
  ptr->name = (type) (*ptr->func) (addr); \
  if (!ptr->name) { \
    if (noerr == 1) { \
      log_error ("ERROR: %s at address %08x is missing from %s shared library.", #name, addr, #libname); \
      exit (-1); \
    } else { \
      log_error ("WARNING: %s at address %08x is missing from %s shared library.", #name, addr, #libname); \
      return -1; \
    } \
  }

 /**
  * system stuff
  */

#ifdef _WIN
#define hc_sleep(x) Sleep ((x) * 1000);
#elif _POSIX
#define hc_sleep(x) sleep ((x));
#endif

#include "ext_OpenCL.h"

  /**
   * temperature management
   */

#include "hwmon/ext_ADL.h"
#include "hwmon/ext_nvapi.h"
#include "hwmon/ext_nvml.h"
#include "hwmon/ext_xnvctrl.h"

   /**
    * shared stuff
    */

#define ETC_MAX                 (60 * 60 * 24 * 365 * 10)

#define DEVICES_MAX             128

#define CL_PLATFORMS_MAX        16

#define CL_VENDOR_AMD           "Advanced Micro Devices, Inc."
#define CL_VENDOR_AMD_USE_INTEL "GenuineIntel"
#define CL_VENDOR_APPLE         "Apple"
#define CL_VENDOR_INTEL_BEIGNET "Intel"
#define CL_VENDOR_INTEL_SDK     "Intel(R) Corporation"
#define CL_VENDOR_MESA          "Mesa"
#define CL_VENDOR_NV            "NVIDIA Corporation"
#define CL_VENDOR_POCL          "The pocl project"

#define VENDOR_ID_AMD           (1u << 0)
#define VENDOR_ID_APPLE         (1u << 1)
#define VENDOR_ID_INTEL_BEIGNET (1u << 2)
#define VENDOR_ID_INTEL_SDK     (1u << 3)
#define VENDOR_ID_MESA          (1u << 4)
#define VENDOR_ID_NV            (1u << 5)
#define VENDOR_ID_POCL          (1u << 6)
#define VENDOR_ID_AMD_USE_INTEL (1u << 7)
#define VENDOR_ID_GENERIC       (1u << 31)

#define BLOCK_SIZE              64

#define CHARSIZ                 0x100
#define INFOSZ                  CHARSIZ

#define SP_HCSTAT               "hashcat.hcstat"
#define SP_PW_MIN               2
#define SP_PW_MAX               64
#define SP_ROOT_CNT             (SP_PW_MAX * CHARSIZ)
#define SP_MARKOV_CNT           (SP_PW_MAX * CHARSIZ * CHARSIZ)

#define TUNING_DB_FILE          "hashcat.hctune"

#define INDUCT_DIR              "induct"
#define OUTFILES_DIR            "outfiles"

#define LOOPBACK_FILE           "hashcat.loopback"

#define DICTSTAT_FILENAME       "hashcat.dictstat"
#define POTFILE_FILENAME        "hashcat.pot"

    /**
     * types
     */

#ifdef _WIN
typedef LARGE_INTEGER     hc_timer_t;
typedef HANDLE            hc_thread_t;
typedef CRITICAL_SECTION  hc_thread_mutex_t;
#elif _POSIX
typedef struct timeval    hc_timer_t;
typedef pthread_t         hc_thread_t;
typedef pthread_mutex_t   hc_thread_mutex_t;
#endif

#include "types.h"
#include "rp_cpu.h"
#include "inc_rp.h"

/**
 * valid project specific global stuff
 */

extern const uint  VERSION_BIN;
extern const uint  RESTORE_MIN;

extern const char *USAGE_MINI[];
extern const char *USAGE_BIG[];

extern const char *PROMPT;

extern int SUPPRESS_OUTPUT;

extern hc_thread_mutex_t mux_display;

#include "consts/hash_names.h"
#include "consts/outfile_formats.h"

/**
 * algo specific
 */

#define DISPLAY_LEN_MIN_0     32
#define DISPLAY_LEN_MAX_0     32
#define DISPLAY_LEN_MIN_10    32 + 1 + 0
#define DISPLAY_LEN_MAX_10    32 + 1 + 51
#define DISPLAY_LEN_MIN_10H   32 + 1 + 0
#define DISPLAY_LEN_MAX_10H   32 + 1 + 102
#define DISPLAY_LEN_MIN_20    32 + 1 + 0
#define DISPLAY_LEN_MAX_20    32 + 1 + 31
#define DISPLAY_LEN_MIN_20H   32 + 1 + 0
#define DISPLAY_LEN_MAX_20H   32 + 1 + 62
#define DISPLAY_LEN_MIN_50    32 + 1 + 0
#define DISPLAY_LEN_MAX_50    32 + 1 + 51
#define DISPLAY_LEN_MIN_50H   32 + 1 + 0
#define DISPLAY_LEN_MAX_50H   32 + 1 + 102
#define DISPLAY_LEN_MIN_100   40
#define DISPLAY_LEN_MAX_100   40
#define DISPLAY_LEN_MIN_110   40 + 1 + 0
#define DISPLAY_LEN_MAX_110   40 + 1 + 51
#define DISPLAY_LEN_MIN_110H  40 + 1 + 0
#define DISPLAY_LEN_MAX_110H  40 + 1 + 102
#define DISPLAY_LEN_MIN_120   40 + 1 + 0
#define DISPLAY_LEN_MAX_120   40 + 1 + 31
#define DISPLAY_LEN_MIN_120H  40 + 1 + 0
#define DISPLAY_LEN_MAX_120H  40 + 1 + 62
#define DISPLAY_LEN_MIN_150   40 + 1 + 0
#define DISPLAY_LEN_MAX_150   40 + 1 + 51
#define DISPLAY_LEN_MIN_150H  40 + 1 + 0
#define DISPLAY_LEN_MAX_150H  40 + 1 + 102
#define DISPLAY_LEN_MIN_200   16
#define DISPLAY_LEN_MAX_200   16
#define DISPLAY_LEN_MIN_300   40
#define DISPLAY_LEN_MAX_300   40
#define DISPLAY_LEN_MIN_400   34
#define DISPLAY_LEN_MAX_400   34
#define DISPLAY_LEN_MIN_500   3 + 1 + 0 + 22
#define DISPLAY_LEN_MIN_501   104
#define DISPLAY_LEN_MAX_500   3 + 1 + 8 + 22
#define DISPLAY_LEN_MAX_501   104
#define DISPLAY_LEN_MIN_900   32
#define DISPLAY_LEN_MAX_900   32
#define DISPLAY_LEN_MIN_910   32 + 1 + 0
#define DISPLAY_LEN_MAX_910   32 + 1 + 51
#define DISPLAY_LEN_MIN_910H  32 + 1 + 0
#define DISPLAY_LEN_MAX_910H  32 + 1 + 102
#define DISPLAY_LEN_MIN_1000  32
#define DISPLAY_LEN_MAX_1000  32
#define DISPLAY_LEN_MIN_1100  32 + 1 + 0
#define DISPLAY_LEN_MAX_1100  32 + 1 + 19
#define DISPLAY_LEN_MIN_1100H 32 + 1 + 0
#define DISPLAY_LEN_MAX_1100H 32 + 1 + 38
#define DISPLAY_LEN_MIN_1400  64
#define DISPLAY_LEN_MAX_1400  64
#define DISPLAY_LEN_MIN_1410  64 + 1 + 0
#define DISPLAY_LEN_MAX_1410  64 + 1 + 51
#define DISPLAY_LEN_MIN_1410H 64 + 1 + 0
#define DISPLAY_LEN_MAX_1410H 64 + 1 + 102
#define DISPLAY_LEN_MIN_1420  64 + 1 + 0
#define DISPLAY_LEN_MAX_1420  64 + 1 + 16
#define DISPLAY_LEN_MIN_1420H 64 + 1 + 0
#define DISPLAY_LEN_MAX_1420H 64 + 1 + 32
#define DISPLAY_LEN_MIN_1421  70
#define DISPLAY_LEN_MAX_1421  70
#define DISPLAY_LEN_MIN_1450  64 + 1 + 0
#define DISPLAY_LEN_MAX_1450  64 + 1 + 51
#define DISPLAY_LEN_MIN_1450H 64 + 1 + 0
#define DISPLAY_LEN_MAX_1450H 64 + 1 + 102
#define DISPLAY_LEN_MIN_1500  13
#define DISPLAY_LEN_MAX_1500  13
#define DISPLAY_LEN_MIN_1600  29 + 0
#define DISPLAY_LEN_MAX_1600  29 + 8
#define DISPLAY_LEN_MIN_1700  128
#define DISPLAY_LEN_MAX_1700  128
#define DISPLAY_LEN_MIN_1710  128 + 1 + 0
#define DISPLAY_LEN_MAX_1710  128 + 1 + 51
#define DISPLAY_LEN_MIN_1710H 128 + 1 + 0
#define DISPLAY_LEN_MAX_1710H 128 + 1 + 102
#define DISPLAY_LEN_MIN_1720  128 + 1 + 0
#define DISPLAY_LEN_MAX_1720  128 + 1 + 16
#define DISPLAY_LEN_MIN_1720H 128 + 1 + 0
#define DISPLAY_LEN_MAX_1720H 128 + 1 + 32
#define DISPLAY_LEN_MIN_1730  128 + 1 + 0
#define DISPLAY_LEN_MAX_1730  128 + 1 + 16
#define DISPLAY_LEN_MIN_1731  128 + 6 + 0
#define DISPLAY_LEN_MAX_1731  128 + 6 + 16
#define DISPLAY_LEN_MIN_1740  128 + 1 + 0
#define DISPLAY_LEN_MAX_1740  128 + 1 + 16
#define DISPLAY_LEN_MIN_1750  128 + 1 + 0
#define DISPLAY_LEN_MAX_1750  128 + 1 + 51
#define DISPLAY_LEN_MIN_1750H 128 + 1 + 0
#define DISPLAY_LEN_MAX_1750H 128 + 1 + 102
#define DISPLAY_LEN_MIN_1800  90 + 0
#define DISPLAY_LEN_MAX_1800  90 + 16
#define DISPLAY_LEN_MIN_2100  6 + 1 + 1 + 32 + 1 + 0
#define DISPLAY_LEN_MAX_2100  6 + 5 + 1 + 32 + 1 + 19
#define DISPLAY_LEN_MIN_2100H 6 + 1 + 1 + 32 + 1 + 0
#define DISPLAY_LEN_MAX_2100H 6 + 5 + 1 + 32 + 1 + 38
#define DISPLAY_LEN_MIN_2400  16
#define DISPLAY_LEN_MAX_2400  16
#define DISPLAY_LEN_MIN_2410  16 + 1 + 0
#define DISPLAY_LEN_MAX_2410  16 + 1 + 16
#define DISPLAY_LEN_MIN_2410H 16 + 1 + 0
#define DISPLAY_LEN_MAX_2410H 16 + 1 + 32
#define DISPLAY_LEN_MIN_2500  64 + 1 + 0
#define DISPLAY_LEN_MAX_2500  64 + 1 + 15
#define DISPLAY_LEN_MIN_2600  32
#define DISPLAY_LEN_MAX_2600  32
#define DISPLAY_LEN_MIN_3000  16
#define DISPLAY_LEN_MAX_3000  16
#define DISPLAY_LEN_MIN_3100  16 + 1 + 0
#define DISPLAY_LEN_MAX_3100  16 + 1 + 30
#define DISPLAY_LEN_MIN_3100H 16 + 1 + 0
#define DISPLAY_LEN_MAX_3100H 16 + 1 + 60
#define DISPLAY_LEN_MIN_3200  60
#define DISPLAY_LEN_MAX_3200  60
#define DISPLAY_LEN_MIN_3711  3 +  0 + 1 + 32
#define DISPLAY_LEN_MAX_3711  3 + 31 + 1 + 32
#define DISPLAY_LEN_MIN_4300  32
#define DISPLAY_LEN_MAX_4300  32
#define DISPLAY_LEN_MIN_4800  32 + 1 + 32 + 1 + 2
#define DISPLAY_LEN_MAX_4800  32 + 1 + 32 + 1 + 2
#define DISPLAY_LEN_MIN_5000  16
#define DISPLAY_LEN_MAX_5000  400
#define DISPLAY_LEN_MIN_5100  16
#define DISPLAY_LEN_MAX_5100  16
#define DISPLAY_LEN_MIN_5300  48
#define DISPLAY_LEN_MAX_5300  1024
#define DISPLAY_LEN_MIN_5400  56
#define DISPLAY_LEN_MAX_5400  1024
#define DISPLAY_LEN_MIN_5500   1 + 1 + 0 +  1 +  1 +  1 +  0 +  1 + 48 + 1 + 16
#define DISPLAY_LEN_MAX_5500  60 + 1 + 0 +  1 + 45 +  1 + 48 +  1 + 48 + 1 + 16
#define DISPLAY_LEN_MIN_5600   1 + 1 + 0 +  1 +  1 +  1 + 16 +  1 + 32 + 1 + 1
#define DISPLAY_LEN_MAX_5600  60 + 1 + 0 +  1 + 45 +  1 + 16 +  1 + 32 + 1 + 1024
#define DISPLAY_LEN_MIN_5700  43
#define DISPLAY_LEN_MAX_5700  43
#define DISPLAY_LEN_MIN_5800  40 + 1 + 1
#define DISPLAY_LEN_MAX_5800  40 + 1 + 16
#define DISPLAY_LEN_MIN_6000  40
#define DISPLAY_LEN_MAX_6000  40
#define DISPLAY_LEN_MIN_6100  128
#define DISPLAY_LEN_MAX_6100  128
#define DISPLAY_LEN_MIN_6300   6 + 1 + 8 + 22
#define DISPLAY_LEN_MAX_6300   6 + 1 + 48 + 22
#define DISPLAY_LEN_MIN_6400   9 + 2 + 1 + 16 + 1 + 43
#define DISPLAY_LEN_MAX_6400   9 + 2 + 1 + 48 + 1 + 43
#define DISPLAY_LEN_MIN_6500   9 + 2 + 1 + 16 + 1 + 86
#define DISPLAY_LEN_MAX_6500   9 + 2 + 1 + 48 + 1 + 86
#define DISPLAY_LEN_MIN_6600   1 + 1 + 16 + 1 + 2080
#define DISPLAY_LEN_MAX_6600   6 + 1 + 16 + 1 + 2080
#define DISPLAY_LEN_MIN_6700   7 + 2 + 1 + 16 + 1 + 27
#define DISPLAY_LEN_MAX_6700   7 + 2 + 1 + 48 + 1 + 27
#define DISPLAY_LEN_MIN_6800  32 + 1 + 1 + 1 + 0
#define DISPLAY_LEN_MAX_6800  32 + 1 + 5 + 1 + 32
#define DISPLAY_LEN_MIN_6900  64
#define DISPLAY_LEN_MAX_6900  64
#define DISPLAY_LEN_MIN_7100   4 + 2 + 1 + 64 + 1 + 128
#define DISPLAY_LEN_MAX_7100   4 + 5 + 1 + 64 + 1 + 128
#define DISPLAY_LEN_MIN_7200  19 + 1 + 1 +   1 + 128
#define DISPLAY_LEN_MAX_7200  19 + 5 + 1 + 224 + 128
#define DISPLAY_LEN_MIN_7300   64 + 1 + 40
#define DISPLAY_LEN_MAX_7300  512 + 1 + 40
#define DISPLAY_LEN_MIN_7400  47 + 0
#define DISPLAY_LEN_MAX_7400  47 + 16
#define DISPLAY_LEN_MIN_7500   1 + 6 + 1 + 2 + 1 +  0 + 1 +  0 + 1 +   0 + 1 + 72 + 32
#define DISPLAY_LEN_MAX_7500   1 + 6 + 1 + 2 + 1 + 64 + 1 + 64 + 1 + 128 + 1 + 72 + 32
#define DISPLAY_LEN_MIN_7700   1 + 1 + 16
#define DISPLAY_LEN_MAX_7700  40 + 1 + 16
#define DISPLAY_LEN_MIN_7800   1 + 1 + 40
#define DISPLAY_LEN_MAX_7800  40 + 1 + 40
#define DISPLAY_LEN_MIN_7900   3 + 1 + 8 + 43
#define DISPLAY_LEN_MAX_7900   3 + 1 + 8 + 43
#define DISPLAY_LEN_MIN_8000   2 + 4 + 16 + 64
#define DISPLAY_LEN_MAX_8000   2 + 4 + 16 + 64
#define DISPLAY_LEN_MIN_8100   1 + 8 + 40
#define DISPLAY_LEN_MAX_8100   1 + 8 + 40
#define DISPLAY_LEN_MIN_8200  64 + 1 + 32 + 1 + 1 + 1 +    1
#define DISPLAY_LEN_MAX_8200  64 + 1 + 32 + 1 + 8 + 1 + 2048
#define DISPLAY_LEN_MIN_8300  32 + 1 +  1 + 1 +  1 + 1 + 1
#define DISPLAY_LEN_MAX_8300  32 + 1 + 32 + 1 + 32 + 1 + 5
#define DISPLAY_LEN_MIN_8400  40 + 1 + 40
#define DISPLAY_LEN_MAX_8400  40 + 1 + 40
#define DISPLAY_LEN_MIN_8500   6 + 1 + 1 + 1 +  1
#define DISPLAY_LEN_MAX_8500   6 + 1 + 8 + 1 + 16
#define DISPLAY_LEN_MIN_8600  32
#define DISPLAY_LEN_MAX_8600  32
#define DISPLAY_LEN_MIN_8700  22
#define DISPLAY_LEN_MAX_8700  22
#define DISPLAY_LEN_MIN_8800  1 + 3 + 1 + 2 + 1 + 32 + 1 + 2 + 1 + 32 + 1 + 3072
#define DISPLAY_LEN_MAX_8800  1 + 3 + 1 + 2 + 1 + 32 + 1 + 2 + 1 + 32 + 1 + 3072
#define DISPLAY_LEN_MIN_8900  6 + 1 + 1 + 1 + 1 + 1 + 1 + 1 +  0 + 1 + 44
#define DISPLAY_LEN_MAX_8900  6 + 1 + 6 + 1 + 2 + 1 + 2 + 1 + 45 + 1 + 44
#define DISPLAY_LEN_MIN_9100  51
#define DISPLAY_LEN_MAX_9100  51
#define DISPLAY_LEN_MIN_9200  3 + 14 + 1 + 43
#define DISPLAY_LEN_MAX_9200  3 + 14 + 1 + 43
#define DISPLAY_LEN_MIN_9300  3 + 14 + 1 + 43
#define DISPLAY_LEN_MAX_9300  3 + 14 + 1 + 43
#define DISPLAY_LEN_MIN_9400  8 + 1 + 4 + 1 + 2 + 1 + 3 + 1 + 2 + 1 + 32 + 1 + 32 + 1 + 40
#define DISPLAY_LEN_MAX_9400  8 + 1 + 4 + 1 + 2 + 1 + 3 + 1 + 2 + 1 + 32 + 1 + 32 + 1 + 40
#define DISPLAY_LEN_MIN_9500  8 + 1 + 4 + 1 + 6 + 1 + 3 + 1 + 2 + 1 + 32 + 1 + 32 + 1 + 64
#define DISPLAY_LEN_MAX_9500  8 + 1 + 4 + 1 + 6 + 1 + 3 + 1 + 2 + 1 + 32 + 1 + 32 + 1 + 64
#define DISPLAY_LEN_MIN_9600  8 + 1 + 4 + 1 + 6 + 1 + 3 + 1 + 2 + 1 + 32 + 1 + 32 + 1 + 64
#define DISPLAY_LEN_MAX_9600  8 + 1 + 4 + 1 + 6 + 1 + 3 + 1 + 2 + 1 + 32 + 1 + 32 + 1 + 64
#define DISPLAY_LEN_MIN_9700  12 + 1 + 32 + 1 + 32 + 1 + 32
#define DISPLAY_LEN_MAX_9700  12 + 1 + 32 + 1 + 32 + 1 + 32
#define DISPLAY_LEN_MIN_9720  12 + 1 + 32 + 1 + 32 + 1 + 32 + 1 + 10
#define DISPLAY_LEN_MAX_9720  12 + 1 + 32 + 1 + 32 + 1 + 32 + 1 + 10
#define DISPLAY_LEN_MIN_9800  12 + 1 + 32 + 1 + 32 + 1 + 40
#define DISPLAY_LEN_MAX_9800  12 + 1 + 32 + 1 + 32 + 1 + 40
#define DISPLAY_LEN_MIN_9820  12 + 1 + 32 + 1 + 32 + 1 + 40 + 1 + 10
#define DISPLAY_LEN_MAX_9820  12 + 1 + 32 + 1 + 32 + 1 + 40 + 1 + 10
#define DISPLAY_LEN_MIN_9900  32
#define DISPLAY_LEN_MAX_9900  32
#define DISPLAY_LEN_MIN_10000 13 + 1 + 1 + 1 +  0 + 44
#define DISPLAY_LEN_MAX_10000 13 + 1 + 6 + 1 + 15 + 44
#define DISPLAY_LEN_MIN_10100 16 + 1 + 1 + 1 + 1 + 1 + 32
#define DISPLAY_LEN_MAX_10100 16 + 1 + 1 + 1 + 1 + 1 + 32
#define DISPLAY_LEN_MIN_10200 10 + 12 + 1 + 44
#define DISPLAY_LEN_MAX_10200 10 + 76 + 1 + 132
#define DISPLAY_LEN_MIN_10300 10 + 1 + 1 + 33
#define DISPLAY_LEN_MAX_10300 10 + 5 + 1 + 49
#define DISPLAY_LEN_MIN_10400 5 + 1 + 1 + 1 + 1 + 2 + 1 + 1 + 1 + 1 + 1 + 2 + 1 + 32 + 1 + 2 + 1 +  64 + 1 + 2 + 1 +  64
#define DISPLAY_LEN_MAX_10400 5 + 1 + 1 + 1 + 1 + 2 + 1 + 5 + 1 + 1 + 1 + 2 + 1 + 32 + 1 + 2 + 1 +  64 + 1 + 2 + 1 +  64
#define DISPLAY_LEN_MIN_10410 5 + 1 + 1 + 1 + 1 + 3 + 1 + 1 + 1 + 1 + 1 + 2 + 1 + 32 + 1 + 2 + 1 +  64 + 1 + 2 + 1 +  64
#define DISPLAY_LEN_MAX_10410 5 + 1 + 1 + 1 + 1 + 3 + 1 + 5 + 1 + 1 + 1 + 2 + 1 + 32 + 1 + 2 + 1 +  64 + 1 + 2 + 1 +  64
#define DISPLAY_LEN_MIN_10420 5 + 1 + 1 + 1 + 1 + 3 + 1 + 1 + 1 + 1 + 1 + 2 + 1 + 32 + 1 + 2 + 1 +  64 + 1 + 2 + 1 +  64 + 1 + 10
#define DISPLAY_LEN_MAX_10420 5 + 1 + 1 + 1 + 1 + 3 + 1 + 5 + 1 + 1 + 1 + 2 + 1 + 32 + 1 + 2 + 1 +  64 + 1 + 2 + 1 +  64 + 1 + 10
#define DISPLAY_LEN_MIN_10500 5 + 1 + 1 + 1 + 1 + 3 + 1 + 1 + 1 + 1 + 1 + 2 + 1 + 32 + 1 + 2 + 1 +  64 + 1 + 2 + 1 +  64
#define DISPLAY_LEN_MAX_10500 5 + 1 + 1 + 1 + 1 + 3 + 1 + 5 + 1 + 1 + 1 + 2 + 1 + 64 + 1 + 2 + 1 +  64 + 1 + 2 + 1 +  64
#define DISPLAY_LEN_MIN_10600 5 + 1 + 1 + 1 + 1 + 3 + 1 + 1 + 1 + 1 + 1 + 2 + 1 + 32 + 1 + 1
#define DISPLAY_LEN_MAX_10600 5 + 1 + 1 + 1 + 1 + 3 + 1 + 5 + 1 + 1 + 1 + 2 + 1 + 32 + 1 + 1000
#define DISPLAY_LEN_MIN_10700 5 + 1 + 1 + 1 + 1 + 3 + 1 + 1 + 1 + 1 + 1 + 2 + 1 + 32 + 1 + 1
#define DISPLAY_LEN_MAX_10700 5 + 1 + 1 + 1 + 1 + 3 + 1 + 5 + 1 + 1 + 1 + 2 + 1 + 32 + 1 + 1000
#define DISPLAY_LEN_MIN_10800 96
#define DISPLAY_LEN_MAX_10800 96
#define DISPLAY_LEN_MIN_10900 7 + 1 + 1 +  0 + 1 + 24
#define DISPLAY_LEN_MAX_10900 7 + 6 + 1 + 64 + 1 + 88
#define DISPLAY_LEN_MIN_11000 32 + 1 + 56
#define DISPLAY_LEN_MAX_11000 32 + 1 + 56
#define DISPLAY_LEN_MIN_11100 10 +  0 + 1 + 8 + 1 + 32
#define DISPLAY_LEN_MAX_11100 10 + 32 + 1 + 8 + 1 + 32
#define DISPLAY_LEN_MIN_11200 9 + 40 + 1 + 40
#define DISPLAY_LEN_MAX_11200 9 + 40 + 1 + 40
#define DISPLAY_LEN_MIN_11300 1 + 7 + 1 + 2 + 1 + 96 + 1 + 2 + 1 + 16 + 1 + 1 + 1 + 2 + 1 + 96 + 1 + 1 + 1 + 2
#define DISPLAY_LEN_MAX_11300 1 + 7 + 1 + 2 + 1 + 96 + 1 + 2 + 1 + 16 + 1 + 6 + 1 + 2 + 1 + 96 + 1 + 3 + 1 + 512
#define DISPLAY_LEN_MIN_11400 6 +   0 + 1 +   0 + 1 +   0 + 1 +   0 + 1 +   0 + 1 +   0 + 1 +   1 + 1 +   0 + 1 +  1 + 1 +  0 + 1 +  0 + 1 +  0 + 1 + 3 + 1 + 32
#define DISPLAY_LEN_MAX_11400 6 + 512 + 1 + 512 + 1 + 116 + 1 + 116 + 1 + 246 + 1 + 245 + 1 + 246 + 1 + 245 + 1 + 50 + 1 + 50 + 1 + 50 + 1 + 50 + 1 + 3 + 1 + 32
#define DISPLAY_LEN_MIN_11500 8 + 1 + 8
#define DISPLAY_LEN_MAX_11500 8 + 1 + 8
#define DISPLAY_LEN_MIN_11600 1 + 2 + 1 + 1 + 1 + 1 + 1 + 1 + 1 +  0 + 1 + 1 + 1 + 32 + 1 +  1 + 1 + 1 + 1 + 1 + 1 +   2
#define DISPLAY_LEN_MAX_11600 1 + 2 + 1 + 1 + 1 + 2 + 1 + 1 + 1 + 64 + 1 + 1 + 1 + 32 + 1 + 10 + 1 + 3 + 1 + 3 + 1 + 768
#define DISPLAY_LEN_MIN_11700 64
#define DISPLAY_LEN_MAX_11700 64
#define DISPLAY_LEN_MIN_11800 128
#define DISPLAY_LEN_MAX_11800 128
#define DISPLAY_LEN_MIN_11900 3 + 1 + 1 +  0 + 1 + 12
#define DISPLAY_LEN_MAX_11900 3 + 6 + 1 + 64 + 1 + 88
#define DISPLAY_LEN_MIN_12000 4 + 1 + 1 +  0 + 1 + 16
#define DISPLAY_LEN_MAX_12000 4 + 6 + 1 + 64 + 1 + 88
#define DISPLAY_LEN_MIN_12100 6 + 1 + 1 +  0 + 1 + 16
#define DISPLAY_LEN_MAX_12100 6 + 6 + 1 + 64 + 1 + 88
#define DISPLAY_LEN_MIN_12100 6 + 1 + 1 +  0 + 1 + 16
#define DISPLAY_LEN_MAX_12100 6 + 6 + 1 + 64 + 1 + 88
#define DISPLAY_LEN_MIN_12200 1 + 8 + 1 + 1 + 1 + 1 + 1 + 16 + 1 + 16
#define DISPLAY_LEN_MAX_12200 1 + 8 + 1 + 1 + 1 + 1 + 1 + 16 + 1 + 16
#define DISPLAY_LEN_MIN_12300 160
#define DISPLAY_LEN_MAX_12300 160
#define DISPLAY_LEN_MIN_12400 1 + 4 + 4 + 11
#define DISPLAY_LEN_MAX_12400 1 + 4 + 4 + 11
#define DISPLAY_LEN_MIN_12500 6 + 1 + 1 + 1 + 16 + 1 + 32
#define DISPLAY_LEN_MAX_12500 6 + 1 + 1 + 1 + 16 + 1 + 32
#define DISPLAY_LEN_MIN_12600 64 + 1 + 64
#define DISPLAY_LEN_MAX_12600 64 + 1 + 64
#define DISPLAY_LEN_MIN_12700  1 + 10 + 1 + 1 + 1 + 64
#define DISPLAY_LEN_MAX_12700  1 + 10 + 1 + 5 + 1 + 20000
#define DISPLAY_LEN_MIN_12800 11 + 1 + 20 + 1 + 1 + 1 + 64
#define DISPLAY_LEN_MAX_12800 11 + 1 + 20 + 1 + 5 + 1 + 64
#define DISPLAY_LEN_MIN_12900 64 + 64 + 32
#define DISPLAY_LEN_MAX_12900 64 + 64 + 32
#define DISPLAY_LEN_MIN_13000 1 + 4 + 1 + 2 + 1 + 32 + 1 + 2 + 1 + 32 + 1 + 1 + 1 + 16
#define DISPLAY_LEN_MAX_13000 1 + 4 + 1 + 2 + 1 + 32 + 1 + 2 + 1 + 32 + 1 + 1 + 1 + 16
#define DISPLAY_LEN_MIN_13100  1 + 7 + 1 + 2 + 1 + 0 + 0 + 32 + 1 + 64
#define DISPLAY_LEN_MAX_13100  1 + 7 + 1 + 2 + 1 + 2 + 512 + 1 + 32 + 1 + 20480
#define DISPLAY_LEN_MIN_13200  1 + 7 + 1 + 1 + 1 + 1 + 1 + 1 + 32 + 1 + 48
#define DISPLAY_LEN_MAX_13200  1 + 7 + 1 + 1 + 1 + 1 + 50 + 1 + 32 + 1 + 48 + 1 + 20480
#define DISPLAY_LEN_MIN_13300  1 + 12 + 1 + 32
#define DISPLAY_LEN_MAX_13300  1 + 12 + 1 + 40
#define DISPLAY_LEN_MIN_13400  1 + 7 + 1 + 1 + 1 + 1 + 1 + 1 + 32 + 1 + 64 + 1 + 32 + 1 + 64 + 1 + 1 + 1 + 1
#define DISPLAY_LEN_MAX_13400  1 + 7 + 1 + 1 + 10 + 1 + 3 + 1 + 64 + 1 + 64 + 1 + 32 + 1 + 64 + 1 + 4 + 1 + 600000 + 1 + 2 + 1 + 64
#define DISPLAY_LEN_MIN_13500 40 + 1 + 32
#define DISPLAY_LEN_MAX_13500 40 + 1 + 1024
#define DISPLAY_LEN_MIN_13600 6 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 16 + 1 + 1 + 1 + 1 + 1 +    0 + 1 + 20 + 1 + 7
#define DISPLAY_LEN_MAX_13600 6 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 32 + 1 + 4 + 1 + 4 + 1 + 8192 + 1 + 20 + 1 + 7
#define DISPLAY_LEN_MIN_13800  64 + 1 + 256
#define DISPLAY_LEN_MAX_13800  64 + 1 + 256
#define DISPLAY_LEN_MIN_13900 40 + 1 + 9
#define DISPLAY_LEN_MAX_13900 40 + 1 + 9

#define DISPLAY_LEN_MIN_11    32 + 1 + 16
#define DISPLAY_LEN_MAX_11    32 + 1 + 32
#define DISPLAY_LEN_MIN_11H   32 + 1 + 32
#define DISPLAY_LEN_MAX_11H   32 + 1 + 64
#define DISPLAY_LEN_MIN_12    32 + 1 + 1
#define DISPLAY_LEN_MAX_12    32 + 1 + 32
#define DISPLAY_LEN_MIN_12H   32 + 1 + 2
#define DISPLAY_LEN_MAX_12H   32 + 1 + 64
#define DISPLAY_LEN_MIN_21    32 + 1 + 1
#define DISPLAY_LEN_MAX_21    32 + 1 + 15
#define DISPLAY_LEN_MIN_21H   32 + 1 + 2
#define DISPLAY_LEN_MAX_21H   32 + 1 + 30
#define DISPLAY_LEN_MIN_22    30 + 1 + 1
#define DISPLAY_LEN_MAX_22    30 + 1 + 28
#define DISPLAY_LEN_MIN_22H   30 + 1 + 2
#define DISPLAY_LEN_MAX_22H   30 + 1 + 56
#define DISPLAY_LEN_MIN_23    32 + 1 + 0
#define DISPLAY_LEN_MAX_23    32 + 1 + 23
#define DISPLAY_LEN_MIN_101    5 + 28
#define DISPLAY_LEN_MAX_101    5 + 28
#define DISPLAY_LEN_MIN_111    6 + 28 + 0
#define DISPLAY_LEN_MAX_111    6 + 28 + 40
#define DISPLAY_LEN_MIN_112   40 + 1 + 20
#define DISPLAY_LEN_MAX_112   40 + 1 + 20
#define DISPLAY_LEN_MIN_121   40 + 1 + 1
#define DISPLAY_LEN_MAX_121   40 + 1 + 32
#define DISPLAY_LEN_MIN_121H  40 + 1 + 2
#define DISPLAY_LEN_MAX_121H  40 + 1 + 64
#define DISPLAY_LEN_MIN_122    8 + 40
#define DISPLAY_LEN_MAX_122    8 + 40
#define DISPLAY_LEN_MIN_124   4 + 1 +  0 + 1 + 40
#define DISPLAY_LEN_MAX_124   4 + 1 + 32 + 1 + 40
#define DISPLAY_LEN_MIN_125   10 + 40
#define DISPLAY_LEN_MAX_125   10 + 40
#define DISPLAY_LEN_MIN_131    6 +  8 + 80
#define DISPLAY_LEN_MAX_131    6 +  8 + 80
#define DISPLAY_LEN_MIN_132    6 +  8 + 40
#define DISPLAY_LEN_MAX_132    6 +  8 + 40
#define DISPLAY_LEN_MIN_133   28
#define DISPLAY_LEN_MAX_133   28
#define DISPLAY_LEN_MIN_141   14 +  0 +  1 + 28
#define DISPLAY_LEN_MAX_141   14 + 44 +  1 + 28
#define DISPLAY_LEN_MIN_1441  14 +  0 +  1 + 43
#define DISPLAY_LEN_MAX_1441  14 + 24 +  1 + 43
#define DISPLAY_LEN_MIN_1711   9 + 86 +  0
#define DISPLAY_LEN_MAX_1711   9 + 86 + 68
#define DISPLAY_LEN_MIN_1722   8 + 128
#define DISPLAY_LEN_MAX_1722   8 + 128
#define DISPLAY_LEN_MIN_2611  32 + 1 + 0
#define DISPLAY_LEN_MAX_2611  32 + 1 + 23
#define DISPLAY_LEN_MIN_2611H 32 + 1 + 0
#define DISPLAY_LEN_MIN_2612  6 +  0 + 1 + 32
#define DISPLAY_LEN_MAX_2611H 32 + 1 + 46
#define DISPLAY_LEN_MAX_2612  6 + 46 + 1 + 32
#define DISPLAY_LEN_MIN_2711  32 + 1 + 23
#define DISPLAY_LEN_MAX_2711  32 + 1 + 31
#define DISPLAY_LEN_MIN_2711H 32 + 1 + 46
#define DISPLAY_LEN_MAX_2711H 32 + 1 + 62
#define DISPLAY_LEN_MIN_2811  32 + 1 + 0
#define DISPLAY_LEN_MAX_2811  32 + 1 + 31
#define DISPLAY_LEN_MIN_2811H 32 + 1 + 0
#define DISPLAY_LEN_MAX_2811H 32 + 1 + 62
#define DISPLAY_LEN_MIN_7600  40 + 1 + 32
#define DISPLAY_LEN_MAX_7600  40 + 1 + 32

#define HASH_TYPE_MD4          1
#define HASH_TYPE_MD5          2
#define HASH_TYPE_MD5H         3
#define HASH_TYPE_SHA1         4
#define HASH_TYPE_SHA256       5
#define HASH_TYPE_SHA384       6
#define HASH_TYPE_SHA512       7
#define HASH_TYPE_DCC2         8
#define HASH_TYPE_WPA          9
#define HASH_TYPE_LM           10
#define HASH_TYPE_DESCRYPT     11
#define HASH_TYPE_ORACLEH      12
#define HASH_TYPE_DESRACF      13
#define HASH_TYPE_BCRYPT       14
#define HASH_TYPE_KECCAK       15
#define HASH_TYPE_NETNTLM      16
#define HASH_TYPE_RIPEMD160    17
#define HASH_TYPE_WHIRLPOOL    18
#define HASH_TYPE_AES          19
#define HASH_TYPE_GOST         20
#define HASH_TYPE_KRB5PA       21
#define HASH_TYPE_SAPB         22
#define HASH_TYPE_SAPG         23
#define HASH_TYPE_MYSQL        24
#define HASH_TYPE_LOTUS5       25
#define HASH_TYPE_LOTUS6       26
#define HASH_TYPE_ANDROIDFDE   27
#define HASH_TYPE_SCRYPT       28
#define HASH_TYPE_LOTUS8       29
#define HASH_TYPE_OFFICE2007   30
#define HASH_TYPE_OFFICE2010   31
#define HASH_TYPE_OFFICE2013   32
#define HASH_TYPE_OLDOFFICE01  33
#define HASH_TYPE_OLDOFFICE34  34
#define HASH_TYPE_SIPHASH      35
#define HASH_TYPE_PDFU16       36
#define HASH_TYPE_PDFU32       37
#define HASH_TYPE_PBKDF2_SHA256  38
#define HASH_TYPE_BITCOIN_WALLET 39
#define HASH_TYPE_CRC32          40
#define HASH_TYPE_GOST_2012SBOG_256  41
#define HASH_TYPE_GOST_2012SBOG_512  42
#define HASH_TYPE_PBKDF2_MD5     43
#define HASH_TYPE_PBKDF2_SHA1    44
#define HASH_TYPE_PBKDF2_SHA512  45
#define HASH_TYPE_ECRYPTFS       46
#define HASH_TYPE_ORACLET        47
#define HASH_TYPE_BSDICRYPT      48
#define HASH_TYPE_RAR3HP         49
#define HASH_TYPE_KRB5TGS        50
#define HASH_TYPE_STDOUT         51

#include "consts/kernel_types.h"
#include "consts/signatures.h"
#include "consts/rounds_count.h"
#include "consts/salt_types.h"
#include "consts/optimizer_options.h"
#include "consts/hash_options.h"

 /**
  * digests
  */

#define DGST_SIZE_4_2               (2  * sizeof (uint))   // 8
#define DGST_SIZE_4_4               (4  * sizeof (uint))   // 16
#define DGST_SIZE_4_5               (5  * sizeof (uint))   // 20
#define DGST_SIZE_4_6               (6  * sizeof (uint))   // 24
#define DGST_SIZE_4_8               (8  * sizeof (uint))   // 32
#define DGST_SIZE_4_16              (16 * sizeof (uint))   // 64 !!!
#define DGST_SIZE_4_32              (32 * sizeof (uint))   // 128 !!!
#define DGST_SIZE_4_64              (64 * sizeof (uint))   // 256
#define DGST_SIZE_8_8               (8  * sizeof (u64))    // 64 !!!
#define DGST_SIZE_8_16              (16 * sizeof (u64))    // 128 !!!
#define DGST_SIZE_8_25              (25 * sizeof (u64))    // 200

#include "consts/parser.h"
#include "consts/devices_statuses.h"

  /**
   * kernel types
   */

#define KERN_RUN_MP          101
#define KERN_RUN_MP_L        102
#define KERN_RUN_MP_R        103

#define KERN_RUN_1           1000
#define KERN_RUN_12          1500
#define KERN_RUN_2           2000
#define KERN_RUN_23          2500
#define KERN_RUN_3           3000

   /*
    * functions
    */

u32 is_power_of_2(u32 v);

u32 rotl32(const u32 a, const u32 n);
u32 rotr32(const u32 a, const u32 n);
u64 rotl64(const u64 a, const u64 n);
u64 rotr64(const u64 a, const u64 n);

u32 byte_swap_32(const u32 n);
u64 byte_swap_64(const u64 n);

u8  hex_convert(const u8 c);
u8  hex_to_u8(const u8 hex[2]);
u32 hex_to_u32(const u8 hex[8]);
u64 hex_to_u64(const u8 hex[16]);

void dump_hex(const u8 *s, const int sz);

void truecrypt_crc32(const char *filename, u8 keytab[64]);

char *get_exec_path();
char *get_install_dir(const char *progname);
char *get_profile_dir(const char *homedir);
char *get_session_dir(const char *profile_dir);
uint count_lines(FILE *fd);

void *rulefind(const void *key, void *base, int nmemb, size_t size, int(*compar) (const void *, const void *));

int sort_by_u32(const void *p1, const void *p2);
int sort_by_mtime(const void *p1, const void *p2);
int sort_by_cpu_rule(const void *p1, const void *p2);
int sort_by_kernel_rule(const void *p1, const void *p2);
int sort_by_stringptr(const void *p1, const void *p2);
int sort_by_dictstat(const void *s1, const void *s2);
int sort_by_bitmap(const void *s1, const void *s2);

int sort_by_pot(const void *v1, const void *v2);
int sort_by_hash(const void *v1, const void *v2);
int sort_by_hash_no_salt(const void *v1, const void *v2);
int sort_by_salt(const void *v1, const void *v2);
int sort_by_salt_buf(const void *v1, const void *v2);
int sort_by_hash_t_salt(const void *v1, const void *v2);
int sort_by_digest_4_2(const void *v1, const void *v2);
int sort_by_digest_4_4(const void *v1, const void *v2);
int sort_by_digest_4_5(const void *v1, const void *v2);
int sort_by_digest_4_6(const void *v1, const void *v2);
int sort_by_digest_4_8(const void *v1, const void *v2);
int sort_by_digest_4_16(const void *v1, const void *v2);
int sort_by_digest_4_32(const void *v1, const void *v2);
int sort_by_digest_4_64(const void *v1, const void *v2);
int sort_by_digest_8_8(const void *v1, const void *v2);
int sort_by_digest_8_16(const void *v1, const void *v2);
int sort_by_digest_8_25(const void *v1, const void *v2);
int sort_by_digest_p0p1(const void *v1, const void *v2);

// special version for hccap (last 2 uints should be skipped where the digest is located)
int sort_by_hash_t_salt_hccap(const void *v1, const void *v2);

void format_debug(char * debug_file, uint debug_mode, unsigned char *orig_plain_ptr, uint orig_plain_len, unsigned char *mod_plain_ptr, uint mod_plain_len, char *rule_buf, int rule_len);
void format_plain(FILE *fp, unsigned char *plain_ptr, uint plain_len, uint outfile_autohex);
void format_output(FILE *out_fp, char *out_buf, unsigned char *plain_ptr, const uint plain_len, const u64 crackpos, unsigned char *username, const uint user_len);
void handle_show_request(pot_t *pot, uint pot_cnt, char *input_buf, int input_len, hash_t *hashes_buf, int(*sort_by_pot) (const void *, const void *), FILE *out_fp);
void handle_left_request(pot_t *pot, uint pot_cnt, char *input_buf, int input_len, hash_t *hashes_buf, int(*sort_by_pot) (const void *, const void *), FILE *out_fp);
void handle_show_request_lm(pot_t *pot, uint pot_cnt, char *input_buf, int input_len, hash_t *hash_left, hash_t *hash_right, int(*sort_by_pot) (const void *, const void *), FILE *out_fp);
void handle_left_request_lm(pot_t *pot, uint pot_cnt, char *input_buf, int input_len, hash_t *hash_left, hash_t *hash_right, int(*sort_by_pot) (const void *, const void *), FILE *out_fp);

u32            setup_opencl_platforms_filter(char *opencl_platforms);
u32            setup_devices_filter(char *opencl_devices);
cl_device_type setup_device_types_filter(char *opencl_device_types);

u32 get_random_num(const u32 min, const u32 max);

u32 mydivc32(const u32 dividend, const u32 divisor);
u64 mydivc64(const u64 dividend, const u64 divisor);

void ascii_digest(char *out_buf, uint salt_pos, uint digest_pos);
void to_hccap_t(hccap_t *hccap, uint salt_pos, uint digest_pos);

void format_speed_display(float val, char *buf, size_t len);
void format_timer_display(struct tm *tm, char *buf, size_t len);
void lowercase(u8 *buf, int len);
void uppercase(u8 *buf, int len);
int fgetl(FILE *fp, char *line_buf);
int in_superchop(char *buf);
char **scan_directory(const char *path);
int count_dictionaries(char **dictionary_files);
char *strparser(const uint parser_status);
char *stroptitype(const uint opti_type);
char *strhashtype(const uint hash_mode);
char *strstatus(const uint threads_status);
void status();

void *mycalloc(size_t nmemb, size_t size);
void myfree(void *ptr);
void *mymalloc(size_t size);
void *myrealloc(void *ptr, size_t oldsz, size_t add);
char *mystrdup(const char *s);

char *logfile_generate_topid();
char *logfile_generate_subid();
void logfile_append(const char *fmt, ...);

#if F_SETLKW
void lock_file(FILE *fp);
void unlock_file(FILE *fp);
#else
#define lock_file(dummy) {}
#define unlock_file(dummy) {}
#endif

#ifdef _WIN
void fsync(int fd);
#endif

#ifdef HAVE_HWMON

int get_adapters_num_adl(void *adl, int *iNumberAdapters);

int hm_get_adapter_index_adl(hm_attrs_t *hm_device, u32 *valid_adl_device_list, int num_adl_adapters, LPAdapterInfo lpAdapterInfo);

int hm_get_adapter_index_nvapi(HM_ADAPTER_NVAPI nvapiGPUHandle[DEVICES_MAX]);

int hm_get_adapter_index_nvml(HM_ADAPTER_NVML nvmlGPUHandle[DEVICES_MAX]);

LPAdapterInfo hm_get_adapter_info_adl(void *adl, int iNumberAdapters);

u32 *hm_get_list_valid_adl_adapters(int iNumberAdapters, int *num_adl_adapters, LPAdapterInfo lpAdapterInfo);

int hm_get_overdrive_version(void *adl, hm_attrs_t *hm_device, u32 *valid_adl_device_list, int num_adl_adapters, LPAdapterInfo lpAdapterInfo);
int hm_check_fanspeed_control(void *adl, hm_attrs_t *hm_device, u32 *valid_adl_device_list, int num_adl_adapters, LPAdapterInfo lpAdapterInfo);

// int hm_get_device_num (void *adl, HM_ADAPTER_ADL hm_adapter_index, int *hm_device_num);
// void hm_get_opencl_busid_devid (hm_attrs_t *hm_device, uint opencl_num_devices, cl_device_id *devices);

int hm_get_threshold_slowdown_with_device_id(const uint device_id);
int hm_get_threshold_shutdown_with_device_id(const uint device_id);
int hm_get_temperature_with_device_id(const uint device_id);
int hm_get_fanspeed_with_device_id(const uint device_id);
int hm_get_fanpolicy_with_device_id(const uint device_id);
int hm_get_buslanes_with_device_id(const uint device_id);
int hm_get_utilization_with_device_id(const uint device_id);
int hm_get_memoryspeed_with_device_id(const uint device_id);
int hm_get_corespeed_with_device_id(const uint device_id);
int hm_get_throttle_with_device_id(const uint device_id);
int hm_set_fanspeed_with_device_id_adl(const uint device_id, const int fanspeed, const int fanpolicy);
int hm_set_fanspeed_with_device_id_nvapi(const uint device_id, const int fanspeed, const int fanpolicy);
int hm_set_fanspeed_with_device_id_xnvctrl(const uint device_id, const int fanspeed);

void hm_device_val_to_str(char *target_buf, int max_buf_size, char *suffix, int value);
#endif // HAVE_HWMON

void myabort();
void myquit();

void set_cpu_affinity(char *cpu_affinity);

void usage_mini_print(const char *progname);
void usage_big_print(const char *progname);

void mp_css_to_uniq_tbl(uint css_cnt, cs_t *css, uint uniq_tbls[SP_PW_MAX][CHARSIZ]);
void mp_cut_at(char *mask, uint max);
void mp_exec(u64 val, char *buf, cs_t *css, int css_cnt);
cs_t *mp_gen_css(char *mask_buf, size_t mask_len, cs_t *mp_sys, cs_t *mp_usr, uint *css_cnt);
u64 mp_get_sum(uint css_cnt, cs_t *css);
void mp_setup_sys(cs_t *mp_sys);
void mp_setup_usr(cs_t *mp_sys, cs_t *mp_usr, char *buf, uint index);
void mp_reset_usr(cs_t *mp_usr, uint index);
char *mp_get_truncated_mask(char *mask_buf, size_t mask_len, uint len);

u64 sp_get_sum(uint start, uint stop, cs_t *root_css_buf);
void sp_exec(u64 ctx, char *pw_buf, cs_t *root_css_buf, cs_t *markov_css_buf, uint start, uint stop);
int sp_comp_val(const void *p1, const void *p2);
void sp_setup_tbl(const char *install_dir, char *hcstat, uint disable, uint classic, hcstat_table_t *root_table_buf, hcstat_table_t *markov_table_buf);
void sp_tbl_to_css(hcstat_table_t *root_table_buf, hcstat_table_t *markov_table_buf, cs_t *root_css_buf, cs_t *markov_css_buf, uint threshold, uint uniq_tbls[SP_PW_MAX][CHARSIZ]);
void sp_stretch_markov(hcstat_table_t *in, hcstat_table_t *out);
void sp_stretch_root(hcstat_table_t *in, hcstat_table_t *out);

void tuning_db_destroy(tuning_db_t *tuning_db);
tuning_db_t *tuning_db_alloc(FILE *fp);
tuning_db_t *tuning_db_init(const char *tuning_db_file);
tuning_db_entry_t *tuning_db_search(tuning_db_t *tuning_db, hc_device_param_t *device_param, int attack_mode, int hash_type);

int bcrypt_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int cisco4_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int dcc_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int dcc2_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int descrypt_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int episerver_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int ipb2_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int joomla_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int postgresql_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int netscreen_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int keccak_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int lm_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int md4_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int md4s_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int md5_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int md5s_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int md5half_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int md5md5_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int md5pix_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int md5asa_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int md5apr1_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int md5crypt_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int mssql2000_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int mssql2005_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int netntlmv1_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int netntlmv2_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int oracleh_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int oracles_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int oraclet_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int osc_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int arubaos_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int osx1_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int osx512_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int phpass_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sha1_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sha1b64_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sha1b64s_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sha1s_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sha256_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sha256s_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sha384_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sha512_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sha512s_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sha512crypt_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int smf_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int vb3_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int vb30_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int wpa_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int psafe2_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int psafe3_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int ikepsk_md5_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int ikepsk_sha1_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int androidpin_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int ripemd160_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int whirlpool_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int truecrypt_parse_hash_1k(char *input_buf, uint input_len, hash_t *hash_buf);
int truecrypt_parse_hash_2k(char *input_buf, uint input_len, hash_t *hash_buf);
int md5aix_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sha256aix_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sha512aix_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int agilekey_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sha1aix_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int lastpass_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int gost_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sha256crypt_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int mssql2012_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sha512osx_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int episerver4_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sha512grub_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sha512b64s_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int hmacsha1_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int hmacsha256_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int hmacsha512_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int hmacmd5_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int krb5pa_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int krb5tgs_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sapb_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sapg_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int drupal7_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sybasease_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int mysql323_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int rakp_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int netscaler_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int chap_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int cloudkey_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int nsec3_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int wbb3_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int racf_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int lotus5_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int lotus6_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int lotus8_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int hmailserver_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int phps_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int mediawiki_b_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int peoplesoft_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int skype_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int androidfde_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int scrypt_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int juniper_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int cisco8_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int cisco9_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int office2007_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int office2010_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int office2013_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int oldoffice01_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int oldoffice01cm1_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int oldoffice01cm2_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int oldoffice34_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int oldoffice34cm1_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int oldoffice34cm2_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int radmin2_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int djangosha1_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int djangopbkdf2_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int siphash_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int crammd5_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int saph_sha1_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int redmine_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int pdf11_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int pdf11cm1_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int pdf11cm2_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int pdf14_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int pdf17l3_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int pdf17l8_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int pbkdf2_sha256_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int prestashop_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int postgresql_auth_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int mysql_auth_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int bitcoin_wallet_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sip_auth_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int crc32_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int seven_zip_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int gost2012sbog_256_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int gost2012sbog_512_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int pbkdf2_md5_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int pbkdf2_sha1_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int pbkdf2_sha512_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int ecryptfs_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int bsdicrypt_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int rar3hp_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int rar5_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int cf10_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int mywallet_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int ms_drsr_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int androidfde_samsung_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int axcrypt_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int sha1axcrypt_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int keepass_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int pstoken_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int zip2_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int veracrypt_parse_hash_200000(char *input_buf, uint input_len, hash_t *hash_buf);
int veracrypt_parse_hash_500000(char *input_buf, uint input_len, hash_t *hash_buf);
int veracrypt_parse_hash_327661(char *input_buf, uint input_len, hash_t *hash_buf);
int veracrypt_parse_hash_655331(char *input_buf, uint input_len, hash_t *hash_buf);
int win8phone_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);
int opencart_parse_hash(char *input_buf, uint input_len, hash_t *hash_buf);

void naive_replace(char *s, const u8 key_char, const u8 replace_char);
void naive_escape(char *s, size_t s_max, const u8 key_char, const u8 escape_char);
void load_kernel(const char *kernel_file, int num_devices, size_t *kernel_lengths, const u8 **kernel_sources);
void writeProgramBin(char *dst, u8 *binary, size_t binary_size);

u64 get_lowest_words_done();

restore_data_t *init_restore(int argc, char **argv);
void            read_restore(const char *eff_restore_file, restore_data_t *rd);
void            write_restore(const char *new_restore_file, restore_data_t *rd);
void            cycle_restore();
void            check_checkpoint();

#ifdef WIN

BOOL WINAPI sigHandler_default(DWORD sig);
BOOL WINAPI sigHandler_benchmark(DWORD sig);
void hc_signal(BOOL WINAPI(callback) (DWORD sig));

#else

void sigHandler_default(int sig);
void sigHandler_benchmark(int sig);
void hc_signal(void c(int));

#endif

bool class_num(u8 c);
bool class_lower(u8 c);
bool class_upper(u8 c);
bool class_alpha(u8 c);

int mangle_lrest(char arr[BLOCK_SIZE], int arr_len);
int mangle_urest(char arr[BLOCK_SIZE], int arr_len);
int mangle_trest(char arr[BLOCK_SIZE], int arr_len);
int mangle_reverse(char arr[BLOCK_SIZE], int arr_len);
int mangle_double(char arr[BLOCK_SIZE], int arr_len);
int mangle_double_times(char arr[BLOCK_SIZE], int arr_len, int times);
int mangle_reflect(char arr[BLOCK_SIZE], int arr_len);
int mangle_rotate_left(char arr[BLOCK_SIZE], int arr_len);
int mangle_rotate_right(char arr[BLOCK_SIZE], int arr_len);
int mangle_append(char arr[BLOCK_SIZE], int arr_len, char c);
int mangle_prepend(char arr[BLOCK_SIZE], int arr_len, char c);
int mangle_delete_at(char arr[BLOCK_SIZE], int arr_len, int upos);
int mangle_extract(char arr[BLOCK_SIZE], int arr_len, int upos, int ulen);
int mangle_omit(char arr[BLOCK_SIZE], int arr_len, int upos, int ulen);
int mangle_insert(char arr[BLOCK_SIZE], int arr_len, int upos, char c);
int mangle_overstrike(char arr[BLOCK_SIZE], int arr_len, int upos, char c);
int mangle_truncate_at(char arr[BLOCK_SIZE], int arr_len, int upos);
int mangle_replace(char arr[BLOCK_SIZE], int arr_len, char oldc, char newc);
int mangle_purgechar(char arr[BLOCK_SIZE], int arr_len, char c);
int mangle_dupeblock_prepend(char arr[BLOCK_SIZE], int arr_len, int ulen);
int mangle_dupeblock_append(char arr[BLOCK_SIZE], int arr_len, int ulen);
int mangle_dupechar_at(char arr[BLOCK_SIZE], int arr_len, int upos, int ulen);
int mangle_dupechar(char arr[BLOCK_SIZE], int arr_len);
int mangle_switch_at_check(char arr[BLOCK_SIZE], int arr_len, int upos, int upos2);
int mangle_switch_at(char arr[BLOCK_SIZE], int arr_len, int upos, int upos2);
int mangle_chr_shiftl(char arr[BLOCK_SIZE], int arr_len, int upos);
int mangle_chr_shiftr(char arr[BLOCK_SIZE], int arr_len, int upos);
int mangle_chr_incr(char arr[BLOCK_SIZE], int arr_len, int upos);
int mangle_chr_decr(char arr[BLOCK_SIZE], int arr_len, int upos);
int mangle_title(char arr[BLOCK_SIZE], int arr_len);

int generate_random_rule(char rule_buf[RP_RULE_BUFSIZ], u32 rp_gen_func_min, u32 rp_gen_func_max);
int _old_apply_rule(char *rule, int rule_len, char in[BLOCK_SIZE], int in_len, char out[BLOCK_SIZE]);

int cpu_rule_to_kernel_rule(char *rule_buf, uint rule_len, kernel_rule_t *rule);
int kernel_rule_to_cpu_rule(char *rule_buf, kernel_rule_t *rule);

void *thread_device_watch(void *p);
void *thread_keypress(void *p);
void *thread_runtime(void *p);

/**
 * checksum for use on cpu
 */

#include "cpu-crc32.h"
#include "cpu-md5.h"

 /**
  * ciphers for use on cpu
  */

#include "cpu-aes.h"

#endif // SHARED_H
