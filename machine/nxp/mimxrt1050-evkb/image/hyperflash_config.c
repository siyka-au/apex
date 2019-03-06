#include <cpu/nxp/imxrt10xx/flexspi.h>

/*
 * FlexSPI configuration for IS26KS512S hyperflash on MIMXRT1050-EVKB and
 * MIMXRT1060-EVK boards.
 *
 * REVISIT: for now we are setting undocumented features so that the generated
 * configuration block is identical to the one generated by EVKB-IMXRT1050.
 *
 * REVISIT: we also set the version to 1.4 but reference manual rev 1 only
 * documents 1.1.
 */
const struct flexspi_boot_nor flexspi_boot_nor_ = {
	.memConfig = {
		.tag = "FCFB",
		.version.ascii = 'V',
		.version.major = 1,
		.version.minor = 4, /* REVISIT: reference manual uses 0/1 but SDK uses 4 */
		.version.bugfix = 0,
		.readSampleClkSrc = 3, /* external input from DQS pad */
		.csHoldTime = 3,
		.csSetupTime = 3,
		.columnAddressWidth = 3,
		.controllerMiscOption =
		    (1 << 6) | /* ddr mode */
		    (1 << 4) | /* safe configuration frequency */
		    (1 << 3) | /* word addressable */
		    (1 << 0),  /* differential clock */
		.sflashPadType = 8,
		.serialClkFreq = 8, /* 166MHz */
		.sflashA1Size = 64 * 1024 * 1024,
		.lookupTable = {
			{
				.opcode0 = FLEXSPI_OPCODE_CMD_DDR,
				.num_pads0 = FLEXSPI_NUM_PADS_8,
				.operand0 = 0xA0,
				.opcode1 = FLEXSPI_OPCODE_RADDR_DDR,
				.num_pads1 = FLEXSPI_NUM_PADS_8,
				.operand1 = 0x18,
			},
			{
				.opcode0 = FLEXSPI_OPCODE_CADDR_DDR,
				.num_pads0 = FLEXSPI_NUM_PADS_8,
				.operand0 = 0x10,
				.opcode1 = FLEXSPI_OPCODE_DUMMY_DDR,
				.num_pads1 = FLEXSPI_NUM_PADS_8,
				.operand1 = 0x06,
			},
			{
				.opcode0 = FLEXSPI_OPCODE_READ_DDR,
				.num_pads0 = FLEXSPI_NUM_PADS_8,
				.operand0 = 0x04,
				.opcode1 = FLEXSPI_OPCODE_STOP,
			},
		},
	},
	.pageSize = 512,
	.sectorSize = 256 * 1024,
	.blockSize = 256 * 1024,	/* REVISIT: undocumented */
	.isUniformBlockSize = 1,	/* REVISIT: undocumented */
};
