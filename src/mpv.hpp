/* Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses/.
 */

// C++ Wrapper for libmpv

#include <mpv/client.h>
#include <mpv/render.h>
#include <mpv/render_gl.h>
#include <cstring>
#include <stdexcept>

namespace Mpv
{
    // Wrapper for mpv_node
    class Node : mpv_node
    {
    public:
        // Iterator
        typedef Node *iterator;

        // Const iterator
        typedef const Node *const_iterator;

        // Constructor
        inline Node() noexcept
        {
            format = MPV_FORMAT_NONE;
        }

        // Move constructor
        inline Node(Node &&other) noexcept
        {
            format = other.format;
            u = other.u;
            other.format = MPV_FORMAT_NONE;
        }

        // Bool constructor
        inline Node(bool v) noexcept
        {
            format = MPV_FORMAT_FLAG;
            u.flag = v;
        }

        // Int64 constructor
        inline Node(int64_t v) noexcept
        {
            format = MPV_FORMAT_INT64;
            u.int64 = v;
        }

        // Double constructor
        inline Node(double v) noexcept
        {
            format = MPV_FORMAT_DOUBLE;
            u.double_ = v;
        }

        // String constructor
        inline Node(const char *v) noexcept
        {
            format = MPV_FORMAT_STRING;
            u.string = const_cast<char*>(v);
        }

        // Get type
        inline mpv_format type() const noexcept
        {
            return format;
        }

        // Convert to bool
        inline operator bool() const noexcept
        {
            assert(format == MPV_FORMAT_FLAG);
            return u.flag;
        }

        // Convert to int64_t
        inline operator int64_t() const noexcept
        {
            assert(format == MPV_FORMAT_INT64);
            return u.int64;
        }

        // Convert to double
        inline operator double() const noexcept
        {
            assert(format == MPV_FORMAT_DOUBLE);
            return u.double_;
        }

        // Convert to string
        inline operator const char*() const noexcept
        {
            assert(format == MPV_FORMAT_STRING);
            return u.string;
        }

        // Equal to string
        inline bool operator==(const char *str) const noexcept
        {
            assert(format == MPV_FORMAT_STRING);
            return std::strcmp(u.string, str) == 0;
        }

        // Get array size
        inline int size() const noexcept
        {
            assert(format == MPV_FORMAT_NODE_ARRAY);
            return u.list->num;
        }

        // Get array member
        inline const Node &operator[](int i) const noexcept
        {
            assert(format == MPV_FORMAT_NODE_ARRAY);
            return static_cast<Node *>(u.list->values)[i];
        }

        // Get array iterator
        inline iterator begin() const noexcept
        {
            assert(format == MPV_FORMAT_NODE_ARRAY);
            return static_cast<Node *>(&u.list->values[0]);
        }

        // Get array iterator
        inline iterator end() const noexcept
        {
            assert(format == MPV_FORMAT_NODE_ARRAY);
            return static_cast<Node *>(&u.list->values[u.list->num]);
        }

        // Get array iterator
        inline const_iterator cbegin() const noexcept
        {
            assert(format == MPV_FORMAT_NODE_ARRAY);
            return static_cast<Node *>(&u.list->values[0]);
        }

        // Get array iterator
        inline const_iterator cend() const noexcept
        {
            assert(format == MPV_FORMAT_NODE_ARRAY);
            return static_cast<Node *>(&u.list->values[u.list->num]);
        }

        // Get map member
        inline const Node &operator[](const char *key) const
        {
            assert(format == MPV_FORMAT_NODE_MAP);
            for (int i = 0; i < u.list->num; i++)
            {
                if (std::strcmp(key, u.list->keys[i]) == 0)
                {
                    return static_cast<Node *>(u.list->values)[i];
                }
            }
            throw std::runtime_error("Mpv::NodeMap::operator[]: key does not exist!");
        }
    };

    class Handle
    {
        private:
        mpv_handle *m_handle = nullptr;
        mpv_render_context *m_rctx = nullptr;

        public:
        // Initialize the object
        inline Handle() noexcept
        {
            m_handle = mpv_create();
        }

        // Deinitialize
        inline ~Handle() noexcept
        {
            if (m_rctx)
            {
                mpv_render_context_set_update_callback(m_rctx, nullptr, nullptr);
            }
            mpv_set_wakeup_callback(m_handle, nullptr, nullptr);

            if (m_rctx)
            {
                mpv_render_context_free(m_rctx);
            }
            mpv_terminate_destroy(m_handle);
        }

        // Initialize mpv after options are set
        inline int initialize() const noexcept
        {
            return mpv_initialize(m_handle);
        }

        // Set option
        inline int set_option(const char *name, const Node& data) const noexcept
        {
            return mpv_set_option(m_handle, name, MPV_FORMAT_NODE, const_cast<Node*>(&data));
        }

        // Async mpv command
        inline int command_async(const char **args, uint64_t reply_userdata = 0) const noexcept
        {
            return mpv_command_async(m_handle, reply_userdata, args);
        }

        // Set mpv property
        inline int set_property_async(const char *name, const Node& data, uint64_t reply_userdata = 0) const noexcept
        {
            return mpv_set_property_async(m_handle, reply_userdata, name, MPV_FORMAT_NODE, const_cast<Node*>(&data));
        }

        // Get mpv property
        inline Node get_property(const char *name) const noexcept
        {
            Node tmp;
            mpv_get_property(m_handle, name, MPV_FORMAT_NODE, &tmp);
            return tmp;
        }

        // Get mpv property in string.
        inline std::string get_property_string(const char *name) const noexcept
        {
            char *s = mpv_get_property_string(m_handle, name);
            std::string result = s ? s : std::string();
            if (s) mpv_free(s);
            return result;
        }

        // Observe mpv property.
        inline int observe_property(const char *name, uint64_t reply_userdata = 0) const noexcept
        {
            return mpv_observe_property(m_handle, reply_userdata, name, MPV_FORMAT_NODE);
        }

        // Wait mpv event
        inline const mpv_event *wait_event(double timeout = 0) const noexcept
        {
            return mpv_wait_event(m_handle, timeout);
        }

        // Set wake up callback
        inline void set_wakeup_callback(void (*callback)(void *userdata), void *userdata) const noexcept
        {
            mpv_set_wakeup_callback(m_handle, callback, userdata);
        }

        // Request log messages
        inline int request_log_messages(const char *min_level) const noexcept
        {
            return mpv_request_log_messages(m_handle, min_level);
        }

        // Check renderer initialized
        inline bool renderer_initialized() const noexcept
        {
            return m_rctx != nullptr;
        }

        // Init renderer
        inline int renderer_initialize(mpv_render_param *params) noexcept
        {
            return mpv_render_context_create(&m_rctx, m_handle, params);
        }

        // Set renderer callback
        inline void set_render_callback(mpv_render_update_fn callback, void *userdata) const noexcept
        {
            mpv_render_context_set_update_callback(m_rctx, callback, userdata);
        }

        // Render
        inline void render(mpv_render_param *params) const noexcept
        {
            mpv_render_context_render(m_rctx, params);
        }
    };
}