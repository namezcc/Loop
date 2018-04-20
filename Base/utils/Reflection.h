#ifndef REFLECTION_H
#define REFLECTION_H
#include <array>
#include <map>
#include <string>
#include <sstream>
#include <stdint.h>
using namespace std;

#define TO_STRING( x ) TO_STRING1( x )
#define TO_STRING1( x ) #x

#define CONCAT(A,B) CONCAT1(A,B)
#define CONCAT1(A,B) A##B

#define CONCATSTR(A,B) TO_STRING(CONCAT(A,B))

#define TABLE_FLAG 
#define FIELD_FLAG 

#define MAKE_ARG_LIST_1(op,t,f, ...)   op(t,f)
#define MAKE_ARG_LIST_2(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_1(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_3(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_2(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_4(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_3(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_5(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_4(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_6(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_5(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_7(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_6(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_8(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_7(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_9(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_8(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_10(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_9(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_11(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_10(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_12(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_11(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_13(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_12(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_14(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_13(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_15(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_14(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_16(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_15(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_17(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_16(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_18(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_17(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_19(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_18(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_20(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_19(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_21(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_20(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_22(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_21(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_23(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_22(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_24(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_23(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_25(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_24(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_26(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_25(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_27(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_26(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_28(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_27(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_29(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_28(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_30(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_29(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_31(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_30(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_32(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_31(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_33(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_32(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_34(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_33(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_35(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_34(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_36(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_35(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_37(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_36(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_38(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_37(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_39(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_38(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_40(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_39(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_41(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_40(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_42(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_41(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_43(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_42(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_44(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_43(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_45(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_44(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_46(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_45(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_47(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_46(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_48(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_47(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_49(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_48(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_50(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_49(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_51(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_50(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_52(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_51(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_53(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_52(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_54(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_53(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_55(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_54(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_56(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_55(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_57(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_56(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_58(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_57(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_59(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_58(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_60(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_59(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_61(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_60(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_62(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_61(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_63(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_62(op,t, __VA_ARGS__))
#define MAKE_ARG_LIST_64(op,t,f, ...)   op(t,f), MARCO_EXPAND(MAKE_ARG_LIST_63(op,t, __VA_ARGS__))

