/* ------------------------ System includes -------------------------------- */
#include <inttypes.h>
#include <stddef.h>

/* ------------------------ Project includes ------------------------------- */
#include "cortex_profile.h"

/* ------------------------ Defines ---------------------------------------- */
/**
 * @brief Debug Exception and Monitor Control Register Base Address
 *
 */
#define CORTEX_DEMCR (*(uint32_t *) 0xE000EDFC)

/**
 * @brief Data Watchpoint and Trace Register Control Register Base Address
 *
 */
#define CORTEX_DWT_CTRL (*(uint32_t *) 0xE0001000)

/**
 * @brief Data Watchpoint and Trace Register Cycle Count Register Address
 *
 */
#define CORTEX_DWT_CYCCNT (*(volatile uint32_t *) 0xE0001004)

/**
 * @brief Disable IRQ Interrupts
 * 
 */
#define ENTER_CRITICAL_SECTION() __asm volatile("cpsid i" : : : "memory")

/**
 * @brief Enable IRQ Interrupts
 * 
 */
#define EXIT_CRITICAL_SECTION() __asm volatile ("cpsie i" : : : "memory")

/* ------------------------ Types ------------------------------------------ */
typedef struct
{
  const char* event_name;
  uint32_t    timestamp;
} event_t;

typedef struct
{
  const char* profile_name;
  event_t     events[CORTEX_PROFILE_MAX_EVENTS];
  uint8_t     event_count;
  uint32_t    timezero;
} cortex_profile_t;

/* ------------------------ Implementation --------------------------------- */
#if defined(USE_CORTEX_PROFILE)

/** Output print callback function (e.g. printf from <stdio.h> or another) */
static int (*printer)(const char*, ...) = NULL; 

/** Processor's tick rate us */
static uint32_t tick_rate_us = 0;

/** Profiler instance */
static cortex_profile_t profiler = {0};

/**
 * @brief Print profiler ouput table
 * 
 */
static void cortex_profile_print()
{
	printer(
	    "Profiling \"%s\" sequence: \r\n"
	    "|---------- Event ----------|--- Timestamp ---|----- Delta -----|\r\n",
	    (&profiler)->profile_name);

	uint32_t timezero_us = 0;
  for (uint8_t i = 0; i < (&profiler)->event_count; ++i) {
    event_t* event        = &(&profiler)->events[i];
    int32_t  timestamp_us = event->timestamp / tick_rate_us;
    int32_t  delta_us     = timestamp_us - timezero_us;
    timezero_us           = timestamp_us;
    printer("| %-25s | %12ld us | %12ld us |\r\n", 
           event->event_name, timestamp_us, delta_us);
  }
  printer("\r\n");
}

/**
 * @brief Initialize cortex profiler.
 * @note Must be called once when initialize processor.
 * 
 * @param core_clock 
 */
void cortex_profile_init(uint32_t core_clock,
                         int (*printer_callback)(const char *, ...))
{
  /* Check whether the implementation supports a cycle counter. */
  if (CORTEX_DWT_CTRL & (1 << 25)) {
    printer("Cycle counter not supported.\n");
    return;
  }

  /* Global enable for all DWT and ITM features. */
  CORTEX_DEMCR |= (1 << 24);

  /* Compute processor tick rate in us */
  tick_rate_us = core_clock / 1000000;

  /* Initialize output callback function */
  printer = printer_callback;
}

/**
 * @brief Start profiling sequence.
 * @warning Disables global IRQ interrupts!
 * 
 * @param profile_name 
 */
void cortex_profile_begin(const char* profile_name)
{
  /* Disable global IRQ */
  ENTER_CRITICAL_SECTION();

  /* Reset a value of the processor cycle counter. */
  CORTEX_DWT_CYCCNT = 0;

  /* Initialize profile sequence */
  (&profiler)->profile_name = profile_name;
  (&profiler)->event_count  = 0;

  /* Enable incrementation of cycle counting value
   * on each processor clock cycle. */
  CORTEX_DWT_CTRL |= (1 << 0);
}

/**
 * @brief Append new profiler event.
 * 
 * @param event_name 
 */
void cortex_profile_event(const char* event_name)
{
  if ((&profiler)->event_count == CORTEX_PROFILE_MAX_EVENTS) {
    printer("Profiler reached maximum number of events.\n");
    return;
  }

  uint32_t event_timestamp = CORTEX_DWT_CYCCNT;
  uint8_t  event_pos       = (&profiler)->event_count;

  (&profiler)->events[event_pos]
      = (event_t){.event_name = event_name, .timestamp = event_timestamp};
  (&profiler)->event_count++;
}

/**
 * @brief End of profile sequence. Output results.
 * @note Restores global IRQ interrupts previously disabled in
 *       cortex_profile_begin(const char *profile_name)
 */
void cortex_profile_end()
{
  if ((&profiler)->event_count == 0) {
    printer("There are no events in profiler.\n");
    return;
  }

  /* Disable incrementation of cycle counting value */
  CORTEX_DWT_CTRL |= (0 << 0);

  /* Print output table */
  cortex_profile_print(); 

  /* Restore global IRQ */
  EXIT_CRITICAL_SECTION();
}

#endif // !USE_CORTEX_PROFILE