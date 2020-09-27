#ifndef UTIL_H
#define UTIL_H

#include <algorithm>
#include <map>
#include <memory>
#include <vector>

template<typename T>
using qobject_unique_ptr = std::unique_ptr<T, std::function<void(T*)>>;

/**
 * Create a unique ptr to a QObject. When the unique pointer is deleted, the function QObject::deleteLater
 * is called on the object to delete it later.
 */
template<typename T>
inline qobject_unique_ptr<T> make_qobject_unique(T* qobject)
{
    return qobject_unique_ptr<T>(qobject, [](T* obj) {
        if (obj) {
            obj->deleteLater();
        }
    });
}

template<typename T>
inline qobject_unique_ptr<T> make_qobject_unique()
{
    return qobject_unique_ptr<T>(nullptr);
}

namespace indoorcycling
{

template <typename T, typename U>
std::map<U,T> toMap(const std::vector<T> &items, std::function<U(T)> &keyCreatorFunction)
{
    std::map<U,T> targetMap;
    std::transform(items.begin(), items.end(), std::inserter(targetMap, targetMap.begin()),
                   [keyCreatorFunction](const T &item) {
        return std::make_pair(keyCreatorFunction(item), item);
    });
    return targetMap;
}

}

#endif // UTIL_H
