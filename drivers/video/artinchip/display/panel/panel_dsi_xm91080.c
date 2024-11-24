// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for xm91080s DSI panel.
 *
 * Copyright (C) 2020-2021 ArtInChip Technology Co., Ltd.
 * Authors: huahui.mai <huahui.ami@artinchip.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <dm/device_compat.h>
#include <video.h>
#include <panel.h>

#include "panel_dsi.h"

#define PANEL_DEV_NAME		"dsi_panel_xm91080"

struct xm91080 {
	struct gpio_desc reset;
};

static inline struct xm91080 *panel_to_xm91080(struct aic_panel *panel)
{
	return (struct xm91080 *)panel->panel_private;
}

static int panel_enable(struct aic_panel *panel)
{
	struct xm91080 *xm91080 = panel_to_xm91080(panel);
	int ret;

	panel_di_enable(panel, 0);

	ret = gpio_request_by_name(panel->dev, "reset-gpios", 0,
					&xm91080->reset, GPIOD_IS_OUT);
	if (ret) {
		dev_err(panel->dev, "Failed to get reset gpio\n");
		return ret;
	}
	dm_gpio_set_value(&xm91080->reset, 0);

	aic_delay_ms(10);
	dm_gpio_set_value(&xm91080->reset, 1);
	aic_delay_ms(1);
	dm_gpio_set_value(&xm91080->reset, 0);
	aic_delay_ms(10);
	dm_gpio_set_value(&xm91080->reset, 1);
	aic_delay_ms(120);

	panel_dsi_send_perpare(panel);

	panel_dsi_dcs_send_seq(panel, 0x00, 0x00);
	panel_dsi_dcs_send_seq(panel, 0xFF, 0x10, 0x80, 0x01);

	/* Set XM Command Password 2 */
	panel_dsi_dcs_send_seq(panel, 0x00, 0x80);
	panel_dsi_dcs_send_seq(panel, 0xFF, 0x10, 0x80);

	/* tcon setting */
	panel_dsi_dcs_send_seq(panel, 0x00, 0x81);
	panel_dsi_dcs_send_seq(panel, 0xb2, 0x74);

	panel_dsi_dcs_send_seq(panel, 0x00, 0x86);
	panel_dsi_dcs_send_seq(panel, 0xb2, 0x01, 0x01, 0x01, 0x01, 0x1B, 0x08);

	/* ckv setting */
	panel_dsi_dcs_send_seq(panel, 0x00, 0x80);
	panel_dsi_dcs_send_seq(panel, 0xb4, 0x18, 0x03, 0x07, 0x80, 0x02, 0x00,
		0x00, 0x02, 0x00, 0x00);
	panel_dsi_dcs_send_seq(panel, 0x00, 0x90);
	panel_dsi_dcs_send_seq(panel, 0xb4, 0x18, 0x02, 0x07, 0x82, 0x02, 0x00,
		0x00, 0x02, 0x00, 0x00);
	panel_dsi_dcs_send_seq(panel, 0x00, 0xa0);
	panel_dsi_dcs_send_seq(panel, 0xb4, 0x18, 0x01, 0x07, 0x82, 0x02, 0x00,
		0x00, 0x02, 0x00, 0x00);
	panel_dsi_dcs_send_seq(panel, 0x00, 0xb0);
	panel_dsi_dcs_send_seq(panel, 0xb4, 0x18, 0x00, 0x07, 0x83, 0x02, 0x00,
		0x00, 0x02, 0x00, 0x00);

	/* vst */
	panel_dsi_dcs_send_seq(panel, 0x00, 0x80);
	panel_dsi_dcs_send_seq(panel, 0xb6, 0x83, 0x02, 0x00, 0x60, 0x82, 0x02,
		0x00, 0x60);

	/* u2d ok */
	panel_dsi_dcs_send_seq(panel, 0x00, 0x80);
	panel_dsi_dcs_send_seq(panel, 0xbc, 0x00, 0x0e, 0x25, 0x26, 0x00, 0x06,
		0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x20, 0x1f);
	panel_dsi_dcs_send_seq(panel, 0x00, 0x90);
	panel_dsi_dcs_send_seq(panel, 0xbc, 0x00, 0x0d, 0x25, 0x26, 0x00, 0x05,
		0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x20, 0x1f);

	/* d2u ok */
	panel_dsi_dcs_send_seq(panel, 0x00, 0xa0);
	panel_dsi_dcs_send_seq(panel, 0xbc, 0x00, 0x0d, 0x26, 0x25, 0x00, 0x07,
		0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x20, 0x1f);
	panel_dsi_dcs_send_seq(panel, 0x00, 0xb0);
	panel_dsi_dcs_send_seq(panel, 0xbc, 0x00, 0x0e, 0x26, 0x25, 0x00, 0x08,
		0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x20, 0x1f);

	/* enmode */
	panel_dsi_dcs_send_seq(panel, 0x00, 0xa0);
	panel_dsi_dcs_send_seq(panel, 0xb9, 0xff, 0xd4, 0xd7, 0xd7, 0xff, 0xd4,
		0xe0, 0xd4, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe4, 0xe4, 0xe4);

	panel_dsi_dcs_send_seq(panel, 0x00, 0xb0);
	panel_dsi_dcs_send_seq(panel, 0xb9, 0xff, 0xd4, 0xd7, 0xd7, 0xff, 0xd4,
		0xe0, 0xd4, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe4, 0xe4, 0xe4);

	panel_dsi_dcs_send_seq(panel, 0x00, 0x80);
	panel_dsi_dcs_send_seq(panel, 0xb9, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);

	panel_dsi_dcs_send_seq(panel, 0x00, 0x90);
	panel_dsi_dcs_send_seq(panel, 0xb9, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);

	/* gip lvd */
	panel_dsi_dcs_send_seq(panel, 0x00, 0x80);
	panel_dsi_dcs_send_seq(panel, 0xba, 0xea, 0xea, 0xff, 0xc0, 0xea, 0xea,
		0xff, 0xc0);

	/* gip ultra setting */
	panel_dsi_dcs_send_seq(panel, 0x00, 0xd0);
	panel_dsi_dcs_send_seq(panel, 0xb6, 0x81, 0x00, 0x02, 0x02);
	panel_dsi_dcs_send_seq(panel, 0x00, 0xe0);
	panel_dsi_dcs_send_seq(panel, 0xb6, 0x00, 0x0c, 0x22, 0x02, 0x20, 0x11,
		0x01, 0x01, 0x04, 0x00);

	/* G-swap */
	panel_dsi_dcs_send_seq(panel, 0x00, 0xA5);
	panel_dsi_dcs_send_seq(panel, 0xC0, 0x20);

	/* mirror X2 */
	panel_dsi_dcs_send_seq(panel, 0x00, 0xA0);
	panel_dsi_dcs_send_seq(panel, 0xA5, 0x20);

	/* VGHO/VGL = VGLO */
	panel_dsi_dcs_send_seq(panel, 0x00, 0xF0);
	panel_dsi_dcs_send_seq(panel, 0xA4, 0x00);

	/* VGL/VGH = -8/9 */
	panel_dsi_dcs_send_seq(panel, 0x00, 0x90);
	panel_dsi_dcs_send_seq(panel, 0xAB, 0xA8, 0x9E);

	/* VCOM */
	panel_dsi_dcs_send_seq(panel, 0x00, 0xB1);
	panel_dsi_dcs_send_seq(panel, 0xA4, 0xB8, 0xB8);

	/* GVDDP/GVDDN */
	panel_dsi_dcs_send_seq(panel, 0x00, 0xA0);
	panel_dsi_dcs_send_seq(panel, 0xA4, 0x23, 0x23);

	panel_dsi_dcs_send_seq(panel, 0x00, 0x80);
	panel_dsi_dcs_send_seq(panel, 0xD4, 0x03, 0x05, 0x07, 0x0C, 0x12, 0x16,
		0x19, 0x1F, 0x23, 0x32, 0x3C, 0x4D, 0x59, 0x6D, 0x7C, 0x7C,
		0x8C, 0x9E, 0xAB, 0xBB, 0xC6, 0xD4, 0xD8, 0xDD, 0xE2, 0xE7,
		0xEC, 0xF3, 0xFB, 0xFF, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
		0x00, 0x00);
	panel_dsi_dcs_send_seq(panel, 0x00, 0x80);
	panel_dsi_dcs_send_seq(panel, 0xD5, 0x03, 0x05, 0x07, 0x0C, 0x12, 0x16,
		0x19, 0x1F, 0x23, 0x32, 0x3C, 0x4D, 0x59, 0x6D, 0x7C, 0x7C,
		0x8C, 0x9E, 0xAB, 0xBB, 0xC6, 0xD4, 0xD8, 0xDD, 0xE2, 0xE7,
		0xEC, 0xF3, 0xFB, 0xFF, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
		0x00, 0x00);
	panel_dsi_dcs_send_seq(panel, 0x00, 0x80);
	panel_dsi_dcs_send_seq(panel, 0xD6, 0x03, 0x05, 0x07, 0x0C, 0x12, 0x16,
		0x19, 0x1F, 0x23, 0x32, 0x3C, 0x4D, 0x59, 0x6D, 0x7C, 0x7C,
		0x8C, 0x9E, 0xAB, 0xBB, 0xC6, 0xD4, 0xD8, 0xDD, 0xE2, 0xE7,
		0xEC, 0xF3, 0xFB, 0xFF, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
		0x00, 0x00);
	panel_dsi_dcs_send_seq(panel, 0x00, 0x80);
	panel_dsi_dcs_send_seq(panel, 0xD7, 0x03, 0x05, 0x07, 0x0C, 0x12, 0x16,
		0x19, 0x1F, 0x23, 0x32, 0x3C, 0x4D, 0x59, 0x6D, 0x7C, 0x7C,
		0x8C, 0x9E, 0xAB, 0xBB, 0xC6, 0xD4, 0xD8, 0xDD, 0xE2, 0xE7,
		0xEC, 0xF3, 0xFB, 0xFF, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
		0x00, 0x00);
	panel_dsi_dcs_send_seq(panel, 0x00, 0x80);
	panel_dsi_dcs_send_seq(panel, 0xD8, 0x03, 0x05, 0x07, 0x0C, 0x12, 0x16,
		0x19, 0x1F, 0x23, 0x32, 0x3C, 0x4D, 0x59, 0x6D, 0x7C, 0x7C,
		0x8C, 0x9E, 0xAB, 0xBB, 0xC6, 0xD4, 0xD8, 0xDD, 0xE2, 0xE7,
		0xEC, 0xF3, 0xFB, 0xFF, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
		0x00, 0x00);
	panel_dsi_dcs_send_seq(panel, 0x00, 0x80);
	panel_dsi_dcs_send_seq(panel, 0xD9, 0x03, 0x05, 0x07, 0x0C, 0x12, 0x16,
		0x19, 0x1F, 0x23, 0x32, 0x3C, 0x4D, 0x59, 0x6D, 0x7C, 0x7C,
		0x8C, 0x9E, 0xAB, 0xBB, 0xC6, 0xD4, 0xD8, 0xDD, 0xE2, 0xE7,
		0xEC, 0xF3, 0xFB, 0xFF, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
		0x00, 0x00);
	panel_dsi_dcs_send_seq(panel, 0x00, 0xC2);
	panel_dsi_dcs_send_seq(panel, 0xA6, 0x08);
	panel_dsi_dcs_send_seq(panel, 0x00, 0x86);
	panel_dsi_dcs_send_seq(panel, 0xA5, 0x19);
	panel_dsi_dcs_send_seq(panel, 0x00, 0x83);
	panel_dsi_dcs_send_seq(panel, 0xA7, 0x88);

	/* mipi skew */
	panel_dsi_dcs_send_seq(panel, 0x00, 0x90);
	panel_dsi_dcs_send_seq(panel, 0xA3, 0x04, 0x04, 0x01, 0x05, 0x06, 0x00);
	panel_dsi_dcs_send_seq(panel, 0x00, 0xC0);
	panel_dsi_dcs_send_seq(panel, 0xA4, 0x01);

	ret = panel_dsi_dcs_exit_sleep_mode(panel);
	if (ret < 0) {
		pr_err("Failed to exit sleep mode: %d\n", ret);
		return ret;
	}

	aic_delay_ms(200);

	ret = panel_dsi_dcs_set_display_on(panel);
	if (ret < 0) {
		pr_err("Failed to set display on: %d\n", ret);
		return ret;
	}

	aic_delay_ms(120);

	panel_dsi_setup_realmode(panel);
	panel_de_timing_enable(panel, 0);
	panel_backlight_enable(panel, 0);
	return 0;
}

