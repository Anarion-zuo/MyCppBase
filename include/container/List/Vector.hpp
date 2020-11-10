//
// Created by 左钰 on 2020/1/21.
//

#ifndef MYCPPLIB_VECTOR_HPP
#define MYCPPLIB_VECTOR_HPP

#include "../base/type_trait.hpp"
#include "../base/container_utility.hpp"
#include <iostream>
#include <container/base/Copier.hpp>

namespace anarion {


#pragma region memory
    template<typename T>
    void clear_space_impl(T *first, size_type num, true_type) {
        // nothing here
    }

    template<typename T>
    void clear_space_impl(T *first, size_type num, false_type) {
        for (size_type i = 0; i < num; ++i) {
            first[i].~T();
        }
    }

    // call destructors
    template<typename T>
    void clear_space(T *first, size_type num) {
        clear_space_impl(first, num, typename pod_traits<T>::is_pod());
    }

    template<typename T>
    T *new_space(size_type num) {
        if (num == 0) { return nullptr; }
        return (T*)operator new(num * sizeof(T));
    }
#pragma endregion

    // optimized copy
#pragma region optimized_copy
    /*
        The reverse functions follow the convention of [) regardless of its copying order
        The input dst and src would not be copied
    */
//    template<typename T>
//    void reverse_copy(T *dst, T *src, size_type num) {
//        reverse_copy_impl(dst, src, num, typename type_trait<T>::is_pod());
//    }
//
//    template<typename T>
//    void reverse_copy_impl(T *dst, T *src, size_type num, true_type) {
//        dst -= num;
//        src -= num;
//        memcpy(dst, src, num * sizeof(T));
//    }
//
//    template<typename T>
//    void reverse_copy_impl(T *dst, T *src, size_type num, false_type) {
//        for (size_type i = 0; i < num; ++i) {
//            --dst;
//            --src;
//            new(dst) T(*src);
//            src->~T();
//        }
//    }
//
//    template<typename T>
//    void reverse_move(T *dst, T *src, size_type num) {
//        reverse_move_impl(dst, src, typename type_trait<T>::has_move_ctor());
//    }
//
//    template<typename T>
//    void reverse_move_impl(T *dst, T *src, size_type num, true_type) {
//        for (size_type i = 0; i < num; ++i) {
//            --dst;
//            --src;
//            new(dst) T(move(*src));
//            src->~T();
//        }
//    }
//
//    template<typename T>
//    void reverse_move_impl(T *dst, T *src, size_type num, false_type) {
//        reverse_copy(dst, src, num);
//    }

#pragma endregion

/**
 * @class Vector Container holding elements in a continuous memory space.
 * @tparam T specifies the type of the elements.
 */
    template<typename T>
    class Vector {
    protected:
        T *begin = nullptr, *cur = nullptr, *end = nullptr;

        void expand_push() { resize(size() << 1u); }

        // a hole that must be filled is left
        void copy_back_expand(size_type index, size_type steps) {
            Copier<T> copier;
            size_type oldsize = size(), oldcap = capacity(), newsize = oldsize + steps;
            if (newsize > oldcap) {
                size_type newcap = newsize << 1ul;
                T *newbegin = new_space<T>(newcap);
                copier.move(newbegin, begin, index);
                copier.move(newbegin + index + steps, begin + index, oldsize - index);
                operator delete (begin, oldcap);
                begin = newbegin;
                cur = begin + newsize;
                end = begin + newcap;
                return;
            }
            copier.moveOverLap(begin + index + steps, begin + index, oldsize - index);
            cur += steps;
        }

        void copy_forward_expand(size_type index, size_type steps) {
            Copier<T> copier;
            size_type oldsize = size();
            if (steps > index) {
                throw IndexOutOfRange();
            }
            copier.moveOverLap(begin + index - steps, begin + index, oldsize - index);
            cur -= steps;
        }
        #pragma endregion

        void clearMove() {
            begin = nullptr;
            end = nullptr;
            cur = nullptr;
        }

    public:

        typedef T *iterator;

        iterator begin_iterator() const { return begin; }
        iterator end_iterator() const { return cur; }
        bool has_iterator(iterator it) const { return it < cur && it >= begin; }

        #pragma region ctors_assigns_dtor
        Vector() = default;

