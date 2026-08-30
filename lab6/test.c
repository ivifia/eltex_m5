/**
Basic netfilter module
for Linux kernel(4.8 and 4.15) work!
 
Makefile
obj-m += nf.o
all:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
*/
 
#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/netfilter_ipv4.h>
#include<linux/skbuff.h>
#include<linux/ip.h>
#include<linux/inet.h>
 
//#include<linux/netfilter.h>
//#include<linux/udp.h>
//#include<linux/icmp.h>
 
MODULE_AUTHOR("Nick Chi");
MODULE_DESCRIPTION("Basic netfilter module");
MODULE_LICENSE("GPL");
 
static struct nf_hook_ops nfin;
 
static unsigned int hook_func_in(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
 
{
    struct ethhdr *eth;
    struct iphdr *ip_header;
 
    eth = (struct ethhdr*)skb_mac_header(skb);
    ip_header = (struct iphdr *)skb_network_header(skb);
    printk(KERN_INFO "src mac %pM, dst mac %pM\n", eth->h_source, eth->h_dest);
    printk(KERN_INFO "src IP addr: %pI4\n", &ip_header->saddr);
    return NF_ACCEPT;
}
static int __init init_main(void)
{
    nfin.hook     = hook_func_in;
    nfin.hooknum  = NF_INET_PRE_ROUTING;
    nfin.pf       = PF_INET;
    nfin.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &nfin); 
    
    return 0;
}
 
 
 
static void __exit cleanup_main(void)
{
    nf_unregister_net_hook(&init_net, &nfin); 
  
}
module_init(init_main);
module_exit(cleanup_main);