#ifndef __ETL_PROFILE_H__
#define __ETL_PROFILE_H__

#define ETL_VERBOSE_ERRORS
#define ETL_CHECK_PUSH_POP
#define ETL_TARGET_OS_FREERTOS

/* Seems <mutex> isn't actually implemented by newlib here, let's just use ETL's version. */
#define ETL_NO_STL

#endif
