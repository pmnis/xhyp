
//#ifdef printk
//#undef printk
//#endif

//extern void printk(const char *fmt, ...);
extern void _hyp_idle(void);
extern int _hyp_preempt_enable(void);
extern int _hyp_preempt_disable(void);
extern char *gets(char *);

