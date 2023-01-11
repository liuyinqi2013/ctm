#ifndef CTM_IO_DEFER_H__
#define CTM_IO_DEFER_H__

#include <stdio.h>
#include <memory>

namespace ctm {
    template <typename F>
    class Defer
    {
    public:
        Defer(F f) : _func(f) { }
        ~Defer() { _func(); }
    private:
        F _func;
    };

    template <typename F>
    std::shared_ptr<Defer<F> > MakeDefer(F && f) {
        return std::make_shared<Defer<F> >(forward<F>(f));
    }

    #define defer(f)   auto df  = MakeDefer(f)
    #define defer1(f)  auto df1 = MakeDefer(f)
    #define defer2(f)  auto df2 = MakeDefer(f)
    #define defer3(f)  auto df3 = MakeDefer(f)
    #define defer4(f)  auto df4 = MakeDefer(f)
    #define defer5(f)  auto df5 = MakeDefer(f)
}

#endif
