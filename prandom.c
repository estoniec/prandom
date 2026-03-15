// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * prandom – misc-драйвер, предоставляющий случайные данные, аналогично
 * /dev/urandom
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

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "asm-generic/errno-base.h"
#include "lib.h"

#define DEVICE_NAME "gfrandom"
#define POLY_DEGREE 256
#define GF256_ONE (GF256_t)0x01
#define GF256_DEGREES_AUTO {0, 2, 5, 10, 255}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Makar Lill");
MODULE_DESCRIPTION("Analogue of /dev/urandom");

struct gf256_gprn
{
    GF256_t coeff[POLY_DEGREE];
    GF256_t t[POLY_DEGREE];
    int     index;
};

static struct gf256_gprn* global_gprn = NULL;

static char* coeff_file   = NULL;
static char* polynom_file = NULL;

static int read_file_bytes(char* filename, GF256_t* row, size_t size)
{
    if (!filename || !row)
        return !EINVAL;

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

static void gf256_gprn_init_t(struct gf256_gprn* gprn)
{
    int i;

    // read bytes from coeff file
    if (coeff_file)
    {
        int res = read_file_bytes(coeff_file, gprn->coeff, sizeof(gprn->coeff));
        if (res != 0)
        {
            goto generate_coeffs;
        }
    }
    else
    {
    generate_coeffs:
        get_random_bytes(gprn->coeff, sizeof(gprn->coeff));

        int all_zero = 1;
        for (i = 0; i < POLY_DEGREE; i++)
        {
            if (gprn->coeff[i] != 0)
            {
                all_zero = 0;
                break;
            }
        }
        if (all_zero)
        {
            gprn->coeff[0] = GF256_ONE;
        }
    }

    // read bytes from polynom file
    if (polynom_file)
    {
        int res = read_file_bytes(polynom_file, gprn->t, sizeof(gprn->t));
        if (res != 0)
        {
            goto default_polynom;
        }
    }
    else
    {
    default_polynom:
        for (i = 0; i < POLY_DEGREE; i++)
        {
            gprn->t[i] = 0;
        }

        // we use polynom t^256 + t^10 + t^5 + t^2 + 1
        const int degrees[] = GF256_DEGREES_AUTO;

        for (int i = 0; i < ARRAY_SIZE(degrees); i++)
        {
            gprn->t[degrees[i]] = GF256_ONE;
        }
    }

    gprn->index = 0;
}

static GF256_t gf256_gprn_next(struct gf256_gprn* gprn)
{
    GF256_t new_byte = 0;
    int     idx;
    int     i;

    for (i = 0; i < POLY_DEGREE; i++)
    {
        if (gprn->t[i] != 0)
        {
            idx      = (gprn->index + i) % POLY_DEGREE;
            new_byte = GF256_Add(new_byte, GF256_Mul(gprn->t[i], gprn->coeff[idx]));
        }
    }

    // "circular" array
    gprn->coeff[gprn->index] = new_byte;
    gprn->index              = (gprn->index + 1) % POLY_DEGREE;

    return new_byte;
}

static void gf256_gprn_generate(struct gf256_gprn* gprn, size_t count, GF256_t* output)
{
    size_t i;
    for (i = 0; i < count; i++)
    {
        output[i] = gf256_gprn_next(gprn);
    }
}

static ssize_t misc_read(struct file* filp, char __user* buf, size_t length, loff_t* f_pos)
{
    GF256_t kernel_buf[256];
    size_t  bytes_to_read = length;
    size_t  chunk;

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

    return length;
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
    global_gprn = kmalloc(sizeof(struct gf256_gprn), GFP_KERNEL);
    if (!global_gprn)
        return -ENOMEM;

    gf256_gprn_init_t(global_gprn);

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
module_param(polynom_file, charp, 0);
MODULE_PARM_DESC(polynom_file, "path to file with bytes for polynom");

module_init(misc_init);
module_exit(misc_exit);