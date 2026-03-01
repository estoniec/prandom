// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * prandom – misc-драйвер, предоставляющий случайные данные, аналогично /dev/urandom
 *
 * Copyright (C) 2025 Makar Lill
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "prandom"

static ssize_t misc_read(struct file *filp, char __user *buf, size_t length,
                         loff_t *f_pos)
{
    u8 kernel_buf[256];
    size_t bytes_to_read = length;
    size_t chunk;

    while (bytes_to_read > 0) {
        chunk = min(bytes_to_read, sizeof(kernel_buf));
        get_random_bytes(kernel_buf, chunk);

        if (copy_to_user(buf, kernel_buf, chunk))
            return -EFAULT;

        buf += chunk;
        bytes_to_read -= chunk;
    }

    return length;
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = misc_read,
};

static struct miscdevice prandom_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &fops,
};

static int __init misc_init(void)
{
    int err = misc_register(&prandom_misc);
    if (err != 0) {
        return err;
    }

    return 0;
}

static void __exit misc_exit(void)
{
    misc_deregister(&prandom_misc);
}

module_init(misc_init);
module_exit(misc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Makar Lill");
MODULE_DESCRIPTION("Analogue of /dev/urandom");