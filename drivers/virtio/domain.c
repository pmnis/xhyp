#include <sys/shared_page.h>
#include <sys/xhyp.h>
#include <xhyp/serial.h>
#include <xhyp/irq.h>
#include <xhyp/stdlib.h>
#include <xhyp/domains.h>
#include <xhyp/hyp.h>
#include <xhyp/event.h>
#include <xhyp/virtio.h>

#include <xhyp/arinc.h>

#include <autoconf.h>

#include "hyp.h"

#undef NB_DOMAIN
#define NB_DOMAIN	16

struct shared_page *xhyp_sp = (struct shared_page *) (0x02000000);

#define wait_next_period()		_hyp_idle()

#define IRQ_STACK_SIZE	1024
static unsigned long irq_stack[IRQ_STACK_SIZE];

extern void msleep(int);
extern int _hyp_hyp(int cmd, int nb, void *);

#define MAX_BUF	256
char fifo_buffer[MAX_BUF];
char cmd_buffer[MAX_BUF];
char colors[10] = "0m";
#define COLOR_BLACK     "[30m"
#define COLOR   "[3"
int color_on = 0;

int show_domain[NB_QUEUING_PORT];

struct command {
	char *cmd;
	int (*func)(char *, char *);
};

int cmd_help(char *s, char *args)
{
	printk("help\n");
	printk("help        : This text \n");
	printk("show        : virtio status\n");
	printk("virtio DID  : virtio usage for domain DID\n");
	printk("pc DID      : print console for domain DID\n");
	printk("console DID : give console to domain DID\n");
	printk("ps          : domains status\n");
	printk("psl         : domains long status\n");
	printk("start DID   : start a domain\n");
	printk("stop  DID   : stop a domain\n");
	printk("dmesg NB    : NB trace events\n");
	printk("events      : per domain events count\n");
	printk("ag          : get current ARINC plan\n");
	printk("as          : set new ARINC plan\n");
	printk("color       : set coloring\n");
	return 0;
}

int cmd_fifo(char *s, char *args)
{
	int i;
	struct queuing_port *qp;

	printk("bytes in virtio\n");

	for (i = 0, qp = xhyp_sp->qp; i < 8; i++, qp++) {
		printk("[%d] %4d %4d   ", i, qp->fifo.ri, qp->fifo.wi);
		if (i % 2) printk("\n");
	}
	return 0;
}

int cmd_pc(char *s, char *args)
{
	int id;
	int n;

	if (!args) {
		return cmd_fifo(s, args);
	}
	n = sscanf(args, "%d", &id);

	if (n != 1) {
		printk("Usage: pc [-] DID\n");
		return 1;
	}
	if (id < 0) {
		id = -id;
		id = 2*id + 1;
		show_domain[id] = 0;
	} else {
		id = 2*id + 1;
		show_domain[id] = 1;
	}

	return 0;
}


int cmd_show(char *s, char *args)
{
	int n;

	for (n = 0; n < NB_QUEUING_PORT; n++) {
		if (!(n % 20)) printk("\n");
		printk("%d ", show_domain[n]); 
	}
	printk("\n");

	return 0;
}

struct major_frame mf;
struct major_frame plan[5] = {
	{},
	{
        .minor_count = 2,
        .frame_start = 0,
        .frame_period = 2,
        .frame_size = 2,
        .minor = {
                {
                .dom_id = 4,
                .slot_start = 0,
                .slot_size = 1,
                },
                {
                .dom_id = 1,
                .slot_start = 0,
                .slot_size = 1,
                },
	   }
	},
	{},
	{},
	{},
};

int first_time = 1;
int cmd_major_set(char *s, char *args)
{
	int n;
	int p = -1;

	if (first_time) {
		first_time = 0;
		_hyp_hyp(HYPCMD_GET_PLAN, 0, &plan[0]);
	}

	if (!args)
		p = 1;
	else 
		n = sscanf(args, "%d", &p);

	if (n != 1 || p > 4) {
		printk("Invalid plan number %d\n", p);
		return 0;
	}

	if (plan[p].minor_count)
		_hyp_hyp(HYPCMD_SET_PLAN, 0, &plan[p]);
	else
		printk("Unknown plan number %d\n", p);

	return 0;
}

