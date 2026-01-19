#ifndef CO_SDO_H_
#define CO_SDO_H_

#include "CO_CAN_Msg.h"
#include "stdint.h"
#include "stdbool.h"
#include "CO_OD.h"
#include "CO_SYNC.h"
#include "CO_config.h"

#define SDO_MASK_INIT_S_BIT		0x01	//init cs, bit 0
#define SDO_MASK_INIT_E_BIT		0x02	//init cs, bit 1
#define SDO_MASK_INIT_N_BIT		0x0C	//init cs, bit 2-3
#define SDO_MASK_INIT_ES_BIT	0x03	//init cs, bit 0-1

#define SDO_MASK_SEG_C_BIT		0x01	//segment cs, bit 0
#define SDO_MASK_SEG_N_BIT		0x0E	//segment cs, bit 1-3
#define SDO_MASK_SEG_T_BIT		0x10	//segment cs, bit 4

typedef struct CO_SDO_t CO_SDO;
/**
 * Internal states of the SDO server state machine
 */
typedef enum {
    CO_SDO_ST_IDLE                  = 0x00U,
	CO_SDO_ST_ABORT					= 0x01U,

    CO_SDO_ST_DOWNLOAD_INITIATE_REQ = 0x10U,
    CO_SDO_ST_DOWNLOAD_INITIATE_RSP = 0x11U,
    CO_SDO_ST_DOWNLOAD_SEGMENTED    = 0x12U,

    CO_SDO_ST_UPLOAD_INITIATE_REQ   = 0x13U,
    CO_SDO_ST_UPLOAD_INITIATE_RSP   = 0x14U,
    CO_SDO_ST_UPLOAD_SEGMENTED      = 0x15U,

    CO_SDO_ST_DOWNLOAD_BL_INITIATE  = 0x21U,
    CO_SDO_ST_DOWNLOAD_BL_SUBBLOCK  = 0x22U,
    CO_SDO_ST_DOWNLOAD_BL_END       = 0x23U,
    CO_SDO_ST_UPLOAD_BL_INITIATE    = 0x24U,
    CO_SDO_ST_UPLOAD_BL_INITIATE_2  = 0x25U,
    CO_SDO_ST_UPLOAD_BL_SUBBLOCK    = 0x26U,
    CO_SDO_ST_UPLOAD_BL_END         = 0x27U
} CO_SDO_state_t;

/* Client command specifier */
#define CCS_DOWNLOAD_INITIATE           1//001
#define CCS_DOWNLOAD_SEGMENT            0//

#define CCS_UPLOAD_INITIATE             2
#define CCS_UPLOAD_SEGMENT              3

#define CCS_ABORT                       4

#define CCS_UPLOAD_BLOCK                5
#define CCS_DOWNLOAD_BLOCK              6

/* Server Command Specifier */
#define SCS_DOWNLOAD_INITIATED          3
#define SCS_DOWNLOAD_SEGMENT            1

#define SCS_UPLOAD_INITIATE             2
#define SCS_UPLOAD_SEGMENT              0

#define SCS_ABORT                       4

#define SCS_DOWNLOAD_BLOCK              5
#define SCS_UPLOAD_BLOCK                6

/**
 * SDO abort codes.
 *
 * Send with Abort SDO transfer message.
 *
 * The abort codes not listed here are reserved.
 */
