// SPDX-License-Identifier: GPL-2.0+
//#define MKIMAGE_DEBUG
#include "imagetool.h"
#include "mkimage.h"
#include <image.h>
#include <u-boot/crc.h>

#define IMAGE_MAGIC     ('v' << 24 | 's' << 16 | 'b' << 8 | 'l')
#define DEFAULT_VERSION         0x01000000  // 默认版本号 v1.0 → 0x01000000

#pragma pack(1)
struct dfd_img_header {
        uint32_t        ih_magic;               /* Image Header Magic Number */
        uint32_t        ih_vern;                /* Image Version Num  8bit Major and 24 Bit Minor */
        uint32_t        ih_time;                /* Image Creation Timestamp */
        uint32_t        ih_size;                /* Image Data Size */
        uint64_t        ih_load;                /* Data  Load  Address */
        uint64_t        ih_ep;                  /* Entry Point Address */
        uint8_t         ih_id[32];              /* User ID for signature version */
        uint8_t         ih_pubkey[64];          /* pubkey of SM2 */
        uint8_t         ih_sign_hash[64];       /* SM2 Signature of Image Data, or SM3 hash of Data for common version */
        uint32_t        ih_second_addr;         /* Second image address */
        uint32_t        ih_second_size;         /* Second image size */
        uint16_t        ih_second_type;         /* Second image type */
        uint16_t        ih_idlen;               /* ID length if for signature version */
        uint32_t        ih_sign_split;          /* split sign for Data/Data2 */
        uint8_t         ih_testkey[16];         /* Test Key for enable debug */
        uint8_t         ih_sign_hash2[64];      /* SM2 Signature of Image Data2, or SM3 hash of Data2 for common version */
        uint64_t        ih_second_load;         /* Sencond Image Data Load Address */
        uint8_t         reserved[204];
        uint32_t        ih_dcrc;                /* Image Data CRC Checksum */
        uint32_t        ih_dcrc2;               /* Second Image Data CRC Checksum */
        uint32_t        ih_hcrc;                /* Image Header CRC Checksum */
};
#pragma pack()
#define IMAGE_HEADER_SIZE       sizeof(struct dfd_img_header)

struct dfd_img_header header;

static int image_check_image_types(uint8_t type)
{
	return (type == IH_TYPE_DFDIMAGE) ? EXIT_SUCCESS : EXIT_FAILURE;
}

static int image_check_params(struct image_tool_params *params)
{
	return 0;
}

static void image_print_header(const void *ptr, struct image_tool_params *params)
{
	image_print_contents(ptr);
}

static int image_verify_header(unsigned char *ptr, int image_size,
			struct image_tool_params *params)
{
	uint32_t len;
	const unsigned char *data;
	uint32_t checksum;
	struct dfd_img_header header;
	struct dfd_img_header *hdr = &header;

	if (image_size < sizeof(struct dfd_img_header)) {
		debug("%s: Bad image size: \"%s\" is no valid image\n",
		      params->cmdname, params->imagefile);
		return -FDT_ERR_BADSTRUCTURE;
	}

	memcpy(hdr, ptr, sizeof(struct dfd_img_header));

	if (le32_to_cpu(hdr->ih_magic) != IMAGE_MAGIC) {
		debug("%s: Bad Magic Number: \"%s\" is no valid image\n",
		      params->cmdname, params->imagefile);
		return -FDT_ERR_BADMAGIC;
	}

	data = (const unsigned char *)hdr;
	len  = sizeof(struct dfd_img_header);

	checksum = le32_to_cpu(hdr->ih_hcrc);
	hdr->ih_hcrc = cpu_to_le32(0);	/* clear for re-calculation */

	if (crc32(0, data, len) != checksum) {
		debug("%s: ERROR: \"%s\" has bad header checksum!\n",
		      params->cmdname, params->imagefile);
		return -FDT_ERR_BADSTATE;
	}

	data = (const unsigned char *)ptr + sizeof(struct dfd_img_header);
	len = le32_to_cpu(hdr->ih_size);

	if (image_size - sizeof(struct dfd_img_header) < len) {
		debug("%s: Bad image size: \"%s\" is no valid image\n",
		      params->cmdname, params->imagefile);
		return -FDT_ERR_BADSTRUCTURE;
	}

	checksum = le32_to_cpu(hdr->ih_dcrc);
	if (crc32(0, data, len) != checksum) {
		debug("%s: ERROR: \"%s\" has corrupted data!\n",
		      params->cmdname, params->imagefile);
		return -FDT_ERR_BADSTRUCTURE;
	}

	return 0;
}

static void image_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct image_tool_params *params)
{
	uint32_t checksum;
	time_t time;
	const uint8_t *imagedata;
	uint32_t imagesize;
	uint32_t ep;
	uint32_t addr;
	uint32_t version = DEFAULT_VERSION;
	struct dfd_img_header *hdr = (struct dfd_img_header *)ptr;

	time = imagetool_get_source_date(params->cmdname, sbuf->st_mtime);
	ep = params->ep;
	addr = params->addr;

	imagedata = (const uint8_t*)ptr + sizeof(struct dfd_img_header);
	imagesize = sbuf->st_size - sizeof(struct dfd_img_header);
	checksum = crc32(0, imagedata, imagesize);

	memset(hdr, 0, sizeof(struct dfd_img_header));
	hdr->ih_magic = cpu_to_le32(IMAGE_MAGIC);
	hdr->ih_vern  = cpu_to_le32(version);
	hdr->ih_time  = cpu_to_le32(time);
	hdr->ih_size  = cpu_to_le32(imagesize);
	hdr->ih_load  = cpu_to_le64(addr);
	hdr->ih_ep    = cpu_to_le64(ep);
	hdr->ih_dcrc  = cpu_to_le32(checksum);
	hdr->ih_hcrc  = cpu_to_le32(0);

	checksum = crc32(0, (const uint8_t*)hdr, sizeof(struct dfd_img_header));
	hdr->ih_hcrc = cpu_to_le32(checksum);
}

U_BOOT_IMAGE_TYPE(
	dfd,
	"Dfd Image",
	sizeof(struct dfd_img_header),
	(void *)&header,
	image_check_params,
	image_verify_header,
	image_print_header,
	image_set_header,
	NULL,
	image_check_image_types,
	NULL,
	NULL
);
