# TODO list

## General
- [X] Create tasks and start them from the main function instead of calling the functions. This will enable us to select the stack size for each task (probably will prevent the stack overflow problems we've been having). Check [FreeRTOS documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html). Once this is done maybe we can do FFTs with more points?
- [ ] Start with the ADC & microphone stuff.
- [ ] Add some comments all around.

## utils.c
- [ ] Finish the `find_formants()` function and test it.

## window.c
- [X] Don't recalculate the window each time. Maybe return window or save pointer to last calculation. IDK, just optimize.
- [X] Find a good number for `nIterations` in `I0()`. Maybe lower is enough, or maybe we need a higher number.