typedef enum{
    CO_SDO_AB_NONE                  = 0x00000000UL, /**< 0x00000000, No abort */
    CO_SDO_AB_TOGGLE_BIT            = 0x05030000UL, /**< 0x05030000, Toggle bit not altered */
    CO_SDO_AB_TIMEOUT               = 0x05040000UL, /**< 0x05040000, SDO protocol timed out */
    CO_SDO_AB_CMD                   = 0x05040001UL, /**< 0x05040001, Command specifier not valid or unknown */
    CO_SDO_AB_BLOCK_SIZE            = 0x05040002UL, /**< 0x05040002, Invalid block size in block mode */
    CO_SDO_AB_SEQ_NUM               = 0x05040003UL, /**< 0x05040003, Invalid sequence number in block mode */
    CO_SDO_AB_CRC                   = 0x05040004UL, /**< 0x05040004, CRC error (block mode only) */
    CO_SDO_AB_OUT_OF_MEM            = 0x05040005UL, /**< 0x05040005, Out of memory */
    CO_SDO_AB_UNSUPPORTED_ACCESS    = 0x06010000UL, /**< 0x06010000, Unsupported access to an object */
    CO_SDO_AB_WRITEONLY             = 0x06010001UL, /**< 0x06010001, Attempt to read a write only object */
    CO_SDO_AB_READONLY              = 0x06010002UL, /**< 0x06010002, Attempt to write a read only object */
    CO_SDO_AB_NOT_EXIST             = 0x06020000UL, /**< 0x06020000, Object does not exist */
    CO_SDO_AB_NO_MAP                = 0x06040041UL, /**< 0x06040041, Object cannot be mapped to the PDO */
    CO_SDO_AB_MAP_LEN               = 0x06040042UL, /**< 0x06040042, Number and length of object to be mapped exceeds PDO length */
    CO_SDO_AB_PRAM_INCOMPAT         = 0x06040043UL, /**< 0x06040043, General parameter incompatibility reasons */
    CO_SDO_AB_DEVICE_INCOMPAT       = 0x06040047UL, /**< 0x06040047, General internal incompatibility in device */
    CO_SDO_AB_HW                    = 0x06060000UL, /**< 0x06060000, Access failed due to hardware error */
    CO_SDO_AB_TYPE_MISMATCH         = 0x06070010UL, /**< 0x06070010, Data type does not match, length of service parameter does not match */
    CO_SDO_AB_DATA_LONG             = 0x06070012UL, /**< 0x06070012, Data type does not match, length of service parameter too high */
    CO_SDO_AB_DATA_SHORT            = 0x06070013UL, /**< 0x06070013, Data type does not match, length of service parameter too short */
    CO_SDO_AB_SUB_UNKNOWN           = 0x06090011UL, /**< 0x06090011, Sub index does not exist */
    CO_SDO_AB_INVALID_VALUE         = 0x06090030UL, /**< 0x06090030, Invalid value for parameter (download only). */
    CO_SDO_AB_VALUE_HIGH            = 0x06090031UL, /**< 0x06090031, Value range of parameter written too high */
    CO_SDO_AB_VALUE_LOW             = 0x06090032UL, /**< 0x06090032, Value range of parameter written too low */
    CO_SDO_AB_MAX_LESS_MIN          = 0x06090036UL, /**< 0x06090036, Maximum value is less than minimum value. */
    CO_SDO_AB_NO_RESOURCE           = 0x060A0023UL, /**< 0x060A0023, Resource not available: SDO connection */
    CO_SDO_AB_GENERAL               = 0x08000000UL, /**< 0x08000000, General error */
    CO_SDO_AB_DATA_TRANSF           = 0x08000020UL, /**< 0x08000020, Data cannot be transferred or stored to application */
    CO_SDO_AB_DATA_LOC_CTRL         = 0x08000021UL, /**< 0x08000021, Data cannot be transferred or stored to application because of local control */
    CO_SDO_AB_DATA_DEV_STATE        = 0x08000022UL, /**< 0x08000022, Data cannot be transferred or stored to application because of present device state */
    CO_SDO_AB_DATA_OD               = 0x08000023UL, /**< 0x08000023, Object dictionary not present or dynamic generation fails */
    CO_SDO_AB_NO_DATA               = 0x08000024UL  /**< 0x08000024, No data available */
}CO_SDO_abortCode_t;

/**
 * Return values from SDO server or client functions.
 */
