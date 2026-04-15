// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * prandom – misc-драйвер, предоставляющий случайные данные, аналогично
 * /dev/urandom
 *
 * Copyright (C) 2026 Makar Lill
 *
 * This driver uses Galois Field arithmetic library written by Denis Potapov
 * (lib.c / lib.h). Used with permission.
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

#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "Galois_Field_256.h"
#include "prandom.h"

#define DEVICE_NAME "gfrandom"

MODULE_LICENSE("GPL-3.0");
MODULE_AUTHOR("Makar Lill");
MODULE_DESCRIPTION("Analogue of /dev/urandom");

struct gf256_gprn* global_gprn = NULL;

static char* coeff_file = NULL;
static char* seed_file  = NULL;

static int read_file_bytes(char* filename, GF256_t* row, size_t size)
{
    if (!filename || !row)
        return -EINVAL;

    struct file* f = filp_open(filename, O_RDONLY, 0);
    if (IS_ERR(f))
        return PTR_ERR(f);

    loff_t pos = 0;
    int    res = kernel_read(f, row, size, &pos);
    filp_close(f, NULL);

    if (res != size)
        return (res < 0) ? res : -EIO;

    return 0;
}

static int gprn_init_t(struct gf256_gprn* gprn)
{
    GF256_t coeff_data[POLY_DEGREE];
    GF256_t seed_data[POLY_DEGREE];
    int     i;
    int     err;

    if (coeff_file)
    {
        err = read_file_bytes(coeff_file, coeff_data, sizeof(coeff_data));
        if (err != 0)
            goto get_random_coeffs;
    }
    else
    {
    get_random_coeffs:
        get_random_bytes(coeff_data, sizeof(coeff_data));
        for (i = 0; i < POLY_DEGREE; i++)
        {
            if (coeff_data[i] != 0)
                break;
        }
        if (i == POLY_DEGREE)
            coeff_data[0] = GF256_ONE;
    }

    if (seed_file)
    {
        err = read_file_bytes(seed_file, seed_data, sizeof(seed_data));
        if (err != 0)
            goto use_auto_seed;
    }
    else
    {
    use_auto_seed:
        memset(seed_data, 0, sizeof(seed_data));
        for (i = 0; i < ARRAY_SIZE(GF256_DEGREES_AUTO); i++)
            seed_data[GF256_DEGREES_AUTO[i]] = GF256_ONE;
    }

    gf256_gprn_init_t(gprn, coeff_data, seed_data);
    return 0;
}

static ssize_t misc_read(struct file* filp, char __user* buf, size_t length, loff_t* f_pos)
{
    static GF256_t kernel_buf[1024];
    size_t         bytes_to_read = length;
    size_t         chunk;

    if (!global_gprn)
        return -ENODEV;

    while (bytes_to_read > 0)
    {
        chunk = min(bytes_to_read, sizeof(kernel_buf));

        gf256_gprn_generate(global_gprn, chunk, kernel_buf);

        if (copy_to_user(buf, kernel_buf, chunk))
            return -EFAULT;

        buf += chunk;
        bytes_to_read -= chunk;
        *f_pos += chunk;
    }

    return length - bytes_to_read;
}

static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .read  = misc_read,
};

static struct miscdevice prandom_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = DEVICE_NAME,
    .fops  = &fops,
};

static int __init misc_init(void)
{
    global_gprn = kzalloc(sizeof(struct gf256_gprn), GFP_KERNEL);
    if (!global_gprn)
        return -ENOMEM;

    if (gprn_init_t(global_gprn) != 0)
    {
        kfree(global_gprn);
        return -EINVAL;
    }

    int err = misc_register(&prandom_misc);
    if (err != 0)
    {
        kfree(global_gprn);
        return err;
    }

    return 0;
}

static void __exit misc_exit(void)
{
    kfree(global_gprn);
    misc_deregister(&prandom_misc);
}

module_param(coeff_file, charp, 0);
MODULE_PARM_DESC(coeff_file, "path to file with bytes for coefficients");
module_param(seed_file, charp, 0);
MODULE_PARM_DESC(seed_file, "path to file with seed bytes");

module_init(misc_init);
module_exit(misc_exit);
