
#define DEFINE_PWAIT(uid)						\
									\
static DEFINE_MUTEX(in_mtx_##uid);					\
static DEFINE_MUTEX(out_mtx_##uid);					\
									\
static void pwait_##uid(void)						\
{									\
	static int in;							\
	static int out;							\
									\
	mutex_lock(&in_mtx_##uid);					\
	if (++in >= 2) {						\
		mutex_lock(&out_mtx_##uid);				\
		out += 2;						\
		mutex_unlock(&out_mtx_##uid);				\
		in -= 2;						\
	}								\
	mutex_unlock(&in_mtx_##uid);					\
									\
	do {								\
		mutex_lock(&out_mtx_##uid);				\
		if (out) {						\
			out -= 1;					\
			mutex_unlock(&out_mtx_##uid);			\
			break;						\
		}							\
		mutex_unlock(&out_mtx_##uid);				\
	} while (1);							\
}
