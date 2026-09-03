// SPDX-License-Identifier: GPL-2.0

#include <dm.h>
#include <sysinfo.h>
#include <net.h>
#include <u-boot/uuid.h>

#define DFD_INFO_MAGIC		('d' << 24 | 'f' << 16 | 'd' << 8 | 's')
#define DFD_UUID_STR_LEN	(32)

struct dfd_info {
	u32 magic;
	u16 size;
	char baseboard_product[18 + 1];
	char baseboard_version[20 + 1];
	char system_serial[16 + 1];
	char system_version[18 + 1];
	char system_uuid[DFD_UUID_STR_LEN + 1];
	u8 mac_addr_cnt;
	u8 mac_addr[8][ARP_HLEN];
	u8 padding[3];
	u32 ddr_size_mb;
} __packed;


static struct dfd_info g_dfd_sysinfo = {
	.magic = DFD_INFO_MAGIC,
	.size = 4321,
	.baseboard_product = "dfd.test",
	.baseboard_version = "V1.0.0",
	.system_serial = "DFD-b7e02e85",
	.system_version = "v0.0.1",
	.system_uuid = "33b1f84f72ae4abfaeb0c4f86da04560",
	.mac_addr_cnt = 1,
	.mac_addr[0] = {0x70, 0xD9, 0x83, 0x00, 0x05, 0xFD},
	.ddr_size_mb = 2048,
};

/**
 * struct sysinfo_dfd_priv - sysinfo private data
 * @info: dfd board info
 */
struct sysinfo_dfd_priv {
	struct dfd_info *info;
	u8 uuid_smbios[16];
};

static int sysinfo_dfd_detect(struct udevice *dev)
{
	struct sysinfo_dfd_priv *priv = dev_get_priv(dev);

	if (!priv->info || priv->info->magic != DFD_INFO_MAGIC)
		return -EFAULT;

	return 0;
}

static int sysinfo_dfd_get_str(struct udevice *dev, int id, size_t size,
				   char *val)
{
	struct sysinfo_dfd_priv *priv = dev_get_priv(dev);

	switch (id) {
	case SYSID_SM_BASEBOARD_VERSION:
		strlcpy(val, priv->info->baseboard_version, size);
		break;
	case SYSID_SM_SYSTEM_SERIAL:
		strlcpy(val, priv->info->system_serial, size);
		break;
	case SYSID_SM_SYSTEM_VERSION:
		strlcpy(val, priv->info->system_version, size);
		break;
	case SYSID_SM_SYSTEM_UUID:
		strlcpy(val, priv->info->system_uuid, size);
		break;
	case SYSID_SM_BASEBOARD_PRODUCT:
		strlcpy(val, priv->info->baseboard_product, size);
		break;
	default:
		return -EINVAL;
	};

	val[size - 1] = '\0';
	return 0;
}

static int sysinfo_dfd_get_int(struct udevice *dev, int id, int *val)
{
	struct sysinfo_dfd_priv *priv = dev_get_priv(dev);

	switch (id) {
	case SYSID_BOARD_RAM_SIZE_MB:
		*val = priv->info->ddr_size_mb;
		return 0;
	default:
		return -EINVAL;
	};
}

static int sysinfo_dfd_get_data(struct udevice *dev, int id, void **data,
				    size_t *size)
{
	struct sysinfo_dfd_priv *priv = dev_get_priv(dev);

	switch (id) {
	case SYSID_SM_SYSTEM_UUID:
		*data = priv->uuid_smbios;
		*size = 16;
		return 0;
	default:
		return -EINVAL;
	};
}

static int sysinfo_dfd_get_item_count(struct udevice *dev, int id)
{
	struct sysinfo_dfd_priv *priv = dev_get_priv(dev);

	switch (id) {
	case SYSID_BOARD_MAC_ADDR:
		return priv->info->mac_addr_cnt;
	default:
		return -EINVAL;
	};
}

static int sysinfo_dfd_get_data_by_index(struct udevice *dev, int id,
					     int index, void **data,
					     size_t *size)
{
	struct sysinfo_dfd_priv *priv = dev_get_priv(dev);

	switch (id) {
	case SYSID_BOARD_MAC_ADDR:
		if (index >= priv->info->mac_addr_cnt)
			return -EINVAL;
		*data = priv->info->mac_addr[index];
		*size = ARP_HLEN;
		return 0;
	default:
		return -EINVAL;
	};
}

static const struct sysinfo_ops sysinfo_dfd_ops = {
	.detect            = sysinfo_dfd_detect,
	.get_str           = sysinfo_dfd_get_str,
	.get_int           = sysinfo_dfd_get_int,
	.get_data          = sysinfo_dfd_get_data,
	.get_item_count    = sysinfo_dfd_get_item_count,
	.get_data_by_index = sysinfo_dfd_get_data_by_index,
};

/**
 * @brief Convert the dfd UUID string to the SMBIOS format
 *
 * @param uuid_raw The dfd UUID string parsed from the eeprom
 * @param uuid_smbios The buffer to hold the SMBIOS formatted UUID
 */
static void sysinfo_dfd_convert_uuid(const char *uuid_dfd,
					 u8 *uuid_smbios)
{
	char uuid_rfc4122_str[DFD_UUID_STR_LEN + 4 + 1] = {0};
	char *tmp = uuid_rfc4122_str;

	for (int i = 0; i < 16; i++) {
		memcpy(tmp, uuid_dfd + i * 2, 2);
		tmp += 2;
		if (i == 3 || i == 5 || i == 7 || i == 9)
			*tmp++ = '-';
	}
	uuid_str_to_bin(uuid_rfc4122_str, uuid_smbios, UUID_STR_FORMAT_GUID);
}

static int sysinfo_dfd_probe(struct udevice *dev)
{
	struct sysinfo_dfd_priv *priv = dev_get_priv(dev);

	priv->info = &g_dfd_sysinfo;

	sysinfo_dfd_convert_uuid(priv->info->system_uuid, priv->uuid_smbios);

	return 0;
}

static const struct udevice_id sysinfo_dfd_ids[] = {
	{ .compatible = "dfd,sysinfo" }
};

U_BOOT_DRIVER(sysinfo_dfd) = {
	.name           = "sysinfo_dfd",
	.id             = UCLASS_SYSINFO,
	.of_match       = sysinfo_dfd_ids,
	.ops		= &sysinfo_dfd_ops,
	.priv_auto	= sizeof(struct sysinfo_dfd_priv),
	.probe          = sysinfo_dfd_probe,
};
