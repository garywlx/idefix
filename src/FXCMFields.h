#ifndef IDEFIX_FXCMFIELDS_H
#define IDEFIX_FXCMFIELDS_H

namespace IDEFIX {
// Custom FXCM FIX fields
  enum FXCM_FIX_FIELDS
  {
  	FXCM_SYM_ID                = 9000,
    FXCM_SYM_PRECISION         = 9001,
    FXCM_SYM_POINT_SIZE        = 9002,
    FXCM_NO_PARAMS             = 9016,
    FXCM_PARAM_NAME            = 9017,
    FXCM_PARAM_VALUE           = 9018,
    FXCM_REQUEST_REJECT_REASON = 9025,
    FXCM_ERROR_DETAILS         = 9029,
    FXCM_USED_MARGIN           = 9038,
    FXCM_POS_INTEREST          = 9040,
    FXCM_POS_ID                = 9041,
    FXCM_POS_OPEN_TIME         = 9042,
    FXCM_CLOSE_SETTLE_PRICE    = 9043,
    FXCM_POS_CLOSE_TIME        = 9044,
    FXCM_MARGIN_CALL           = 9045,
    FXCM_ORD_TYPE              = 9050,
    FXCM_ORD_STATUS            = 9051,
    FXCM_CLOSE_PNL             = 9052,
    FXCM_POS_COMMISSION        = 9053,
    FXCM_PEG_FLUCTUATE_PTS     = 9061,
    FXCM_FIELD_PRODUCT_ID      = 9080,
    FXCM_TRADING_STATUS        = 9096
  };
};

#endif