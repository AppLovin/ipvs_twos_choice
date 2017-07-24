/*
 * IPVS:        Power of Twos Choice Scheduling module
 *
 * Authors:     Darby Payne <darby.payne@applovin.com>
 *
 *              This program is free software; you can redistribute it and/or
 *              modify it under the terms of the GNU General Public License
 *              as published by the Free Software Foundation; either version
 *              2 of the License, or (at your option) any later version.
 *
 */

#define KMSG_COMPONENT "IPVS"
#define pr_fmt(fmt) KMSG_COMPONENT ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/random.h>

#include <net/ip_vs.h>

/*
 *    Power of Twos Choice scheduling
 */
static struct ip_vs_dest *ip_vs_twos_schedule(struct ip_vs_service *svc,
                                              const struct sk_buff *skb,
                                              struct ip_vs_iphdr *iph) {
  struct ip_vs_dest *dest, *choice1 = NULL, *choice2 = NULL;
  int rweight1, rweight2, weight1 = 0, weight2 = 0, total_weight = 0,
                          weight = 0;

  IP_VS_DBG(6, "ip_vs_twos_schedule(): Scheduling...\n");

  /*
   * Generate a random weight between [0,sum of all weights)
   */
  list_for_each_entry_rcu(dest, &svc->destinations, n_list) {
    if (!(dest->flags & IP_VS_DEST_F_OVERLOAD)) {
      weight = atomic_read(&dest->weight);
      if (weight > 0) {
        total_weight += weight;
        choice1 = dest;
      }
    }
  }

  if (choice1 == NULL) {
    ip_vs_scheduler_err(svc, "no destination available");
    return NULL;
  }

  rweight1 = prandom_u32() % total_weight;

  /*
   * Find the first weighted dest
   */
  list_for_each_entry_rcu(dest, &svc->destinations, n_list) {
    if (!(dest->flags & IP_VS_DEST_F_OVERLOAD)) {
      weight = atomic_read(&dest->weight);
      if (weight > 0) {
        rweight1 -= weight;
        if (rweight1 <= 0 && weight1 != 0) {
          choice1 = dest;
          weight1 = weight * ip_vs_dest_conn_overhead(dest);
          goto secondstage;
        }
      }
    }
  }

/*
 * Find the second weighted dest, do not include the first choice in the search
 */
secondstage:
  rweight2 = prandom_u32() % (total_weight - weight1);
  list_for_each_entry_rcu(dest, &svc->destinations, n_list) {
    if (!(dest->flags & IP_VS_DEST_F_OVERLOAD) && dest != choice1) {
      weight = atomic_read(&dest->weight);
      if (weight > 0) {
        rweight2 -= weight;
        if (rweight2 <= 0 && weight2 != 0) {
          choice2 = dest;
          weight2 = weight * ip_vs_dest_conn_overhead(dest);
          goto choicestage;
        }
      }
    }
  }

choicestage:
  if (choice2 != NULL && weight2 < weight1) {
    choice1 = choice2;
    weight1 = weight2;
  }

  IP_VS_DBG_BUF(6, "twos: server %s:%u "
                   "activeconns %d refcnt %d weight %d effective weight %d\n",
                IP_VS_DBG_ADDR(choice1->af, &choice1->addr),
                ntohs(choice1->port), atomic_read(&choice1->activeconns),
                refcount_read(&choice1->refcnt), atomic_read(&choice1->weight),
                weight1);

  return choice1;
}

static struct ip_vs_scheduler ip_vs_twos_scheduler = {
    .name = "twos",
    .refcnt = ATOMIC_INIT(0),
    .module = THIS_MODULE,
    .n_list = LIST_HEAD_INIT(ip_vs_twos_scheduler.n_list),
    .schedule = ip_vs_twos_schedule,
};

static int __init ip_vs_twos_init(void) {
  return register_ip_vs_scheduler(&ip_vs_twos_scheduler);
}

static void __exit ip_vs_twos_cleanup(void) {
  unregister_ip_vs_scheduler(&ip_vs_twos_scheduler);
  synchronize_rcu();
}

module_init(ip_vs_twos_init);
module_exit(ip_vs_twos_cleanup);
MODULE_LICENSE("GPL");
