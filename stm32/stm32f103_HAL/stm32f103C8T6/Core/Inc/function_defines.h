# ifndef FUNTION_DEFINES_H
# define FUNTION_DEFINES_H

/**
  * @brief  Macro to run a code block every specified interval in milliseconds.
  * @param  tick_var: A variable to store the last tick time (must be of type uint32_t).
  * @param  interval: The interval in milliseconds to run the code block.
  * @param  code_block: The code block to execute (should be a lambda or function call).
  */
#define RUN_EVERY_MS(tick_var, interval, code_block)                           \
  do                                                                           \
  {                                                                            \
    if (HAL_GetTick() - (tick_var) >= (interval))                              \
    {                                                                          \
      {code_block}(tick_var) = HAL_GetTick();                                  \
    }                                                                          \
  } while (0)












# endif /* FUNTION_DEFINES_H */