static struct aic_panel_funcs panel_funcs = {
	.prepare = panel_default_prepare,
	.enable = panel_enable,
	.get_video_mode = panel_default_get_video_mode,
	.register_callback = panel_register_callback,
};

/* Init the videomode parameter, dts will override the initial value. */
static struct fb_videomode panel_vm = {
	.pixclock = 130000000,
	.xres = 1080,
	.right_margin = 160,
	.left_margin = 160,
	.hsync_len = 40,
	.yres = 1920,
	.lower_margin = 10,
	.upper_margin = 20,
	.vsync_len = 8,
	.flag = DISPLAY_FLAGS_HSYNC_LOW | DISPLAY_FLAGS_VSYNC_LOW |
		DISPLAY_FLAGS_DE_HIGH | DISPLAY_FLAGS_PIXDATA_POSEDGE
};

static struct panel_dsi dsi = {
	.format = DSI_FMT_RGB888,
	.mode = DSI_MOD_VID_PULSE,
	.lane_num = 4,
};

static int panel_probe(struct udevice *dev)
{
	struct panel_priv *priv = dev_get_priv(dev);
	struct xm91080 *xm91080;

	xm91080 = malloc(sizeof(*xm91080));
	if (!xm91080)
		return -ENOMEM;

	if (panel_parse_dts(dev) < 0) {
		free(xm91080);
		return -1;
	}

	priv->panel.dsi = &dsi;
	panel_init(priv, dev, &panel_vm, &panel_funcs, xm91080);

	return 0;
}

static const struct udevice_id panel_match_ids[] = {
	{.compatible = "artinchip,aic-dsi-panel-simple"},
	{ /* sentinel */}
};

U_BOOT_DRIVER(panel_dsi_xm91080) = {
	.name      = PANEL_DEV_NAME,
	.id        = UCLASS_PANEL,
	.of_match  = panel_match_ids,
	.probe     = panel_probe,
	.priv_auto = sizeof(struct panel_priv),
};