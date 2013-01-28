
// An example, trying to understand percpu

// include/linux/percpu-defs.h
/*
#define __PCPU_ATTRS(sec)                       \
	    __percpu __attribute__((section(PER_CPU_BASE_SECTION sec))) \
    PER_CPU_ATTRIBUTES
*/

// include/linux/percpu-defs.h
/*
#define DEFINE_PER_CPU_SECTION(type, name, sec)             \
	    __PCPU_ATTRS(sec) PER_CPU_DEF_ATTRIBUTES            \
    __typeof__(type) name
#endif
*/

// include/linux/percpu-defs.h
/*
#define DEFINE_PER_CPU(type, name)                  \
	    DEFINE_PER_CPU_SECTION(type, name, "")
*/

// include/asm/percpu.h
#define PER_CPU(var, reg)                       \
	    __percpu_mov_op %__percpu_seg:this_cpu_off, reg;        \
    lea var(reg), reg

// init/calibrate.c
//static DEFINE_PER_CPU(unsigned long, cpu_loops_per_jiffy) = { 0 };
// subst 1
//static DEFINE_PER_CPU_SECTION(unsigned long, cpu_loops_per_jiffy, "") = { 0 };
// subst 2
//static __PCPU_ATTRS("") PER_CPU_DEF_ATTRIBUTES __typeof__(unsigned long) cpu_loops_per_jiffy = { 0 };
//#define __PCPU_ATTRS(sec)
//static __percpu __attribute__((section(PER_CPU_BASE_SECTION ""))) PER_CPU_ATTRIBUTES PER_CPU_DEF_ATTRIBUTES __typeof__(unsigned long) cpu_loops_per_jiffy = { 0 };


int main(int argc, char** argv) {

//	per_cpu(cpu_loops_per_jiffy, this_cpu);

	lea unsigned long(loops_per_jiffy), loops_per_jiffy;
	return 0;
}
