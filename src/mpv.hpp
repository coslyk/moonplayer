
// C++ Wrapper for libmpv

#include <mpv/client.h>
#include <mpv/render.h>
#include <mpv/render_gl.h>

namespace Mpv {

    constexpr mpv_format to_mpv_format(int) { return MPV_FORMAT_FLAG; }
    constexpr mpv_format to_mpv_format(int64_t) { return MPV_FORMAT_INT64; }
    constexpr mpv_format to_mpv_format(double) { return MPV_FORMAT_DOUBLE; }
    constexpr mpv_format to_mpv_format(const char*) { return MPV_FORMAT_STRING; }
    constexpr mpv_format to_mpv_format(mpv_node*) { return MPV_FORMAT_NODE; }

    class Handle {
        private:
        mpv_handle *m_handle = nullptr;
        mpv_render_context *m_rctx = nullptr;

        public:
        inline Handle()
        {
            m_handle = mpv_create();
        }

        inline ~Handle()
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

        inline int initialize() const
        {
            return mpv_initialize(m_handle);
        }

        // Set mpv option. T must be: int, int64_t, double, const char* or mpv_node*
        template<class T>
        inline int set_option(const char* name, T data) const
        {
            static_assert(!std::is_same<bool, T>(), "Type can't be bool, use int instead.");
            return mpv_set_option(m_handle, name, to_mpv_format(data), &data);
        }

        // Set option with string value
        inline int set_option_string(const char* name, const char* data) const
        {
            return mpv_set_option_string(m_handle, name, data);
        }

        // Mpv command
        inline int command(const char** args) const
        {
            return mpv_command(m_handle, args);
        }

        // Async mpv command
        inline int command_async(const char **args, uint64_t reply_userdata = 0) const
        {
            return mpv_command_async(m_handle, reply_userdata, args);
        }

        // Set mpv property. T must be: int, int64_t, double, const char* or mpv_node*
        template <class T>
        inline int set_property(const char *name, T data) const
        {
            static_assert(!std::is_same<bool, T>(), "Type can't be bool, use int instead.");
            return mpv_set_property(m_handle, name, to_mpv_format(data), &data);
        }

        // Set mpv property asynchronally. T must be: int, int64_t, double, const char* or mpv_node*
        template <class T>
        inline int set_property_async(const char *name, T data, uint64_t reply_userdata = 0) const
        {
            static_assert(!std::is_same<bool, T>(), "Type can't be bool, use int instead.");
            return mpv_set_property_async(m_handle, reply_userdata, name, to_mpv_format(data), &data);
        }

        // Get mpv property. T must be: int, int64_t, double, const char* or mpv_node*
        template <class T>
        inline T get_property(const char *name) const
        {
            static_assert(!std::is_same<bool, T>(), "Type can't be bool, use int instead.");
            T v = 0;
            mpv_get_property(m_handle, name, to_mpv_format(v), &v);
            return v;
        }

        // Observe mpv property. T must be: int, int64_t, double, const char* or mpv_node*
        template <class T>
        inline int observe_property(const char *name, uint64_t reply_userdata = 0) const
        {
            static_assert(!std::is_same<bool, T>(), "Type can't be bool, use int instead.");
            T tmp = 0;
            return mpv_observe_property(m_handle, reply_userdata, name, to_mpv_format(tmp));
        }

        // Wait mpv event
        inline mpv_event* wait_event(double timeout = 0) const
        {
            return mpv_wait_event(m_handle, timeout);
        }

        // Set wake up callback
        inline void set_wakeup_callback(void (*callback)(void *userdata), void *userdata) const
        {
            mpv_set_wakeup_callback(m_handle, callback, userdata);
        }

        // Request log messages
        inline int request_log_messages(const char* min_level) const
        {
            return mpv_request_log_messages(m_handle, min_level);
        }

        // Check renderer initialized
        inline bool renderer_initialized() const
        {
            return m_rctx != nullptr;
        }

        // Init renderer
        inline int renderer_initialize(mpv_render_param* params)
        {
            return mpv_render_context_create(&m_rctx, m_handle, params);
        }

        // Set renderer callback
        inline void set_render_callback(mpv_render_update_fn callback, void *userdata) const
        {
            mpv_render_context_set_update_callback(m_rctx, callback, userdata);
        }

        // Render
        inline void render(mpv_render_param* params) const
        {
            mpv_render_context_render(m_rctx, params);
        }
    };
}