#define MAKE_FUNC_LIST_1(op,t,N,f,...) op(t,f,N,1)
#define MAKE_FUNC_LIST_2(op,t,N,f,...) op(t,f,N,2) MARCO_EXPAND(MAKE_FUNC_LIST_1(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_3(op,t,N,f,...) op(t,f,N,3) MARCO_EXPAND(MAKE_FUNC_LIST_2(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_4(op,t,N,f,...) op(t,f,N,4) MARCO_EXPAND(MAKE_FUNC_LIST_3(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_5(op,t,N,f,...) op(t,f,N,5) MARCO_EXPAND(MAKE_FUNC_LIST_4(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_6(op,t,N,f,...) op(t,f,N,6) MARCO_EXPAND(MAKE_FUNC_LIST_5(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_7(op,t,N,f,...) op(t,f,N,7) MARCO_EXPAND(MAKE_FUNC_LIST_6(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_8(op,t,N,f,...) op(t,f,N,8) MARCO_EXPAND(MAKE_FUNC_LIST_7(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_9(op,t,N,f,...) op(t,f,N,9) MARCO_EXPAND(MAKE_FUNC_LIST_8(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_10(op,t,N,f,...) op(t,f,N,10) MARCO_EXPAND(MAKE_FUNC_LIST_9(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_11(op,t,N,f,...) op(t,f,N,11) MARCO_EXPAND(MAKE_FUNC_LIST_10(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_12(op,t,N,f,...) op(t,f,N,12) MARCO_EXPAND(MAKE_FUNC_LIST_11(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_13(op,t,N,f,...) op(t,f,N,13) MARCO_EXPAND(MAKE_FUNC_LIST_12(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_14(op,t,N,f,...) op(t,f,N,14) MARCO_EXPAND(MAKE_FUNC_LIST_13(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_15(op,t,N,f,...) op(t,f,N,15) MARCO_EXPAND(MAKE_FUNC_LIST_14(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_16(op,t,N,f,...) op(t,f,N,16) MARCO_EXPAND(MAKE_FUNC_LIST_15(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_17(op,t,N,f,...) op(t,f,N,17) MARCO_EXPAND(MAKE_FUNC_LIST_16(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_18(op,t,N,f,...) op(t,f,N,18) MARCO_EXPAND(MAKE_FUNC_LIST_17(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_19(op,t,N,f,...) op(t,f,N,19) MARCO_EXPAND(MAKE_FUNC_LIST_18(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_20(op,t,N,f,...) op(t,f,N,20) MARCO_EXPAND(MAKE_FUNC_LIST_19(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_21(op,t,N,f,...) op(t,f,N,21) MARCO_EXPAND(MAKE_FUNC_LIST_20(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_22(op,t,N,f,...) op(t,f,N,22) MARCO_EXPAND(MAKE_FUNC_LIST_21(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_23(op,t,N,f,...) op(t,f,N,23) MARCO_EXPAND(MAKE_FUNC_LIST_22(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_24(op,t,N,f,...) op(t,f,N,24) MARCO_EXPAND(MAKE_FUNC_LIST_23(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_25(op,t,N,f,...) op(t,f,N,25) MARCO_EXPAND(MAKE_FUNC_LIST_24(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_26(op,t,N,f,...) op(t,f,N,26) MARCO_EXPAND(MAKE_FUNC_LIST_25(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_27(op,t,N,f,...) op(t,f,N,27) MARCO_EXPAND(MAKE_FUNC_LIST_26(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_28(op,t,N,f,...) op(t,f,N,28) MARCO_EXPAND(MAKE_FUNC_LIST_27(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_29(op,t,N,f,...) op(t,f,N,29) MARCO_EXPAND(MAKE_FUNC_LIST_28(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_30(op,t,N,f,...) op(t,f,N,30) MARCO_EXPAND(MAKE_FUNC_LIST_29(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_31(op,t,N,f,...) op(t,f,N,31) MARCO_EXPAND(MAKE_FUNC_LIST_30(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_32(op,t,N,f,...) op(t,f,N,32) MARCO_EXPAND(MAKE_FUNC_LIST_31(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_33(op,t,N,f,...) op(t,f,N,33) MARCO_EXPAND(MAKE_FUNC_LIST_32(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_34(op,t,N,f,...) op(t,f,N,34) MARCO_EXPAND(MAKE_FUNC_LIST_33(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_35(op,t,N,f,...) op(t,f,N,35) MARCO_EXPAND(MAKE_FUNC_LIST_34(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_36(op,t,N,f,...) op(t,f,N,36) MARCO_EXPAND(MAKE_FUNC_LIST_35(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_37(op,t,N,f,...) op(t,f,N,37) MARCO_EXPAND(MAKE_FUNC_LIST_36(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_38(op,t,N,f,...) op(t,f,N,38) MARCO_EXPAND(MAKE_FUNC_LIST_37(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_39(op,t,N,f,...) op(t,f,N,39) MARCO_EXPAND(MAKE_FUNC_LIST_38(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_40(op,t,N,f,...) op(t,f,N,40) MARCO_EXPAND(MAKE_FUNC_LIST_39(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_41(op,t,N,f,...) op(t,f,N,41) MARCO_EXPAND(MAKE_FUNC_LIST_40(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_42(op,t,N,f,...) op(t,f,N,42) MARCO_EXPAND(MAKE_FUNC_LIST_41(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_43(op,t,N,f,...) op(t,f,N,43) MARCO_EXPAND(MAKE_FUNC_LIST_42(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_44(op,t,N,f,...) op(t,f,N,44) MARCO_EXPAND(MAKE_FUNC_LIST_43(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_45(op,t,N,f,...) op(t,f,N,45) MARCO_EXPAND(MAKE_FUNC_LIST_44(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_46(op,t,N,f,...) op(t,f,N,46) MARCO_EXPAND(MAKE_FUNC_LIST_45(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_47(op,t,N,f,...) op(t,f,N,47) MARCO_EXPAND(MAKE_FUNC_LIST_46(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_48(op,t,N,f,...) op(t,f,N,48) MARCO_EXPAND(MAKE_FUNC_LIST_47(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_49(op,t,N,f,...) op(t,f,N,49) MARCO_EXPAND(MAKE_FUNC_LIST_48(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_50(op,t,N,f,...) op(t,f,N,50) MARCO_EXPAND(MAKE_FUNC_LIST_49(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_51(op,t,N,f,...) op(t,f,N,51) MARCO_EXPAND(MAKE_FUNC_LIST_50(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_52(op,t,N,f,...) op(t,f,N,52) MARCO_EXPAND(MAKE_FUNC_LIST_51(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_53(op,t,N,f,...) op(t,f,N,53) MARCO_EXPAND(MAKE_FUNC_LIST_52(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_54(op,t,N,f,...) op(t,f,N,54) MARCO_EXPAND(MAKE_FUNC_LIST_53(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_55(op,t,N,f,...) op(t,f,N,55) MARCO_EXPAND(MAKE_FUNC_LIST_54(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_56(op,t,N,f,...) op(t,f,N,56) MARCO_EXPAND(MAKE_FUNC_LIST_55(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_57(op,t,N,f,...) op(t,f,N,57) MARCO_EXPAND(MAKE_FUNC_LIST_56(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_58(op,t,N,f,...) op(t,f,N,58) MARCO_EXPAND(MAKE_FUNC_LIST_57(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_59(op,t,N,f,...) op(t,f,N,59) MARCO_EXPAND(MAKE_FUNC_LIST_58(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_60(op,t,N,f,...) op(t,f,N,60) MARCO_EXPAND(MAKE_FUNC_LIST_59(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_61(op,t,N,f,...) op(t,f,N,61) MARCO_EXPAND(MAKE_FUNC_LIST_60(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_62(op,t,N,f,...) op(t,f,N,62) MARCO_EXPAND(MAKE_FUNC_LIST_61(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_63(op,t,N,f,...) op(t,f,N,63) MARCO_EXPAND(MAKE_FUNC_LIST_62(op,t,N,__VA_ARGS__))
#define MAKE_FUNC_LIST_64(op,t,N,f,...) op(t,f,N,64) MARCO_EXPAND(MAKE_FUNC_LIST_63(op,t,N,__VA_ARGS__))

