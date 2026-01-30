// SPDX-License-Identifier: GPL-2.0+

#include <clk.h>
#include <dm.h>
#include <malloc.h>
#include <sdhci.h>
#include <linux/err.h>
#include <dm/lists.h>

struct dfd_sdhci_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

static int dfd_sdhci_probe(struct udevice *dev)
{
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct dfd_sdhci_plat *plat = dev_get_plat(dev);
	struct sdhci_host *host = dev_get_priv(dev);
	u32 max_clk;
	struct clk clk;
	int ret;

#if 0
	ret = clk_get_by_index(dev, 0, &clk);
	if (ret) {
		log_err("%s: clock get failed %d\n", __func__, ret);
		return ret;
	}
#endif

	ret = clk_enable(&clk);
	if (ret) {
		log_err("%s: clock enable failed %d\n", __func__, ret);
		return ret;
	}

	host->name = dev->name;
	host->ioaddr = dev_read_addr_ptr(dev);

	max_clk = clk_get_rate(&clk);
	if (IS_ERR_VALUE(max_clk)) {
		ret = max_clk;
		log_err("%s: clock rate get failed %d\n", __func__, ret);
		goto err;
	}

	host->max_clk   = max_clk;
	host->mmc       = &plat->mmc;
	host->mmc->dev  = dev;
	host->mmc->priv = host;
	upriv->mmc      = host->mmc;

	ret = sdhci_setup_cfg(&plat->cfg, host, 0, 0);
	if (ret)
		goto err;

	ret = sdhci_probe(dev);
	if (ret)
		goto err;

	return 0;

err:
	clk_disable(&clk);
	return ret;
}

static int dfd_sdhci_bind(struct udevice *dev)
{
	struct dfd_sdhci_plat *plat = dev_get_plat(dev);

	return sdhci_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct udevice_id dfd_sdhci_ids[] = {
	{ .compatible = "dfd,sdhci" },
	{ }
};

U_BOOT_DRIVER(dfd_sdhci_drv) = {
	.name      = "dfd_sdhci",
	.id        = UCLASS_MMC,
	.of_match  = dfd_sdhci_ids,
	.ops       = &sdhci_ops,
	.bind      = dfd_sdhci_bind,
	.probe     = dfd_sdhci_probe,
	.priv_auto = sizeof(struct sdhci_host),
	.plat_auto = sizeof(struct dfd_sdhci_plat),
};