int cmd_major_get(char *s, char *args)
{
	int i;
	struct minor_frame *f;

	if (_hyp_hyp(HYPCMD_GET_PLAN, 0, &mf)) {
		printk("Major frame period %d start %d size %d\n", mf.frame_period, mf.frame_start, mf.frame_size);
		printk("%04s %03s %05s %04s\n", "id", "pid", "start", "size");
		for (i = 0; i < mf.minor_count; i++) {
			f = &mf.minor[i];
			colors[0] = '0' + f->dom_id;
			if (color_on)
				printk("%s%s", COLOR, colors);
			printk("[%02d] %3d %5d %4d\n", i, f->dom_id, f->slot_start, f->slot_size);
		}
	if (color_on)
		printk(COLOR_BLACK);
	}
	return 0;
}

char *ps_str_type[] = { "H", "D", "R", "G" };
char *ps_str_mode[] = { "N", "S", "I", "U" };
unsigned long total = 0;
unsigned long d_time[NB_DOMAINS];
unsigned long s_time[NB_DOMAINS];
unsigned long u_time[NB_DOMAINS];

char *ps_str_state(unsigned long state)
{
	switch (state) {
	case DSTATE_NONE:	return "N";
	case DSTATE_READY:	return "Y";
	case DSTATE_RUN:	return "R";
	case DSTATE_SLEEP:	return "S";
	case DSTATE_STOP:	return "P";
	case DSTATE_DEAD:	return "D";
	}
	return "-";
}

int cmd_ps(char *s, char *args)
{
	int i;
	struct domain d;

	printk("  ID Tp St Pr ");
	printk("%11s %11s %11s %11s %-11s %-4s\n", "IRQ", "ABT", "SYS", "USR", "Name", "%USE");
	for (i = 0; i < NB_DOMAINS; i++) {
		if (_hyp_hyp(HYPCMD_DOM_GET, i, &d)) {
			s_time[i] = (d.t_irq.tv_sec + d.t_abt.tv_sec + d.t_sys.tv_sec + d.t_usr.tv_sec);
			u_time[i] = d.t_irq.tv_usec + d.t_abt.tv_usec + d.t_sys.tv_usec + d.t_usr.tv_usec;
			total += s_time[i];
		}
	}
	total = 0;
	for (i = 0; i < NB_DOMAINS; i++) {
		d_time[i] = s_time[i];
		total += d_time[i];
	}
	for (i = 0; i < NB_DOMAINS; i++) {
		if (_hyp_hyp(HYPCMD_DOM_GET, i, &d)) {
			colors[0] = '0' + i;
			if (color_on)
				printk("%s%s", COLOR, colors);
			printk("[%2d] %2s %2s",
				i, ps_str_type[d.type], ps_str_state(d.state));
			printk(" %2d", d.prio);
			printk(" %7u.%03lu", d.t_irq.tv_sec, d.t_irq.tv_usec/1000);
			printk(" %7u.%03lu", d.t_abt.tv_sec, d.t_abt.tv_usec/1000);
			printk(" %7u.%03lu", d.t_sys.tv_sec, d.t_sys.tv_usec/1000);
			printk(" %7u.%03lu", d.t_usr.tv_sec, d.t_usr.tv_usec/1000);
			printk(" %-11s", d.name);
			if (total) printk(" %4lu", (100 * d_time[i])/total);
			else printk(" %4lu", 0);
			printk("\n");
		}
	}
	if (color_on)
		printk(COLOR_BLACK);
	return 0;
}

int cmd_psl(char *s, char *args)
{
	int i;
	struct domain d;

	//printk("SP at %p size SP: %d\n", xhyp_sp, sizeof(struct shared_page));
	printk("  ID Tp St Md oM base_add load_add   rights ");
	printk("Pr Bg Pd     sl     ir     nr  hy  irq_mask irq_enab irq_pend Name"
		"\n");
	for (i = 0; i < NB_DOMAINS; i++) {
		if (_hyp_hyp(HYPCMD_DOM_GET, i, &d)) {
			colors[0] = '0' + i;
			if (color_on)
				printk("%s%s", COLOR, colors);
			printk("[%2d] %2s %2s %2s %2s",
				i, ps_str_type[d.type], ps_str_state(d.state),
				ps_str_mode[d.mode], ps_str_mode[d.old_mode]);
			printk(" %08x %08x %08x", 
				 d.base_addr, d.load_addr, d.rights);
			printk(" %2d %2d %2d", 
				 d.prio, d.budget, d.period);
			printk(" %6d %6d %6d %3d", 
				 d.slices, d.irq, d.nb_hypercalls, d.hypercall);
			printk("  %08x %08x %08x", 
				 d.d_irq_mask, d.d_irq_enabled, d.d_irq_pending);
			printk(" %-08s ", d.name);
			printk("\n");
		}
	}
	if (color_on)
		printk(COLOR_BLACK);
	return 0;
}

