#ifndef STORAGE_HPP
#define STORAGE_HPP

#include "common.hpp"


namespace sgs {

/*
 * Thread safe class.
*/

    class Storage {
        public:
            explicit Storage() = default;
            Storage(const Storage& ) = delete;
            Storage& operator=(const Storage& ) = delete;
        
            bool set(const std::int32_t fd, const Sequence& seq) {
                std::lock_guard<SharedMutex> lock(m_mtx);
                m_storage[fd] = {seq};
                return true;
            }
            bool get(const std::int32_t fd, Sequence& seq) {
                std::shared_lock<SharedMutex> lock(m_mtx);
                if(m_storage.find(fd) != m_storage.cend())
                    seq = m_storage[fd];
                else
                    return false;

                return true;
            }
            bool erase(const std::int32_t fd) {
                std::lock_guard<SharedMutex> lock(m_mtx);
                return m_storage.erase(fd);
            }

        private:
            SharedMutex m_mtx;
            storage m_storage;
        };
} /* sgc */

#endif // STORAGE_HPP
