
// C++ Wrapper for libmpv

#include <mpv/client.h>

namespace Mpv {

    constexpr mpv_format to_mpv_format(int v) { return MPV_FORMAT_FLAG; }
    constexpr mpv_format to_mpv_format(int64_t v) { return MPV_FORMAT_INT64; }
    constexpr mpv_format to_mpv_format(double v) { return MPV_FORMAT_DOUBLE; }
    constexpr mpv_format to_mpv_format(const char *v) { return MPV_FORMAT_STRING; }
    constexpr mpv_format to_mpv_format(mpv_node* v) { return MPV_FORMAT_NODE; }

    class Handle {
        private:
        mpv_handle *m_handle;

        public:
        inline Handle()
        {
            m_handle = mpv_create();
        }

        inline ~Handle()
        {
            mpv_set_wakeup_callback(m_handle, nullptr, nullptr);
            mpv_terminate_destroy(m_handle);
        }

        // Get the raw mpv_handle* pointer
        inline mpv_handle* get_raw_handle() const
        {
            return m_handle;
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
            T v;
            mpv_get_property(m_handle, name, to_mpv_format(v), &v);
            return v;
        }

        // Observe mpv property. T must be: int, int64_t, double, const char* or mpv_node*
        template <class T>
        inline int observe_property(const char *name, uint64_t reply_userdata = 0) const
        {
            static_assert(!std::is_same<bool, T>(), "Type can't be bool, use int instead.");
            T tmp;
            return mpv_observe_property(m_handle, reply_userdata, name, to_mpv_format(tmp));
        }

        // Wait mpv event
        inline mpv_event* wait_event(double timeout = 0) const
        {
            return mpv_wait_event(m_handle, timeout);
        }

        // Set wake up callback
        inline void set_wakeup_callback(void (*callback)(void *userdata), void *userdata)
        {
            mpv_set_wakeup_callback(m_handle, callback, userdata);
        }

        // Request log messages
        inline int request_log_messages(const char* min_level) const
        {
            return mpv_request_log_messages(m_handle, min_level);
        }
    };
}