        /**
         * @param initialSize gives container this amount of memory jurisdiction. It gives the container initializeSize of in #T.
         * @details Can have 0 bytes of memory.
         */
        explicit Vector(size_type initialSize) {
            if (initialSize == 0) {
                return;
            }
            begin = new_space<T>(initialSize);
            end = (begin + initialSize);
            cur = begin;
        }
        /**
         * @param initialSize specifies initial memory size in the unit of #T.
         * @param rhs initialize all controlled memory with copies of this object by the copy constructor of T.
         * @details It uses operator new to invoke copy constructors on specified address.
         */
        Vector(size_type initialSize, const T &rhs) {
            if (initialSize == 0) {
                return;
            }
            begin = new_space<T>(initialSize);
            end = (begin + initialSize);
            cur = begin + initialSize;
            for (size_type i = 0; i < initialSize; ++i) {
                new(&begin[i]) T(rhs);
            }
        }

        /**
         * @param rhs copy constructor.
         */
        Vector(const Vector<T> &rhs) : begin(new_space<T>(rhs.size())), cur(begin + rhs.size()), end(cur) {
            if (rhs.empty()) {
                return;
            }
            Copier<T>().copy(begin, rhs.begin, rhs.size());
        }

        /**
         * @param rhs move constructor.
         */
        Vector(Vector<T> &&rhs) noexcept : begin(rhs.begin), cur(rhs.cur), end(rhs.end) {
            rhs.clearMove();
        }

        /**
         * @param initList
         */
        Vector(std::initializer_list<T> &&initList) {
            size_type length = initList.size();
            if (length == 0) { return; }
            begin = new_space<T>(length);
            end = begin + length;
            cur = begin + length;
            size_type index = 0;
            for (auto &item : initList) {
                new (&begin[index]) T(move(item));
                ++index;
            }
        }

        /**
         * @param p
         * @param len
         * @details take hold of the address range defined by these 2 params without actual copying.
         */
        Vector(T *p, size_type len) : begin(p), cur(begin + len), end(cur) {}   // move

        /**
         * @details implemented by clear().
         */
        virtual ~Vector() {
            clear();
        }

        Vector<T> &operator=(const Vector<T> &rhs) {
            if (&rhs == this) {
                return *this;
            }
            clear();
            // reset members
            begin = new_space<T>(rhs.size());
            cur = begin + rhs.size();
            end = cur;
            // make copy
//            copyCtorObjects(begin, rhs.begin, rhs.size());
            Copier<T>().copy(begin, rhs.begin, rhs.size());
            return *this;
        }

        Vector<T> &operator=(Vector<T> &&rhs) noexcept {
            clear();
            // reset members
            begin = rhs.begin;
            cur = rhs.cur;
            end = rhs.end;
            rhs.clearMove();
            return *this;
        }

        /**
         * @details calls in order 1) destructor, 2) operator delete.
         */
        void clear() {
            clear_space(begin, size());
            operator delete(begin, (end - begin) * sizeof(T));
            clearMove();
        }
        #pragma endregion

        #pragma region sizes
        constexpr bool empty() const {
            return !size();
        }

        /**
         * @return address length actually holding elements in the unit of sizeof(T).
         */
        constexpr size_type size() const {
            return cur - begin;
        }

        /**
         *
         * @return address length of all addresses under control.
         */
        constexpr size_type capacity() const {
            return end - begin;
        }

        /**
         *
         * @param new_size Change size into this new_size.
         * @details Virtually, this function change the output of capacity(). If the requesting new_size is larger than current size(), container address control is expanded and no more. If the requesting new_size is smaller than current size(), elements at the end of the container is deleted by calling their destructors.
         */
        void resize(size_type new_size) {
            if (new_size == 0) {
                clear();
                return;
            }
            size_type oldsize = size(), oldcap = capacity(), newcap, newsize;
            if (oldcap == 0) {
                begin = new_space<T>(new_size);
                cur = begin;
                end = begin + new_size;
                return;
            }
            if (new_size < oldsize) {
                newcap = new_size;
                newsize = new_size;
                // release the extra objects
//                clear_space(&begin[new_size], oldsize - new_size);
            } else {
                newcap = new_size;
                newsize = oldsize;
            }
            // create new space
            T *newp = new_space<T>(newcap);
            // move to new space
            Copier<T>().move(newp, begin, newsize);
            // release old space
            operator delete(begin, oldsize * sizeof(T));
            // update members
            begin = newp;
            cur = newp + newsize;
            end = newp + newcap;
        }
        #pragma endregion

