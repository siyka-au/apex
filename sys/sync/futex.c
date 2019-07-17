#include <futex.h>

#include <access.h>
#include <debug.h>
#include <errno.h>
#include <kernel.h>
#include <limits.h>
#include <sch.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <task.h>
#include <thread.h>
#include <vm.h>

#define trace(...)

/*
 * TODO: optimise search using hash map?
 * TODO: free unused futexes?
 */

static struct futex *
futex_find(struct task *t, int *uaddr)
{
	struct futex *f;
	list_for_each_entry(f, &t->futexes, task_link) {
		if (f->addr == virt_to_phys(uaddr))
			return f;
	}
	return 0;
}

static struct futex *
futex_get(struct task *t, int *uaddr)
{
	struct futex *f;

	if ((f = futex_find(t, uaddr)))
		return f;

	if (!(f = malloc(sizeof(struct futex))))
		return 0;

	f->addr = virt_to_phys(uaddr);
	event_init(&f->event, "futex", ev_LOCK);
	f->owner = 0;
	list_insert(&t->futexes, &f->task_link);

	return f;
}

static int
futex_wait(struct task *t, int *uaddr, int val, const struct timespec *ts)
{
	int err, uval;

	if ((err = vm_read(t->as, &uval, uaddr, sizeof uval)) < 0)
		return err;
	if (uval != val)
		return -EAGAIN;
	if (ts && (ts->tv_sec < 0 || ts->tv_nsec > 1000000000))
		return -EINVAL;

	trace("futex_wait th:%p uaddr:%p val:%x ns:%llu\n",
	    thread_cur(), uaddr, val, ts ? ts_to_ns(ts) : 0);

	struct futex *f;
	if (!(f = futex_get(t, uaddr)))
		return DERR(-ENOMEM);
	return sch_nanosleep(&f->event, ts ? ts_to_ns(ts) : 0);
	/* Be _very_ careful. Requeue can move us from one futex to another, so
	 * we are not necessarily waiting on 'f' anymore. */
}

static int
futex_wake(struct task *t, int *uaddr, int val)
{
	trace("futex_wake th:%p uaddr:%p val:%d\n", thread_cur(), uaddr, val);

	if (val < 0)
		return DERR(-EINVAL);
	if (val == 0)
		return 0;

	struct futex *f;
	if (!(f = futex_find(t, uaddr)))
		return 0;

	if (val == 1) {
		sch_wakeone(&f->event);
		return 1;
	}

	if (val == INT_MAX)
		return sch_wakeup(&f->event, 0);

	int n = val;
	while (n && sch_wakeone(&f->event)) --n;
	return val - n;
}

static int
futex_requeue(struct task *t, int *uaddr, int val, int val2, int *uaddr2)
{
	trace("futex_requeue th:%p uaddr:%p val:%d val2:%d uaddr2:%p\n",
	    thread_cur(), uaddr, val, val2, uaddr2);

	if (val < 0 || val2 < 0)
		return DERR(-EINVAL);

	struct futex *l;
	if (!(l = futex_find(t, uaddr)))
		return 0;

	int n = val;
	while (n && sch_wakeone(&l->event)) --n;

	if (val2) {
		struct futex *r;
		if (!(r = futex_get(t, uaddr2)))
			return DERR(-ENOMEM);

		while (val2-- && sch_requeue(&l->event, &r->event));
	}

	return val - n;
}

int
futex(struct task *t, int *uaddr, int op, int val, void *val2, int *uaddr2)
{
	if ((op & FUTEX_OP_MASK) == FUTEX_REQUEUE && !u_addressfor(t->as, uaddr2))
		return DERR(-EFAULT);

	/* no support for realtime clock */
	if ((op & FUTEX_CLOCK_REALTIME))
		return DERR(-ENOSYS);

	if (!(op & FUTEX_PRIVATE))
		dbg("WARNING: shared futexes not yet supported\n");

	int ret;
	sch_lock();

	switch (op & FUTEX_OP_MASK) {
	case FUTEX_WAIT:
		ret = futex_wait(t, uaddr, val, val2);
		break;
	case FUTEX_WAKE:
		ret = futex_wake(t, uaddr, val);
		break;
	case FUTEX_REQUEUE:
		ret = futex_requeue(t, uaddr, val, (int)val2, uaddr2);
		break;
	/*
	 * TODO(FUTEX_LOCK_PI)
	 * TODO(FUTEX_UNOCK_PI)
	 */
	default:
		ret = DERR(-ENOTSUP);
	}

	sch_unlock();
	return ret;
}

int
sc_futex(int *uaddr, int op, int val, void *val2, int *uaddr2)
{
	int ret;
	struct timespec ts;

	/* copy in userspace timespec */
	switch (op & FUTEX_OP_MASK) {
	case FUTEX_WAIT:
		if (!val2)
			break;
		if ((ret = vm_read(task_cur()->as, &ts, val2, sizeof(ts))) < 0)
			return ret;
		val2 = &ts;
	}

	return futex(task_cur(), uaddr, op, val, val2, uaddr2);
}
