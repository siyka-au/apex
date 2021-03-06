#ifndef dev_fsl_imx_usbphy_init_h
#define dev_fsl_imx_usbphy_init_h

/*
 * Driver for USB PHY in IMX processors.
 */

#ifdef __cplusplus
extern "C" {
#endif

struct fsl_imx_usbphy_desc {
	unsigned long base;
	unsigned long analog_base;
	unsigned d_cal;
	unsigned txcal45dp;
	unsigned txcal45dn;
};

void fsl_imx_usbphy_init(const struct fsl_imx_usbphy_desc *);

#ifdef __cplusplus
}
#endif

#endif