        #pragma region back_front
        /**
         * @param o add this object to the end of the container by copy constructor.
         */
        void push_back(const T &o) {
            if (begin == nullptr) {
                resize(1);
            }
            if (cur == end) {
                expand_push();
            }
            new(cur++) T(o);
        }
        /**
         * @param o add this object to the end of the container by move constructor.
         */
        void push_back(T &&o) {
            if (begin == nullptr) {
                resize(1);
            }
            if (cur == end) {
                expand_push();
            }
            new(cur++) T(forward<T>(o));
        }

        /**
         * @return the last element of the container and remove it from the position.
         */
        T pop_back() {
            if (empty()) { throw EmptyContainer(); }
            --cur;
            T &last = *cur;
            return move(last);
        }

        /**
         * @return the first element of the container and remove it from the position.
         */
        T pop_front() {
            if (empty()) { throw EmptyContainer(); }
            T ret = move(begin[0]);
            remove(0ul);
            return move(ret);
        }
        #pragma endregion
        /**
         * @return element reference at the given index.
         */
        constexpr T &get(size_type index) {
            if (index >= size()) { throw IndexOutOfRange(); }
            return begin[index];
        }

        /**
         * @return element reference at the given index.
         * @details This is the const version, not changing the element from the returned reference.
         */
        constexpr const T &get(size_type index) const {
            if (index >= size()) { throw IndexOutOfRange(); }
            return begin[index];
        }

        /**
         * @return element reference at the given index.
         * @details wraps function get(index)
         */
        T &operator[](size_type index) {
            return get(index);
        }
        /**
         * @return element reference at the given index.
         * @details wraps function get(index).
         * @details This is the const version, not changing the element from the returned reference.
         */
        const T &operator[](size_type index) const {
            return get(index);
        }

        #pragma region insert
        /*
         * Inserting Single element.
         * All iterators are transformed into indices.
         *
         * In helper method @insertPrepare, necessary space in memory should be prepared.
         * In public methods @insert, the only extra work is running constructor upon give pointer.
         */
    protected:
        /**
         * The helper method for functions insert*.
         * @param index
         * @details moves all elements starting at given index to the right by 1.
         */
        void insertPrepare(size_type index) {
            if (capacity() == 0) {
                resize(1);
            }
            if (index > size()) { throw IndexOutOfRange(); }
//            if (index == size()) { push_back(o); return cur - 1; }
            copy_back_expand(index, 1);
        };
    public:
        // iterator transformed into index
        /**
         * @details Insert object o at the position of iterator it. After this operation, the element on the position of this old iterator should be the result of the copy constructor on object o.
         * @param it specifies an position in the container.
         * @param o
         * @return the iterator of the newly inserted object.
         * @details The parameter it may hold the same value as the return value, for they points to the same address. It may be the container expanded itself when copying, so that all old iterators are made invalid.
         * @details This wraps function insert(index, o).
         */
        iterator insert(iterator it, const T &o) {
            return insert(it - begin, o);
        }
        /**
         * @details Insert object o at the position of iterator it. After this operation, the element on the position of this old iterator should be the result of the move constructor on object o.
         * @param it specifies a position in the container.
         * @param o
         * @return the iterator of the newly inserted object.
         * @details The parameter it may hold the same value as the return value, for they points to the same address. It may be the container expanded itself when copying, so that all old iterators are made invalid.
         * @details This wraps function insert(index, o).
         */
        iterator insert(iterator it, T &&o) {
            return insert(it - begin, forward<T>(o));
        }
        /**
         * @details Insert object o at the position of index. After this operation, the element on the position of this old index should be the result of the copy constructor on object o.
         * @param index specifies an index in the container.
         * @param o
         * @return the iterator of the newly inserted object.
         * @details The parameter it may hold the same value as the return value, for they points to the same address. It may be the container expanded itself when copying, so that all old iterators are made invalid.
         */
        iterator insert(size_type index, const T &o) {
            insertPrepare(index);
            new(begin + index) T(o);
            return begin + index;
        }
        /**
         * @details Insert object o at the position of index. After this operation, the element on the position of this old index should be the result of the move constructor on object o.
         * @param index specifies an index in the container.
         * @param o
         * @return the iterator of the newly inserted object.
         * @details The parameter it may hold the same value as the return value, for they points to the same address. It may be the container expanded itself when copying, so that all old iterators are made invalid.
         */
        iterator insert(size_type index, T &&o) {
            insertPrepare(index);
            new(begin + index) T(forward<T>(o));
            return begin + index;
        }

