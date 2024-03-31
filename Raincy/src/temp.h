#pragma once
#define option_of(X) struct {X val; bool tag;}
#define is_some(v) v.tag
#define is_none(v) !v.tag



//#define unwrap(v) v.tag ? v.val : assert(false, "Unreachable")
#if defined(__cplusplus)
	#define CLITERAL(type)      type
#else
	#define CLITERAL(type)      (type)
#endif
//#define Some(T, v) CLITERAL(option_of(T)) {.val=v, .tag=true}
#define Some(v) {.val=v, .tag=true}
//#define None(T) CLITERAL(option_of(T)) {.tag=false}
#define None {.tag=false}


#define unwrap_or(v, r) v.tag ? v.val : r

//#define _tuple_t(T, ...) decltype(T) T, _tuple_t(__VA_ARGS__)
//#define _tuple_v(T, ...) .T=T, _tuple_v(__VA_ARGS__)
////#define tuple_of(...) 
//#define tuplev(...) (struct {_tuple_t(__VA_ARGS__)}) {_tuple_v(__VA_ARGS__)}
