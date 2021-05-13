#pragma once

#include <utility>

#include <wil/com.h>
#include <wil/cppwinrt.h>

#include <winrt/base.h>

namespace com
{
    template<typename T, typename... I>
    class object : public winrt::implements<T, I...>
    {
        template<typename Policy>
        struct make_impl
        {
            template<typename... Args>
            wil::com_ptr_t<T, Policy> operator()(Args&& ...args) const
            {
                wil::com_ptr_t<T, Policy> ptr;
                ptr.attach(static_cast<T*>(winrt::make<T>(std::forward<Args>(args)...).detach()));
                return ptr;
            }
        };
    public:
        template<typename Policy = wil::err_exception_policy>
        static constexpr auto make = make_impl<Policy>{};
    };
}