/*
 * DMA Engine test module
 *
 * Copyright (C) 2017 C-SKY Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/freezer.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/time.h>

#define DMA_CHAN_NAME		"dma0chan0"
#define DMA_TEST_DATA_SIZE	0x500000

unsigned long time_start;

struct dma_chan *chan;
dma_addr_t dma_src;
dma_addr_t dma_dst;
char *src = NULL;
char *dst = NULL;
struct dma_device *dev;
struct dma_async_tx_descriptor *tx = NULL;
enum dma_ctrl_flags flags;
dma_cookie_t cookie;

static int ck_dmatest_run_set(const char *val, const struct kernel_param *kp);
static int ck_dmatest_run_get(char *val, const struct kernel_param *kp);
static const struct kernel_param_ops ck_run_ops = {
	.set = ck_dmatest_run_set,
	.get = ck_dmatest_run_get,
};
static bool ck_dmatest_run;
module_param_cb(run, &ck_run_ops, &ck_dmatest_run, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(run, "Run the test (default: false)");

static int ck_dmatest_run_get(char *val, const struct kernel_param *kp)
{
	return 0;
}

void dma_callback_func(void)
{
	int i;
	long dur_ms;

	dur_ms = (jiffies - time_start) * 1000 / HZ;
	printk("dma end\n");
	printk("size: %d bytes(0x%X), duration: %d ms\n",
		DMA_TEST_DATA_SIZE, DMA_TEST_DATA_SIZE, dur_ms);

	for (i=0; i<16; i++) {
		printk("%c ", dst[i]);
	}
	if (memcmp(src, dst, DMA_TEST_DATA_SIZE) == 0)
		printk("\nOK\n");
	else
		printk("\nFAIL\n");
}

static int ck_dmatest_run_set(const char *val, const struct kernel_param *kp)
{
	tx = dev->device_prep_dma_memcpy(chan, dma_dst, dma_src,
					 DMA_TEST_DATA_SIZE, flags);
	if (tx == NULL) {
		printk("Failed to prepare DMA memcpy\n");
	}
	tx->callback = dma_callback_func;
	tx->callback_param = NULL;

	cookie = tx->tx_submit(tx);
	if (dma_submit_error(cookie)) {
		printk("Failed to do DMA tx_submit\n");
	}

	printk("dma start\n");
	time_start = jiffies;
	dma_async_issue_pending(chan);
	return 0;
}

static bool filter(struct dma_chan *chan, void *param)
{
	if (param == NULL)
		return true;
	return strcmp(dma_chan_name(chan), param) == 0;
}

static int __init ck_dmatest_init(void)
{
	int i;
	dma_cap_mask_t mask;

	printk("*** ck_dmatest_init ***\n");

	src = dma_alloc_coherent(NULL, DMA_TEST_DATA_SIZE, &dma_src, GFP_KERNEL);
	printk("src=0x%X dma_src=0x%X\n", src, dma_src);
	dst = dma_alloc_coherent(NULL, DMA_TEST_DATA_SIZE, &dma_dst, GFP_KERNEL);
	printk("dst=0x%X dma_dst=0x%X\n", dst, dma_dst);

	for (i=0; i<DMA_TEST_DATA_SIZE; i++) {
		*(src + i) = 'a';
	}

	dma_cap_zero(mask);
	dma_cap_set(DMA_MEMCPY, mask);
	chan = dma_request_channel(mask, filter, DMA_CHAN_NAME);
	printk("dma channel id = %d\n", chan->chan_id);

	flags = DMA_CTRL_ACK | DMA_PREP_INTERRUPT;
	dev = chan->device;

	tx = dev->device_prep_dma_memcpy(chan, dma_dst, dma_src,
					 DMA_TEST_DATA_SIZE, flags);
	if (tx == NULL) {
		printk("Failed to prepare DMA memcpy\n");
	}
	tx->callback = dma_callback_func;
	tx->callback_param = NULL;
	cookie = tx->tx_submit(tx);
	if (dma_submit_error(cookie)) {
		printk("Failed to do DMA tx_submit\n");
	}

	printk("dma start\n");
	time_start = jiffies;
	dma_async_issue_pending(chan);
	return 0;
}
late_initcall(ck_dmatest_init);

static void __exit ck_dmatest_exit(void)
{
	dma_free_coherent(NULL, DMA_TEST_DATA_SIZE, src, dma_src);
	dma_free_coherent(NULL, DMA_TEST_DATA_SIZE, dst, dma_dst);
	dma_release_channel(chan);
}
module_exit(ck_dmatest_exit);

MODULE_LICENSE("GPL v2");
