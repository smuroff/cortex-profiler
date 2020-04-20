#ifndef CORTEX_PROFILE_H
#define CORTEX_PROFILE_H

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Nmmber of possible events */
#define CORTEX_PROFILE_MAX_EVENTS 16

#if defined(USE_CORTEX_PROFILE)
  #define CORTEX_PROFILE_INIT(CORE_CLOCK, PRINT_FUNCTION) (cortex_profile_init((CORE_CLOCK), (PRINT_FUNCTION)))
  #define CORTEX_PROFILE_BEGIN(NAME)                      (cortex_profile_begin((NAME)))
  #define CORTEX_PROFILE_EVENT(NAME)                      (cortex_profile_event((NAME)))
  #define CORTEX_PROFILE_END()                            (cortex_profile_end())

  void cortex_profile_init(uint32_t core_clock,
                           int (*printer_callback)(const char*, ...));
  void cortex_profile_begin(const char* profile_name);
  void cortex_profile_event(const char* event_name);
  void cortex_profile_end();
#else
  #define CORTEX_PROFILE_INIT(CORE_CLOCK, PRINT_FUNCTION) ((void) 0)
  #define CORTEX_PROFILE_BEGIN(NAME)                      ((void) 0)
  #define CORTEX_PROFILE_EVENT(NAME)                      ((void) 0)
  #define CORTEX_PROFILE_END()                            ((void) 0)
#endif

#ifdef __cplusplus
}
#endif

#endif  // !CORTEX_PROFILE_H
