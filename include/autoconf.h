/*
 * Automatically generated C config: don't edit
 * Xhyp version: 
 */
#define AUTOCONF_TIMESTAMP "2013-10-10 04:49:49 CEST"

#define CONFIG_HAVE_DOT_CONFIG 1
#define ENABLE_HAVE_DOT_CONFIG 1
#define IF_HAVE_DOT_CONFIG(...) __VA_ARGS__
#define IF_NOT_HAVE_DOT_CONFIG(...)
#define CONFIG_ARM 1
#define ENABLE_ARM 1
#define IF_ARM(...) __VA_ARGS__
#define IF_NOT_ARM(...)
#undef CONFIG_LEON
#define ENABLE_LEON 0
#define IF_LEON(...)
#define IF_NOT_LEON(...) __VA_ARGS__
#undef CONFIG_MIPS
#define ENABLE_MIPS 0
#define IF_MIPS(...)
#define IF_NOT_MIPS(...) __VA_ARGS__
#undef CONFIG_SH4
#define ENABLE_SH4 0
#define IF_SH4(...)
#define IF_NOT_SH4(...) __VA_ARGS__
#undef CONFIG_X86
#define ENABLE_X86 0
#define IF_X86(...)
#define IF_NOT_X86(...) __VA_ARGS__

/*
 * Board
 */
#define CONFIG_BOARD_VERSATILE 1
#define ENABLE_BOARD_VERSATILE 1
#define IF_BOARD_VERSATILE(...) __VA_ARGS__
#define IF_NOT_BOARD_VERSATILE(...)
#undef CONFIG_BOARD_GEAM6425
#define ENABLE_BOARD_GEAM6425 0
#define IF_BOARD_GEAM6425(...)
#define IF_NOT_BOARD_GEAM6425(...) __VA_ARGS__
#undef CONFIG_BOARD_REALVIEW
#define ENABLE_BOARD_REALVIEW 0
#define IF_BOARD_REALVIEW(...)
#define IF_NOT_BOARD_REALVIEW(...) __VA_ARGS__
#undef CONFIG_BOARD_BEAGLE
#define ENABLE_BOARD_BEAGLE 0
#define IF_BOARD_BEAGLE(...)
#define IF_NOT_BOARD_BEAGLE(...) __VA_ARGS__
#define CONFIG_MEMORY_SIZE 0x08000000
#define ENABLE_MEMORY_SIZE 1
#define IF_MEMORY_SIZE(...) __VA_ARGS__
#define IF_NOT_MEMORY_SIZE(...)
#define CONFIG_PERIPH_BASE 0x10000000
#define ENABLE_PERIPH_BASE 1
#define IF_PERIPH_BASE(...) __VA_ARGS__
#define IF_NOT_PERIPH_BASE(...)
#define CONFIG_PERIPH_SIZE 0x02000000
#define ENABLE_PERIPH_SIZE 1
#define IF_PERIPH_SIZE(...) __VA_ARGS__
#define IF_NOT_PERIPH_SIZE(...)

/*
 * Hypervizor
 */
#undef CONFIG_DEBUG
#define ENABLE_DEBUG 0
#define IF_DEBUG(...)
#define IF_NOT_DEBUG(...) __VA_ARGS__
#undef CONFIG_XHYP_XIP
#define ENABLE_XHYP_XIP 0
#define IF_XHYP_XIP(...)
#define IF_NOT_XHYP_XIP(...) __VA_ARGS__
#define CONFIG_STACK_SIZE 0x1000
#define ENABLE_STACK_SIZE 1
#define IF_STACK_SIZE(...) __VA_ARGS__
#define IF_NOT_STACK_SIZE(...)
#define CONFIG_TIMER_PERIOD 100
#define ENABLE_TIMER_PERIOD 1
#define IF_TIMER_PERIOD(...) __VA_ARGS__
#define IF_NOT_TIMER_PERIOD(...)
#define CONFIG_PREEMPT_NONE 1
#define ENABLE_PREEMPT_NONE 1
#define IF_PREEMPT_NONE(...) __VA_ARGS__
#define IF_NOT_PREEMPT_NONE(...)
#undef CONFIG_PREEMPT_HYP
#define ENABLE_PREEMPT_HYP 0
#define IF_PREEMPT_HYP(...)
#define IF_NOT_PREEMPT_HYP(...) __VA_ARGS__
#undef CONFIG_SCHED_ARINC
#define ENABLE_SCHED_ARINC 0
#define IF_SCHED_ARINC(...)
#define IF_NOT_SCHED_ARINC(...) __VA_ARGS__
#define CONFIG_SCHED_POSIX 1
#define ENABLE_SCHED_POSIX 1
#define IF_SCHED_POSIX(...) __VA_ARGS__
#define IF_NOT_SCHED_POSIX(...)
#undef CONFIG_SCHED_EDF
#define ENABLE_SCHED_EDF 0
#define IF_SCHED_EDF(...)
#define IF_NOT_SCHED_EDF(...) __VA_ARGS__
#undef CONFIG_SCHED_CBS
#define ENABLE_SCHED_CBS 0
#define IF_SCHED_CBS(...)
#define IF_NOT_SCHED_CBS(...) __VA_ARGS__
#undef CONFIG_SCHED_SPORADIC
#define ENABLE_SCHED_SPORADIC 0
#define IF_SCHED_SPORADIC(...)
#define IF_NOT_SCHED_SPORADIC(...) __VA_ARGS__
#undef CONFIG_ARINC_INT
#define ENABLE_ARINC_INT 0
#define IF_ARINC_INT(...)
#define IF_NOT_ARINC_INT(...) __VA_ARGS__
#undef CONFIG_ARINC_POLL
#define ENABLE_ARINC_POLL 0
#define IF_ARINC_POLL(...)
#define IF_NOT_ARINC_POLL(...) __VA_ARGS__
#undef CONFIG_ARINC_SPORADIC
#define ENABLE_ARINC_SPORADIC 0
#define IF_ARINC_SPORADIC(...)
#define IF_NOT_ARINC_SPORADIC(...) __VA_ARGS__
