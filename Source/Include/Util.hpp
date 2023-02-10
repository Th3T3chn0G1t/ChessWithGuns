#pragma once

using Dimension = int;
static constexpr Dimension DimensionMax = INT_MAX;
static constexpr Dimension DimensionMin = INT_MIN;

template<typename T>
class Span {
public:
    T* m_Data;
    Dimension m_Size;

public:
    template<class S>
    explicit Span(S& container) : m_Data(container.data()), m_Size(container.size()) {}
};

template<class T, Dimension ResourcePoolSize>
class ResourceLoader {
private:
    std::string m_ResourceDirectory;
    std::array<T, ResourcePoolSize> m_Resources;
    Dimension m_ResourcesLast{0};
    std::unordered_map<std::string, Dimension> m_Map;

public:
    explicit ResourceLoader(std::string resource_directory) : m_ResourceDirectory(std::move(resource_directory)) {}

    template<class S>
    T& Get(const std::string& path, S& ctx) {
        auto emplaced = m_Map.try_emplace(path, ResourcePoolSize);
        auto it = emplaced.first;
        auto added = emplaced.second;

        if(added) {
            std::string full_path = m_ResourceDirectory + "/" + path;
            m_Resources[m_ResourcesLast] = std::move(T(full_path, ctx));
            it->second = m_ResourcesLast++;
        }

        return m_Resources[it->second];
    }

    T& Get(const std::string& path) {
        auto emplaced = m_Map.try_emplace(path, ResourcePoolSize);
        auto it = emplaced.first;
        auto added = emplaced.second;

        if(added) {
            std::string full_path = m_ResourceDirectory + "/" + path;
            m_Resources[m_ResourcesLast] = std::move(T(full_path));
            it->second = m_ResourcesLast++;
        }

        return m_Resources[it->second];
    }
};

bool IsPointInRect(Dimension px, Dimension py, Dimension rx, Dimension ry, Dimension rw, Dimension rh);
