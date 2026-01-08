#pragma once

#define PP_SEQ_HEAD_0(x) x, PP_NIL

#define PP_SEQ_HEAD(seq) PP_SEQ_HEAD_I(seq)
#define PP_SEQ_HEAD_I(seq) PP_SEQ_HEAD_II(PP_SEQ_HEAD_0 seq)
#define PP_SEQ_HEAD_II(im) PP_SEQ_HEAD_III(im)
#define PP_SEQ_HEAD_III(x, _) x

#define PP_SEQ_TAIL_I(x)
#define PP_SEQ_TAIL(seq) PP_SEQ_TAIL_I seq

#define PP_CAT_II(p, res) res
#define PP_CAT_I(a, b) PP_CAT_II(~, a ## b)
#define PP_CAT(a, b, ...) PP_CAT_I(a, b)
#define PP_CAT3(a, b, c) PP_CAT(PP_CAT(a, b), c)
#define PP_CAT4(a, b, c, d) PP_CAT(PP_CAT(PP_CAT(a, b), c), d)

#define PP_SEQ_R_I(_) PP_SEQ_R_II
#define PP_SEQ_R_II(_) PP_SEQ_R_III
#define PP_SEQ_R_III(_) PP_SEQ_R_II

#define PP_SEQ_R_TRUE(r) r
#define PP_SEQ_R_FALSE(r) X
#define PP_SEQ_R_PP_SEQ_R_I PP_SEQ_R_FALSE
#define PP_SEQ_R_PP_SEQ_R_II PP_SEQ_R_TRUE
#define PP_SEQ_R_PP_SEQ_R_III PP_SEQ_R_TRUE
#define PP_SEQ_R_H(r, seq) PP_CAT(PP_SEQ_R_, PP_SEQ_R_I seq)(r)
#define PP_SEQ_R(r, seq) PP_SEQ_R_H(r, PP_SEQ_TAIL(seq))

#define PP_SEQ_FOR_EACH_X(macro, data, seq)

#define PP_SEQ_FOR_EACH_0(macro, data, seq) macro(data, PP_SEQ_HEAD(seq)) PP_CAT(PP_SEQ_FOR_EACH_, PP_SEQ_R(1, seq))(macro, data, PP_SEQ_TAIL(seq))
#define PP_SEQ_FOR_EACH_1(macro, data, seq) macro(data, PP_SEQ_HEAD(seq)) PP_CAT(PP_SEQ_FOR_EACH_, PP_SEQ_R(2, seq))(macro, data, PP_SEQ_TAIL(seq))
#define PP_SEQ_FOR_EACH_2(macro, data, seq) macro(data, PP_SEQ_HEAD(seq)) PP_CAT(PP_SEQ_FOR_EACH_, PP_SEQ_R(3, seq))(macro, data, PP_SEQ_TAIL(seq))
#define PP_SEQ_FOR_EACH_3(macro, data, seq) macro(data, PP_SEQ_HEAD(seq)) PP_CAT(PP_SEQ_FOR_EACH_, PP_SEQ_R(4, seq))(macro, data, PP_SEQ_TAIL(seq))
#define PP_SEQ_FOR_EACH_4(macro, data, seq) macro(data, PP_SEQ_HEAD(seq)) PP_CAT(PP_SEQ_FOR_EACH_, PP_SEQ_R(5, seq))(macro, data, PP_SEQ_TAIL(seq))
#define PP_SEQ_FOR_EACH_5(macro, data, seq) macro(data, PP_SEQ_HEAD(seq)) PP_CAT(PP_SEQ_FOR_EACH_, PP_SEQ_R(6, seq))(macro, data, PP_SEQ_TAIL(seq))
#define PP_SEQ_FOR_EACH_6(macro, data, seq) macro(data, PP_SEQ_HEAD(seq)) PP_CAT(PP_SEQ_FOR_EACH_, PP_SEQ_R(7, seq))(macro, data, PP_SEQ_TAIL(seq))
#define PP_SEQ_FOR_EACH_7(macro, data, seq) macro(data, PP_SEQ_HEAD(seq)) PP_CAT(PP_SEQ_FOR_EACH_, PP_SEQ_R(8, seq))(macro, data, PP_SEQ_TAIL(seq))
#define PP_SEQ_FOR_EACH_8(macro, data, seq) macro(data, PP_SEQ_HEAD(seq)) PP_CAT(PP_SEQ_FOR_EACH_, PP_SEQ_R(9, seq))(macro, data, PP_SEQ_TAIL(seq))
#define PP_SEQ_FOR_EACH_9(macro, data, seq) macro(data, PP_SEQ_HEAD(seq)) PP_CAT(PP_SEQ_FOR_EACH_, PP_SEQ_R(10, seq))(macro, data, PP_SEQ_TAIL(seq))

#define PP_UNWRAP(t) PP_UNWRAP_I t
#define PP_UNWRAP_I(...) __VA_ARGS__

#define PP_CONCAT_TUPLES(t1, t2) (PP_UNWRAP(t1), PP_UNWRAP(t2))

#define PP_SEQ_FOR_EACH(macro, data, seq) PP_SEQ_FOR_EACH_0(macro, data, seq)
#define PP_INVOKER(macro, arg) macro(arg)
#define PP_SEQ_FOR_EACH_SIMPLE(macro, seq) PP_SEQ_FOR_EACH(PP_INVOKER, macro, seq)
#define PP_UNPACKER(macro, arg) macro arg
#define PP_SEQ_FOR_EACH_UNPACK(macro, seq) PP_SEQ_FOR_EACH(PP_UNPACKER, macro, seq)
#define PP_EXTRACT_MACRO(macro, data) macro
#define PP_EXTRACT_DATA(macro, data) data
#define PP_REPACKER(data, arg) PP_UNPACKER(PP_EXTRACT_MACRO data, PP_CONCAT_TUPLES(PP_EXTRACT_DATA data, arg))
#define PP_SEQ_FOR_EACH_REPACK(macro, data, seq) PP_SEQ_FOR_EACH(PP_REPACKER, (macro, data), seq)
