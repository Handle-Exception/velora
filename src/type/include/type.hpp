#pragma once

#include <utility>
#include <memory>

#include "native.hpp"
#include <asio.hpp>

namespace velora::type
{
    /**
     * @brief Interface class that defines the interface for the implementation object.
     * 
     */
    class Interface
    {
        public:
        virtual ~Interface(){}
    };

    /**
     * @brief Dispatcher class that holds the implementation object and provides access to it through the interface.
     * 
     * @tparam InterfaceType The interface type
     * @tparam ImplType The implementation type
     */
    template<class InterfaceType, class ImplType>
    class Dispatcher : public InterfaceType
    {
        public:
            // no copy ctor
            Dispatcher(const Dispatcher & other) = delete;
            // no copy assign
            Dispatcher & operator = (const Dispatcher & other) = delete;
            // construct implementation object in place
            template<class ... Args>
            inline Dispatcher(Args && ... args)
            : _impl{std::forward<Args>(args)...}
            {}
            // construct Obj using move ctor
            explicit inline Dispatcher(ImplType && impl)
            : _impl{std::move(impl)}
            {}
            // move ctor
            inline Dispatcher(Dispatcher && other)
            :_impl{std::move(other._impl)}
            {}
            // move assign
            inline Dispatcher& operator =(Dispatcher && other)
            {
                if(this == &other)return *this;
                _impl = std::move(other._impl);
                return *this;
            }
            // dtor
            virtual ~Dispatcher(){}
            // access implementation object
            constexpr inline ImplType & getImpl(){return _impl;}
            // access const implementation object
            constexpr inline const ImplType & getImpl() const {return _impl;}
        private:
            ImplType _impl;
    };

    template<class ... Args>
    Dispatcher(Args && ... args) -> Dispatcher<Args...>;

    /**
     * @brief Implementation class that holds a unique_ptr to the implementation object
     * and provides access to it through the interface.
     * 
     * @tparam Interface The interface type
     * @tparam Dispather The dispatcher type
     */
    template<class Interface, template<class> class Dispather>
    class Implementation
    {
        public:
            using interface_type = Interface;            
            Implementation(std::nullptr_t)
            {
                static_assert("Implementation cannot be constructed with nullptr. Use default constructor instead.");
            }

            //ctor using move ctor of ImplType
            template<std::move_constructible ImplType>
            explicit inline Implementation(ImplType && impl)
            : _pimpl(std::make_unique<Dispather<ImplType>>(std::move(impl)))
            {}
            //move ctor
            Implementation(Implementation && other)
            : _pimpl{std::move(other._pimpl)}
            {}
            //move assign 
            Implementation & operator  = (Implementation && other) = default;
            //dtor
            virtual ~Implementation(){}
            // access object
            constexpr inline Interface & operator*() noexcept {return *_pimpl; }
            // access const object
            constexpr inline const Interface & operator*() const noexcept{return *_pimpl; }
            // access object pointer
            constexpr inline Interface * operator->() noexcept {return _pimpl.get(); }
            // access object pointer
            constexpr inline Interface * get() const noexcept {return _pimpl.get(); }
            // access const object pointer
            constexpr inline const Interface * operator->() const noexcept{return _pimpl.get(); }
            // checks whether own implementation object
            constexpr explicit operator bool() const noexcept { return bool(_pimpl);}

            //asynchronous constructs implementation in place
            template<class ImplType, class ... Args>
            static inline asio::awaitable<Implementation> construct(asio::use_awaitable_t<>, Args && ... args)
            {
                std::unique_ptr<Interface> pimpl = std::make_unique<Dispather<ImplType>>(co_await ImplType::asyncConstructor(std::forward<Args>(args)...));
                co_return Implementation(std::move(pimpl));
            }

            //constructs implementation in place
            template<class ImplType, class ... Args>
            static inline Implementation construct(Args && ... args)
            {
                std::unique_ptr<Interface> pimpl = std::make_unique<Dispather<ImplType>>(std::forward<Args>(args)...);
                return Implementation(std::move(pimpl));
            }


        protected:
            //ctor taking ovnership of unique ptr
            explicit inline Implementation(std::unique_ptr<Interface> pimpl)
            : _pimpl{std::move(pimpl)}
            {} 

        private:
            std::unique_ptr<Interface> _pimpl;
    };
}