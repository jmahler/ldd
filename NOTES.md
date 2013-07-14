
# Linux Device Driver Notes

kernel 3.2


## time, jiffies, HZ

\cite[Pg. 64]{Essential Linux Device Drivers}

    HZ [Jiffies/s]

    unsigned long timeout = jiffies + (3*HZ);  // 3 seconds
    time_after(jiffies, timeout);

    time_before();
    time_before_eq();
    time_after_eq();

    // sleep wait instead of busy wait
    schedule_timeout(timeout);

    #include <linux/timer.h>

TSC - Time Stamp Counter

    low_tsc_ticks
    high_tsc_ticks

### Short Delays (sub jiffy)

Short delays must busy wait.

    mdelay(), udelay(), ndelay()

### Real Time Clock

Tracks absoloute time, nonvolatile memory.

## init, exit

Basic init and exit operations

static void __init name_init();

__init - tells kernel to drop the function after initialization
__initdata - tells kernel to drop the data after initialization
\cite[Pg. 51]{LDD}

    static void __exit name_exit(void);

    module_init(name_init);
    module_exit(name_exit);


Things created during init() must be released during exit().
The following are some of the pairs.

    // get a device major number
    alloc_chrdev_region();
    register_chrdev_region();
    --
    unregister_chrdev_region();

    // allocate our device specific structure
    kmalloc()
    kfree();

    // setup the cdev struc file_operations
    cdev_init();
    // add our cdev to the kernel
    cdev_add();
    cdev_del();

    // notify udev; to create the devices
    device_create();
    device_destroy();

    // create sysfs entries
    class_create();
    class_destroy();

    pci_alloc_consistent()
    pci_free_consistent()

    pci_enable_device()
    pci_disable_device()

## current process

The kernel can get information about the current process
that is accessing the driver \cite[Pg. 42]{LDD}

    #include <asm/current.h>
    #include <linux/sched.h>

    printk(KERN_INFO "The process is \"%s\" (pid %i)\n",
           current->comm, current->pid);

## PCI

struct pci_device_id
\cite[Pg. 142]{ELDD}

## interrupts

Normally irq requested when device open and freed
when closed.  Not during module load and remove.

    request_irq();
    free_irq(int irq, void *dev_id);

    enable_irq(IRQ_NUMBER);
    disable_irq(IRQ_NUMBER);
    disable_irq_nosync(IRQ_NUMBER);

Shared interrupts, IRQ_HANDLED, IRQ_NONE (irqreturn_t).

PCI express should use MSI interrupts.

    unsigned long flags;
  
    // disable interrupts, save flags
    local_irq_save(flags);
  
    // enable interrupts, restore flags
    local_irq_restore(flags);

### bottom half scheduling

These schedule processing of the fat bottom half when
called from the skinny top half (interrupt handler).

#### Softirqs

Softirqs are only used for very special situations \cite[Pg. 131]{ELDD}

#### Tasklets

Cannot go to sleep \cite[Pg. 134]{ELDD}

    (inside __init)
    tasklet_init(&dev_struct->tsklt, fn, dev);

    (inside _interrupt)
    tasklet_schedule(&dev_struct->tsklt);

\cite[Pg. 133]{ELDD}

    tasklet_enable();
    tasklet_disable();
    tasklet_disable_nosync();

May become obsolete in the future.
Functions may need to be converted to Softirqs
or Work queus.  \cite[Pg. 135]{ELDD}

#### Work queues

Can go to sleep.

### spurrious interrupts

\cite[Pg. 132]{ELDD}

### TODO how to switch to polled mode?

## spinlocks, mutexes, semaphores

Why are spinlocks/mutexes and semaphores needed?
  Because with multicore cpus it is possible for
  more than one CPU to try and access a single resource.

Both interrupts and spinlocks/mutexes need to be employed
to properly protect a critical section with SMP processors.

