#ifndef IDEFIX_FXCMFIELDS_H
#define IDEFIX_FXCMFIELDS_H

namespace idefix {
// Custom FXCM FIX fields
  enum FXCM_FIX_FIELDS
  {
  	FXCM_SYM_ID                  = 9000, // Int
    FXCM_SYM_PRECISION           = 9001, // Int
    FXCM_SYM_POINT_SIZE          = 9002, // Float
    FXCM_SYM_INTEREST_BUY        = 9003, // Float
    FXCM_SYM_INTEREST_SELL       = 9004, // Float
    FXCM_SYM_SORT_ORDER          = 9005, // Int
    // 6-10 Reserved
    FXCM_TIMING_INTERVAL         = 9011, // Int
    FXCM_START_DATE              = 9012, // UTCDate
    FXCM_START_TIME              = 9013, // UTCTimeOnly
    FXCM_END_DATE                = 9014, // UTCDate
    FXCM_END_TIME                = 9015, // UTCTimeOnly
    FXCM_NO_PARAMS               = 9016, // Int
    FXCM_PARAM_NAME              = 9017, // String
    FXCM_PARAM_VALUE             = 9018, // String
    FXCM_SERVER_TIMEZONE         = 9019, // Int
    FXCM_CONTINOUS_FLAG          = 9020, // Int
    // 21-24 Not in use for Buyer Interface
    FXCM_REQUEST_REJECT_REASON   = 9025, // Int
    // 26-28 Not in use for Buyer Interface
    FXCM_ERROR_DETAILS           = 9029, // String
    FXCM_SERVER_TIMEZONE_NAME    = 9030, // String
    // 31-35 Not in use for Buyer Interface
    // 36-37 Reserved
    FXCM_USED_MARGIN             = 9038, // Float
    // 39 Reserved
    FXCM_POS_INTEREST            = 9040, // Float
    FXCM_POS_ID                  = 9041, // String
    FXCM_POS_OPEN_TIME           = 9042, // UTCTimestamp
    FXCM_CLOSE_SETTLE_PRICE      = 9043, // Float
    FXCM_POS_CLOSE_TIME          = 9044, // UTCTimestamp
    FXCM_MARGIN_CALL             = 9045, // String
    FXCM_USED_MARGIN3            = 9046, // Float
    FXCM_CASH_DAILY              = 9047, // Float
    FXCM_CLOSE_CLORDID           = 9048, // String
    FXCM_CLOSE_SECONDARY_CLORDID = 9049, // String
    FXCM_ORD_TYPE                = 9050, // String
    FXCM_ORD_STATUS              = 9051, // String
    FXCM_CLOSE_PNL               = 9052, // String
    FXCM_POS_COMMISSION          = 9053, // Float
    FXCM_CLOSE_ORDERID           = 9054, // String
    // 55-59 Not set
    FXCM_MAX_NO_RESULTS          = 9060, // Int
    FXCM_PEG_FLUCTUATE_PTS       = 9061, // Int
    // 62-75 Not set
    FXCM_SUBSCRIPTION_STATUS     = 9076, // String
    // 77 Not set
    FXCM_POS_ID_REF              = 9078, // String
    FXCM_CONTINGENCY_ID          = 9079, // String
    FXCM_FIELD_PRODUCT_ID        = 9080, // Int
    // 81-89 Not set
    FXCM_COND_DIST_STOP          = 9090, // Float
    FXCM_COND_DIST_LIMIT         = 9091, // Float
    FXCM_COND_DIST_ENTRY_STOP    = 9092, // Float
    FXCM_COND_DIST_ENTRY_LIMIT   = 9093, // Float
    FXCM_MAX_QUANTITY            = 9094, // Float
    FXCM_MIN_QUANTITY            = 9095, // Float
    FXCM_TRADING_STATUS          = 9096  // String
  };
};

#endif