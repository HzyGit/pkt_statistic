#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

static int pkt_statistic_init(void) {
	return 0;
}

static void pkt_statistic_exit(void) {
}

module_init(pkt_statistic_init);
module_exit(pkt_statistic_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zhiyuan");
MODULE_VERSION("0.1");
