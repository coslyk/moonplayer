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

namespace Mpv
{
    class Node;

    constexpr mpv_format to_mpv_format(int) { return MPV_FORMAT_FLAG; }
    constexpr mpv_format to_mpv_format(int64_t) { return MPV_FORMAT_INT64; }
    constexpr mpv_format to_mpv_format(double) { return MPV_FORMAT_DOUBLE; }
    constexpr mpv_format to_mpv_format(const char*) { return MPV_FORMAT_STRING; }
    constexpr mpv_format to_mpv_format(Node*) { return MPV_FORMAT_NODE; }

    constexpr bool node_to_value(const mpv_node* node, bool) { return node->u.flag; }
    constexpr int64_t node_to_value(const mpv_node* node, int64_t) { return node->u.int64; }
    constexpr double node_to_value(const mpv_node* node, double) { return node->u.double_; }
    constexpr const char* node_to_value(const mpv_node* node, const char*) { return node->u.string; }


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

        // Set mpv option. T must be: int, int64_t, double, const char* or mpv_node*
        template <class T>
        inline int set_option(const char *name, T data) const noexcept
        {
            static_assert(!std::is_same<bool, T>(), "Type can't be bool, use int instead.");
            return mpv_set_option(m_handle, name, to_mpv_format(data), &data);
        }

        // Set option with string value
        inline int set_option_string(const char *name, const char *data) const noexcept
        {
            return mpv_set_option_string(m_handle, name, data);
        }

        // Mpv command
        inline int command(const char **args) const noexcept
        {
            return mpv_command(m_handle, args);
        }

        // Async mpv command
        inline int command_async(const char **args, uint64_t reply_userdata = 0) const noexcept
        {
            return mpv_command_async(m_handle, reply_userdata, args);
        }

        // Set mpv property. T must be: int, int64_t, double, const char* or mpv_node*
        template <class T>
        inline int set_property(const char *name, T data) const noexcept
        {
            static_assert(!std::is_same<bool, T>(), "Type can't be bool, use int instead.");
            return mpv_set_property(m_handle, name, to_mpv_format(data), &data);
        }

        // Set mpv property asynchronally. T must be: int, int64_t, double, const char* or mpv_node*
        template <class T>
        inline int set_property_async(const char *name, T data, uint64_t reply_userdata = 0) const noexcept
        {
            static_assert(!std::is_same<bool, T>(), "Type can't be bool, use int instead.");
            return mpv_set_property_async(m_handle, reply_userdata, name, to_mpv_format(data), &data);
        }

        // Get mpv property. T must be: int, int64_t, double, const char* or mpv_node*
        template <class T>
        inline T get_property(const char *name) const noexcept
        {
            static_assert(!std::is_same<bool, T>(), "Type can't be bool, use int instead.");
            T v = 0;
            mpv_get_property(m_handle, name, to_mpv_format(v), &v);
            return v;
        }

        // Get mpv property in string.
        inline std::string get_property_string(const char *name) const noexcept
        {
            char *s = mpv_get_property_string(m_handle, name);
            std::string result = s ? s : std::string();
            if (s) mpv_free(s);
            return result;
        }

        // Observe mpv property. T must be: int, int64_t, double, const char* or mpv_node*
        template <class T>
        inline int observe_property(const char *name, uint64_t reply_userdata = 0) const noexcept
        {
            static_assert(!std::is_same<bool, T>(), "Type can't be bool, use int instead.");
            T tmp = 0;
            return mpv_observe_property(m_handle, reply_userdata, name, to_mpv_format(tmp));
        }

        // Wait mpv event
        inline mpv_event *wait_event(double timeout = 0) const noexcept
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


    // Wrapper for mpv_node
    class Node : mpv_node
    {
    public:
        // Iterator
        typedef Node *iterator;

        // Const iterator
        typedef const Node *const_iterator;

        // Get type
        inline mpv_format type() const noexcept
        {
            return format;
        }

        // Get value
        template <class T>
        T value() const noexcept
        {
            T tmp = 0;
            assert(format == to_mpv_format(tmp));
            return node_to_value(this, tmp);
        }

        // Equal
        inline bool operator==(const char* str) const noexcept
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
            return static_cast<Node *>(&u.list->values[0]);
        }

        // Get array iterator
        inline iterator end() const noexcept
        {
            return static_cast<Node *>(&u.list->values[u.list->num]);
        }

        // Get array iterator
        inline const_iterator cbegin() const noexcept
        {
            return static_cast<Node *>(&u.list->values[0]);
        }

        // Get array iterator
        inline const_iterator cend() const noexcept
        {
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
}