typedef enum
{
	CO_SDO_RT_idle					= 0,	/** SDO is free */
	CO_SDO_RT_busy 					= 1,	/** Node sending, internal processing or waiting response */
    CO_SDO_RT_success 				= 2,	/** Success, end of communication */
	CO_SDO_RT_abort 				= 3,	/** Abort / fail */
} CO_SDO_return_t;

struct CO_SDO_t {
    bool		valid;
	CO_CAN_Msg 	rx_msg;
    CO_CAN_Msg 	tx_msg;

	uint16_t	buffer_offset;		//<< Offset in buffer of next data segment being read/written
	uint16_t	trans_data_len;		//<< Size of data, which will be transferred. It is optionally
									//indicated by client in case of download or by server in case of upload
    uint8_t		toggle;

    uint16_t	trans_type;
    uint32_t	sync_mask_reg;

	uint16_t	object_no;			//<< Only used for SDO server
    CO_OD*		p_od;				//<< Only used for SDO server
	uint16_t	index;
	uint8_t 	sub_index;

	CO_Sub_Object* p_sub_obj;
	uint8_t*	p_data_itf;			//<<< data pointer interfaces, used in the case the device is server + download mode or client + upload mode


	uint16_t	timeout_cter_ms;
	uint16_t	allowed_timeout_ms;

    CO_SYNC*	p_sync;
    bool		is_internal_processing;
    uint32_t	tx_abort_code;
    uint32_t	rx_abort_code;
    CO_SDO_state_t state;
    CO_SDO_return_t status;

	/* Block variable */
	uint8_t		blksize;			/** Number of segments per block with 1 <= blksize <= 127 */
	bool		crcEnabled;			/** True, if CRC calculation by block transfer is enabled */
	uint16_t	crc;				/** Calculated CRC code */
	uint8_t 	lastLen;			/** Length of data in the last segment in block upload */
	bool		endOfTransfer;		/** Indication end of block transfer */
};

void CO_SDO_abort_and_response(CO_SDO* p_sdo,const uint32_t code);
void CO_SDO_abort(CO_SDO *p_sdo, const uint32_t code);
static inline void CO_SDO_set_tx_abort_code(CO_SDO* p_sdo,const uint32_t code)
{
	p_sdo->tx_abort_code = code;
}
static inline void CO_SDO_reset_tx_abort_code(CO_SDO* p_sdo)
{
	p_sdo->tx_abort_code = CO_SDO_AB_NONE;
}

static inline CO_SDO_return_t CO_SDO_get_status(CO_SDO* p_sdo)
{
	return p_sdo->status;
}
static inline void CO_SDO_reset_status(CO_SDO* p_sdo)
{
	p_sdo->status = CO_SDO_RT_idle;
}
void CO_SDO_set_transmission_type(CO_SDO *p_sdo, uint16_t type);
void CO_SDO_set_sync_mask_reg(CO_SDO *p_sdo, uint32_t mask);
/************************************************************************************************************
 * SDO timeout function
 ************************************************************************************************************/
static inline void CO_SDOtimeout_reset_and_activate(CO_SDO *p_sdo)
{
	p_sdo->timeout_cter_ms = 1;
}

static inline void CO_SDOtimeout_reset_and_inactivate(CO_SDO *p_sdo)
{
	p_sdo->timeout_cter_ms = 0;
}

static inline void CO_SDOtimeout_increase(CO_SDO *p_sdo, uint16_t time_diff_ms)
{
	p_sdo->timeout_cter_ms += time_diff_ms;
}

static inline bool CO_SDOtimeout_is_timeout(CO_SDO *p_sdo)
{
	return (p_sdo->timeout_cter_ms > p_sdo->allowed_timeout_ms);
}

static inline bool CO_SDOtimeout_is_activated(CO_SDO *p_sdo)
{
	return (p_sdo->timeout_cter_ms > 0);
}

#endif
