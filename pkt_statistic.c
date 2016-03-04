#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

#define PROC_NAME "pkt_statistic"
#define PROC_DIR "my_pkt"

const char *module_name="pkt_statistic";
#define module_version "0.1"

static struct proc_dir_entry *pkt_proc_dir=NULL;

/// 报文类型定义
enum pkt_type {
	PKT_TYPE_HTTP=0,
	PKT_TYPE_SSH,
	PKT_TYPE_DNS,
	PKT_TYPE_ICMP,
	PKT_TYPE_HTTPS,
	PKT_TYPE_COUNT,
};

const char *pkt_type_name[PKT_TYPE_COUNT]={
	"http",
	"ssh",
	"dns",
	"icmp",
	"https",
};

/// 报文统计结构
struct pkt_data {
	unsigned long pkt_len;     ///< 报文长度
	enum pkt_type type;   ///< 报文类型
	size_t count;         ///< 报文个数
	int dir;            ///< 报文长度
};

struct pkt_statistic {
	spinlock_t lock;
	unsigned long pkt_count;
	unsigned long pkt_len;
	struct pkt_data pkts[PKT_TYPE_COUNT];
};

/// @brief proc读函数
ssize_t pkt_statistic_read(struct file *filp,char * __user buff,
		size_t buflen,loff_t *offset){
	return 0;
}

/// @brief proc文件结构体
static const struct file_operations pkt_fops={
	.owner=THIS_MODULE,
	.read=pkt_statistic_read,
};

static int create_proc_files(void) {
	int ret=0;
	pkt_proc_dir=proc_mkdir(PROC_DIR,NULL);
	if(NULL==pkt_proc_dir)
		return -ENODEV;
	if(proc_create(PROC_NAME,0444,pkt_proc_dir,&pkt_fops)==NULL){
		ret=-EINVAL;
		remove_proc_entry(PROC_DIR,NULL);
	}
	return ret;
}

static void remove_proc_files(void) {
	remove_proc_entry(PROC_NAME,pkt_proc_dir);
	remove_proc_entry(PROC_DIR,NULL);
}

static int pkt_statistic_init(void) {
	int ret=create_proc_files();
	if(ret<0){
		pr_alert("%s(%s) load error!\n",module_name,module_version);
	}else{
		pr_info("%s(%s) load ok!\n",module_name,module_version);
	}
	return ret;
}

static void pkt_statistic_exit(void) {
	remove_proc_files();
	pr_info("%s(%s) unload ok!\n",module_name,module_version);
}

module_init(pkt_statistic_init);
module_exit(pkt_statistic_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("zhiyuan");
MODULE_VERSION(module_version);
