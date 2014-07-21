
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>

struct identity {
	char name[20];
	int  id;
	bool busy;

	struct list_head list;
};

LIST_HEAD(identities);

int identity_create(char *name, int id)
{
	struct identity *tmp;
	int len;

	/* make sure the string will fit */
	len = strlen(name);
	if (len >= sizeof(tmp->name))
		return -ENOMEM;

	tmp = kmalloc(sizeof(struct identity), GFP_KERNEL);
	if (NULL == tmp)
		return -ENOMEM;

	/* assign values to the new identity */
	memcpy(tmp->name, name, len + 1);
	tmp->id = id;
	tmp->busy = false;

	list_add(&tmp->list, &identities);

	return 0;
}

struct identity *identity_find(int id)
{
	struct identity *pos;

	list_for_each_entry(pos, &identities, list) {
		if (pos->id == id)
			return pos;
	}

	return NULL;
}

void identity_destroy(int id)
{
	struct identity *found;

	found = identity_find(id);
	if (NULL == found)
		return;

	list_del(&found->list);
	kfree(found);
}

void identity_destroy_all(void)
{
	struct identity *pos, *n;

	list_for_each_entry_safe(pos, n, &identities, list) {
		list_del(&pos->list);
		kfree(pos);
	}
}

static int __init linked_list_init(void)
{
	struct identity *temp;

	if (identity_create("Alice", 1))
		goto err_identity_create;
	if (identity_create("Bob", 2))
		goto err_identity_create;
	if (identity_create("Dave", 3))
		goto err_identity_create;
	if (identity_create("Gena", 10))
		goto err_identity_create;

	temp = identity_find(3);
	pr_debug("id 3 = %s\n", temp->name);

	temp = identity_find(42);
	if (temp == NULL)
		pr_debug("id 42 not found\n");

	identity_destroy(2);
	identity_destroy(1);
	identity_destroy(10);
	identity_destroy(42);
	identity_destroy(3);

	return 0;

err_identity_create:
	identity_destroy_all();
	return -ENOMEM;
}

static void __exit linked_list_exit(void)
{
	identity_destroy_all();
}

module_init(linked_list_init);
module_exit(linked_list_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jeremiah Mahler <jmmahler@gmail.com>");