        /*
         * Inserting multiple elements.
         * All insertions can be transformed into the form (dst_begin, src_begin, size_type num)
         */
    protected:
        /**
         * The helper method for functions insert* on multiple objects.
         * @param index
         * @param num
         * @details moves all elements starting at given index to the right by num.
         */
        void insertPrepare(size_type index, size_type num) {
            if (capacity() == 0) {
                if (index != 0) { throw IndexOutOfRange(); }
                resize(num + 1);
                cur += num;
                return;
            }
            if (index > size()) { throw IndexOutOfRange(); }
            copy_back_expand(index, num);
        }
    public:
        /**
         * @details Insert objects, starting at iterator b, ending at iterator e, at the position of iterator it. After this operation, the element on the position of this old iterator should be the result of the copy constructor on the object at b.
         * @details iterator e is not included in the range, making this range left-closed, right-opened, [).
         * @param it specifies a position in the container.
         * @param b
         * @param e
         * @return the iterator of the newly inserted object.
         * @details The parameter it may hold the same value as the return value, for they points to the same address. It may be the container expanded itself when copying, so that all old iterators are made invalid.
         * @details This wraps function insert(index, p, num).
         */
        iterator insert(iterator it, iterator b, iterator e) {
            return insert(it, b, e - b);
        }
        /**
         * @details Insert objects, starting at iterator b, ending at iterator e, at the position of iterator it. After this operation, the element on the position of this old iterator should be the result of the copy constructor on the object at b.
         * @details iterator e is not included in the range, making this range left-closed, right-opened, [).
         * @param it specifies a position in the container.
         * @param p
         * @param num
         * @return the iterator of the newly inserted object.
         * @details The parameter it may hold the same value as the return value, for they points to the same address. It may be the container expanded itself when copying, so that all old iterators are made invalid.
         * @details This wraps function insert(index, p, num).
         */
        iterator insert(iterator it, const T *p, size_type num) {
            return insert(it - begin, p, num);
        }

        iterator insert(size_type index, const T *p, size_type num) {
            insertPrepare(index, num);
            Copier<T>().copy(begin + index, p, num);
            return begin + index;
        }
        /*
         * Inserting one element for multiple times
         */
        iterator insert(iterator it, const T &obj, size_type num) {
            if (capacity() == 0) {
                size_type oldIndex = it - begin;
                resize(num + 1);
                it = begin + oldIndex;
            }
            if (it > cur) { throw IndexOutOfRange(); }
            size_type index = it - begin;
            copy_back_expand(index, num);
            for (size_type i = index; i < num; ++i) {
                new(begin + index) T(obj);
            }
            return begin + index;
        }
        #pragma endregion

        #pragma region assign_remove
        void assign(const T &o, size_type num) {
            clear();
            resize(num);
            for (size_type i = 0; i < num; ++i) {
                push_back(o);
            }
        }

        template<typename I>
        void assign(I begin, I end) {
            clear();
            while (begin != end) {
                push_back(*begin);
                ++begin;
            }
        }

        void remove(size_type index) {
            remove(index, 1);
        }

        void remove(iterator it) {
            copy_forward_expand(it - begin);
        }

        void remove(iterator b, iterator e) {
            remove(b - begin, e - b);
        }

        void remove(iterator b, size_type num) {
            remove(b - begin, num);
        }

        void remove(size_type index, size_type length) {
            if (length == 0) { return; }
            size_type oldsize = size();
            if (index >= oldsize || index + length > oldsize) {
                throw IndexOutOfRange();
            }
            copy_forward_expand(index + length, length);
            if (size() < oldsize / 3) {
                resize(oldsize / 3);
            }
        }
        #pragma endregion

        constexpr T *getArr() const {
            return begin;
        }

        bool operator==(const Vector &rhs) const {
            if (&rhs == this) {
                return true;
            }
            if (rhs.size() != size()) {
                return false;
            }
            for (size_type i = 0; i < size(); ++i) {
                if (begin[i] != rhs.begin[i]) {
                    return false;
                }
            }
            return true;
        }

        bool operator!=(const Vector &rhs) const {
            return !(rhs == *this);
        }

        iterator find(const T &o) const {
            T *p = begin;
            while (p != cur) {
                if (o == *p) {
                    return iterator(p);
                }
                ++p;
            }
            return end_iterator();
        }

    };

    template <typename T>
    void swap(typename Vector<T>::iterator x, typename Vector<T>::iterator y) {
        T o = move(*x);
        x->~T();
        new (x) T(move(*y));
        y->~T();
        new (y) T(move(o));
    }

};

#endif //MYCPPLIB_VECTOR_HPP
