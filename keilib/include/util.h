#ifndef _KEIUTIL_H_
#define _KEIUTIL_H_


#ifdef __cplusplus
extern "C"{
#endif

#define ACCESS_ONCE(x) (*(volatile typeof(x) *)&(x))
#define WRITE_ONCE(dst, src) (ACCESS_ONCE(dst) = (typeof(dst))(src))


#ifdef __cplusplus
}
#endif

#endif //_KEIUTIL_H_