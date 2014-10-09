
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/textsearch.h>

static const char PATTERN[] = "FIXME";
#define PATTERN_LEN (ARRAY_SIZE(PATTERN) - 1)

static struct ts_config *search;
static struct ts_state state;

static unsigned int logid_fn(const struct nf_hook_ops *ops,
				struct sk_buff *skb,
				const struct net_device *in,
				const struct net_device *out,
				int (*okfn)(struct sk_buff *))
{
	if (UINT_MAX != skb_find_text(skb, 0, skb->len, search, &state))
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
	int ret = 0;

	search = textsearch_prepare("kmp", PATTERN, PATTERN_LEN, GFP_KERNEL,
							TS_AUTOLOAD);
	if (IS_ERR(search)) {
		ret = PTR_ERR(search);
		goto textsearch_prepare_failed;
	}

	ret = nf_register_hook(&logid_ops);
	if (ret)
		goto nf_register_hook_failed;

	return ret;

nf_register_hook_failed:
	textsearch_destroy(search);
textsearch_prepare_failed:

	return ret;
}
module_init(init_logid);

static void __exit exit_logid(void)
{
	nf_unregister_hook(&logid_ops);
	textsearch_destroy(search);
}
module_exit(exit_logid);

MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");
MODULE_LICENSE("GPL");
