#ifndef SRC_HPP
#define SRC_HPP

#include <stdexcept>
#include <initializer_list>
#include <typeinfo>
#include <utility>
#include <type_traits>

namespace sjtu {

class any_ptr {
private:
    struct control_block_base {
        int ref_count;
        control_block_base() : ref_count(1) {}
        virtual ~control_block_base() = default;
        virtual const std::type_info& type() const = 0;
        virtual void* get_ptr() = 0;
    };

    template <typename T>
    struct control_block : control_block_base {
        T* ptr;
        control_block(T* p) : ptr(p) {}
        ~control_block() override { delete ptr; }
        const std::type_info& type() const override { return typeid(T); }
        void* get_ptr() override { return ptr; }
    };

    control_block_base* cb;

    void release() {
        if (cb) {
            if (--cb->ref_count == 0) {
                delete cb;
            }
            cb = nullptr;
        }
    }

public:
    any_ptr() : cb(nullptr) {}

    any_ptr(const any_ptr &other) : cb(other.cb) {
        if (cb) {
            cb->ref_count++;
        }
    }

    template <class T> 
    any_ptr(T *ptr) {
        if (ptr) {
            cb = new control_block<T>(ptr);
        } else {
            cb = nullptr;
        }
    }

    ~any_ptr() {
        release();
    }

    any_ptr &operator=(const any_ptr &other) {
        if (this != &other) {
            release();
            cb = other.cb;
            if (cb) {
                cb->ref_count++;
            }
        }
        return *this;
    }

    template <class T> 
    any_ptr &operator=(T *ptr) {
        release();
        if (ptr) {
            cb = new control_block<T>(ptr);
        }
        return *this;
    }

    template <class T> 
    typename std::remove_reference<T>::type &unwrap() {
        using RawT = typename std::remove_reference<T>::type;
        if (!cb) throw std::bad_cast();
        if (cb->type() != typeid(RawT)) throw std::bad_cast();
        return *static_cast<RawT*>(cb->get_ptr());
    }

    template <class T> 
    const typename std::remove_reference<T>::type &unwrap() const {
        using RawT = typename std::remove_reference<T>::type;
        if (!cb) throw std::bad_cast();
        if (cb->type() != typeid(RawT)) throw std::bad_cast();
        return *static_cast<RawT*>(cb->get_ptr());
    }
};

template <class T> 
any_ptr make_any_ptr(const T &t) { 
    return any_ptr(new T(t)); 
}

template <class T, class... Args>
any_ptr make_any_ptr(Args&&... args) {
    return any_ptr(new T{std::forward<Args>(args)...});
}

template <class T, class U, class... Args>
any_ptr make_any_ptr(std::initializer_list<U> il, Args&&... args) {
    return any_ptr(new T{il, std::forward<Args>(args)...});
}

}  // namespace sjtu

#endif