Also, consider two process, one in interrupt context and
the other in regular process context.  Spinlocks are needed
to prevent them from both accessing the critical section.

    unsigned long flags;
  
    // disable interrupts, spin lock, save flags
    spin_lock_irqsave(&mylock, flags);
  
    // restore interrupts, release spin lock
    spin_unlock_irqrestore(&mylock, flags);

### spinlocks

    #include <linux/spinlock.h>
  
    spinlock_t lock = SPIN_LOCK_UNLOCKED;
  
    spin_lock(&lock);
    spin_unlock(&lock);

Cannot use copy_to_user() or copy_from_user() because
they may sleep.

spinlocks put threads in to a spin, mutexes put threads to sleep

spinlocks (short wait)

spinlocks are used inside interrupt handlers

For read and write operations, which can operate concurrently.

### reader/writer locks

  rwlock_t myrwlock = RW_LOCK_UNLOCKED;
  
    read_lock(&myrwlock);
    // critical region
    read_unlock(&myrwlock);

    // irq variant
    read_lock_irqsave();
    read_lock_irqrestore();

    write_lock(&myrwlock);
    // critical region
    write_unlock(&myrwlock);

    // irq variant
    write_lock_irqsave();
    write_lock_irqrestore();

#### Sequence locks

Sequence locks are reader/writer locks where the writers are
favored over readers.  Used when write operations far outnumber
read operations.

    unsigned long seq;
    u64 ret;

    do {
      read_seqbegin(&xtime_lock);
      ret = jiffies_64;
    } while (read_seqretry(&xtime_lock, seq);

    return ret;

    write_seqlock();
    write_sequnlock();

Read-Copy Update (RCU)
When readers outnumber writers.

### mutexes

spinlocks put threads in to a spin, mutexes put threads to sleep

mutexes (long wait)

The mutex interface replaces the older semaphore interface.

    #include <linux/mutex.h>
  
    static DEFINE_MUTEX(mymutex);
  
    mutex_lock(&mymutex);
  
    mutex_unlock(&mymutex);

### semaphores

Semaphore interface is old, mutexes should be used instead.

    #include <asm/semaphore.h>
  
    static DECLARE_MUTEX(mysem);
  
    // acquire
    down(&mysem);
  
    // release
    up(&mysem);

## memory allocation

kmalloc()

GFP_KERNEL
GFP_ATOMIC

kzalloc()

vmalloc()
  (Commonly used with DMA)

look aside buffers, slabs, mempools

## kernel threads, work queues

kernel_thread() is depreceated in favor of the kthread API

    ret = kernel_thread(mykthread, NULL,
                        CLONE_FS | CLONE_FILES | CLONE_SIGNHAND | SIGCHLD);

    daemonize();
    allow_signal();
    signal_pending();

## atomic operators

    #include <include/asm-your-arch/atomic.h>

    atomic_sub_return()

    set_bit()

    clear_bit()

    test_and_set_bit()

## managing drivers (udev)

insmod, modprobe
rmmod

procfs - kernel internals

Sysfs - kernel device model
/sys

    $ udevinfo -a -p /sys/block/sr0

    $ udevmonitor --env

/etc/udev/rules.d/

## char drivers

A character driver provides sequential (not random) access to a device.

Char drivers will have a 'c' at the begining of the perimissions
when listed in /dev

    init()
    open()
    read()
    ioctl()
    llseek()
    write()

a per device structure
struct cdev  (usually in the per device structure)
struct file_operations

### data transfer, kernel space, user space

(read(), write())
Buffers in user space cannot be accessed directly from
kernel space and vice versa.  To copy data copy_from_user()
and copy_to_user() must be used.

For single variables instead of blocks use:
get_user(), put_user()

Architecture independent functions for copying data

    in[b|w|l|sb|sl]()
    out[b|w|l|sb|sl]()

\cite[Pg. 161]{ELDD}

## stacking
## misc drivers

misc_register()

## optimizations

    likely()
    unlikely()

Suggestions to the compiler as to how likely a
predicate is.

    if (likely(1))
