require_extension(EXT_ZICBOM);
DECLARE_XENVCFG_VARS(CBCFE);
require_envcfg(CBCFE);
MMU.clean_inval(BASE_RS1, true, true);