#define SEPERATOR ,
#define CON_STR_1(e, ...) CONCATSTR(FIELD_FLAG,e)
#define CON_STR_2(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_1(__VA_ARGS__))
#define CON_STR_3(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_2(__VA_ARGS__))
#define CON_STR_4(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_3(__VA_ARGS__))
#define CON_STR_5(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_4(__VA_ARGS__))
#define CON_STR_6(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_5(__VA_ARGS__))
#define CON_STR_7(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_6(__VA_ARGS__))
#define CON_STR_8(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_7(__VA_ARGS__))
#define CON_STR_9(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_8(__VA_ARGS__))
#define CON_STR_10(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_9(__VA_ARGS__))
#define CON_STR_11(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_10(__VA_ARGS__))
#define CON_STR_12(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_11(__VA_ARGS__))
#define CON_STR_13(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_12(__VA_ARGS__))
#define CON_STR_14(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_13(__VA_ARGS__))
#define CON_STR_15(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_14(__VA_ARGS__))
#define CON_STR_16(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_15(__VA_ARGS__))
#define CON_STR_17(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_16(__VA_ARGS__))
#define CON_STR_18(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_17(__VA_ARGS__))
#define CON_STR_19(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_18(__VA_ARGS__))
#define CON_STR_20(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_19(__VA_ARGS__))
#define CON_STR_21(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_20(__VA_ARGS__))
#define CON_STR_22(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_21(__VA_ARGS__))
#define CON_STR_23(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_22(__VA_ARGS__))
#define CON_STR_24(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_23(__VA_ARGS__))
#define CON_STR_25(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_24(__VA_ARGS__))
#define CON_STR_26(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_25(__VA_ARGS__))
#define CON_STR_27(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_26(__VA_ARGS__))
#define CON_STR_28(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_27(__VA_ARGS__))
#define CON_STR_29(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_28(__VA_ARGS__))
#define CON_STR_30(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_29(__VA_ARGS__))
#define CON_STR_31(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_30(__VA_ARGS__))
#define CON_STR_32(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_31(__VA_ARGS__))
#define CON_STR_33(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_32(__VA_ARGS__))
#define CON_STR_34(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_33(__VA_ARGS__))
#define CON_STR_35(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_34(__VA_ARGS__))
#define CON_STR_36(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_35(__VA_ARGS__))
#define CON_STR_37(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_36(__VA_ARGS__))
#define CON_STR_38(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_37(__VA_ARGS__))
#define CON_STR_39(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_38(__VA_ARGS__))
#define CON_STR_40(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_39(__VA_ARGS__))
#define CON_STR_41(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_40(__VA_ARGS__))
#define CON_STR_42(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_41(__VA_ARGS__))
#define CON_STR_43(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_42(__VA_ARGS__))
#define CON_STR_44(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_43(__VA_ARGS__))
#define CON_STR_45(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_44(__VA_ARGS__))
#define CON_STR_46(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_45(__VA_ARGS__))
#define CON_STR_47(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_46(__VA_ARGS__))
#define CON_STR_48(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_47(__VA_ARGS__))
#define CON_STR_49(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_48(__VA_ARGS__))
#define CON_STR_50(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_49(__VA_ARGS__))
#define CON_STR_51(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_50(__VA_ARGS__))
#define CON_STR_52(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_51(__VA_ARGS__))
#define CON_STR_53(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_52(__VA_ARGS__))
#define CON_STR_54(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_53(__VA_ARGS__))
#define CON_STR_55(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_54(__VA_ARGS__))
#define CON_STR_56(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_55(__VA_ARGS__))
#define CON_STR_57(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_56(__VA_ARGS__))
#define CON_STR_58(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_57(__VA_ARGS__))
#define CON_STR_59(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_58(__VA_ARGS__))
#define CON_STR_60(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_59(__VA_ARGS__))
#define CON_STR_61(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_60(__VA_ARGS__))
#define CON_STR_62(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_61(__VA_ARGS__))
#define CON_STR_63(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_62(__VA_ARGS__))
#define CON_STR_64(e, ...) CONCATSTR(FIELD_FLAG,e) SEPERATOR MARCO_EXPAND(CON_STR_63(__VA_ARGS__))