int cmd_color(char *s, char *args)
{
	int val;
	int n;

	if (!args)
		return 0;

	n = sscanf(args, "%d", &val);
	if (n != 1) {
		printk("Erreur : bad arg %s\n", args);
		return 1;
	}
	color_on = (val)? 1 : 0;
	return 0;
}

int cmd_events(char *s, char *args)
{
	_hyp_hyp(HYPCMD_EVENTS, 0, NULL);
	return 0;
}

int togle(int on, char *args)
{
	int id;
	int n;

	if (!args) {
		printk("Erreur : bad DID\n");
		return 1;
	}

	n = sscanf(args, "%d", &id);
	if (n != 1) {
		printk("Erreur : bad DID\n");
		return 1;
	}
	if (on)
		_hyp_hyp(HYPCMD_DOM_RESTART, id, NULL);
	else
		_hyp_hyp(HYPCMD_DOM_STOP, id, NULL);
	return 0;
}

int cmd_start(char *s, char *args)
{
	return togle(1, args);
}
int cmd_stop(char *s, char *args)
{
	return togle(0, args);
}

char *mode_str[DMODE_SIZE] = {
	"DEAD", "SVC", "IRQ", "USR", "ABT", "UND", "FIQ"
};

char *event_str[] = {
	"start", "sched_in", "sched_out", "irq", "irq_in", "irq_out",
	"irq_ret", "sys_in", "sys_out", "abt", "abt_in", "abt_out",
	"abt_ret", "wfi"
};

void event_show(struct event *event)
{
	int sec;
	int usec;

	sec = event->timestamp.tv_sec;
	usec = event->timestamp.tv_usec;

	printk("%6d.%06d %5d %2d %08lx %12s %2d %08s %08s\n", sec, usec, event->nr,
		event->id, event->state,
		event_str[event->event], event->hypercall,
		mode_str[event->c_mode], mode_str[event->o_mode]);
}

#define MAX_EVENTS	64
struct event event[MAX_EVENTS];

int cmd_dmesg(char *s, char *args)
{
	int retval;
	int n;

	retval = sscanf(args, "%d", &n);
	if (retval != 1)
		n = 1;

	printk("Messages:\n");
	if ((retval = _hyp_hyp(HYPCMD_DMESG, n, event))) {
		for (n = 0; n < retval; n++)
			event_show(&event[n]);
	}
	return 0;
}

struct command command[] = {
	{"help",	cmd_help},
	{"events",	cmd_events},
	{"color",	cmd_color},
	{"show",	cmd_show},
	{"virtio",	cmd_fifo},
	{"ps",		cmd_ps},
	{"psl",		cmd_psl},
	{"as",		cmd_major_set},
	{"ag",		cmd_major_get},
	{"stop",	cmd_stop},
	{"start",	cmd_start},
	{"dmesg",	cmd_dmesg},
	{"pc",		cmd_pc},
	{"console",	cmd_pc},
};
void mstrntok(char *s, char *c, char **next, int n)
{
	char *q;

	while(n--) {
		q = c;
		while(*q) {
			if (*s == *q)
				goto got_it;
			q++;
		}
		s++;
	}
	*next = NULL;
	return;
got_it:
	*s = 0;
	if (*(s + 1))
		*next = s + 1;
	else
		*next = NULL;
}
#define LINE_SIZE 128

void interprete(char *s)
{
	struct command *p = command;
	char cmd[LINE_SIZE];
	char *ptr = NULL;

	for(p = command; p < command + (sizeof(command)/sizeof(*p)); p++) {
		strncpy(cmd, s, LINE_SIZE);
		mstrntok(cmd, " \n", &ptr, LINE_SIZE);
		if (!strcmp(cmd, p->cmd)) {
			p->func(s, ptr);
			return;
		}
	}
	if (strlen(s))
		printk("Unknown command: %s\n", s);
	else
		printk("\n");
}

