#pragma once
#include <cstdint>
#include <array>
#include <memory>
#include "face_type.h"
#include "PixelTheater/core/crgb.h"
#include "PixelTheater/limits.h"

namespace PixelTheater {

// Vertex structure to hold 3D coordinates
struct Vertex {
    float x, y, z;
};

class Face {
private:
    uint8_t _id;
    FaceType _type;
    uint16_t _led_offset;
    uint16_t _led_count;
    uint16_t _vertex_count;  // Store number of vertices
    CRGB* _leds;  // Pointer to LED array
    std::unique_ptr<std::array<Vertex, Limits::MAX_EDGES_PER_FACE>> _vertices;

public:
    Face() = default;
    
    Face(FaceType type, uint8_t id, uint16_t offset, uint16_t count, CRGB* leds, uint16_t vertex_count)
        : _id(id)
        , _type(type)
        , _led_offset(offset)
        , _led_count(count)
        , _vertex_count(vertex_count)
        , _leds(leds)
        , leds{leds, offset, count}  // Initialize leds member
    {
        // Initialize vertices to zero
        _vertices = std::make_unique<std::array<Vertex, Limits::MAX_EDGES_PER_FACE>>();
        for(auto& v : *_vertices) {
            v = {0.0f, 0.0f, 0.0f};
        }
        vertices = Vertices{_vertices.get(), _vertex_count};  // Initialize vertices member with actual count
    }

    // Copy assignment operator
    Face& operator=(const Face& other) {
        if (this != &other) {
            _id = other._id;
            _type = other._type;
            _led_offset = other._led_offset;
            _led_count = other._led_count;
            _vertex_count = other._vertex_count;
            _leds = other._leds;
            
            // Create new vertices array and copy data
            _vertices = std::make_unique<std::array<Vertex, Limits::MAX_EDGES_PER_FACE>>();
            if (other._vertices) {
                std::copy(other._vertices->begin(), other._vertices->end(), _vertices->begin());
            }
            
            // Update vertices member to point to new array
            vertices = Vertices{_vertices.get(), _vertex_count};
            leds = Leds{_leds, _led_offset, _led_count};
        }
        return *this;
    }

    // Simple accessors
    uint8_t id() const { return _id; }
    FaceType type() const { return _type; }
    uint16_t led_offset() const { return _led_offset; }
    uint16_t led_count() const { return _led_count; }

    // LED array access - matches Model.md
    struct Leds {
        CRGB* _data;
        uint16_t _offset;
        uint16_t _count;

        // Array access
        CRGB& operator[](size_t i) const {
            if (i >= _count) i = _count - 1;
            return _data[_offset + i];
        }

        // Allow iteration
        CRGB* begin() const { return _data + _offset; }
        CRGB* end() const { return _data + _offset + _count; }
    } leds;  // Direct member access

    // Vertex array access
    struct Vertices {
        std::array<Vertex, Limits::MAX_EDGES_PER_FACE>* _data;  // Pointer to array
        uint16_t _count;  // Actual number of vertices

        // Array access
        Vertex& operator[](size_t i) {
            if (i >= _count) i = _count - 1;
            return (*_data)[i];
        }
        const Vertex& operator[](size_t i) const {
            if (i >= _count) i = _count - 1;
            return (*_data)[i];
        }

        // Allow iteration
        auto begin() { return _data->begin(); }
        auto end() { return _data->begin() + _count; }
        auto begin() const { return _data->begin(); }
        auto end() const { return _data->begin() + _count; }

        // Size info - return array size for compatibility
        size_t size() const { return Limits::MAX_EDGES_PER_FACE; }
    } vertices;  // Direct member access
};

} // namespace PixelTheater 