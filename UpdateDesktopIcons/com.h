#pragma once

#include <utility>
#include <memory>

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
            auto operator()(Args&& ...args) const
            {
                auto rawPtr = winrt::make<T>(std::forward<Args>(args)...).detach();
                using U = std::pointer_traits<decltype(rawPtr)>::element_type;
                wil::com_ptr_t<U, Policy> ptr;
                ptr.attach(rawPtr);
                return ptr;
            }
        };
    public:
        template<typename Policy = wil::err_exception_policy>
        static constexpr auto make = make_impl<Policy>{};
    };
}