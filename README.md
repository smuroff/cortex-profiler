### Simple code profiler for ARMv7-M Cortex-M3 and higher CPU families.   

---

### Features

- Accuracy is around 1us.
- Easy switch between Release/Debug mode by defining `USE_CORTEX_PROFILE`.
- FreeRTOS compatibility.

---

#### Usage

Start profiling session.

```c
CORTEX_PROFILE_BEGIN("session name");
```

Insert timestamp command as many times as necessary.

> The maximum number of events may be configured in `cortex_profile.h` file by assign a new value to `CORTEX_PROFILE_MAX_EVENTS`

```c
CORTEX_PROFILE_EVENT("event name");
```

Close profiling session.  


```c
CORTEX_PROFILE_END();
```
> :warning: **Don't forget** to end the seesion to restore global IRQ settings.

Build your program with `USE_CORTEX_PROFILE` define.

```bash
make -D USE_CORTEX_PROFILE ...
```

Run.

---

### Example code

```cpp
  CORTEX_PROFILE_INIT(SystemCoreClock); /** e.g. 72000000 Hz */

  CORTEX_PROFILE_BEGIN("Startup your job");

  InitializeFlawlessTask();
  CORTEX_PROFILE_EVENT("Initialized flawless task");

  HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, GPIO_PIN_SET);
  CORTEX_PROFILE_EVENT("Turn on LED");

  DoAwesomeWork(1000);
  CORTEX_PROFILE_EVENT("DoAwesomeWork 1000 ticks");
  
  HAL_GPIO_WritePin(LED_YELLOW_GPIO_Port, LED_YELLOW_Pin, GPIO_PIN_RESET);
  CORTEX_PROFILE_EVENT("Turn off LED");

  DoAwesomeWork(5000);
  CORTEX_PROFILE_EVENT("DoAwesomeWork 5000 ticks");

  CORTEX_PROFILE_END();
```

### Example output

```
Profiling "Startup your job" sequence: 
|---------- Event ----------|--- Timestamp ---|----- Delta -----|
| Initialized flawless task |            6 us |            6 us |
| Turn on LED               |            7 us |            1 us |
| DoAwesomeWork 1000 ticks  |          133 us |          126 us |
| Turn off LED              |          135 us |            2 us |
| DoAwesomeWork 5000 ticks  |          761 us |          626 us |
```

---

### Notes

To redirect data to SWV or UART I prefer to use tiny [printf](https://github.com/mpaland/printf) implementation 
which included in my projects by default. Feel free to choose whatever you want ;)

### Redirection

`redirect.c`
```c
/* Example standard GNU C library printf redirection */

#include <errno.h>
#include <sys/unistd.h>

int _write(int file, char* data, int len)
{
  if ((file != STDOUT_FILENO) && (file != STDERR_FILENO)) {
    errno = EBADF;
    return -1;
  }

  for (int idx = 0; idx < len; ++idx) {
    ITM_SendChar(data[idx]);
  }

  return len;
}
```