#define RSEQ_N() \
		 119,118,117,116,115,114,113,112,111,110,\
		 109,108,107,106,105,104,103,102,101,100,\
		 99,98,97,96,95,94,93,92,91,90, \
		 89,88,87,86,85,84,83,82,81,80, \
		 79,78,77,76,75,74,73,72,71,70, \
		 69,68,67,66,65,64,63,62,61,60, \
         59,58,57,56,55,54,53,52,51,50, \
         49,48,47,46,45,44,43,42,41,40, \
         39,38,37,36,35,34,33,32,31,30, \
         29,28,27,26,25,24,23,22,21,20, \
         19,18,17,16,15,14,13,12,11,10, \
         9,8,7,6,5,4,3,2,1,0

#define ARG_N( \
         _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
		 _61,_62,_63,_64,_65,_66,_67,_68,_69,_70, \
		 _71,_72,_73,_74,_75,_76,_77,_78,_79,_80, \
		 _81,_82,_83,_84,_85,_86,_87,_88,_89,_90, \
		 _91,_92,_93,_94,_95,_96,_97,_98,_99,_100, \
		 _101,_102,_103,_104,_105,_106,_107,_108,_109,_110, \
         _111,_112,_113,_114,_115,_116,_117,_118,_119,N, ...) N


#define MARCO_EXPAND(...)                 __VA_ARGS__
#define GET_ARG_COUNT_INNER(...)    MARCO_EXPAND(ARG_N(__VA_ARGS__))
#define GET_ARG_N(...) GET_ARG_COUNT_INNER(__VA_ARGS__,RSEQ_N())

#define TO_STRING( x ) TO_STRING1( x )
#define TO_STRING1( x ) #x

#define MACRO_CONCAT(A, B) MACRO_CONCAT1(A, B)
#define MACRO_CONCAT1(A, B) A##_##B

