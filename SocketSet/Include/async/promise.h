/**
 * Created by TekuConcept on January 2, 2021
 */

#ifndef IMPACT_PROMISE_H
#define IMPACT_PROMISE_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include "utils/let.h"

namespace impact {

    #define PROMISE_CALLBACK_R(name, ...) \
        (promise::callback_t)[__VA_ARGS__](const std::vector<let>& name) -> let
    #define PROMISE_CALLBACK(name, ...) \
        promise_proxy_callback() = [__VA_ARGS__](const std::vector<let>& name) -> void

    class promise {
    public:
        promise(const promise& other) = default;
        virtual ~promise() = default;

        typedef std::function<let(const std::vector<let>&)> callback_t;

        promise then(
            callback_t done,
            callback_t fail = nullptr,
            callback_t progress = nullptr);


        template <class ... Callback>
        promise& done(Callback ... callbacks)
        { return _M_done({ callbacks... }); }

        template <class ... Callback>
        promise& fail(const Callback& ... callbacks)
        { return _M_fail({ callbacks... }); }

        template <class ... Callback>
        promise& always(const Callback& ... callbacks)
        {
            std::vector<callback_t> list{ callbacks... };
            _M_fail(list);
            _M_done(list);
            return *this;
        }

        template <class ... Callback>
        promise& progress(const Callback& ... callbacks)
        { return _M_progress({ callbacks... }); }

        std::string state() const;

    protected:
        promise();

        promise& _M_done(std::vector<callback_t> callbacks);
        promise& _M_fail(std::vector<callback_t> callbacks);
        promise& _M_progress(std::vector<callback_t> callbacks);

        void _M_resolve(std::vector<let> args);
        void _M_reject(std::vector<let> args);
        void _M_notify(std::vector<let> args);

        enum class state_t { PENDING, RESOLVED, REJECTED };
        struct link_t {
            state_t state_id;
            std::vector<let> final_args;
            std::vector<callback_t> done_callbacks;
            std::vector<callback_t> fail_callbacks;
            std::vector<callback_t> progress_callbacks;
            std::vector<std::shared_ptr<struct link_t>> children;
            link_t();
        };
        std::shared_ptr<struct link_t> m_link;

    private:
        promise(std::shared_ptr<struct link_t>);

        typedef void (*quick_callback_t)(
            const std::shared_ptr<struct link_t>&,
            const std::vector<let>&);
        static void _S_resolve(
            const std::shared_ptr<struct link_t>& link,
            const std::vector<let>& args);
        static void _S_reject(
            const std::shared_ptr<struct link_t>& link,
            const std::vector<let>& args);
        static void _S_notify(
            const std::shared_ptr<struct link_t>& link,
            const std::vector<let>& args);
        static void _M_cleanup(const std::shared_ptr<struct link_t>& link);

        inline callback_t _M_wrap_callback(
            const std::shared_ptr<struct link_t>& link,
            const callback_t& callback,
            const quick_callback_t& action);
    };


    class deferred : public promise {
    public:
        deferred(std::function<void(deferred&)> init_cb = nullptr);
        virtual ~deferred() = default;

        // TODO: add then(), done(), fail(), always(), progress(),
        // which all return a deferred type

        inline deferred&
        notify(std::vector<let> params)
        {
            promise::_M_notify(params);
            return *this;
        }
        template <class ... Params>
        deferred&
        notify(const Params& ... params)
        {
            promise::_M_notify({ let(params)... });
            return *this;
        }

        inline deferred&
        resolve(std::vector<let> params)
        {
            promise::_M_resolve(params);
            return *this;
        }
        template <class ... Params>
        deferred&
        resolve(const Params& ... params)
        {
            promise::_M_resolve({ let(params)... });
            return *this;
        }

        inline deferred&
        reject(std::vector<let> params)
        {
            promise::_M_reject(params);
            return *this;
        }
        template <class ... Params>
        deferred&
        reject(const Params& ... params)
        {
            promise::_M_reject({ let(params)... });
            return *this;
        }

        inline promise& promise()
        { return *this; }
    };

    class promise_proxy_callback {
    public:
        typedef std::function<void(std::vector<let>)> callback_t;

        promise_proxy_callback() = default;
        promise_proxy_callback(callback_t callback);
        ~promise_proxy_callback() = default;

        promise_proxy_callback& operator=(callback_t callback);
        let operator() (std::vector<let> args);

        inline const callback_t& get() const { return this->m_base; }
        inline const let& return_value() const { return m_return; }
        inline void return_value(let value) { m_return = value; }

    private:
        callback_t m_base;
        let m_return;
    };

} /* namespace impact */

#endif /* IMPACT_PROMISE_H */
