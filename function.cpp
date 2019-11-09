#include <memory>
#include <iostream>

namespace pointer_heap
{
    template <typename T>
    class function;

    template <typename ReturnType, typename... ArgsType>
    class function<ReturnType(ArgsType...)>
    {
        private:

            template<typename Functor>
            static ReturnType invoke(Functor* f, ArgsType&&... args)
            {
                return (*f)(std::forward<ArgsType>(args)...);
            }

            /*Do not forget that this function should to be static*/
            template<typename Functor>
            static void create(Functor* des, Functor* src)
            {
                new (des) Functor(*src);
            }

            template<typename Functor>
            static void destroy(Functor* f)
            {
                f->~Functor();
            }
            
            using invokePtr_t = ReturnType(*)(void*, ArgsType&&...);
            using createPtr_t = void(*)(void*, void*);
            using destroyPtr_t = void(*)(void*);

            invokePtr_t invokePtr;
            createPtr_t createPtr;
            destroyPtr_t destroyPtr;
            size_t storageSize;
            std::unique_ptr<char[]> storage;       

        public:
            template<typename Functor>
            function(Functor f) : invokePtr(reinterpret_cast<invokePtr_t> (invoke<Functor>)),
                                  createPtr(reinterpret_cast<createPtr_t> (create<Functor>)),
                                  destroyPtr(reinterpret_cast<destroyPtr_t>(destroy<Functor>)),
                                  storageSize(sizeof(Functor)),
                                  storage(new char[storageSize])  
            {
                createPtr(storage.get(), std::addressof(f));
            }

            ReturnType operator()(ArgsType&&... args) const
            {
                return (*invokePtr)(storage.get(), std::forward<ArgsType>(args)...);
            }

            function() = default;

            ~function()
            {
                if(storage != nullptr)
                {
                    destroyPtr (storage.get());
                }
            }

            function(const function& other)
            {
                if(other.storage != nullptr)
                {
                    invokePtr = other.invokePtr;
                    createPtr  = other.createPtr;
                    destroyPtr = other.destroyPtr;

                    storageSize = other.storageSize;
                    storage.reset (new char[storageSize]);
                    createPtr (storage.get(),other.storage.get());
                }
            }

            function& operator=(const function& other)
            {
                if(storage != nullptr)
                {
                    destroyPtr (storage.get());
                    storage.reset();
                }

                if(other.storage != nullptr)
                {
                    invokePtr = other.invokePtr;
                    createPtr  = other.createPtr;
                    destroyPtr = other.destroyPtr;

                    storageSize = other.storageSize;
                    storage.reset (new char[storageSize]);
                    createPtr (storage.get(),other.storage.get());
                }

                return *this;
            }
    };
} // namespace pointer_heap


void func(int)
{
    std::cout<<"This is marco"<<std::endl;
}

int main()
{
    pointer_heap::function<void(int)> f = func;
    f(2);
}