#define MAKE_ARG_LISTS(N, op, t, arg, ...) \
       MARCO_EXPAND(MACRO_CONCAT(MAKE_ARG_LIST,N)(op, t, arg, __VA_ARGS__))

#define MAKE_FUNC_LISTS(N,F,t,a,...) \
		MARCO_EXPAND(MACRO_CONCAT(MAKE_FUNC_LIST,N)(F,t,N,a,__VA_ARGS__))

#define MAKE_STR_LIST(...) \
    MARCO_EXPAND(MACRO_CONCAT(CON_STR, GET_ARG_N(__VA_ARGS__))(__VA_ARGS__))

enum REFLECT_TYPE
{
	T_INT = 1,
	T_FLOAT = 2,
	T_INT64 = 3,
	T_DOUBLE = 4,
	T_STRING = 5,
};


template<typename T>
struct TypeID;

#define MAKE_TYPE_ID(T,ID) \
template<> struct TypeID<T> \
{constexpr static size_t id = ID;};

MAKE_TYPE_ID(int, REFLECT_TYPE::T_INT)
MAKE_TYPE_ID(float, REFLECT_TYPE::T_FLOAT)
MAKE_TYPE_ID(int64_t, REFLECT_TYPE::T_INT64)
MAKE_TYPE_ID(double, REFLECT_TYPE::T_DOUBLE)
MAKE_TYPE_ID(string, REFLECT_TYPE::T_STRING)

template<typename T>
struct ClassMember;
template<typename T, typename R>
struct ClassMember<R T::*>
{
	using type = R;
};

#define TYPEID(t,m) TypeID<ClassMember<decltype(&t::m)>::type>::id

template<typename T>
struct ReflectField
{
	T* ptr;
	int64_t flag;
	typedef map<string, int> FieldMap;

	ReflectField(T* p) :ptr(p),flag(0)
	{};

	static FieldMap& GetFieldMap()
	{
		static FieldMap m;
		return m;
	}

	static void SetVal(char* p, const int& tp, const string& val)
	{
		switch (tp)
		{
		case T_INT:
		{
			auto np = (int*)p;
			*np = std::stoi(val);
		}
		break;
		case T_FLOAT:
		{
			auto np = (float*)p;
			*np = std::stof(val);
		}
		break;
		case T_INT64:
		{
			auto np = (int64_t*)p;
			*np = std::stoll(val);
		}
		break;
		case T_DOUBLE:
		{
			auto np = (double*)p;
			*np = std::stod(val);
		}
		break;
		case T_STRING:
		{
			auto np = (string*)p;
			*np = val;
		}
		break;
		default:
			break;
		}
	}
	static string GetVal(char* p, const int& tp)
	{
		switch (tp)
		{
		case T_INT:
			return std::to_string(*(int*)p);
		case T_FLOAT:
			return std::to_string(*(float*)p);
		case T_INT64:
			return std::to_string(*(int64_t*)p);
		case T_DOUBLE:
			return std::to_string(*(double*)p);
		case T_STRING:
			return *(string*)p;
		}
		return "";
	}
};


template<typename T>
struct Reflect;

#define MAKE_FIELD_FUNC(T,f,N,i)	\
\
void Set_##f(const ClassMember<decltype(&T::f)>::type& v)\
{ptr->f=v;flag|=(((int64_t)1)<<(N-i));}

#define MAKE_REFLECT(T,N,...) \
template<> struct Reflect<T>:public ReflectField<T>{ \
	static constexpr int Size(){return N;} \
	static constexpr char const* Name(){return CONCATSTR(TABLE_FLAG,T);};	\
	static constexpr array<const char*,N> arr_fields = {MAKE_STR_LIST(__VA_ARGS__)}; \
	static constexpr array<int, N> arr_offset = { MAKE_ARG_LISTS(N,offsetof,T,__VA_ARGS__) }; \
	static constexpr array<int, N> arr_type = { MAKE_ARG_LISTS(N,TYPEID,T,__VA_ARGS__) }; \
	MAKE_FUNC_LISTS(N,MAKE_FIELD_FUNC,T,__VA_ARGS__) \
	Reflect(T* p):ReflectField(p) \
	{};	\
	static int GetFieldIndex(const string& f) \
	{ \
		auto& m = GetFieldMap();\
		if (m.size() == 0) \
			for (size_t i = 0; i < Reflect<T>::arr_fields.size(); i++) \
				m[string(Reflect<T>::arr_fields[i])] = i; \
		return m[f]; \
	}	\
	static void SetFieldValue(T& t,const string& f,const string& v) \
	{\
		int idx = Reflect::GetFieldIndex(f);	\
		char* p = (char*)(&t);	\
		SetVal(p+Reflect::arr_offset[idx],Reflect::arr_type[idx],v); \
	} \
}; 

