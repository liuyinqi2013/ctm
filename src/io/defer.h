#ifndef CTM_IO_DEFER_H__
#define CTM_IO_DEFER_H__

#include <stdio.h>
#include <memory>

namespace ctm {
    template <typename F, typename T>
    class FuncArg1
    {
    public:
        FuncArg1(F f, T arg) : _func(f), _arg(arg) { }
        ~FuncArg1() { _func(_arg); }
    private:
        F _func;
        T _arg;
    };

    template <typename F, typename T, typename T1>
    class FuncArg2
    {
    public:
        FuncArg2(F f, T arg, T1 arg1) : _func(f), _arg(arg), _arg1(arg1) { }
        ~FuncArg2() { _func(_arg, _arg1); }
    private:
        F _func;
        T _arg;
        T1 _arg1;
    };

    template <typename O, typename F>
    class ObjFunc
    {
    public:
        ObjFunc(O& obj, F f) : _obj(obj), _f(f) { }
        ~ObjFunc() { (_obj.*_f)(); }
    private:
        O& _obj;
        F _f;
    };

    template <typename F, typename T>
    FuncArg1<F, T> Defer(F f, T t) {
        return FuncArg1<F, T>(f, t);
    }

    template <typename F, typename T, typename T1>
    FuncArg2<F, T, T1> Defer(F f, T t, T1 t1) {
        return FuncArg2<F, T, T1>(f, t, t1);
    }

    template <typename O, typename F>
    std::shared_ptr<ObjFunc<O, F> >
    DeferObj(O& o, F f) {
        return std::make_shared<ObjFunc<O, F> >(o, f);
    }

    #define defer(o, f) auto df = DeferObj(o, f)
}

#endif
