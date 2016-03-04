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

/**************pkt_statistic 报文统计信息定义************/

/// 报文类型定义
enum pkt_type {
	PKT_TYPE_HTTP=0,
	PKT_TYPE_SSH,
	PKT_TYPE_DNS,
	PKT_TYPE_ICMP,
	PKT_TYPE_HTTPS,
	PKT_TYPE_COUNT,
};

enum {
	PKT_DIR_IN=0,
	PKT_DIR_OUT,
	PKT_DIR_COUNT,
};

const char *pkt_type_name[PKT_TYPE_COUNT+1]={
	"http",
	"ssh",
	"dns",
	"icmp",
	"https",
	"error type",
};

/// 报文统计结构
struct pkt_data {
	unsigned long pkt_len;     ///< 报文长度
	enum pkt_type pkt_type;   ///< 报文类型
	unsigned long pkt_count;         ///< 报文个数
};

struct pkt_statistic {
	spinlock_t lock;
	unsigned long pkt_count[PKT_DIR_COUNT];
	unsigned long pkt_len[PKT_DIR_COUNT];
	struct pkt_data pkt_stat_data[PKT_DIR_COUNT][PKT_TYPE_COUNT]; 
} pkt_stat; 

static const char * get_pkt_type_name(enum pkt_type type) {
	if(type<0||type>=PKT_TYPE_COUNT)
			return pkt_type_name[PKT_TYPE_COUNT];
	return pkt_type_name[type];
}

static void __clean_pkt_statistic(struct pkt_statistic * pkt_stat) {
	int i=0;
	for(i=0;i<PKT_DIR_COUNT;++i){
		pkt_stat->pkt_len[i]=0;
		pkt_stat->pkt_count[i]=0;
	}
	memset(pkt_stat->pkt_stat_data,0,sizeof(pkt_stat->pkt_stat_data));
}

static void init_pkt_statistic(struct pkt_statistic * pkt_stat) {
	spin_lock_init(&pkt_stat->lock);
	__clean_pkt_statistic(pkt_stat);
}

static void clean_pkt_statistic(struct pkt_statistic * pkt_stat) {
	spin_lock_bh(&pkt_stat->lock);
	__clean_pkt_statistic(pkt_stat);
	spin_unlock_bh(&pkt_stat->lock);
}


static int __add_pkt_stat_data(struct pkt_statistic * pkt_stat,
		unsigned long pkt_len,
		enum pkt_type type,
		int dir) {

	struct pkt_data *pkt_stat_data;

	if(type<0||type>=PKT_TYPE_COUNT)
		return -EINVAL;
	if(dir<0||dir>=PKT_DIR_COUNT)
		return -EINVAL;
	pkt_stat->pkt_len[dir]=pkt_stat->pkt_len[dir]*pkt_stat->pkt_count[dir]+pkt_len;
	pkt_stat->pkt_count[dir]++;
	pkt_stat->pkt_len[dir]/=pkt_stat->pkt_count[dir];
	pkt_stat_data=&(pkt_stat->pkt_stat_data[dir][type]);
	pkt_stat_data->pkt_len=pkt_stat_data->pkt_len*pkt_stat_data->pkt_count+pkt_len;
	pkt_stat_data->pkt_len/=++pkt_stat_data->pkt_count;
	pkt_stat_data->pkt_type=type;
}

static int add_pkt_stat_data(struct pkt_statistic *pkt_stat,
		unsigned long pkt_len,
		enum pkt_type type,
		int dir) {
	int ret=0;
	spin_lock_bh(&pkt_stat->lock);
	ret=__add_pkt_stat_data(pkt_stat,pkt_len,type,dir);
	spin_unlock_bh(&pkt_stat->lock);
	return ret;
}

/*******************proc 读写实现****************/
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
