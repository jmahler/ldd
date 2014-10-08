
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

static const char PATTERN[] = "FIXME";
#define PATTERN_LEN (ARRAY_SIZE(PATTERN) - 1)

/* Test whether a pattern is in a data buffer.
 * Returns: TRUE if yes, FALSE if no.
 */
static unsigned char in_buf(const char *ptrn, const unsigned int ptrn_len,
				const char *data, const unsigned int data_len)
{
	unsigned int i;

	/* impossible to find pattern without enough data */
	if (ptrn_len > data_len)
		return 0;

	/* look for the pattern */
	for (i = 0; i < ptrn_len; i++) {
		if (*(data + i) != *(ptrn + i)) {
			break;
		}
	}
	if (i == ptrn_len)
		return 1;  /* found a match */

	/* continue searching */
	return (in_buf(ptrn, ptrn_len, data + 1, data_len - 1));
}

static unsigned int logid_fn(const struct nf_hook_ops *ops,
				struct sk_buff *skb,
				const struct net_device *in,
				const struct net_device *out,
				int (*okfn)(struct sk_buff *))
{
	if (in_buf(PATTERN, PATTERN_LEN, skb->data, skb_headlen(skb)))
		pr_debug("pattern '%s' matched\n", PATTERN);

	return NF_ACCEPT;
}

static struct nf_hook_ops logid_ops = {
	.hook = logid_fn,
	.pf = NFPROTO_IPV4,
	.hooknum = NF_INET_PRE_ROUTING,
	.priority = NF_IP_PRI_FIRST,
};

static int __init init_logid(void)
{
	return nf_register_hook(&logid_ops);
}
module_init(init_logid);

static void __exit exit_logid(void)
{
	nf_unregister_hook(&logid_ops);
}
module_exit(exit_logid);

MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");
MODULE_LICENSE("GPL");
