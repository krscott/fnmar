#ifndef KRS_CC_EXT_H_
#define KRS_CC_EXT_H_

#if defined(__GNUC__) || defined(__clang__)
#define nodiscard __attribute__((warn_unused_result))
#else
#define nodiscard
#endif

#endif