void serial_irq(void)
{
	unsigned long status;

	_hyp_console("serial_irq\n", 11);
	status = chip->rirq;
	printk("Serial status %08lx\n", status);
	if (status) {
		printk("Serial status %08lx\n", status);
	}
}


void poll_virtio(void )
{
	struct queuing_port *qp;
	int i;
	int n;

	for (i = 0, qp = xhyp_sp->qp; i < NB_QUEUING_PORT; i++, qp++) {
		if (!(qp->flags & QPORT_TO_DRV))
			continue;
		if (!show_domain[i])
			continue;
		if (fifo_empty(&qp->fifo)) {
			continue;
		}
		//printk("%d\n", qp->remote);
		_hyp_preempt_disable();
		while ((n = fifo_get(&qp->fifo, fifo_buffer, 64)) > 0) {
			fifo_buffer[n] = 0;
			colors[0] = '0' + qp->remote;
			if (color_on)
				printk("%s%s", COLOR, colors);
			printk(fifo_buffer);
			if (color_on)
				printk(COLOR_BLACK);
		}
		_hyp_preempt_enable();
	}
}

void handler(unsigned long msk)
{
	unsigned long mask = xhyp_sp->v_irq_pending;

	xhyp_sp->v_irq_pending = 0;
	if (mask & IRQ_MASK_TIMER)
		poll_virtio();
#ifndef CONFIG_ARINC
	if (mask & IRQ_MASK_VIRTIO)
		poll_virtio();
	if (mask & IRQ_MASK_UART)
		serial_irq();
	if (mask & IRQ_MASK_EVENT)
		printk("Event on sampling port: %08lx\n", xhyp_sp->sampling_port);
#endif

	/* acknowledge the irq	*/
	xhyp_sp->v_irq_ack = mask;
	/* Return from interrupt	*/
	_hyp_irq_return(0);
}

void virtio_init(void)
{
	struct queuing_port *qp;
	int i;

	for (i = 0, qp = xhyp_sp->qp; i < (NB_QUEUING_PORT/2); i++, qp++) {
		/* Do not overwrite ready ports			*/
		if (qp->flags & QPORT_READY)
			continue;
		/* QPORT_OUT means init write queue for us	*/
		queuing_port_init(qp, QPORT_OUT);
		/* FROM_DRV means data comes from the driver	*/
		qp->flags |= QPORT_READY|QPORT_FROM_DRV;
		/* same with the input port			*/
		qp++;
		queuing_port_init(qp, QPORT_IN);
		qp->flags |= QPORT_READY|QPORT_TO_DRV;
	}
}
void show_irqs(char *p)
{
	sprintf(p, "msk: %08lx enab: %08lx\n", xhyp_sp->v_irq_mask, xhyp_sp->v_irq_enabled);
	_hyp_console(p, strlen(p));
}

void start_kernel(void)
{
	char *s;
	int f;

	f = IRQ_mask(-1);

	virtio_init();

	_hyp_irq_request(handler, irq_stack + STACK_SIZE);
	_hyp_irq_enable(IRQ_MASK_TIMER);
#ifndef CONFIG_ARINC
	_hyp_irq_enable(IRQ_MASK_UART);
#endif
	_hyp_irq_enable(IRQ_MASK_EVENT);
	_hyp_irq_enable(IRQ_MASK_QPORT);
	f &= ~(IRQ_MASK_TIMER|IRQ_MASK_EVENT|IRQ_MASK_QPORT);
	IRQ_mask(f);

	colors[0] = '4';
	if (color_on)
		printk("%s%s", COLOR, colors);
	printk("x-hyp: color demo serial interface started SP: %08lx\n", xhyp_sp);
        printk("Shared page: %08lx\n", xhyp_sp->magic);
	if (xhyp_sp->magic != XHYP_SP_MAGIC) {
		printk("BAD MAGIC\n");
		while (1) {IRQ_mask(-1); _hyp_idle(); }
	}
	if (color_on)
		printk(COLOR_BLACK);
	serial_init();

	printk("Hello\n");

	_hyp_irq_enable(0);
	while(1) {
		f = IRQ_mask(-1);
		printk("x-hyp: ");
		IRQ_mask(0);
		s = gets(cmd_buffer);
		f = IRQ_mask(-1);
		interprete(s);
#ifdef CONFIG_ARINC
		printk("Serial poll\n");
		poll_virtio();
#endif
		IRQ_mask(0);
		wait_next_period();
	}
}

