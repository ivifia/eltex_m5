#include <linux/module.h> 
#include <linux/printk.h> 
#include <linux/kobject.h> 
#include <linux/sysfs.h> 
#include <linux/init.h> 
#include <linux/fs.h> 
#include <linux/string.h>
#include<linux/kernel.h>
#include<linux/netfilter_ipv4.h>
#include<linux/skbuff.h>
#include<linux/ip.h>
#include<linux/inet.h>

#define MAX_IPS 100

MODULE_AUTHOR("Bakuk Aleksandr");
MODULE_DESCRIPTION("Basic netfilter module");
MODULE_LICENSE("GPL");
 
static struct kobject *example_kobject;
static struct nf_hook_ops nfin;
static __be32 block_ips[MAX_IPS];
static int block_count=0;

static ssize_t foo_show(struct kobject *kobj, struct kobj_attribute *attr,
                      char *buf)
{
    size_t len=0;
    for (int i=0;i<block_count;i++) {
        len+=sprintf(buf+len, "%pI4\n", &block_ips[i]);
    }
    return len;
        
}
 
static ssize_t foo_store(struct kobject *kobj, struct kobj_attribute *attr,
                      const char *buf, size_t count)
{
    char tmp_buf[MAX_IPS];
    char *begin,*curr= tmp_buf;
    __be32 parsed_ip;
    strscpy(tmp_buf, buf, count + 1);
    while ((begin = strsep(&curr, " \n\r")) != NULL) { 
        if (in4_pton(begin, strlen(begin), (u8 *)&parsed_ip, -1, NULL) == 1) {
            block_ips[block_count] = parsed_ip;
            block_count++;
        }
    }
    return count;
        
}

static ssize_t foo_delete(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    char tmp_buf[MAX_IPS];
    char *begin, *curr = tmp_buf;
    __be32 parsed_ip;
    int i, j;


    strscpy(tmp_buf, buf, count + 1);

    while ((begin = strsep(&curr, " \n\r")) != NULL) { 

        if (in4_pton(begin, strlen(begin), (u8 *)&parsed_ip, -1, NULL) == 1) {
           
            for (i = 0; i < block_count; i++) {
                if (block_ips[i] == parsed_ip) {
                    for (j = i; j < block_count - 1; j++) {
                        block_ips[j] = block_ips[j + 1];
                    }
                    block_count--;
                    i--; 
                }
            }
        }
    }
    return count;
}
 
static unsigned int hook_func_in(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
 
{
    struct ethhdr *eth;
    struct iphdr *ip_header;
    eth = (struct ethhdr*)skb_mac_header(skb);
    ip_header = (struct iphdr *)skb_network_header(skb);
    for (int i=0;i<block_count;i++) {
        if (ip_header->saddr==block_ips[i]) return NF_DROP;
    }
    printk(KERN_INFO "src mac %pM, dst mac %pM\n", eth->h_source, eth->h_dest);
    printk(KERN_INFO "src IP addr: %pI4\n", &ip_header->saddr);
    return NF_ACCEPT;
}
 
static struct kobj_attribute foo_attribute =__ATTR(test, 0660, foo_show,
                                                   foo_store);
static struct kobj_attribute delete_attribute = __ATTR(delete, 0660, NULL, foo_delete);
 
static int __init sys_init (void)
{
        int error = 0;
 
        pr_debug("Module initialized successfully \n");
 
        example_kobject = kobject_create_and_add("systest",
                                                 kernel_kobj);
        if(!example_kobject)
                return -ENOMEM;
 
        error = sysfs_create_file(example_kobject, &foo_attribute.attr);
        if (error) {
                pr_debug("failed to create the foo file in /sys/kernel/systest \n");
                return error;
        }

        error = sysfs_create_file(example_kobject, &delete_attribute.attr);
        if (error) {
                pr_debug("failed to create the 'delete' file in /sys/kernel/systest \n");
                return error;
        }

        nfin.hook     = hook_func_in;
        nfin.hooknum  = NF_INET_PRE_ROUTING;
        nfin.pf       = PF_INET;
        nfin.priority = NF_IP_PRI_FIRST;
        nf_register_net_hook(&init_net, &nfin); 
 
        return 0;
}
 
static void __exit sys_exit (void)
{
        pr_debug ("Module un initialized successfully \n");
        kobject_put(example_kobject);
        nf_unregister_net_hook(&init_net, &nfin); 
}
module_init(sys_init);
module_exit(sys_exit);