//×î´ó64¸ö×Ö¶Î
#define REFLECT(T,...) \
	MAKE_REFLECT(T,GET_ARG_N(__VA_ARGS__),__VA_ARGS__)

enum SQL_FIELD_TYPE
{
	SQL_INT,
	SQL_BIGINT,
	SQL_VARCHAR,
	SQL_TIMESTAMP,
};

struct FieldDesc
{
	char* name;
	char type;
	short len;
	bool nullable;
	char* defval;
	bool index;
	char* comment;
};


template<typename T>
struct TableDesc;

template<typename T>
struct TableQuery
{
	static void InitTable(string& newName,function<void(string& query)> call)
	{
		string tableName = Reflect<T>::Name();
		if (!newName.empty())
			tableName = newName;
		string tablesql;
		tablesql.append("create table if not exists ").append(tableName);
		tablesql.append("(");
		vector<string> alters;
		vector<string> indexs;
		for (auto& f : TableDesc<T>::fields)
		{
			bool prim = false;
			for (auto& p : TableDesc<T>::paramkey)
			{
				if (strcmp(f.name,p)==0)
				{
					tablesql.append(GetFieldSql(f)).append(",");
					prim = true;
					break;
				}
			}
			if (prim)
				continue;
			if (f.index)
				indexs.push_back(f.name);
			alters.push_back(GetFieldSql(f).append(";"));
		}
		tablesql.append("primary key(");
		for (auto& p : TableDesc<T>::paramkey)
		{
			tablesql.append(p).append(",");
		}
		tablesql.pop_back();
		tablesql.append("));");
		call(tablesql);

		for (auto& s : alters)
		{
			string alter("alter table ");
			alter.append(tableName);
			alter.append(" add ").append(s);
			call(alter);
		}
		for (auto& s : indexs)
		{
			string alter("alter table ");
			alter.append(tableName);
			alter.append(" add index ").append(s).append("(").append(s).append(");");
			call(alter);
		}
	}
private:
	static string GetFieldSql(const FieldDesc& f)
	{
		string sql;
		sql.append(f.name);
		switch (f.type)
		{
		case SQL_INT:
			sql.append(" int");
			break;
		case SQL_BIGINT:
			sql.append(" bigint");
			break;
		case SQL_VARCHAR:
			sql.append(" varchar");
			break;
		case SQL_TIMESTAMP:
			sql.append(" timestamp");
			break;
		default:
			throw exception("error SQL_TYPE");
		}
		if(f.len>0)
			sql.append("(").append(std::to_string(f.len)).append(")");
		if (!f.nullable)
			sql.append(" NOT NULL");
		if (string(f.defval).size()>0)
			sql.append(" default ").append(f.defval);
		if (string(f.comment).size()>0)
			sql.append(" comment ").append(f.comment);
		return sql;
	}
};


#define MAKE_PRIMKEY(N,...) \
	static constexpr array<const char*,N> paramkey{MAKE_STR_LIST(__VA_ARGS__)};

#define PRIMKEY(...) \
	MAKE_PRIMKEY(GET_ARG_N(__VA_ARGS__),__VA_ARGS__)

#define TABLE_DESC_BEGAN(T,...) \
template<> struct TableDesc<T>{ \
	PRIMKEY(__VA_ARGS__)	\
	static constexpr array<FieldDesc, Reflect<T>::Size()> fields = {

#define FIELD_DESC(field,type,len,nullable,defval,index,comment)	\
	FieldDesc{CONCATSTR(FIELD_FLAG,field),type,len,nullable,defval,index,comment},

#define TABLE_DESC_END };};

#endif